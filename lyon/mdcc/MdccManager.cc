
#include "MdccManager.hh"


using namespace mdcc;

void MdccManager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," CMD: Opening");

  std::string device;
  if (utils::isMember(params(),"device"))
    device=params()["device"].as_string();

  this->doOpen(device);
  
  if (utils::isMember(params(),"spillon") && _mdcc!=NULL)
    {
      _mdcc->setSpillOn(params()["spillon"].as_integer()); 
    }
  if (utils::isMember(params(),"spilloff") && _mdcc!=NULL)
    {
      _mdcc->setSpillOff(params()["spilloff"].as_integer()); 
    }
  if (utils::isMember(params(),"spillregister") && _mdcc!=NULL)
    {
      _mdcc->setSpillRegister(params()["spillregister"].as_integer()); 
    }
  
  _mdcc->maskTrigger();
  _mdcc->resetCounter();
  par["status"]=json::value::string(U("Opened"));
  Reply(status_codes::OK,par);  
    
}
void MdccManager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," CMD: Closing");
  if (_mdcc==NULL)
    {
      PMF_ERROR(_logMdcc,"Please open MDC01 first");
      return;
    }
  _mdcc->close();
  //delete _mdcc;
  _mdcc=NULL;
  par["status"]=json::value::string(U("Closed"));
  Reply(status_codes::OK,par);  
    
}

void MdccManager::c_pause(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Pause called ");

  if (_mdcc==NULL)
    {
      PMF_ERROR(_logMdcc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _mdcc->maskTrigger();
  par["STATUS"]=web::json::value::string(U("DONE"));
  
  Reply(status_codes::OK,par);  

  
}
void MdccManager::c_resume(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Resume called ");

  if (_mdcc==NULL)
    {
      PMF_ERROR(_logMdcc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _mdcc->unmaskTrigger();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}
void MdccManager::c_ecalpause(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Ecal Pause called ");
  if (_mdcc==NULL)
    {
      PMF_ERROR(_logMdcc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _mdcc->maskEcal();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}
void MdccManager::c_ecalresume(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Ecal Resume called ");
  if (_mdcc==NULL)
    {
      PMF_ERROR(_logMdcc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _mdcc->unmaskEcal();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}
void MdccManager::c_calibon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Calib On called ");
  if (_mdcc==NULL)
    {
      PMF_ERROR(_logMdcc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _mdcc->calibOn();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}
void MdccManager::c_caliboff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Calib Off called ");
  if (_mdcc==NULL)
    {
      PMF_ERROR(_logMdcc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _mdcc->calibOff();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}
void MdccManager::c_reloadcalib(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Calib reload called ");
  if (_mdcc==NULL)
    {
      PMF_ERROR(_logMdcc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _mdcc->reloadCalibCount();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}

void MdccManager::c_setcalibcount(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Calib count called ");
  if (_mdcc==NULL)
    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));
      Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"nclock",5000000);
  _mdcc->setCalibCount(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["NCLOCK"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
} 

void MdccManager::c_reset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," RESET called ");
  if (_mdcc==NULL)
    {
      PMF_ERROR(_logMdcc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _mdcc->resetCounter();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}

void MdccManager::c_readreg(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc,"Read Register called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t adr=utils::queryIntValue(m,"address",2);
  uint32_t val =_mdcc->readRegister(adr);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["ADDRESS"]=web::json::value::number(adr);
  par["VALUE"]=web::json::value::number(val);
  Reply(status_codes::OK,par);  
} 
void MdccManager::c_writereg(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Write Register called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t adr=utils::queryIntValue(m,"address",2);
  uint32_t value=utils::queryIntValue(m,"value",1234);
  _mdcc->writeRegister(adr,value);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["ADDRESS"]=web::json::value::number(adr);
  par["VALUE"]=web::json::value::number(value);
  Reply(status_codes::OK,par);  
} 
void MdccManager::c_spillon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Spill ON called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"nclock",50);
  _mdcc->setSpillOn(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["NCLOCK"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
} 
void MdccManager::c_spilloff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Spill Off called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"nclock",5000);
  _mdcc->setSpillOff(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["NCLOCK"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  

} 
void MdccManager::c_resettdc(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Reset TDC called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"value",0);
  _mdcc->resetTDC(nc&0xF);

  par["STATUS"]=web::json::value::string(U("DONE"));
  //par["NCLOCK"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
} 

void MdccManager::c_beamon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," beam on time called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"nclock",5000000);
  _mdcc->setBeam(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["NCLOCK"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
}

void MdccManager::c_sethardreset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Hard reset called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"value",0);
  _mdcc->setHardReset(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
}

void MdccManager::c_setspillregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc,"Spill register called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);   return;}
  uint32_t nc=utils::queryIntValue(m,"value",0);
  _mdcc->setSpillRegister(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
}
void MdccManager::c_setexternaltrigger(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc,"Spill register called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"value",0);
  _mdcc->setExternalTrigger(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
}

void MdccManager::c_setregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc,"Set register called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t adr=utils::queryIntValue(m,"address",2);
  uint32_t val=utils::queryIntValue(m,"value",0);
  PMF_INFO(_logMdcc,"Set register called with "<<adr<<" => "<<val);

  _mdcc->writeRegister(adr,val);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(_mdcc->readRegister(adr));
  Reply(status_codes::OK,par);  
}
void MdccManager::c_getregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc,"Get register called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t adr=utils::queryIntValue(m,"address",2);
  PMF_INFO(_logMdcc,"Get register called with "<<adr);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(_mdcc->readRegister(adr));
  //std::cout<<response<<std::endl;
  Reply(status_codes::OK,par);  
}
void MdccManager::c_setcalibregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc,"Calib register called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"value",0);
  PMF_INFO(_logMdcc,"Calib register called "<<nc);
  _mdcc->setCalibRegister(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
}

void MdccManager::c_settrigext(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Trig ext setting called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t delay=utils::queryIntValue(m,"delay",20);
  uint32_t busy=utils::queryIntValue(m,"busy",20);
  _mdcc->setTriggerDelay(delay);
  _mdcc->setTriggerBusy(busy);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["DELAY"]=web::json::value::number(delay);
  par["BUSY"]=web::json::value::number(busy);
  Reply(status_codes::OK,par);  
}

void MdccManager::c_status(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMdcc," Status called ");
  if (_mdcc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Mdcc created"));        Reply(status_codes::OK,par);  return;}
  web::json::value rc;
  rc["version"]=json::value::number(_mdcc->version());
  rc["id"]=json::value::number(_mdcc->id());
  rc["mask"]=json::value::number(_mdcc->mask());
  rc["hard"]=json::value::number(_mdcc->hardReset());
  rc["spill"]=json::value::number(_mdcc->spillCount());
  rc["busy1"]=json::value::number(_mdcc->busyCount(1));
  rc["busy2"]=json::value::number(_mdcc->busyCount(2));
  rc["busy3"]=json::value::number(_mdcc->busyCount(3));
  rc["busy4"]=json::value::number(_mdcc->busyCount(4));
  rc["busy5"]=json::value::number(_mdcc->busyCount(5));
  rc["busy6"]=json::value::number(_mdcc->busyCount(6));
  rc["busy7"]=json::value::number(_mdcc->busyCount(7));
  rc["busy8"]=json::value::number(_mdcc->busyCount(8));
  rc["busy9"]=json::value::number(_mdcc->busyCount(9));
  rc["busy10"]=json::value::number(_mdcc->busyCount(10));
  rc["busy11"]=json::value::number(_mdcc->busyCount(11));
  rc["busy12"]=json::value::number(_mdcc->busyCount(12));
  rc["spillon"]=json::value::number(_mdcc->spillOn());
  rc["spilloff"]=json::value::number(_mdcc->spillOff());
  rc["ecalmask"]=json::value::number(_mdcc->ecalmask());
  rc["beam"]=json::value::number(_mdcc->beam());
  rc["calib"]=json::value::number(_mdcc->calibCount());
  rc["spillreg"]=json::value::number(_mdcc->spillRegister());
  rc["trigdelay"]=json::value::number(_mdcc->triggerDelay());
  rc["external"]=json::value::number(_mdcc->externalTrigger());
  par["COUNTERS"]=rc;
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  

} 



MdccManager::MdccManager() : _mdcc(NULL)
{;}
void MdccManager::end()
{

  if (_mdcc!=NULL)
    _mdcc->close();}
void MdccManager::initialise()			   
{
  
  // Register state
  this->addState("OPENED");


  this->addTransition("INITIALISE","CREATED","OPENED",std::bind(&MdccManager::fsm_initialise, this,std::placeholders::_1));
  this->addTransition("DESTROY","OPENED","CREATED",std::bind(&MdccManager::destroy, this,std::placeholders::_1));


  this->addCommand("PAUSE",std::bind(&MdccManager::c_pause,this,std::placeholders::_1));
  this->addCommand("RESUME",std::bind(&MdccManager::c_resume,this,std::placeholders::_1));
  this->addCommand("RESET",std::bind(&MdccManager::c_reset,this,std::placeholders::_1));
  this->addCommand("ECALPAUSE",std::bind(&MdccManager::c_ecalpause,this,std::placeholders::_1));
  this->addCommand("ECALRESUME",std::bind(&MdccManager::c_ecalresume,this,std::placeholders::_1));
  this->addCommand("WRITEREG",std::bind(&MdccManager::c_writereg,this,std::placeholders::_1));
  this->addCommand("READREG",std::bind(&MdccManager::c_readreg,this,std::placeholders::_1));
  this->addCommand("STATUS",std::bind(&MdccManager::c_status,this,std::placeholders::_1));
  this->addCommand("SPILLON",std::bind(&MdccManager::c_spillon,this,std::placeholders::_1));
  this->addCommand("SPILLOFF",std::bind(&MdccManager::c_spilloff,this,std::placeholders::_1));
  this->addCommand("BEAMON",std::bind(&MdccManager::c_beamon,this,std::placeholders::_1));

  this->addCommand("RESETTDC",std::bind(&MdccManager::c_resettdc,this,std::placeholders::_1));
  this->addCommand("CALIBON",std::bind(&MdccManager::c_calibon,this,std::placeholders::_1));
  this->addCommand("CALIBOFF",std::bind(&MdccManager::c_caliboff,this,std::placeholders::_1));
  this->addCommand("RELOADCALIB",std::bind(&MdccManager::c_reloadcalib,this,std::placeholders::_1));
  this->addCommand("SETCALIBCOUNT",std::bind(&MdccManager::c_setcalibcount,this,std::placeholders::_1));
  this->addCommand("SETSPILLREGISTER",std::bind(&MdccManager::c_setspillregister,this,std::placeholders::_1));
  this->addCommand("SETCALIBREGISTER",std::bind(&MdccManager::c_setcalibregister,this,std::placeholders::_1));
  this->addCommand("SETHARDRESET",std::bind(&MdccManager::c_sethardreset,this,std::placeholders::_1));
  this->addCommand("SETTRIGEXT",std::bind(&MdccManager::c_settrigext,this,std::placeholders::_1));

  this->addCommand("SETREG",std::bind(&MdccManager::c_setregister,this,std::placeholders::_1));
  this->addCommand("GETREG",std::bind(&MdccManager::c_getregister,this,std::placeholders::_1));

  this->addCommand("SETEXTERNAL",std::bind(&MdccManager::c_setexternaltrigger,this,std::placeholders::_1));
 
  
}


void MdccManager::doOpen(std::string s)
{
  //  std::cout<<"calling open "<<std::endl;
  PMF_INFO(_logMdcc," Opening "<<s);
  if (_mdcc!=NULL)
    delete _mdcc;
  _mdcc= new MdccHandler(s);
  _mdcc->open();
  //std::cout<<" Open Ptr "<<_mdcc<<std::endl;
}
extern "C" 
{
  // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
  {
    fprintf(stderr,"Creating MdccManager \n");
    return (new  MdccManager);
  }
  // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
  // to it.  This isn't a very safe function, since there's no 
  // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
  {
    delete obj;
  }
}
