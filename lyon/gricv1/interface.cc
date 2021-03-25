#include "Gricv1Interface.hh"
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








void gricv1::Interface::dolisten()
{
 
  while(!_onClientDisconnect->disconnected()) {
    if (!_running) break;
    if(!_group->listen(1000))
      std::cout << "\nNo msg recieved during the last 1 seconds";
  }

    
}
void gricv1::Interface::listen()
{
   _running=true;
  g_store=std::thread(std::bind(&gricv1::Interface::dolisten, this));

  // Comment out for ZDAQ running
  //g_run.create_thread(boost::bind(&lydaq::TdcManager::doStart, this));

}
void gricv1::Interface::terminate()
{
  if (_running)
    {
      PM_INFO(_logGricv1,"TERMINATEING");

    _running=false;
    ::sleep(2);
    g_store.join();
    }
  PM_INFO(_logGricv1,"CLOSING");
  this->close();

}

gricv1::Interface::Interface() :  _group(NULL)
{
  // Initialise NetLink
  NL::init();
  _msh =new mpi::MpiMessageHandler("/dev/shm");

 
}

void gricv1::Interface::initialise()
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
void gricv1::Interface:: addDevice(std::string address)
{
  fprintf(stderr,"Creating Board at address %s  \n",address.c_str());

  gricv1::board* b= new gricv1::board(address);

  fprintf(stderr,"Adding registeraccess socket  \n");
  _group->add(b->reg()->socket());
  fprintf(stderr,"Adding slcaccess socket  \n");
  _group->add(b->slc()->socket());
  fprintf(stderr,"Adding dataaccess socket  \n");
  _group->add(b->data()->socket());

    fprintf(stderr,"Binding reg  \n");
  _msh->addHandler(b->reg()->id(),std::bind(&gricv1::socketHandler::processBuffer,b->reg(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
  fprintf(stderr,"Binding slc  \n");
  _msh->addHandler(b->slc()->id(),std::bind(&gricv1::socketHandler::processBuffer,b->slc(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
  fprintf(stderr,"Binding data  \n");
  _msh->addHandler(b->data()->id(),std::bind(&gricv1::socketHandler::processBuffer,b->data(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
		   
  std::pair<std::string, gricv1::board*> p1(address,b);

  printf("Board at address %s added \n",address.c_str());
  _boards.insert(p1);
}


void gricv1::Interface::close()
{
  for (auto x=_boards.begin();x!=_boards.end();x++)
    {
      _group->remove(x->second->data()->socket());
      x->second->data()->socket()->disconnect();
      _group->remove(x->second->slc()->socket());
      x->second->slc()->socket()->disconnect();
      _group->remove(x->second->reg()->socket());
      x->second->reg()->socket()->disconnect();

    }
  _boards.clear();
  
}
