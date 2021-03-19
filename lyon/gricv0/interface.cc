#include "Gricv0Interface.hh"
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
#include <boost/format.hpp>




using namespace lydaq;


void gricv0::Interface::dolisten()
{
 
  while(!_onClientDisconnect->disconnected()) {

                if(!_group->listen(4000))
		  std::cout << "\nNo msg recieved during the last 4 seconds";
        }

    
}
void gricv0::Interface::listen()
{
  g_store.create_thread(boost::bind(&gricv0::Interface::dolisten, this));
  _running=true;
  // Comment out for ZDAQ running
  //g_run.create_thread(boost::bind(&lydaq::TdcManager::doStart, this));

}

gricv0::Interface::Interface() :  _group(NULL)
{
  // Initialise NetLink
  NL::init();
  _msh =new mpi::MpiMessageHandler("/dev/shm");

 
}

void gricv0::Interface::initialise()
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

}
void gricv0::Interface:: addDevice(std::string address)
{
  fprintf(stderr,"Creating Board at address %s  \n",address.c_str());

  gricv0::board* b= new gricv0::board(address);

  fprintf(stderr,"Adding registeraccess socket  \n");
  _group->add(b->reg()->socket());
  fprintf(stderr,"Adding slcaccess socket  \n");
  _group->add(b->sensor()->socket());
  fprintf(stderr,"Adding dataaccess socket  \n");
  _group->add(b->data()->socket());

    fprintf(stderr,"Binding reg  \n");
  _msh->addHandler(b->reg()->id(),boost::bind(&gricv0::socketHandler::processBuffer,b->reg(),_1,_2,_3));
  fprintf(stderr,"Binding slc  \n");
  _msh->addHandler(b->sensor()->id(),boost::bind(&gricv0::socketHandler::processBuffer,b->sensor(),_1,_2,_3));
  fprintf(stderr,"Binding data  \n");
  _msh->addHandler(b->data()->id(),boost::bind(&gricv0::socketHandler::processBuffer,b->data(),_1,_2,_3));
		   
  std::pair<std::string, gricv0::board*> p1(address,b);

  printf("Board at address %s added \n",address.c_str());
  _boards.insert(p1);
}


void gricv0::Interface::close()
{
  for (auto x=_boards.begin();x!=_boards.end();x++)
    {
      _group->remove(x->second->data()->socket());
      x->second->data()->socket()->disconnect();
      _group->remove(x->second->sensor()->socket());
      x->second->sensor()->socket()->disconnect();
      _group->remove(x->second->reg()->socket());
      x->second->reg()->socket()->disconnect();

    }
  _boards.clear();
  
}
