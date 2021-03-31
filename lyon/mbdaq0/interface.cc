#include "Mbdaq0Interface.hh"
using namespace mpi;
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>








void mbdaq0::Interface::dolisten()
{
 
  while(!_onClientDisconnect->disconnected()) {
    if (!_running) break;
    if(!_group->listen(1000))
      std::cout << "\nNo msg recieved during the last 1 seconds";
  }

    
}
void mbdaq0::Interface::listen()
{
   _running=true;
  g_store=std::thread(std::bind(&mbdaq0::Interface::dolisten, this));

  // Comment out for ZDAQ running
  //g_run.create_thread(boost::bind(&lydaq::TdcManager::doStart, this));

}
void mbdaq0::Interface::terminate()
{
  PM_INFO(_logMbdaq0,"Entering terminate "<<std::flush);
  if (_running)
    {
      PM_INFO(_logMbdaq0,"STOPPING the thread "<<std::flush);
      
    _running=false;
    ::sleep(2);
    g_store.join();
    }
  PM_INFO(_logMbdaq0,"CLOSING"<<std::flush);
  this->close();

}

mbdaq0::Interface::Interface() :  _group(NULL)
{
  // Initialise NetLink
  NL::init();
  _msh =new mbdaq0::messageHandler();

 
}

void mbdaq0::Interface::initialise()
{
 
   // Initialise the network
   if (_group!=NULL) delete _group;
   _group=new NL::SocketGroup();
   _onRead= new mpi::OnRead(_msh);
  _onClientDisconnect= new mpi::OnClientDisconnect();
  _onDisconnect= new mpi::OnDisconnect(_msh);
  _onAccept=new mpi::OnAccept(_msh);
  _group->setCmdOnRead(_onRead);
  _group->setCmdOnAccept(_onAccept);
  _group->setCmdOnDisconnect(_onClientDisconnect);
   // Loop on Asic Map and find existing DIF
  // Register their slow control socket (10001) and readout one (10002)
  _boards.clear();
  _running=false;

}
void mbdaq0::Interface:: addDevice(std::string address)
{
  fprintf(stderr,"Creating Board at address %s  \n",address.c_str());

  mbdaq0::board* b= new mbdaq0::board(address);

  fprintf(stderr,"Adding registeraccess socket  \n");
  _group->add(b->reg()->socket());
#ifdef FULLDAQ
  fprintf(stderr,"Adding slcaccess socket  \n");
  _group->add(b->slc()->socket());
  fprintf(stderr,"Adding dataaccess socket  \n");
  _group->add(b->data()->socket());
#endif
    fprintf(stderr,"Binding reg  \n");
  _msh->addHandler(b->reg()->id(),std::bind(&mbdaq0::socketHandler::processBuffer,b->reg(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
#ifdef FULLDAQ
  fprintf(stderr,"Binding slc  \n");
  _msh->addHandler(b->slc()->id(),std::bind(&mbdaq0::socketHandler::processBuffer,b->slc(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
  fprintf(stderr,"Binding data  \n");
  _msh->addHandler(b->data()->id(),std::bind(&mbdaq0::socketHandler::processBuffer,b->data(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
#endif
		   
  std::pair<std::string, mbdaq0::board*> p1(address,b);

  printf("Board at address %s added \n",address.c_str());
  _boards.insert(p1);
}


void mbdaq0::Interface::close()
{
  for (auto x=_boards.begin();x!=_boards.end();x++)
    {
#ifdef FULLDAQ
      PM_DEBUG(_logMbdaq0,"Remove data socket "<<std::flush);
      _group->remove(x->second->data()->socket());
      PM_DEBUG(_logMbdaq0,"disconnect data socket "<<std::flush);
      x->second->data()->socket()->disconnect();
      PM_DEBUG(_logMbdaq0,"Remove slc socket "<<std::flush);
      _group->remove(x->second->slc()->socket());
      PM_DEBUG(_logMbdaq0,"disconnect slc socket "<<std::flush);
      x->second->slc()->socket()->disconnect();
#endif
      PM_DEBUG(_logMbdaq0,"Remove reg socket "<<std::flush);
      _group->remove(x->second->reg()->socket());
      PM_DEBUG(_logMbdaq0,"disconnect reg socket "<<std::flush);
      x->second->reg()->socket()->disconnect();

    }
  _boards.clear();
  
}
