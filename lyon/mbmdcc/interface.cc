#include "MBMDCCInterface.hh"

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








void mbmdcc::Interface::dolisten()
{
 
  while(!_onClientDisconnect->disconnected()) {
    if (!_running) break;
    if(!_group->listen(4000))
      std::cout << "\nNo msg recieved during the last 4 seconds";
  }

    
}
void mbmdcc::Interface::listen()
{
  _running=true;  
  g_store=std::thread(std::bind(&mbmdcc::Interface::dolisten, this));

  // Comment out for ZDAQ running
  //g_run.create_thread(boost::bind(&lydaq::TdcManager::doStart, this));

}
void mbmdcc::Interface::terminate()
{
  if (_running)
    {
    _running=false;
    g_store.join();
    }
}

mbmdcc::Interface::Interface() :  _group(NULL)
{
  // Initialise NetLink
  NL::init();
  _msh =new mbmdcc::messageHandler();

 
}

void mbmdcc::Interface::initialise()
{
 
   // Initialise the network
   if (_group!=NULL) delete _group;
   _group=new NL::SocketGroup();
   _onRead= new mbmdcc::OnRead(_msh);
  _onClientDisconnect= new mbmdcc::OnClientDisconnect();
  //_onDisconnect= new mbmdcc::OnDisconnect(_msh);
  //_onAccept=new mbmdcc::OnAccept(_msh);
  _group->setCmdOnRead(_onRead);
  //  _group->setCmdOnAccept(_onAccept);
  _group->setCmdOnDisconnect(_onClientDisconnect);
   // Loop on Asic Map and find existing DIF
  // Register their slow control socket (10001) and readout one (10002)
  _boards.clear();

}
void mbmdcc::Interface:: addDevice(std::string address)
{
  PM_INFO(_logMbmdcc,"Creating Board at address "<<address.c_str());

  mbmdcc::board* b= new mbmdcc::board(address);

  PM_INFO(_logMbmdcc,"Adding registeraccess socket");
  _group->add(b->reg()->socket());
    PM_INFO(_logMbmdcc,"Binding registers ");
    _msh->addHandler(b->reg()->id(),std::bind(&mbmdcc::socketHandler::processBuffer,b->reg(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
  std::pair<std::string, mbmdcc::board*> p1(address,b);

  //printf("Board at address %s added \n",address.c_str());
  _boards.insert(p1);
}


void mbmdcc::Interface::close()
{
  if (_running)
    this->terminate();

  for (auto x=_boards.begin();x!=_boards.end();x++)
    {
      _group->remove(x->second->reg()->socket());
      x->second->reg()->socket()->disconnect();

    }
  _boards.clear();
  
}
