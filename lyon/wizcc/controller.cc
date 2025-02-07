#include "WizccInterface.hh"
using namespace wizcc;
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








void wizcc::Controller::dolisten()
{
 
  while(!_onClientDisconnect->disconnected()) {
    if (!_running) break;
    if(!_group->listen(1000))
      std::cout << "\nNo msg recieved during the last 1 seconds";
  }

    
}
void wizcc::Controller::listen()
{
   _running=true;
  g_store=std::thread(std::bind(&wizcc::Controller::dolisten, this));

  // Comment out for ZDAQ running
  //g_run.create_thread(boost::bind(&lydaq::TdcManager::doStart, this));

}
void wizcc::Controller::terminate()
{
  PM_INFO(_logWizcc,"Entering terminate "<<std::flush);
  if (_running)
    {
      PM_INFO(_logWizcc,"STOPPING the thread "<<std::flush);
      
    _running=false;
    ::sleep(2);
    g_store.join();
    }
  PM_INFO(_logWizcc,"CLOSING"<<std::flush);
  this->close();

}

wizcc::Controller::Controller() :  _group(NULL)
{
  // Initialise NetLink
  NL::init();

}

void wizcc::Controller::initialise()
{
 
   // Initialise the network
   if (_group!=NULL) delete _group;
   _group=new NL::SocketGroup();
   //Read callback , use the buffer handler
   _onRead= new wizcc::bufferHandler();
   // Connection defaults callback
  _onClientDisconnect= new wizcc::OnClientDisconnect();
  _onDisconnect= new wizcc::OnDisconnect();

  // Set the callbacks of the select class 
  _group->setCmdOnRead(_onRead);
  _group->setCmdOnDisconnect(_onClientDisconnect);

  // Clear the list of connected boards
  _boards.clear();
  _running=false;

}
void wizcc::Controller:: add_board(wizcc::board* b)
{
  fprintf(stderr,"Adding Board at address %s  \n",b->ip_address().c_str());
  auto sm=b->processors();
  for (auto p=sm.begin();p!=sm.end();p++)
    {
      _group->add(p->second->socket());
      _onRead->add_handler(p->second->id(),p->second);
		  
    }
  std::pair<std::string, wizcc::board*> p1(b->ip_address(),b);

  printf("Board at address %s added \n",b->ip_address().c_str());
  _boards.insert(p1);
}


void wizcc::Controller::close()
{
  for (auto x=_boards.begin();x!=_boards.end();x++)
    {
      auto sm=x->second->processors();
      for (auto p=sm.begin();p!=sm.end();p++)
	{
	  _group->remove(p->second->socket());
	  p->second->socket()->disconnect();		  
	}
    }
  _boards.clear();
  
}
