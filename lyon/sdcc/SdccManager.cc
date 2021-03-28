
#include "SdccManager.hh"


using namespace sdcc;
using namespace mdcc;


void SdccManager::open(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logSdcc," CMD: Opening");
  
  std::string device;
  if (!utils::isMember(params(),"device"))
    {
      PMF_ERROR(_logSdcc,"Missing device in parameters");
      par["status"]=json::value::string(U("Missing device"));
      Reply(status_codes::OK,par);  
    }
  device=params()["device"].as_string();
  this->Open(device);
  par["status"]=json::value::string(U("Opened"));
  Reply(status_codes::OK,par);  
}

void SdccManager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logSdcc," CMD: Initialising");

  sdcc::interface* ccc= this->getInterface();
  if (ccc==NULL)
    {
      PMF_ERROR(_logSdcc,"Please open CCC DCCCCC01 first");
      par["status"]=json::value::string(U("Please open CCC DCCCCC01 first"));
      Reply(status_codes::OK,par);  
      return;
    }
  this->getInterface()->initialise();
  par["status"]=json::value::string(U("initialised"));
  Reply(status_codes::OK,par);  
}
void SdccManager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logSdcc," CMD: Configuring");

  sdcc::interface* ccc= this->getInterface();
  if (ccc==NULL)
    {
      PMF_ERROR(_logSdcc,"Please open CCC DCCCCC01 first");
      par["status"]=json::value::string(U("Please open CCC DCCCCC01 first"));
      Reply(status_codes::OK,par);  

      return;
    }
  this->getInterface()->configure();
  par["status"]=json::value::string(U("Configured"));
  Reply(status_codes::OK,par);  
}
void SdccManager::start(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logSdcc," CMD: Starting");

  sdcc::interface* ccc= this->getInterface();
  if (ccc==NULL)
    {
      PMF_ERROR(_logSdcc,"Please open CCC DCCCCC01 first");
      par["status"]=json::value::string(U("Please open CCC DCCCCC01 first"));
      Reply(status_codes::OK,par);  

      return;
    }
  this->getInterface()->start();
  par["status"]=json::value::string(U("Started"));
  Reply(status_codes::OK,par);  
}
void SdccManager::stop(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logSdcc," CMD: Stopping");

  sdcc::interface* ccc= this->getInterface();
  if (ccc==NULL)
    {
      PMF_ERROR(_logSdcc,"Please open CCC DCCCCC01 first");
      par["status"]=json::value::string(U("Please open CCC DCCCCC01 first"));
      Reply(status_codes::OK,par);  
      
      return;
    }
  this->getInterface()->stop();
  par["status"]=json::value::string(U("Stopped"));
  Reply(status_codes::OK,par);  
}


void SdccManager::pause(http_request m)
{
  auto par = json::value::object();
  sdcc::interface* ccc= this->getInterface();
  if (ccc==NULL)
    {
      PMF_ERROR(_logSdcc,"Please open CCC DCCCCC01 first");
      par["status"]=json::value::string(U("Please open CCC DCCCCC01 first"));
      Reply(status_codes::OK,par);  

      return;
    }
  ccc->getReader()->DoSendPauseTrigger();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}
void SdccManager::resume(http_request m)
{
  auto par = json::value::object();
  sdcc::interface* ccc= this->getInterface();
  if (ccc==NULL)
    {
      PMF_ERROR(_logSdcc,"Please open CCC DCCCCC01 first");
      par["status"]=json::value::string(U("Please open CCC DCCCCC01 first"));
      Reply(status_codes::OK,par);  

    }
  ccc->getReader()->DoSendResumeTrigger();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);  
}
void SdccManager::difreset(http_request m)
{
  auto par = json::value::object();
  sdcc::interface* ccc= this->getInterface();
  if (ccc==NULL)  {
    PMF_ERROR(_logSdcc,"Please open CCC DCCCCC01 first");
    par["status"]=json::value::string(U("Please open CCC DCCCCC01 first"));
    Reply(status_codes::OK,par);  
  }
  ccc->getReader()->DoSendDIFReset();
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}
void SdccManager::cccreset(http_request m)
{
  auto par = json::value::object();
  sdcc::interface* ccc= this->getInterface();
  if (ccc==NULL)
    {
      PMF_ERROR(_logSdcc,"Please open CCC DCCCCC01 first");
      par["status"]=json::value::string(U("Please open CCC DCCCCC01 first"));
      Reply(status_codes::OK,par);  

    }
  ccc->getReader()->DoSendCCCReset();
  par["STATUS"]=web::json::value::string(U("DONE"));

  Reply(status_codes::OK,par);  
}
 
void SdccManager::readreg(http_request m)
{
  auto par = json::value::object();
  sdcc::interface* ccc= this->getInterface();
  if (ccc==NULL)    {
    PMF_ERROR(_logSdcc,"Please open CCC DCCCCC01 first");
    par["status"]=json::value::string(U("Please open CCC DCCCCC01 first"));
    Reply(status_codes::OK,par);  


  }
  uint32_t adr=utils::queryIntValue(m,"address",2);
  uint32_t val=ccc->getReader()->DoReadRegister(adr);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["ADDRESS"]=web::json::value::number(adr);
  par["VALUE"]=web::json::value::number(val);
  Reply(status_codes::OK,par);  
} 
void SdccManager::writereg(http_request m)
{
  auto par = json::value::object();
  sdcc::interface* ccc= this->getInterface();
  if (ccc==NULL)
    {
      PMF_ERROR(_logSdcc,"Please open CCC DCCCCC01 first");
      par["status"]=json::value::string(U("Please open CCC DCCCCC01 first"));
      Reply(status_codes::OK,par);  
    }
  uint32_t adr=utils::queryIntValue(m,"address",2);
  uint32_t value=utils::queryIntValue(m,"value",1234);
  ccc->getReader()->DoWriteRegister(adr,value);

  par["STATUS"]=web::json::value::string(U("DONE"));
  par["ADDRESS"]=web::json::value::number(adr);
  par["VALUE"]=web::json::value::number(value);
  Reply(status_codes::OK,par);  
} 



SdccManager::SdccManager() : _manager(NULL) {;}

void SdccManager::initialise()
{
  
  // Register state
  this->addState("OPENED");
  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");

  this->addTransition("OPEN","CREATED","OPENED",std::bind(&SdccManager::open, this,std::placeholders::_1));
  this->addTransition("INITIALISE","OPENED","INITIALISED",std::bind(&SdccManager::fsm_initialise, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","INITIALISED","CONFIGURED",std::bind(&SdccManager::configure, this,std::placeholders::_1));
  this->addTransition("START","CONFIGURED","RUNNING",std::bind(&SdccManager::start, this,std::placeholders::_1));
  this->addTransition("STOP","RUNNING","CONFIGURED",std::bind(&SdccManager::stop, this,std::placeholders::_1));
  this->addTransition("STOP","CONFIGURED","CONFIGURED",std::bind(&SdccManager::stop, this,std::placeholders::_1));
  this->addTransition("DESTROY","CONFIGURED","OPENED",std::bind(&SdccManager::open, this,std::placeholders::_1));
  





  this->addCommand("PAUSE",std::bind(&SdccManager::pause,this,std::placeholders::_1));
  this->addCommand("RESUME",std::bind(&SdccManager::resume,this,std::placeholders::_1));
  this->addCommand("DIFRESET",std::bind(&SdccManager::difreset,this,std::placeholders::_1));
  this->addCommand("CCCRESET",std::bind(&SdccManager::cccreset,this,std::placeholders::_1));
  this->addCommand("WRITEREG",std::bind(&SdccManager::writereg,this,std::placeholders::_1));
  this->addCommand("READREG",std::bind(&SdccManager::readreg,this,std::placeholders::_1));


  _manager=NULL;	

}

void SdccManager::end()
{
  if (_manager!=NULL)
    delete _manager;
  _manager=NULL;
}
void SdccManager::Open(std::string s)
{
  if (_manager!=NULL)
    delete _manager;
  _manager= new sdcc::interface(s);
}
