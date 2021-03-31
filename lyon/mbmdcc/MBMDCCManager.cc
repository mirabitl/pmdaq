#include "MBMDCCManager.hh"

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







MbmdccManager::MbmdccManager() : _mpi(NULL)
{;}

void MbmdccManager::initialise()
{
    // Register state

  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  
  this->addTransition("INITIALISE","CREATED","INITIALISED",std::bind(&MbmdccManager::fsm_initialise, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","INITIALISED","CONFIGURED",std::bind(&MbmdccManager::configure, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",std::bind(&MbmdccManager::configure, this,std::placeholders::_1));
  
  this->addTransition("DESTROY","CONFIGURED","CREATED",std::bind(&MbmdccManager::destroy, this,std::placeholders::_1));
  this->addTransition("DESTROY","INITIALISED","CREATED",std::bind(&MbmdccManager::destroy, this,std::placeholders::_1));
  
  
  
  // Commands
  this->addCommand("STATUS",std::bind(&MbmdccManager::c_status,this,std::placeholders::_1));
  this->addCommand("READREG",std::bind(&MbmdccManager::c_readreg,this,std::placeholders::_1));
  this->addCommand("WRITEREG",std::bind(&MbmdccManager::c_writereg,this,std::placeholders::_1));
  this->addCommand("PAUSE",std::bind(&MbmdccManager::c_pause,this,std::placeholders::_1));
  this->addCommand("RESUME",std::bind(&MbmdccManager::c_resume,this,std::placeholders::_1));
  this->addCommand("RESET",std::bind(&MbmdccManager::c_reset,this,std::placeholders::_1));

  this->addCommand("SPILLON",std::bind(&MbmdccManager::c_spillon,this,std::placeholders::_1));
  this->addCommand("SPILLOFF",std::bind(&MbmdccManager::c_spilloff,this,std::placeholders::_1));
  this->addCommand("CHANNELON",std::bind(&MbmdccManager::c_channelon,this,std::placeholders::_1));
  
  this->addCommand("RESETTDC",std::bind(&MbmdccManager::c_resettdc,this,std::placeholders::_1));
  this->addCommand("RESETFSM",std::bind(&MbmdccManager::c_resetfsm,this,std::placeholders::_1));
  this->addCommand("CALIBON",std::bind(&MbmdccManager::c_calibon,this,std::placeholders::_1));
  this->addCommand("CALIBOFF",std::bind(&MbmdccManager::c_caliboff,this,std::placeholders::_1));
  this->addCommand("RELOADCALIB",std::bind(&MbmdccManager::c_reloadcalib,this,std::placeholders::_1));
  this->addCommand("SETCALIBCOUNT",std::bind(&MbmdccManager::c_setcalibcount,this,std::placeholders::_1));
  this->addCommand("SETSPILLREGISTER",std::bind(&MbmdccManager::c_setspillregister,this,std::placeholders::_1));
  this->addCommand("SETCALIBREGISTER",std::bind(&MbmdccManager::c_setcalibregister,this,std::placeholders::_1));
  this->addCommand("SETHARDRESET",std::bind(&MbmdccManager::c_sethardreset,this,std::placeholders::_1));
  this->addCommand("SETTRIGEXT",std::bind(&MbmdccManager::c_settrigext,this,std::placeholders::_1));
  this->addCommand("SETREG",std::bind(&MbmdccManager::c_setregister,this,std::placeholders::_1));
  this->addCommand("GETREG",std::bind(&MbmdccManager::c_getregister,this,std::placeholders::_1));
  this->addCommand("SETEXTERNAL",std::bind(&MbmdccManager::c_setexternaltrigger,this,std::placeholders::_1));


 
  // Initialise NetLink
  _mpi=NULL;

}
void MbmdccManager::end()
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


void MbmdccManager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc,"****** CMD: INITIALISING");
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 

  // Need a MBMDCC tag
  if (!utils::isMember(params(),"mbmdcc"))
    {
      PMF_ERROR(_logMbmdcc," No mbmdcc tag found ");
      par["status"]=json::value::string(U("Missing mbmdcc tag "));
      Reply(status_codes::OK,par);
      return;
    }
  // Now create the Message handler
  if (_mpi==NULL)
    _mpi= new mbmdcc::Interface();
  PMF_INFO(_logMbmdcc,"MPI: INITIALISING");
  _mpi->initialise();

   
  web::json::value jMBMDCC=params()["mbmdcc"];
  //_msh =new MpiMessageHandler("/dev/shm");
  if (!utils::isMember(jMBMDCC,"network"))
    {
      PMF_ERROR(_logMbmdcc," No mbmdcc:network tag found ");
      par["status"]=json::value::string(U("Missing mbmdcc::network tag "));
      Reply(status_codes::OK,par);

      return;
    }
  if (!utils::isMember(jMBMDCC,"address"))
    {
      PMF_ERROR(_logMbmdcc," No mbmdcc:address tag found ");
      par["status"]=json::value::string(U("Missing mbmdcc::address tag "));
      Reply(status_codes::OK,par);
      
      return;
    }
  uint32_t ipboard= utils::convertIP(jMBMDCC["address"].as_string());
  // Scan the network
  std::map<uint32_t,std::string> diflist=utils::scanNetwork(jMBMDCC["network"].as_string());
  // Initialise the network
  std::map<uint32_t,std::string>::iterator idif=diflist.find(ipboard);
  if (idif==diflist.end())
    {
      PMF_ERROR(_logMbmdcc," No board found at address "<<jMBMDCC["address"].as_string());
      par["status"]=json::value::string(U("No board at given address "));
      Reply(status_codes::OK,par);

      return;
    }
  PMF_INFO(_logMbmdcc," New MBMDCC found in db "<<std::hex<<ipboard<<std::dec<<" IP address "<<idif->second);
  
  _mpi->addDevice(idif->second);

  PMF_INFO(_logMbmdcc," Registration done for "<<std::hex<<ipboard<<std::dec);
  PMF_INFO(_logMbmdcc,"START Listenning");

  // Listen All Mbmdcc sockets
  _mpi->listen();

  // Reset Busy state
  this->resetFSM(0x1);
  ::usleep(100000);
  this->resetFSM(0x0);
  
  if (utils::isMember(params(),"spillon"))
    {
      this->setSpillOn(params()["spillon"].as_integer());
    }
  if (utils::isMember(params(),"spilloff"))
    {
      this->setSpillOff(params()["spilloff"].as_integer());
    }
  if (utils::isMember(params(),"spillregister"))
    {
      this->setSpillRegister(params()["spillregister"].as_integer());
    }
 
  PMF_INFO(_logMbmdcc," Init done  ");
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);

}

void MbmdccManager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," CMD: CONFIGURING");

  // Now loop on slowcontrol socket


  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);


}


void MbmdccManager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," CMD: CLOSING");
  PMF_INFO(_logMbmdcc,"CLOSE called ");

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
  PMF_INFO(_logMbmdcc," Data sockets deleted");
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);



  // To be done: _mbmdcc->clear();
}


uint32_t MbmdccManager::version(){return this->readRegister(mbmdcc::Message::Register::VERSION);}
uint32_t MbmdccManager::id(){return this->readRegister(mbmdcc::Message::Register::ID);}
uint32_t MbmdccManager::mask(){return this->readRegister(mbmdcc::Message::Register::MASK);}
void MbmdccManager::maskTrigger(){this->writeRegister(mbmdcc::Message::Register::MASK,0xFFFFFFFF);}
void MbmdccManager::unmaskTrigger(){this->writeRegister(mbmdcc::Message::Register::MASK,0x0);}
uint32_t MbmdccManager::spillCount(){return this->readRegister(mbmdcc::Message::Register::SPILL_CNT);}
void MbmdccManager::resetCounter(){this->writeRegister(mbmdcc::Message::Register::ACQ_CTRL,0x1);this->writeRegister(mbmdcc::Message::Register::ACQ_CTRL,0x0);}
uint32_t MbmdccManager::spillOn(){return this->readRegister(mbmdcc::Message::Register::SPILL_ON);}
uint32_t MbmdccManager::spillOff(){return this->readRegister(mbmdcc::Message::Register::SPILL_OFF);}
void MbmdccManager::setSpillOn(uint32_t nc){this->writeRegister(mbmdcc::Message::Register::SPILL_ON,nc);}
void MbmdccManager::setSpillOff(uint32_t nc){this->writeRegister(mbmdcc::Message::Register::SPILL_OFF,nc);}
uint32_t MbmdccManager::Channels(){return this->readRegister(mbmdcc::Message::Register::CHANNEL_ENABLE);}
void MbmdccManager::setChannels(uint32_t nc){this->writeRegister(mbmdcc::Message::Register::CHANNEL_ENABLE,nc);}
void MbmdccManager::calibOn(){this->writeRegister(mbmdcc::Message::Register::CALIB_CTRL,0x2);}
void MbmdccManager::calibOff(){this->writeRegister(mbmdcc::Message::Register::CALIB_CTRL,0x0);}
uint32_t MbmdccManager::calibCount(){return this->readRegister(mbmdcc::Message::Register::CALIB_NWIN);}
void MbmdccManager::setCalibCount(uint32_t nc){this->writeRegister(mbmdcc::Message::Register::CALIB_NWIN,nc);}

void MbmdccManager::setCalibRegister(uint32_t nc){this->writeRegister(mbmdcc::Message::Register::CALIB_CTRL,nc);}

uint32_t MbmdccManager::hardReset(){return this->readRegister(mbmdcc::Message::Register::RESET_FE);}
void MbmdccManager::setHardReset(uint32_t nc){this->writeRegister(mbmdcc::Message::Register::RESET_FE,nc);}

void MbmdccManager::setSpillRegister(uint32_t nc){this->writeRegister(mbmdcc::Message::Register::WIN_CTRL,nc);}
uint32_t MbmdccManager::spillRegister(){return this->readRegister(mbmdcc::Message::Register::WIN_CTRL);}
void MbmdccManager::useSPSSpill(bool t)
{
  uint32_t reg=this->spillRegister();
  if (t)
    this->setSpillRegister(reg|1);
  else
    this->setSpillRegister(reg&~1);
}
void MbmdccManager::useTrigExt(bool t)
{
  uint32_t reg=this->spillRegister();
  if (t)
    this->setSpillRegister(reg|2);
  else
    this->setSpillRegister(reg&~2);
}

void MbmdccManager::setTriggerDelay(uint32_t nc){this->writeRegister(mbmdcc::Message::Register::TRG_EXT_DELAY,nc);}
uint32_t MbmdccManager::triggerDelay(){return this->readRegister(mbmdcc::Message::Register::TRG_EXT_DELAY);}
void MbmdccManager::setTriggerBusy(uint32_t nc){this->writeRegister(mbmdcc::Message::Register::TRG_EXT_LEN,nc);}
uint32_t MbmdccManager::triggerBusy(){return this->readRegister(mbmdcc::Message::Register::TRG_EXT_LEN);}

void MbmdccManager::setExternalTrigger(uint32_t nc){this->writeRegister(mbmdcc::Message::Register::EN_BUSY_TRG,nc);}
uint32_t MbmdccManager::externalTrigger(){return this->readRegister(mbmdcc::Message::Register::EN_BUSY_TRG);}

void MbmdccManager::reloadCalibCount(){

  this->maskTrigger();
  this->writeRegister(mbmdcc::Message::Register::WIN_CTRL,0x8);
  this->writeRegister(mbmdcc::Message::Register::CALIB_CTRL,0x4);
  // sleep(1);
  // this->writeRegister(0x8,0x0);
  // sleep(1);
  this->unmaskTrigger();
  this->calibOn();


}



void MbmdccManager::resetFSM(uint8_t b){this->writeRegister(mbmdcc::Message::Register::RESET_FSM,b);}
void MbmdccManager::resetTDC(uint8_t b){this->writeRegister(mbmdcc::Message::Register::RESET_FE,b);}
uint32_t MbmdccManager::busyCount(uint8_t b){return this->readRegister(mbmdcc::Message::Register::BUSY_0+(b&0xF));}



uint32_t MbmdccManager::readRegister(uint32_t adr)
{
  uint32_t rc;
  for (auto x:_mpi->boards())
    rc=x.second->reg()->readRegister(adr);
  return rc;
}

void MbmdccManager::writeRegister(uint32_t adr,uint32_t val)
{
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(adr,val);

}


void MbmdccManager::c_readreg(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logMbmdcc,"Pulse called ");
  par["STATUS"]=json::value::string(U("DONE"));

  
  uint32_t adr=utils::queryIntValue(m,"adr",0);

  web::json::value r;
  for (auto x:_mpi->boards())
    {    
      uint32_t value=x.second->reg()->readRegister(adr);
      PMF_INFO(_logMbmdcc,"Read reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<value);
      r[x.second->ipAddress()]=json::value::number(value);
    }
  

  par["READREG"]=r;
  Reply(status_codes::OK,par);
}
void MbmdccManager::c_writereg(http_request m)
{
  auto par = json::value::object();
  par["STATUS"]=json::value::string(U("DONE"));

  
  uint32_t adr=utils::queryIntValue(m,"adr",0);
  uint32_t val=utils::queryIntValue(m,"val",0);

  web::json::value r;
  for (auto x:_mpi->boards())
    {    
      x.second->reg()->writeRegister(adr,val);
      PMF_INFO(_logMbmdcc,"Write reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<val);
      r[x.second->ipAddress()]=json::value::number(val);
    }
  

  par["WRITEREG"]=r;
  Reply(status_codes::OK,par);
}

void MbmdccManager::c_pause(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Pause called ");

  this->maskTrigger();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}
void MbmdccManager::c_resume(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Resume called ");

  this->unmaskTrigger();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}
void MbmdccManager::c_calibon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Calib On called ");
  this->calibOn();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}
void MbmdccManager::c_caliboff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Calib Off called ");
  this->calibOff();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}
void MbmdccManager::c_reloadcalib(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Calib reload called ");
  this->reloadCalibCount();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}

void MbmdccManager::c_setcalibcount(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Calib count called ");
  uint32_t nc=utils::queryIntValue(m,"nclock",5000000);
  this->setCalibCount(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["NCLOCK"]=json::value::number(nc);
  Reply(status_codes::OK,par);
} 

void MbmdccManager::c_reset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," RESET called ");
  this->resetCounter();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}


void MbmdccManager::c_spillon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Spill ON called ");
  
  uint32_t nc=utils::queryIntValue(m,"nclock",50);
  this->setSpillOn(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["NCLOCK"]=json::value::number(nc);
  Reply(status_codes::OK,par);

} 
void MbmdccManager::c_spilloff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Spill Off called ");

  uint32_t nc=utils::queryIntValue(m,"nclock",5000);
  this->setSpillOff(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["NCLOCK"]=json::value::number(nc);
  Reply(status_codes::OK,par);

} 
void MbmdccManager::c_resettdc(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Reset TDC called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->resetTDC(nc&0xF);

  par["STATUS"]=json::value::string(U("DONE"));

  Reply(status_codes::OK,par);
} 
void MbmdccManager::c_resetfsm(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Reset FSM called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->resetFSM(nc&0xF);

  par["STATUS"]=json::value::string(U("DONE"));

  Reply(status_codes::OK,par);
} 

void MbmdccManager::c_channelon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," beam on time called ");

  uint32_t nc=utils::queryIntValue(m,"value",1023);
  this->setChannels(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["NCLOCK"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}

void MbmdccManager::c_sethardreset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Hard reset called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->setHardReset(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}

void MbmdccManager::c_setspillregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc,"Spill register called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->setSpillRegister(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}
void MbmdccManager::c_setexternaltrigger(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc,"Spill register called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->setExternalTrigger(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}

void MbmdccManager::c_setregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc,"Set register called ");

  uint32_t adr=utils::queryIntValue(m,"address",2);
  uint32_t val=utils::queryIntValue(m,"value",0);
  PMF_INFO(_logMbmdcc,"Set register called with "<<adr<<" => "<<val);

  this->writeRegister(adr,val);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(this->readRegister(adr));
  Reply(status_codes::OK,par);
}
void MbmdccManager::c_getregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc,"Get register called ");

  uint32_t adr=utils::queryIntValue(m,"address",2);
  PMF_INFO(_logMbmdcc,"Get register called with "<<adr);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(this->readRegister(adr));
  //std::cout<<response<<std::endl;
  Reply(status_codes::OK,par);
}
void MbmdccManager::c_setcalibregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc,"Calib register called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  PMF_INFO(_logMbmdcc,"Calib register called "<<nc);
  this->setCalibRegister(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}

void MbmdccManager::c_settrigext(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Trig ext setting called ");

  uint32_t delay=utils::queryIntValue(m,"delay",20);
  uint32_t busy=utils::queryIntValue(m,"busy",20);
  this->setTriggerDelay(delay);
  this->setTriggerBusy(busy);

  par["STATUS"]=json::value::string(U("DONE"));
  par["DELAY"]=json::value::number(delay);
  par["BUSY"]=json::value::number(busy);
  Reply(status_codes::OK,par);
} 

void MbmdccManager::c_status(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc," Status called ");

  web::json::value rc;
  rc["version"]=json::value::number(this->version());
  rc["id"]=json::value::number(this->id());
  rc["mask"]=json::value::number(this->mask());
  rc["hard"]=json::value::number(this->hardReset());
  rc["spill"]=json::value::number(this->spillCount());
  rc["busy1"]=json::value::number(this->busyCount(1));
  rc["busy2"]=json::value::number(this->busyCount(2));
  rc["busy3"]=json::value::number(this->busyCount(3));
  rc["busy4"]=json::value::number(this->busyCount(4));
  rc["busy5"]=json::value::number(this->busyCount(5));
  rc["busy6"]=json::value::number(this->busyCount(6));
  rc["busy7"]=json::value::number(this->busyCount(7));
  rc["busy8"]=json::value::number(this->busyCount(8));
  rc["busy9"]=json::value::number(this->busyCount(9));
  rc["busy10"]=json::value::number(this->busyCount(10));
  rc["spillon"]=json::value::number(this->spillOn());
  rc["spilloff"]=json::value::number(this->spillOff());
  rc["channels"]=json::value::number(this->Channels());
  rc["calib"]=json::value::number(this->calibCount());
  rc["spillreg"]=json::value::number(this->spillRegister());
  rc["trigdelay"]=json::value::number(this->triggerDelay());
  rc["external"]=json::value::number(this->externalTrigger());
  par["COUNTERS"]=rc;
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);

} 


extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new  MbmdccManager);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
