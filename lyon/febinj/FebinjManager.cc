#include "FebinjManager.hh"


void FebinjManager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logFebinj, " CMD: CONFIGURE" );

  std::cout << "calling open " << std::endl;

  //Settings

  uint32_t HR = 0, LR = 0, SOURCE = 0, NBT = 0, DELAY = 0, DURATION = 0, PH = 0;
  if (utils::isMember(params(),"HR"))
    {
      HR = params()["HR"].as_integer();
      _inj->setMask(HR, 1);
    }

  if (utils::isMember(params(),"LR"))
  {
    LR = params()["LR"].as_integer();
    _inj->setMask(LR, 0);
  }

  if (utils::isMember(params(),"TriggerSource"))
  {
    SOURCE = params()["TriggerSource"].as_integer();
    _inj->setTriggerSource(SOURCE);
  }

  if (utils::isMember(params(),"TriggerMax"))
  {
    NBT = params()["TriggerMax"].as_integer();
  _inj->setNumberOfTrigger(NBT);
  }
  if (utils::isMember(params(),"Delay"))
  {
    DELAY = params()["Delay"].as_integer();
  _inj->setDelay(DELAY);
  }
  if (utils::isMember(params(),"Duration"))
  {
    DURATION = params()["Duration"].as_integer();
  _inj->setDuration(DURATION);
  }
  if (utils::isMember(params(),"PulseHeight"))
  {
    PH = params()["PulseHeight"].as_integer();
  _inj->setPulseHeight(PH);
  }
  par["STATUS"]=web::json::value::String(U("done"));
  Reply(status_codes::OK,par);  
}
void FebinjManager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logFebinj, " CMD: DESTROY " );
  if (_inj == NULL)
  {
    PMF_ERROR(_logFebinj, "No FebInj opened");
    par["STATUS"]=web::json::value::String(U("No febinj opened"));
    Reply(status_codes::OK,par);  

    return;
  }
  par["STATUS"]=web::json::value::String(U("done"));
  Reply(status_codes::OK,par);  
}
void FebinjManager::c_set_mask(http_request m)
{
  auto par = json::value::object();
  if (_inj == NULL)
  {
    PMF_ERROR(_logFebinj, "Please open FebInj first");
    par["STATUS"]=web::json::value::string(U("Please open FebInj first"));
    Reply(status_codes::OK,par);  
    return;
  }
  uint32_t mask = utils::queryIntValue(m,"mask",0);
  uint32_t side = utils::queryIntValue(m,"side",0);
  _inj->setMask(mask, side);
  par["STATUS"]=web::json::value::string(U("done"));
  Reply(status_codes::OK,par);  
}
void FebinjManager::c_set_trigger_source(http_request m)
{
  auto par = json::value::object();
  if (_inj == NULL)
  {
    PMF_ERROR(_logFebinj, "Please open FebInj first");
    par["STATUS"]=web::json::value::string(U("Please open FebInj first"));
    Reply(status_codes::OK,par);  
    return;
  }
  uint32_t value = utils::queryIntValue(m,"value",0);
  _inj->setTriggerSource(value);
  par["STATUS"]=web::json::value::string(U("done"));
  Reply(status_codes::OK,par);  
}
void FebinjManager::c_pause_external_trigger(http_request m)
{
  auto par = json::value::object();
  if (_inj == NULL)
  {
    PMF_ERROR(_logFebinj, "Please open FebInj first");
    par["STATUS"]=web::json::value::string(U("Please open FebInj first"));
    Reply(status_codes::OK,par);  
    return;
  }

  _inj->pauseExternalTrigger();
  par["STATUS"]=web::json::value::string(U("done"));
  Reply(status_codes::OK,par);  
}
void FebinjManager::c_resume_external_trigger(http_request m)
{
  auto par = json::value::object();
  if (_inj == NULL)
  {
    PMF_ERROR(_logFebinj, "Please open FebInj first");
    par["STATUS"]=web::json::value::string(U("Please open FebInj first"));
    Reply(status_codes::OK,par);  
    return;
  }

  _inj->resumeExternalTrigger();
  par["STATUS"]=web::json::value::string(U("done"));
  Reply(status_codes::OK,par);  
}
void FebinjManager::c_software_trigger(http_request m)
{
  auto par = json::value::object();
  if (_inj == NULL)
  {
    PMF_ERROR(_logFebinj, "Please open FebInj first");
    par["STATUS"]=web::json::value::string(U("Please open FebInj first"));
    Reply(status_codes::OK,par);  
    return;
  }

  _inj->softwareTrigger();
  par["STATUS"]=web::json::value::string(U("done"));
  Reply(status_codes::OK,par);  
}
void FebinjManager::c_internal_trigger(http_request m)
{
  auto par = json::value::object();
  if (_inj == NULL)
  {
    PMF_ERROR(_logFebinj, "Please open FebInj first");
    par["STATUS"]=web::json::value::string(U("Please open FebInj first"));
    Reply(status_codes::OK,par);  
    return;
  }

  _inj->internalTrigger();
  par["STATUS"]=web::json::value::string(U("done"));
  Reply(status_codes::OK,par);  
}
void FebinjManager::c_set_number_of_trigger(http_request m)
{
  auto par = json::value::object();
  if (_inj == NULL)
  {
    PMF_ERROR(_logFebinj, "Please open FebInj first");
    par["STATUS"]=web::json::value::string(U("Please open FebInj first"));
    Reply(status_codes::OK,par);  
    return;
  }
  uint32_t value = utils::queryIntValue(m,"value",0);
  _inj->setNumberOfTrigger(value);
  par["STATUS"]=web::json::value::string(U("done"));
  Reply(status_codes::OK,par);  
}
void FebinjManager::c_set_delay(http_request m)
{
  auto par = json::value::object();
  if (_inj == NULL)
  {
    PMF_ERROR(_logFebinj, "Please open FebInj first");
    par["STATUS"]=web::json::value::string(U("Please open FebInj first"));
    Reply(status_codes::OK,par);  
    return;
  }
  uint32_t value = utils::queryIntValue(m,"value",0);
  _inj->setDelay(value);
  par["STATUS"]=web::json::value::string(U("done"));
  Reply(status_codes::OK,par);  
}
void FebinjManager::c_set_duration(http_request m)
{
  auto par = json::value::object();
  if (_inj == NULL)
  {
    PMF_ERROR(_logFebinj, "Please open FebInj first");
    par["STATUS"]=web::json::value::string(U("Please open FebInj first"));
    Reply(status_codes::OK,par);  
    return;
  }
  uint32_t value = utils::queryIntValue(m,"value",0);
  _inj->setDuration(value);
  par["STATUS"]=web::json::value::string(U("done"));
  Reply(status_codes::OK,par);  
}
void FebinjManager::c_set_pulse_height(http_request m)
{
  auto par = json::value::object();
  if (_inj == NULL)
  {
    PMF_ERROR(_logFebinj, "Please open FebInj first");
    par["STATUS"]=web::json::value::string(U("Please open FebInj first"));
    Reply(status_codes::OK,par);  
    return;
  }
  uint32_t value = utils::queryIntValue(m,"value",0);
  _inj->setPulseHeight(value);
  par["STATUS"]=web::json::value::string(U("done"));
  Reply(status_codes::OK,par);  
}

FebinjManager::FebinjManager() : _inj(NULL)
{;}
void FebinjManager::end()
{
  if (_inj!=NULL)
    delete _inj;
  _inj=NULL;

}
void FebinjManager::initialise()
{

  _inj = new febinj::board();

  //_fsm=new fsm(name);
  this->setState("CREATED");

  this->addState("CONFIGURED");

  this->addTransition("CONFIGURE", "CREATED", "CONFIGURED", std::bind(&FebinjManager::configure, this, _1));
  ;
  this->addTransition("DESTROY", "CONFIGURED", "CREATED", std::bind(&FebinjManager::destroy, this, _1));

  this->addCommand("MASK", std::bind(&FebinjManager::c_set_mask, this, _1));
  this->addCommand("TRIGGERSOURCE", std::bind(&FebinjManager::c_set_trigger_source, this, _1));
  this->addCommand("TRIGGERSOFT", std::bind(&FebinjManager::c_software_trigger, this, _1));
  this->addCommand("TRIGGERINT", std::bind(&FebinjManager::c_internal_trigger, this, _1));
  this->addCommand("PAUSE", std::bind(&FebinjManager::c_pause_external_trigger, this, _1));
  this->addCommand("RESUME", std::bind(&FebinjManager::c_resume_external_trigger, this, _1));
  this->addCommand("TRIGGERMAX", std::bind(&FebinjManager::c_set_number_of_trigger, this, _1));
  this->addCommand("DELAY", std::bind(&FebinjManager::c_set_delay, this, _1));
  this->addCommand("DURATION", std::bind(&FebinjManager::c_set_duration, this, _1));
  this->addCommand("PULSEHEIGHT", std::bind(&FebinjManager::c_set_pulse_height, this, _1));

}
