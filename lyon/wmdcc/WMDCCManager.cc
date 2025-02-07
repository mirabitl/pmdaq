#include "WMDCCManager.hh"

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







WmdccManager::WmdccManager() : _mpi(NULL)
{;}

void WmdccManager::initialise()
{
    // Register state

  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  
  this->addTransition("INITIALISE","CREATED","INITIALISED",std::bind(&WmdccManager::fsm_initialise, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","INITIALISED","CONFIGURED",std::bind(&WmdccManager::configure, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",std::bind(&WmdccManager::configure, this,std::placeholders::_1));
  
  this->addTransition("DESTROY","CONFIGURED","CREATED",std::bind(&WmdccManager::destroy, this,std::placeholders::_1));
  this->addTransition("DESTROY","INITIALISED","CREATED",std::bind(&WmdccManager::destroy, this,std::placeholders::_1));
  
  
  
  // Commands
  this->addCommand("STATUS",std::bind(&WmdccManager::c_status,this,std::placeholders::_1));
  this->addCommand("READREG",std::bind(&WmdccManager::c_readreg,this,std::placeholders::_1));
  this->addCommand("WRITEREG",std::bind(&WmdccManager::c_writereg,this,std::placeholders::_1));
  this->addCommand("PAUSE",std::bind(&WmdccManager::c_pause,this,std::placeholders::_1));
  this->addCommand("RESUME",std::bind(&WmdccManager::c_resume,this,std::placeholders::_1));
  this->addCommand("RESET",std::bind(&WmdccManager::c_reset,this,std::placeholders::_1));
  this->addCommand("RESYNC",std::bind(&WmdccManager::c_resync,this,std::placeholders::_1));
  this->addCommand("SPILLON",std::bind(&WmdccManager::c_spillon,this,std::placeholders::_1));
  this->addCommand("SPILLOFF",std::bind(&WmdccManager::c_spilloff,this,std::placeholders::_1));
  this->addCommand("CHANNELON",std::bind(&WmdccManager::c_channelon,this,std::placeholders::_1));
  
  this->addCommand("RESETTDC",std::bind(&WmdccManager::c_resettdc,this,std::placeholders::_1));
  this->addCommand("RESETFSM",std::bind(&WmdccManager::c_resetfsm,this,std::placeholders::_1));
  this->addCommand("CALIBON",std::bind(&WmdccManager::c_calibon,this,std::placeholders::_1));
  this->addCommand("CALIBOFF",std::bind(&WmdccManager::c_caliboff,this,std::placeholders::_1));
  this->addCommand("RELOADCALIB",std::bind(&WmdccManager::c_reloadcalib,this,std::placeholders::_1));
  this->addCommand("SETCALIBCOUNT",std::bind(&WmdccManager::c_setcalibcount,this,std::placeholders::_1));
  this->addCommand("SETSPILLREGISTER",std::bind(&WmdccManager::c_setspillregister,this,std::placeholders::_1));
  this->addCommand("SETCALIBREGISTER",std::bind(&WmdccManager::c_setcalibregister,this,std::placeholders::_1));
  this->addCommand("SETHARDRESET",std::bind(&WmdccManager::c_sethardreset,this,std::placeholders::_1));
  this->addCommand("SETTRIGEXT",std::bind(&WmdccManager::c_settrigext,this,std::placeholders::_1));
  this->addCommand("SETREG",std::bind(&WmdccManager::c_setregister,this,std::placeholders::_1));
  this->addCommand("GETREG",std::bind(&WmdccManager::c_getregister,this,std::placeholders::_1));
  this->addCommand("SETEXTERNAL",std::bind(&WmdccManager::c_setexternaltrigger,this,std::placeholders::_1));
  this->addCommand("SETSPSSPILL",std::bind(&WmdccManager::c_setspsspill,this,std::placeholders::_1));


 
  // Initialise NetLink
  _mpi=NULL;

}
void WmdccManager::end()
{
  //Stop listening
 
if (_mpi!=NULL)
    {
        PMF_INFO(_logWmdcc,"=> TERMINATE");

      _mpi->terminate();
      PMF_INFO(_logWmdcc,"=> CLOSE");
      _mpi->close();

      PMF_INFO(_logWmdcc,"=> DELETE");
      if (_mpi!=NULL) delete _mpi;
      _mpi=NULL;
    }
  
}


void WmdccManager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc,"****** CMD: INITIALISING");
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 

  // Need a WMDCC tag
  if (!utils::isMember(params(),"wmdcc"))
    {
      PMF_ERROR(_logWmdcc," No wmdcc tag found ");
      par["status"]=json::value::string(U("Missing wmdcc tag "));
      Reply(status_codes::OK,par);
      return;
    }
  // Now create the Message handler
  if (_mpi==NULL)
    _mpi= new wizcc::Controller();
  PMF_INFO(_logWmdcc,"MPI: INITIALISING");
  _mpi->initialise();

   
  web::json::value jWMDCC=params()["wmdcc"];
  //_msh =new MpiMessageHandler("/dev/shm");
  if (!utils::isMember(jWMDCC,"network"))
    {
      PMF_ERROR(_logWmdcc," No wmdcc:network tag found ");
      par["status"]=json::value::string(U("Missing wmdcc::network tag "));
      Reply(status_codes::OK,par);

      return;
    }
  if (!utils::isMember(jWMDCC,"address"))
    {
      PMF_ERROR(_logWmdcc," No wmdcc:address tag found ");
      par["status"]=json::value::string(U("Missing wmdcc::address tag "));
      Reply(status_codes::OK,par);
      
      return;
    }
  uint32_t ipboard= utils::convertIP(jWMDCC["address"].as_string());
  // Scan the network
  std::map<uint32_t,std::string> diflist=utils::scanNetwork(jWMDCC["network"].as_string());
  // Initialise the network
  std::map<uint32_t,std::string>::iterator idif=diflist.find(ipboard);
  if (idif==diflist.end())
    {
      PMF_ERROR(_logWmdcc," No board found at address "<<jWMDCC["address"].as_string());
      par["status"]=json::value::string(U("No board at given address "));
      Reply(status_codes::OK,par);

      return;
    }
  PMF_INFO(_logWmdcc," New WMDCC found in db "<<std::hex<<ipboard<<std::dec<<" IP address "<<idif->second);

  wizcc::board* b= new wizcc::board();
  wdmdcc::registerHandler* rh=new wdmdcc::registerHandler(idif->second);
  b->add_processor("REGISTER",rh);
  _mpi->add_board(b);

  PMF_INFO(_logWmdcc," Registration done for "<<std::hex<<ipboard<<std::dec);
  PMF_INFO(_logWmdcc,"START Listenning"<<std::flush);

  // Listen All Wmdcc sockets
  _mpi->listen();

  // Mask the trigger and reset
  this->maskTrigger();
  this->resetCounter();
  // Reset Busy state
  PMF_INFO(_logWmdcc,"Resetting FSM"<<std::flush);
  //this->resetFSM(0x1);
  //::usleep(100000);
  //this->resetFSM(0x0);

  PMF_INFO(_logWmdcc,"Set parameters"<<std::flush);
  
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
  if (utils::isMember(params(),"channels"))
    {
      this->setChannels(params()["channels"].as_integer());
    }
 
  PMF_INFO(_logWmdcc," Init done  ");
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);

}

void WmdccManager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," CMD: CONFIGURING");

  // Now loop on slowcontrol socket


  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);


}


void WmdccManager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," CMD: CLOSING");
  PMF_INFO(_logWmdcc,"CLOSE called ");

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
  PMF_INFO(_logWmdcc," Data sockets deleted");
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);



  // To be done: _wmdcc->clear();
}

void WmdccManager::resyncOn(){this->writeRegister(wmdcc::Message::Register::RESYNC_MASK,0x1);}
void WmdccManager::resyncOff(){this->writeRegister(wmdcc::Message::Register::RESYNC_MASK,0x0);}


uint32_t WmdccManager::version(){return this->readRegister(wmdcc::Message::Register::VERSION);}
uint32_t WmdccManager::id(){return this->readRegister(wmdcc::Message::Register::ID);}
uint32_t WmdccManager::mask(){return this->readRegister(wmdcc::Message::Register::MASK);}
void WmdccManager::maskTrigger(){this->writeRegister(wmdcc::Message::Register::MASK,0xFFFFFFFF);}
void WmdccManager::unmaskTrigger(){this->writeRegister(wmdcc::Message::Register::MASK,0x0);}
uint32_t WmdccManager::spillCount(){return this->readRegister(wmdcc::Message::Register::SPILL_CNT);}
void WmdccManager::resetCounter(){this->writeRegister(wmdcc::Message::Register::ACQ_CTRL,0x1);this->writeRegister(wmdcc::Message::Register::ACQ_CTRL,0x0);}
uint32_t WmdccManager::spillOn(){return this->readRegister(wmdcc::Message::Register::SPILL_ON);}
uint32_t WmdccManager::spillOff(){return this->readRegister(wmdcc::Message::Register::SPILL_OFF);}
void WmdccManager::setSpillOn(uint32_t nc){this->writeRegister(wmdcc::Message::Register::SPILL_ON,nc);}
void WmdccManager::setSpillOff(uint32_t nc){this->writeRegister(wmdcc::Message::Register::SPILL_OFF,nc);}
uint32_t WmdccManager::Channels(){return this->readRegister(wmdcc::Message::Register::CHANNEL_ENABLE);}
void WmdccManager::setChannels(uint32_t nc){this->writeRegister(wmdcc::Message::Register::CHANNEL_ENABLE,nc);}
void WmdccManager::calibOn(){this->writeRegister(wmdcc::Message::Register::CALIB_CTRL,0x2);}
void WmdccManager::calibOff(){this->writeRegister(wmdcc::Message::Register::CALIB_CTRL,0x0);}
uint32_t WmdccManager::calibCount(){return this->readRegister(wmdcc::Message::Register::CALIB_NWIN);}
void WmdccManager::setCalibCount(uint32_t nc){this->writeRegister(wmdcc::Message::Register::CALIB_NWIN,nc);}

void WmdccManager::setCalibRegister(uint32_t nc){this->writeRegister(wmdcc::Message::Register::CALIB_CTRL,nc);}

uint32_t WmdccManager::hardReset(){return this->readRegister(wmdcc::Message::Register::RESET_FE);}
void WmdccManager::setHardReset(uint32_t nc){this->writeRegister(wmdcc::Message::Register::RESET_FE,nc);}

void WmdccManager::setSpillRegister(uint32_t nc){this->writeRegister(wmdcc::Message::Register::WIN_CTRL,nc);}
uint32_t WmdccManager::spillRegister(){return this->readRegister(wmdcc::Message::Register::WIN_CTRL);}
void WmdccManager::useSPSSpill(uint32_t len)
{

  if (len==0)
    {
      this->writeRegister(wmdcc::Message::Register::SPS_SPILL_CTRL,0);
      this->writeRegister(wmdcc::Message::Register::SPS_SPILL_DURATION,0);
    }
  else
    {
      this->writeRegister(wmdcc::Message::Register::SPS_SPILL_CTRL,1);
      this->writeRegister(wmdcc::Message::Register::SPS_SPILL_DURATION,len);
    }

}
void WmdccManager::useTrigExt(bool t)
{
  uint32_t reg=this->spillRegister();
  if (t)
    this->setSpillRegister(reg|2);
  else
    this->setSpillRegister(reg&~2);
}

void WmdccManager::setTriggerDelay(uint32_t nc){this->writeRegister(wmdcc::Message::Register::TRG_EXT_DELAY,nc);}
uint32_t WmdccManager::triggerDelay(){return this->readRegister(wmdcc::Message::Register::TRG_EXT_DELAY);}
void WmdccManager::setTriggerBusy(uint32_t nc){this->writeRegister(wmdcc::Message::Register::TRG_EXT_LEN,nc);}
uint32_t WmdccManager::triggerBusy(){return this->readRegister(wmdcc::Message::Register::TRG_EXT_LEN);}

void WmdccManager::setExternalTrigger(uint32_t nc){this->writeRegister(wmdcc::Message::Register::EN_BUSY_TRG,nc);}
uint32_t WmdccManager::externalTrigger(){return this->readRegister(wmdcc::Message::Register::EN_BUSY_TRG);}

void WmdccManager::reloadCalibCount(){

  this->maskTrigger();
  this->writeRegister(wmdcc::Message::Register::WIN_CTRL,0x8);
  this->writeRegister(wmdcc::Message::Register::CALIB_CTRL,0x4);
  // sleep(1);
  // this->writeRegister(0x8,0x0);
  // sleep(1);
  this->unmaskTrigger();
  this->calibOn();


}



void WmdccManager::resetFSM(uint32_t b){this->writeRegister(wmdcc::Message::Register::RESET_FSM,b);}
void WmdccManager::resetTDC(uint32_t b){this->writeRegister(wmdcc::Message::Register::RESET_FE,b);}
uint32_t WmdccManager::busyCount(uint32_t b){return this->readRegister(wmdcc::Message::Register::BUSY_0+(b&0xF));}



uint32_t WmdccManager::readRegister(uint32_t adr)
{
  uint32_t rc;
  for (auto x:_mpi->boards())
    rc=x.second->reg()->readRegister(adr);
  return rc;
}

void WmdccManager::writeRegister(uint32_t adr,uint32_t val)
{
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(adr,val);

}


void WmdccManager::c_readreg(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logWmdcc,"Pulse called ");
  par["STATUS"]=json::value::string(U("DONE"));

  
  uint32_t adr=utils::queryIntValue(m,"adr",0);

  web::json::value r;
  for (auto x:_mpi->boards())
    {    
      uint32_t value=x.second->reg()->readRegister(adr);
      PMF_INFO(_logWmdcc,"Read reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<value);
      r[x.second->ipAddress()]=json::value::number(value);
    }
  

  par["READREG"]=r;
  Reply(status_codes::OK,par);
}
void WmdccManager::c_writereg(http_request m)
{
  auto par = json::value::object();
  par["STATUS"]=json::value::string(U("DONE"));

  
  uint32_t adr=utils::queryIntValue(m,"adr",0);
  uint32_t val=utils::queryIntValue(m,"val",0);

  web::json::value r;
  for (auto x:_mpi->boards())
    {    
      x.second->reg()->writeRegister(adr,val);
      PMF_INFO(_logWmdcc,"Write reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<val);
      r[x.second->ipAddress()]=json::value::number(val);
    }
  

  par["WRITEREG"]=r;
  Reply(status_codes::OK,par);
}

void WmdccManager::c_pause(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Pause called ");

  this->maskTrigger();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}
void WmdccManager::c_resume(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Resume called ");

  this->unmaskTrigger();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}
void WmdccManager::c_calibon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Calib On called ");
  this->calibOn();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}
void WmdccManager::c_caliboff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Calib Off called ");
  this->calibOff();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}
void WmdccManager::c_reloadcalib(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Calib reload called ");
  this->reloadCalibCount();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}

void WmdccManager::c_setcalibcount(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Calib count called ");
  uint32_t nc=utils::queryIntValue(m,"nclock",5000000);
  this->setCalibCount(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["NCLOCK"]=json::value::number(nc);
  Reply(status_codes::OK,par);
} 

void WmdccManager::c_reset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," RESET called ");
  this->resetCounter();
  par["STATUS"]=json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}


void WmdccManager::c_resync(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Resync switch called ");
  
  uint32_t nc=utils::queryIntValue(m,"value",0);
  if (nc==0)
    this->resyncOff();
  else
    this->resyncOn();
  par["STATUS"]=json::value::string(U("DONE"));
  par["RESYNC"]=json::value::number(nc);
  Reply(status_codes::OK,par);

} 
void WmdccManager::c_spillon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Spill ON called ");
  
  uint32_t nc=utils::queryIntValue(m,"nclock",50);
  this->setSpillOn(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["NCLOCK"]=json::value::number(nc);
  Reply(status_codes::OK,par);

} 
void WmdccManager::c_spilloff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Spill Off called ");

  uint32_t nc=utils::queryIntValue(m,"nclock",5000);
  this->setSpillOff(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["NCLOCK"]=json::value::number(nc);
  Reply(status_codes::OK,par);

} 
void WmdccManager::c_resettdc(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Reset TDC called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->resetTDC(nc&0xFFFF);

  par["STATUS"]=json::value::string(U("DONE"));

  Reply(status_codes::OK,par);
} 
void WmdccManager::c_resetfsm(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Reset FSM called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->resetFSM(nc&0xFFFF);

  par["STATUS"]=json::value::string(U("DONE"));

  Reply(status_codes::OK,par);
} 

void WmdccManager::c_channelon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," beam on time called ");

  uint32_t nc=utils::queryIntValue(m,"value",1023);
  this->setChannels(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["NCLOCK"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}

void WmdccManager::c_sethardreset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Hard reset called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->setHardReset(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}

void WmdccManager::c_setspillregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc,"Spill register called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->setSpillRegister(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}
void WmdccManager::c_setexternaltrigger(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc,"Spill register called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->setExternalTrigger(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}
void WmdccManager::c_setspsspill(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc,"SPS Spill register called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  this->useSPSSpill(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}

void WmdccManager::c_setregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc,"Set register called ");

  uint32_t adr=utils::queryIntValue(m,"address",2);
  uint32_t val=utils::queryIntValue(m,"value",0);
  PMF_INFO(_logWmdcc,"Set register called with "<<adr<<" => "<<val);

  this->writeRegister(adr,val);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(this->readRegister(adr));
  Reply(status_codes::OK,par);
}
void WmdccManager::c_getregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc,"Get register called ");

  uint32_t adr=utils::queryIntValue(m,"address",2);
  PMF_INFO(_logWmdcc,"Get register called with "<<adr);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(this->readRegister(adr));
  //std::cout<<response<<std::endl;
  Reply(status_codes::OK,par);
}
void WmdccManager::c_setcalibregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc,"Calib register called ");

  uint32_t nc=utils::queryIntValue(m,"value",0);
  PMF_INFO(_logWmdcc,"Calib register called "<<nc);
  this->setCalibRegister(nc);

  par["STATUS"]=json::value::string(U("DONE"));
  par["VALUE"]=json::value::number(nc);
  Reply(status_codes::OK,par);
}

void WmdccManager::c_settrigext(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Trig ext setting called ");

  uint32_t delay=utils::queryIntValue(m,"delay",20);
  uint32_t busy=utils::queryIntValue(m,"busy",20);
  this->setTriggerDelay(delay);
  this->setTriggerBusy(busy);

  par["STATUS"]=json::value::string(U("DONE"));
  par["DELAY"]=json::value::number(delay);
  par["BUSY"]=json::value::number(busy);
  Reply(status_codes::OK,par);
} 

void WmdccManager::c_status(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logWmdcc," Status called ");

  web::json::value rc;
  rc["version"]=json::value::number(this->version());
  rc["id"]=json::value::number(this->id());
  rc["mask"]=json::value::number(this->mask());
  rc["hard"]=json::value::number(this->hardReset());
  rc["spill"]=json::value::number(this->spillCount());
  uint32_t chen=this->Channels();
    for (int i=0;i<16;i++)
    {
      if ((chen>>i)&1)
	{
	  std::stringstream sb;
	  sb<<"busy"<<i;
	  rc[sb.str()]=json::value::number(this->busyCount(i));
	}
    }
  rc["spillon"]=json::value::number(this->spillOn());
  rc["spilloff"]=json::value::number(this->spillOff());
  rc["channels"]=json::value::number(this->Channels());
  rc["calib"]=json::value::number(this->calibCount());
  rc["spillreg"]=json::value::number(this->spillRegister());
  rc["trigdelay"]=json::value::number(this->triggerDelay());
  rc["external"]=json::value::number(this->externalTrigger());
  par["COUNTERS"]=rc;
  par["STATUS"]=json::value::string(U("DONE"));
  mqtt_publish("status",par);
  Reply(status_codes::OK,par);

} 


extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new  WmdccManager);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
