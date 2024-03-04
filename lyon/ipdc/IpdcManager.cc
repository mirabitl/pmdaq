
#include "IpdcManager.hh"


using namespace ipdc;

void IpdcManager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," CMD: Opening");

  std::string device;
  if (utils::isMember(params(),"device"))
    device=params()["device"].as_string();

  this->doOpen(device);
  
  if (utils::isMember(params(),"spillon") && _ipdc!=NULL)
    {
      _ipdc->setSpillOn(params()["spillon"].as_integer()); 
    }
  if (utils::isMember(params(),"spilloff") && _ipdc!=NULL)
    {
      _ipdc->setSpillOff(params()["spilloff"].as_integer()); 
    }
  if (utils::isMember(params(),"spillregister") && _ipdc!=NULL)
    {
      _ipdc->setSpillRegister(params()["spillregister"].as_integer()); 
    }
  
  _ipdc->maskTrigger();
  _ipdc->resetCounter();
  par["status"]=json::value::string(U("Opened"));
  Reply(status_codes::OK,par);  
    
}
void IpdcManager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," CMD: Closing");
  if (_ipdc==NULL)
    {
      PMF_ERROR(_logIpdc,"Please open MDC01 first");
      return;
    }
  _ipdc->close();
  //delete _ipdc;
  _ipdc=NULL;
  par["status"]=json::value::string(U("Closed"));
  Reply(status_codes::OK,par);  
    
}

void IpdcManager::c_pause(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Pause called ");

  if (_ipdc==NULL)
    {
      PMF_ERROR(_logIpdc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _ipdc->maskTrigger();
  par["STATUS"]=web::json::value::string(U("DONE"));
  
  Reply(status_codes::OK,par);  

  
}
void IpdcManager::c_resume(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Resume called ");

  if (_ipdc==NULL)
    {
      PMF_ERROR(_logIpdc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _ipdc->unmaskTrigger();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}
void IpdcManager::c_calibon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Calib On called ");
  if (_ipdc==NULL)
    {
      PMF_ERROR(_logIpdc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _ipdc->calibOn();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}
void IpdcManager::c_caliboff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Calib Off called ");
  if (_ipdc==NULL)
    {
      PMF_ERROR(_logIpdc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _ipdc->calibOff();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}
void IpdcManager::c_reloadcalib(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Calib reload called ");
  if (_ipdc==NULL)
    {
      PMF_ERROR(_logIpdc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _ipdc->reloadCalibCount();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}

void IpdcManager::c_setcalibcount(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Calib count called ");
  if (_ipdc==NULL)
    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));
      Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"nclock",5000000);
  _ipdc->setCalibCount(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["NCLOCK"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
} 

void IpdcManager::c_lemo_mask(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," LEMO MASK called ");
  if (_ipdc==NULL)
    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));
      Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"value",0);
  _ipdc->setLemoMask(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["MASK"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
} 

void IpdcManager::c_reset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," RESET called ");
  if (_ipdc==NULL)
    {
      PMF_ERROR(_logIpdc,"Please open MDC01 first");
      par["STATUS"]=web::json::value::string(U("Please open MDC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  _ipdc->resetCounter();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}

void IpdcManager::c_readreg(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc,"Read Register called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t adr=utils::queryIntValue(m,"address",2);
  uint32_t val =_ipdc->readRegister(adr);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["ADDRESS"]=web::json::value::number(adr);
  par["VALUE"]=web::json::value::number(val);
  Reply(status_codes::OK,par);  
} 
void IpdcManager::c_writereg(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Write Register called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t adr=utils::queryIntValue(m,"address",2);
  uint32_t value=utils::queryIntValue(m,"value",1234);
  _ipdc->writeRegister(adr,value);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["ADDRESS"]=web::json::value::number(adr);
  par["VALUE"]=web::json::value::number(value);
  Reply(status_codes::OK,par);  
} 
void IpdcManager::c_spillon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Spill ON called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"nclock",50);
  _ipdc->setSpillOn(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["NCLOCK"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
} 
void IpdcManager::c_spilloff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Spill Off called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"nclock",5000);
  _ipdc->setSpillOff(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["NCLOCK"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  

} 
void IpdcManager::c_resettdc(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Reset TDC called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"value",0);
  _ipdc->resetTDC(nc&0xF);

  par["STATUS"]=web::json::value::string(U("DONE"));
  //par["NCLOCK"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
} 


void IpdcManager::c_sethardreset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Hard reset called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"value",0);
  _ipdc->setHardReset(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
}

void IpdcManager::c_setspillregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc,"Spill register called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);   return;}
  uint32_t nc=utils::queryIntValue(m,"value",0);
  _ipdc->setSpillRegister(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
}
void IpdcManager::c_setexternaltrigger(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc,"Spill register called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"value",0);
  _ipdc->setExternalTrigger(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
}

void IpdcManager::c_setregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc,"Set register called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t adr=utils::queryIntValue(m,"address",2);
  uint32_t val=utils::queryIntValue(m,"value",0);
  PMF_INFO(_logIpdc,"Set register called with "<<adr<<" => "<<val);

  _ipdc->writeRegister(adr,val);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(_ipdc->readRegister(adr));
  Reply(status_codes::OK,par);  
}
void IpdcManager::c_getregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc,"Get register called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t adr=utils::queryIntValue(m,"address",2);
  PMF_INFO(_logIpdc,"Get register called with "<<adr);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(_ipdc->readRegister(adr));
  //std::cout<<response<<std::endl;
  Reply(status_codes::OK,par);  
}
void IpdcManager::c_setcalibregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc,"Calib register called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t nc=utils::queryIntValue(m,"value",0);
  PMF_INFO(_logIpdc,"Calib register called "<<nc);
  _ipdc->setCalibRegister(nc);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["VALUE"]=web::json::value::number(nc);
  Reply(status_codes::OK,par);  
}

void IpdcManager::c_settrigext(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Trig ext setting called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t delay=utils::queryIntValue(m,"delay",20);
  uint32_t busy=utils::queryIntValue(m,"busy",20);
  _ipdc->setTriggerDelay(delay);
  _ipdc->setTriggerBusy(busy);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["DELAY"]=web::json::value::number(delay);
  par["BUSY"]=web::json::value::number(busy);
  Reply(status_codes::OK,par);  
}
void IpdcManager::c_enable(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Busy enable setting called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  uint32_t mask=utils::queryIntValue(m,"value",1);

  _ipdc->setBusyEnable(mask);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["MASK"]=web::json::value::number(mask);
  Reply(status_codes::OK,par);  
}

void IpdcManager::c_status(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logIpdc," Status called ");
  if (_ipdc==NULL)    {par["STATUS"]=web::json::value::string(U("NO Ipdc created"));        Reply(status_codes::OK,par);  return;}
  web::json::value rc;
  uint32_t chen=_ipdc->busyEnable();
  rc["version"]=json::value::number(_ipdc->version());
  rc["id"]=json::value::number(_ipdc->id());
  rc["mask"]=json::value::number(_ipdc->mask());
  rc["hard"]=json::value::number(_ipdc->hardReset());
  rc["enable"]=json::value::number(_ipdc->busyEnable());
  rc["spill"]=json::value::number(_ipdc->spillCount());
  for (int i=0;i<16;i++)
    {
      if ((chen>>i)&1)
	{
	  std::stringstream sb;
	  sb<<"busy"<<i;
	  rc[sb.str()]=json::value::number(_ipdc->busyCount(i));
	}
    }
  rc["spillon"]=json::value::number(_ipdc->spillOn());
  rc["spilloff"]=json::value::number(_ipdc->spillOff());
  rc["calib"]=json::value::number(_ipdc->calibCount());
  rc["spillreg"]=json::value::number(_ipdc->spillRegister());
  rc["trigdelay"]=json::value::number(_ipdc->triggerDelay());
  rc["external"]=json::value::number(_ipdc->externalTrigger());
  par["COUNTERS"]=rc;
  par["STATUS"]=web::json::value::string(U("DONE"));
  mqtt_publish("status",par);
  Reply(status_codes::OK,par);  

} 



IpdcManager::IpdcManager() : _ipdc(NULL)
{;}
void IpdcManager::end()
{

  if (_ipdc!=NULL)
    _ipdc->close();}
void IpdcManager::initialise()			   
{
  
  // Register state
  this->addState("OPENED");


  this->addTransition("INITIALISE","CREATED","OPENED",std::bind(&IpdcManager::fsm_initialise, this,std::placeholders::_1));
  this->addTransition("DESTROY","OPENED","CREATED",std::bind(&IpdcManager::destroy, this,std::placeholders::_1));


  this->addCommand("PAUSE",std::bind(&IpdcManager::c_pause,this,std::placeholders::_1));
  this->addCommand("RESUME",std::bind(&IpdcManager::c_resume,this,std::placeholders::_1));
  this->addCommand("RESET",std::bind(&IpdcManager::c_reset,this,std::placeholders::_1));
  this->addCommand("WRITEREG",std::bind(&IpdcManager::c_writereg,this,std::placeholders::_1));
  this->addCommand("READREG",std::bind(&IpdcManager::c_readreg,this,std::placeholders::_1));
  this->addCommand("STATUS",std::bind(&IpdcManager::c_status,this,std::placeholders::_1));
  this->addCommand("SPILLON",std::bind(&IpdcManager::c_spillon,this,std::placeholders::_1));
  this->addCommand("SPILLOFF",std::bind(&IpdcManager::c_spilloff,this,std::placeholders::_1));

  this->addCommand("RESETTDC",std::bind(&IpdcManager::c_resettdc,this,std::placeholders::_1));
  this->addCommand("CALIBON",std::bind(&IpdcManager::c_calibon,this,std::placeholders::_1));
  this->addCommand("CALIBOFF",std::bind(&IpdcManager::c_caliboff,this,std::placeholders::_1));
  this->addCommand("RELOADCALIB",std::bind(&IpdcManager::c_reloadcalib,this,std::placeholders::_1));
  this->addCommand("SETCALIBCOUNT",std::bind(&IpdcManager::c_setcalibcount,this,std::placeholders::_1));
  this->addCommand("SETSPILLREGISTER",std::bind(&IpdcManager::c_setspillregister,this,std::placeholders::_1));
  this->addCommand("SETCALIBREGISTER",std::bind(&IpdcManager::c_setcalibregister,this,std::placeholders::_1));
  this->addCommand("SETHARDRESET",std::bind(&IpdcManager::c_sethardreset,this,std::placeholders::_1));
  this->addCommand("SETTRIGEXT",std::bind(&IpdcManager::c_settrigext,this,std::placeholders::_1));

  this->addCommand("SETREG",std::bind(&IpdcManager::c_setregister,this,std::placeholders::_1));
  this->addCommand("GETREG",std::bind(&IpdcManager::c_getregister,this,std::placeholders::_1));

  this->addCommand("SETEXTERNAL",std::bind(&IpdcManager::c_setexternaltrigger,this,std::placeholders::_1));
  this->addCommand("ENABLE",std::bind(&IpdcManager::c_enable,this,std::placeholders::_1));
  this->addCommand("LEMOMASK",std::bind(&IpdcManager::c_lemo_mask,this,std::placeholders::_1));

  
}


void IpdcManager::doOpen(std::string s)
{
  //  std::cout<<"calling open "<<std::endl;
  PMF_INFO(_logIpdc," Opening "<<s);
  if (_ipdc!=NULL)
    delete _ipdc;
  _ipdc= new IpdcHandler(s);
  _ipdc->open();
  //std::cout<<" Open Ptr "<<_ipdc<<std::endl;
}
extern "C" 
{
  // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
  {
    fprintf(stderr,"Creating IpdcManager \n");
    return (new  IpdcManager);
  }
  // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
  // to it.  This isn't a very safe function, since there's no 
  // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
  {
    delete obj;
  }
}
