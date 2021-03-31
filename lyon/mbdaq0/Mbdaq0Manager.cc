#include "Mbdaq0Manager.hh"

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







Mbdaq0Manager::Mbdaq0Manager() : _mpi(NULL)
{;}

void Mbdaq0Manager::initialise()
{
    // Register state

  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  
  this->addTransition("INITIALISE","CREATED","INITIALISED",std::bind(&Mbdaq0Manager::fsm_initialise, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","INITIALISED","CONFIGURED",std::bind(&Mbdaq0Manager::configure, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",std::bind(&Mbdaq0Manager::configure, this,std::placeholders::_1));
  
  this->addTransition("DESTROY","CONFIGURED","CREATED",std::bind(&Mbdaq0Manager::destroy, this,std::placeholders::_1));
  this->addTransition("DESTROY","INITIALISED","CREATED",std::bind(&Mbdaq0Manager::destroy, this,std::placeholders::_1));
  
  
  
  // Commands
  this->addCommand("STATUS",std::bind(&Mbdaq0Manager::c_status,this,std::placeholders::_1));
  this->addCommand("READREG",std::bind(&Mbdaq0Manager::c_readreg,this,std::placeholders::_1));
  this->addCommand("WRITEREG",std::bind(&Mbdaq0Manager::c_writereg,this,std::placeholders::_1));
  this->addCommand("CHANNELON",std::bind(&Mbdaq0Manager::c_channelon,this,std::placeholders::_1));
  
  this->addCommand("RESETTDC",std::bind(&Mbdaq0Manager::c_resettdc,this,std::placeholders::_1));
  this->addCommand("RESETFSM",std::bind(&Mbdaq0Manager::c_resetfsm,this,std::placeholders::_1));


 
  // Initialise NetLink
  _mpi=NULL;

}
void Mbdaq0Manager::end()
{
  //Stop listening
 
if (_mpi!=NULL)
    {
      _mpi->terminate();
      
      _mpi->close();
      for (auto x:_mpi->boards())
	delete x.second;
      _mpi->boards().clear();
      delete _mpi;
      _mpi=NULL;
    }
  
}


void Mbdaq0Manager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbdaq0,"****** CMD: INITIALISING");
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 

  // Need a Mbdaq0 tag
  if (!utils::isMember(params(),"mbdaq0"))
    {
      PMF_ERROR(_logMbdaq0," No mbdaq0 tag found ");
      par["status"]=json::value::string(U("Missing mbdaq0 tag "));
      Reply(status_codes::OK,par);
      return;
    }
  // Now create the Message handler
  if (_mpi==NULL)
    _mpi= new mbdaq0::Interface();
  PMF_INFO(_logMbdaq0,"MPI: INITIALISING");
  _mpi->initialise();

   
  web::json::value jMbdaq0=params()["mbdaq0"];
  //_msh =new MpiMessageHandler("/dev/shm");
  if (!utils::isMember(jMbdaq0,"network"))
    {
      PMF_ERROR(_logMbdaq0," No mbdaq0:network tag found ");
      par["status"]=json::value::string(U("Missing mbdaq0::network tag "));
      Reply(status_codes::OK,par);

      return;
    }
  if (!utils::isMember(jMbdaq0,"address"))
    {
      PMF_ERROR(_logMbdaq0," No mbdaq0:address tag found ");
      par["status"]=json::value::string(U("Missing mbdaq0::address tag "));
      Reply(status_codes::OK,par);
      
      return;
    }
  uint32_t ipboard= utils::convertIP(jMbdaq0["address"].as_string());
  // Scan the network
  std::map<uint32_t,std::string> diflist=utils::scanNetwork(jMbdaq0["network"].as_string());
  // Initialise the network
  std::map<uint32_t,std::string>::iterator idif=diflist.find(ipboard);
  if (idif==diflist.end())
    {
      PMF_ERROR(_logMbdaq0," No board found at address "<<jMbdaq0["address"].as_string());
      par["status"]=json::value::string(U("No board at given address "));
      Reply(status_codes::OK,par);

      return;
    }
  PMF_INFO(_logMbdaq0," New Mbdaq0 found in db "<<std::hex<<ipboard<<std::dec<<" IP address "<<idif->second);
  
  _mpi->addDevice(idif->second);

  PMF_INFO(_logMbdaq0," Registration done for "<<std::hex<<ipboard<<std::dec);
  PMF_INFO(_logMbdaq0,"START Listenning"<<std::flush);

  // Listen All Mbdaq0 sockets
  _mpi->listen();

  // Reset Busy state
  PMF_INFO(_logMbdaq0,"Resetting FSM"<<std::flush);
  this->resetFSM(0x1);
  ::usleep(100000);
  this->resetFSM(0x0);

  PMF_INFO(_logMbdaq0,"Set parameters"<<std::flush);
  PMF_INFO(_logMbdaq0," Init done  ");
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);

}

void Mbdaq0Manager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbdaq0," CMD: CONFIGURING");

  // Now loop on slowcontrol socket


  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);


}


void Mbdaq0Manager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbdaq0," CMD: CLOSING");
  PMF_INFO(_logMbdaq0,"CLOSE called ");

  if (_mpi!=NULL)
    {
      _mpi->terminate();
      
      _mpi->close();
      for (auto x:_mpi->boards())
	delete x.second;
      _mpi->boards().clear();
      delete _mpi;
      _mpi=0;
    }
  PMF_INFO(_logMbdaq0," Data sockets deleted");
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);



  // To be done: _mbdaq0->clear();
}


uint32_t Mbdaq0Manager::version(){return this->readRegister(mbdaq0::Message::Register::VERSION);}
uint32_t Mbdaq0Manager::id(){return this->readRegister(mbdaq0::Message::Register::ID);}

uint32_t Mbdaq0Manager::Channels(){return this->readRegister(mbdaq0::Message::Register::CHANNEL_ENABLE);}
void Mbdaq0Manager::setChannels(uint32_t nc){this->writeRegister(mbdaq0::Message::Register::CHANNEL_ENABLE,nc);}


void Mbdaq0Manager::resetFSM(uint8_t b){this->writeRegister(mbdaq0::Message::Register::RESET_FSM,b);}
void Mbdaq0Manager::resetTDC(uint8_t b){this->writeRegister(mbdaq0::Message::Register::RESET_FE,b);}




uint32_t Mbdaq0Manager::readRegister(uint32_t adr)
{
  uint32_t rc;
  for (auto x:_mpi->boards())
    rc=x.second->reg()->readRegister(adr);
  return rc;
}

void Mbdaq0Manager::writeRegister(uint32_t adr,uint32_t val)
{
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(adr,val);

}


void Mbdaq0Manager::c_readreg(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logMbdaq0,"Pulse called ");
  par["STATUS"]=json::value::string(U("DONE"));

  
  uint32_t adr=utils::queryIntValue(m,"adr",0);

  web::json::value r;
  for (auto x:_mpi->boards())
    {    
      uint32_t value=x.second->reg()->readRegister(adr);
      PMF_INFO(_logMbdaq0,"Read reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<value);
      r[x.second->ipAddress()]=json::value::number(value);
    }
  

  par["READREG"]=r;
  Reply(status_codes::OK,par);
}
void Mbdaq0Manager::c_writereg(http_request m)
{
  auto par = json::value::object();
  par["STATUS"]=json::value::string(U("DONE"));

  
  uint32_t adr=utils::queryIntValue(m,"adr",0);
  uint32_t val=utils::queryIntValue(m,"val",0);

  web::json::value r;
  for (auto x:_mpi->boards())
    {    
      x.second->reg()->writeRegister(adr,val);
      PMF_INFO(_logMbdaq0,"Write reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<val);
      r[x.second->ipAddress()]=json::value::number(val);
    }
  

  par["WRITEREG"]=r;
  Reply(status_codes::OK,par);
}

void Mbdaq0Manager::c_resettdc(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbdaq0," Reset TDC called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->resetTDC(nc&0xF);

  par["STATUS"]=json::value::string(U("DONE"));

  Reply(status_codes::OK,par);
} 
void Mbdaq0Manager::c_resetfsm(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbdaq0," Reset FSM called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->resetFSM(nc&0xF);

  par["STATUS"]=json::value::string(U("DONE"));

  Reply(status_codes::OK,par);
} 

void Mbdaq0Manager::c_channelon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbdaq0," beam on time called ");

  uint32_t nc=utils::queryIntValue(m,"value",1023);
  this->setChannels(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["NCLOCK"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}

void Mbdaq0Manager::c_status(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbdaq0," Status called ");

  web::json::value rc;
  rc["version"]=json::value::number(this->version());
  rc["id"]=json::value::number(this->id());
  rc["channels"]=json::value::number(this->Channels());
  par["COUNTERS"]=rc;
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);

} 


extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new  Mbdaq0Manager);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
