#include "MBMDCCManager.hh"
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




#include "fsmwebCaller.hh"

using namespace zdaq;
using namespace lydaq;


lydaq::MbmdccManager::MbmdccManager(std::string name) : zdaq::baseApplication(name),_mpi(NULL)
{
  _fsm=this->fsm();
  // Register state

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&lydaq::MbmdccManager::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&lydaq::MbmdccManager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::MbmdccManager::configure, this,_1));
  
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&lydaq::MbmdccManager::destroy, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&lydaq::MbmdccManager::destroy, this,_1));
  
  
  
  // Commands
  _fsm->addCommand("STATUS",boost::bind(&lydaq::MbmdccManager::c_status,this,_1,_2));
  _fsm->addCommand("READREG",boost::bind(&lydaq::MbmdccManager::c_readreg,this,_1,_2));
  _fsm->addCommand("WRITEREG",boost::bind(&lydaq::MbmdccManager::c_writereg,this,_1,_2));
  _fsm->addCommand("PAUSE",boost::bind(&lydaq::MbmdccManager::c_pause,this,_1,_2));
  _fsm->addCommand("RESUME",boost::bind(&lydaq::MbmdccManager::c_resume,this,_1,_2));
  _fsm->addCommand("RESET",boost::bind(&lydaq::MbmdccManager::c_reset,this,_1,_2));

  _fsm->addCommand("SPILLON",boost::bind(&lydaq::MbmdccManager::c_spillon,this,_1,_2));
  _fsm->addCommand("SPILLOFF",boost::bind(&lydaq::MbmdccManager::c_spilloff,this,_1,_2));
  _fsm->addCommand("CHANNELON",boost::bind(&lydaq::MbmdccManager::c_channelon,this,_1,_2));
  
  _fsm->addCommand("RESETTDC",boost::bind(&lydaq::MbmdccManager::c_resettdc,this,_1,_2));
  _fsm->addCommand("CALIBON",boost::bind(&lydaq::MbmdccManager::c_calibon,this,_1,_2));
  _fsm->addCommand("CALIBOFF",boost::bind(&lydaq::MbmdccManager::c_caliboff,this,_1,_2));
  _fsm->addCommand("RELOADCALIB",boost::bind(&lydaq::MbmdccManager::c_reloadcalib,this,_1,_2));
  _fsm->addCommand("SETCALIBCOUNT",boost::bind(&lydaq::MbmdccManager::c_setcalibcount,this,_1,_2));
  _fsm->addCommand("SETSPILLREGISTER",boost::bind(&lydaq::MbmdccManager::c_setspillregister,this,_1,_2));
  _fsm->addCommand("SETCALIBREGISTER",boost::bind(&lydaq::MbmdccManager::c_setcalibregister,this,_1,_2));
  _fsm->addCommand("SETHARDRESET",boost::bind(&lydaq::MbmdccManager::c_sethardreset,this,_1,_2));
  _fsm->addCommand("SETTRIGEXT",boost::bind(&lydaq::MbmdccManager::c_settrigext,this,_1,_2));
  _fsm->addCommand("SETREG",boost::bind(&lydaq::MbmdccManager::c_setregister,this,_1,_2));
  _fsm->addCommand("GETREG",boost::bind(&lydaq::MbmdccManager::c_getregister,this,_1,_2));
  _fsm->addCommand("SETEXTERNAL",boost::bind(&lydaq::MbmdccManager::c_setexternaltrigger,this,_1,_2));


  /// Test
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Service "<<name<<" is starting on "<<atoi(wp));

      
      _fsm->start(atoi(wp));
    }
    
  
 
  // Initialise NetLink


}


void lydaq::MbmdccManager::initialise(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"****** CMD: "<<m->command());
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 

  // Need a MBMDCC tag
  if (m->content().isMember("mbmdcc"))
    {
      printf ("found mbmdcc/n");
      this->parameters()["mbmdcc"]=m->content()["mbmdcc"];
    }
  if (!this->parameters().isMember("mbmdcc"))
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No mbmdcc tag found ");
      return;
    }
  // Now create the Message handler
  if (_mpi==NULL)
    _mpi= new lydaq::mbmdcc::Interface();
  _mpi->initialise();

   
  Json::Value jMBMDCC=this->parameters()["mbmdcc"];
  //_msh =new lydaq::MpiMessageHandler("/dev/shm");
  if (!jMBMDCC.isMember("network"))
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No mbmdcc:network tag found ");
      return;
    }
  if (!jMBMDCC.isMember("address"))
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No mbmdcc:address tag found ");
      return;
    }
  uint32_t ipboard= mpi::MpiMessageHandler::convertIP(jMBMDCC["address"].asString());
  // Scan the network
  std::map<uint32_t,std::string> diflist=mpi::MpiMessageHandler::scanNetwork(jMBMDCC["network"].asString());
  // Initialise the network
  std::map<uint32_t,std::string>::iterator idif=diflist.find(ipboard);
  if (idif==diflist.end())
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No board found at address "<<jMBMDCC["address"].asString());
      return;
    }
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" New MBMDCC found in db "<<std::hex<<ipboard<<std::dec<<" IP address "<<idif->second);
  _mpi->addDevice(idif->second);
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Registration done for "<<std::hex<<ipboard<<std::dec);


  if (this->parameters().isMember("spillon"))
    {
      this->setSpillOn(this->parameters()["spillon"].asUInt());
    }
  if (this->parameters().isMember("spilloff"))
    {
      this->setSpillOff(this->parameters()["spilloff"].asUInt());
    }
  if (this->parameters().isMember("spillregister"))
    {
      this->setSpillRegister(this->parameters()["spillregister"].asUInt());
    }
  // Listen All Mbmdcc sockets
  _mpi->listen();

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Init done  "); 
}

void lydaq::MbmdccManager::configure(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());

  // Now loop on slowcontrol socket




}


void lydaq::MbmdccManager::destroy(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"CLOSE called ");
  
  _mpi->close();
  for (auto x:_mpi->boards())
    delete x.second;
  _mpi->boards().clear();
  delete _mpi;
  _mpi=0;

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data sockets deleted");



  // To be done: _mbmdcc->clear();
}


uint32_t lydaq::MbmdccManager::version(){return this->readRegister(lydaq::mbmdcc::Message::Register::VERSION);}
uint32_t lydaq::MbmdccManager::id(){return this->readRegister(lydaq::mbmdcc::Message::Register::ID);}
uint32_t lydaq::MbmdccManager::mask(){return this->readRegister(lydaq::mbmdcc::Message::Register::MASK);}
void lydaq::MbmdccManager::maskTrigger(){this->writeRegister(lydaq::mbmdcc::Message::Register::MASK,0xFFFFFFFF);}
void lydaq::MbmdccManager::unmaskTrigger(){this->writeRegister(lydaq::mbmdcc::Message::Register::MASK,0x0);}
uint32_t lydaq::MbmdccManager::spillCount(){return this->readRegister(lydaq::mbmdcc::Message::Register::SPILL_CNT);}
void lydaq::MbmdccManager::resetCounter(){this->writeRegister(lydaq::mbmdcc::Message::Register::ACQ_CTRL,0x1);this->writeRegister(lydaq::mbmdcc::Message::Register::ACQ_CTRL,0x0);}
uint32_t lydaq::MbmdccManager::spillOn(){return this->readRegister(lydaq::mbmdcc::Message::Register::SPILL_ON);}
uint32_t lydaq::MbmdccManager::spillOff(){return this->readRegister(lydaq::mbmdcc::Message::Register::SPILL_OFF);}
void lydaq::MbmdccManager::setSpillOn(uint32_t nc){this->writeRegister(lydaq::mbmdcc::Message::Register::SPILL_ON,nc);}
void lydaq::MbmdccManager::setSpillOff(uint32_t nc){this->writeRegister(lydaq::mbmdcc::Message::Register::SPILL_OFF,nc);}
uint32_t lydaq::MbmdccManager::Channels(){return this->readRegister(lydaq::mbmdcc::Message::Register::CHANNEL_ENABLE);}
void lydaq::MbmdccManager::setChannels(uint32_t nc){this->writeRegister(lydaq::mbmdcc::Message::Register::CHANNEL_ENABLE,nc);}
void lydaq::MbmdccManager::calibOn(){this->writeRegister(lydaq::mbmdcc::Message::Register::CALIB_CTRL,0x2);}
void lydaq::MbmdccManager::calibOff(){this->writeRegister(lydaq::mbmdcc::Message::Register::CALIB_CTRL,0x0);}
uint32_t lydaq::MbmdccManager::calibCount(){return this->readRegister(lydaq::mbmdcc::Message::Register::CALIB_NWIN);}
void lydaq::MbmdccManager::setCalibCount(uint32_t nc){this->writeRegister(lydaq::mbmdcc::Message::Register::CALIB_NWIN,nc);}

void lydaq::MbmdccManager::setCalibRegister(uint32_t nc){this->writeRegister(lydaq::mbmdcc::Message::Register::CALIB_CTRL,nc);}

uint32_t lydaq::MbmdccManager::hardReset(){return this->readRegister(lydaq::mbmdcc::Message::Register::RESET_FE);}
void lydaq::MbmdccManager::setHardReset(uint32_t nc){this->writeRegister(lydaq::mbmdcc::Message::Register::RESET_FE,nc);}

void lydaq::MbmdccManager::setSpillRegister(uint32_t nc){this->writeRegister(lydaq::mbmdcc::Message::Register::WIN_CTRL,nc);}
uint32_t lydaq::MbmdccManager::spillRegister(){return this->readRegister(lydaq::mbmdcc::Message::Register::WIN_CTRL);}
void lydaq::MbmdccManager::useSPSSpill(bool t)
{
  uint32_t reg=this->spillRegister();
  if (t)
    this->setSpillRegister(reg|1);
  else
    this->setSpillRegister(reg&~1);
}
void lydaq::MbmdccManager::useTrigExt(bool t)
{
  uint32_t reg=this->spillRegister();
  if (t)
    this->setSpillRegister(reg|2);
  else
    this->setSpillRegister(reg&~2);
}

void lydaq::MbmdccManager::setTriggerDelay(uint32_t nc){this->writeRegister(lydaq::mbmdcc::Message::Register::TRG_EXT_DELAY,nc);}
uint32_t lydaq::MbmdccManager::triggerDelay(){return this->readRegister(lydaq::mbmdcc::Message::Register::TRG_EXT_DELAY);}
void lydaq::MbmdccManager::setTriggerBusy(uint32_t nc){this->writeRegister(lydaq::mbmdcc::Message::Register::TRG_EXT_LEN,nc);}
uint32_t lydaq::MbmdccManager::triggerBusy(){return this->readRegister(lydaq::mbmdcc::Message::Register::TRG_EXT_LEN);}

void lydaq::MbmdccManager::setExternalTrigger(uint32_t nc){this->writeRegister(lydaq::mbmdcc::Message::Register::EN_BUSY_TRG,nc);}
uint32_t lydaq::MbmdccManager::externalTrigger(){return this->readRegister(lydaq::mbmdcc::Message::Register::EN_BUSY_TRG);}

void lydaq::MbmdccManager::reloadCalibCount(){

  this->maskTrigger();
  this->writeRegister(lydaq::mbmdcc::Message::Register::WIN_CTRL,0x8);
  this->writeRegister(lydaq::mbmdcc::Message::Register::CALIB_CTRL,0x4);
  // sleep(1);
  // this->writeRegister(0x8,0x0);
  // sleep(1);
  this->unmaskTrigger();
  this->calibOn();


}




void lydaq::MbmdccManager::resetTDC(uint8_t b){this->writeRegister(lydaq::mbmdcc::Message::Register::RESET_FE,b);}
uint32_t lydaq::MbmdccManager::busyCount(uint8_t b){return this->readRegister(lydaq::mbmdcc::Message::Register::BUSY_0+(b&0xF));}



uint32_t lydaq::MbmdccManager::readRegister(uint32_t adr)
{
  uint32_t rc;
  for (auto x:_mpi->boards())
    rc=x.second->reg()->readRegister(adr);
  return rc;
}

void lydaq::MbmdccManager::writeRegister(uint32_t adr,uint32_t val)
{
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(adr,val);

}


void lydaq::MbmdccManager::c_readreg(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Pulse called ");
  response["STATUS"]="DONE";

  
  uint32_t adr=atol(request.get("adr","0").c_str());

  Json::Value r;
  for (auto x:_mpi->boards())
    {    
      uint32_t value=x.second->reg()->readRegister(adr);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Read reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<value);
      r[x.second->ipAddress()]=value;
    }
  

  response["READREG"]=r;
}
void lydaq::MbmdccManager::c_writereg(Mongoose::Request &request, Mongoose::JsonResponse &response)
{

  response["STATUS"]="DONE";

  
  uint32_t adr=atol(request.get("adr","0").c_str());
  uint32_t val=atol(request.get("val","0").c_str());

  Json::Value r;
  for (auto x:_mpi->boards())
    {    
      x.second->reg()->writeRegister(adr,val);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Write reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<val);
      r[x.second->ipAddress()]=val;
    }
  

  response["WRITEREG"]=r;
}

void lydaq::MbmdccManager::c_pause(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Pause called ");

  this->maskTrigger();
  response["STATUS"]="DONE";
}
void lydaq::MbmdccManager::c_resume(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
    LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Resume called ");

  this->unmaskTrigger();
  response["STATUS"]="DONE";
}
void lydaq::MbmdccManager::c_calibon(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Calib On called ");
  this->calibOn();
  response["STATUS"]="DONE";
}
void lydaq::MbmdccManager::c_caliboff(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Calib Off called ");
  this->calibOff();
  response["STATUS"]="DONE";
}
void lydaq::MbmdccManager::c_reloadcalib(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Calib reload called ");
  this->reloadCalibCount();
  response["STATUS"]="DONE";
}

void lydaq::MbmdccManager::c_setcalibcount(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Calib count called ");
   uint32_t nc=atol(request.get("nclock","5000000").c_str());
  this->setCalibCount(nc);

  response["STATUS"]="DONE";
  response["NCLOCK"]=nc;

} 

void lydaq::MbmdccManager::c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
   LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" RESET called ");
  this->resetCounter();
  response["STATUS"]="DONE";
}


void lydaq::MbmdccManager::c_spillon(Mongoose::Request &request, Mongoose::JsonResponse &response)
{

  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Spill ON called ");

  uint32_t nc=atol(request.get("nclock","50").c_str());
  this->setSpillOn(nc);

  response["STATUS"]="DONE";
  response["NCLOCK"]=nc;

} 
void lydaq::MbmdccManager::c_spilloff(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Spill Off called ");

  uint32_t nc=atol(request.get("nclock","5000").c_str());
  this->setSpillOff(nc);

  response["STATUS"]="DONE";
  response["NCLOCK"]=nc;

} 
void lydaq::MbmdccManager::c_resettdc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Reset TDC called ");

  uint32_t nc=atol(request.get("value","0").c_str());
  this->resetTDC(nc&0xF);

  response["STATUS"]="DONE";
  //response["NCLOCK"]=nc;

} 

void lydaq::MbmdccManager::c_channelon(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" beam on time called ");

  uint32_t nc=atol(request.get("value","1023").c_str());
  this->setChannels(nc);

  response["STATUS"]="DONE";
  response["NCLOCK"]=nc;

}

void lydaq::MbmdccManager::c_sethardreset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Hard reset called ");

  uint32_t nc=atol(request.get("value","0").c_str());
  this->setHardReset(nc);

  response["STATUS"]="DONE";
  response["VALUE"]=nc;

}

void lydaq::MbmdccManager::c_setspillregister(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Spill register called ");

  uint32_t nc=atol(request.get("value","0").c_str());
  this->setSpillRegister(nc);

  response["STATUS"]="DONE";
  response["VALUE"]=nc;

}
void lydaq::MbmdccManager::c_setexternaltrigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Spill register called ");

  uint32_t nc=atol(request.get("value","0").c_str());
  this->setExternalTrigger(nc);

  response["STATUS"]="DONE";
  response["VALUE"]=nc;

}

void lydaq::MbmdccManager::c_setregister(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Set register called ");

  uint32_t adr=atol(request.get("address","2").c_str());
  uint32_t val=atol(request.get("value","0").c_str());
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Set register called with "<<adr<<" => "<<val);

  this->writeRegister(adr,val);

  response["STATUS"]="DONE";
  response["VALUE"]=this->readRegister(adr);

}
void lydaq::MbmdccManager::c_getregister(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Get register called ");

  uint32_t adr=atol(request.get("address","2").c_str());
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Get register called with "<<adr);

  response["STATUS"]="DONE";
  response["VALUE"]=this->readRegister(adr);
  //std::cout<<response<<std::endl;
}
void lydaq::MbmdccManager::c_setcalibregister(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Calib register called ");

  uint32_t nc=atol(request.get("value","0").c_str());
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Calib register called "<<nc);
  this->setCalibRegister(nc);

  response["STATUS"]="DONE";
  response["VALUE"]=nc;

}

void lydaq::MbmdccManager::c_settrigext(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Trig ext setting called ");

  uint32_t delay=atol(request.get("delay","20").c_str());
  uint32_t busy=atol(request.get("busy","20").c_str());
  this->setTriggerDelay(delay);
  this->setTriggerBusy(busy);

  response["STATUS"]="DONE";
  response["DELAY"]=delay;
  response["BUSY"]=busy;

} 

void lydaq::MbmdccManager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Status called ");

  Json::Value rc;
  rc["version"]=this->version();
  rc["id"]=this->id();
  rc["mask"]=this->mask();
  rc["hard"]=this->hardReset();
  rc["spill"]=this->spillCount();
  rc["busy1"]=this->busyCount(1);
  rc["busy2"]=this->busyCount(2);
  rc["busy3"]=this->busyCount(3);
  rc["busy4"]=this->busyCount(4);
  rc["busy5"]=this->busyCount(5);
  rc["busy6"]=this->busyCount(6);
  rc["busy7"]=this->busyCount(7);
  rc["busy8"]=this->busyCount(8);
  rc["busy9"]=this->busyCount(9);
  rc["busy10"]=this->busyCount(10);
  rc["spillon"]=this->spillOn();
  rc["spilloff"]=this->spillOff();
  rc["channels"]=this->Channels();
  rc["calib"]=this->calibCount();
  rc["spillreg"]=this->spillRegister();
  rc["trigdelay"]=this->triggerDelay();
  rc["external"]=this->externalTrigger();
  response["COUNTERS"]=rc;
  response["STATUS"]="DONE";


} 


