#include "localCollector.hh"

using namespace pm;
using namespace pm::builder;
static LoggerPtr _logCollector(Logger::getLogger("PMDAQ_COLLECTOR"));

pm::builder::collector::collector() :  _running(false), _merger(NULL) {;}

void pm::builder::collector::initialise()
{
  // Create the context and the merger
  _context = new zmq::context_t();
  _merger = new pm::pmMerger(_context);

  // Register state
  this->addState("CONFIGURED");
  this->addState("RUNNING");

  // Register transitions
  this->addTransition("CONFIGURE", "CREATED", "CONFIGURED", std::bind(&pm::builder::collector::configure, this,std::placeholders::_1));
  this->addTransition("CONFIGURE", "CONFIGURED", "CONFIGURED", std::bind(&pm::builder::collector::configure, this,std::placeholders::_1));
  this->addTransition("START", "CONFIGURED", "RUNNING", std::bind(&pm::builder::collector::start, this,std::placeholders::_1));
  this->addTransition("STOP", "RUNNING", "CONFIGURED", std::bind(&pm::builder::collector::stop, this,std::placeholders::_1));
  this->addTransition("HALT", "RUNNING", "CREATED", std::bind(&pm::builder::collector::halt, this,std::placeholders::_1));
  this->addTransition("HALT", "CONFIGURED", "CREATED", std::bind(&pm::builder::collector::halt, this,std::placeholders::_1));

  // Standalone command
  this->addCommand("STATUS", std::bind(&pm::builder::collector::status, this,std::placeholders::_1));
  this->addCommand("SETHEADER", std::bind(&pm::builder::collector::setheader, this,std::placeholders::_1));
  this->addCommand("PURGE", std::bind(&pm::builder::collector::purge, this,std::placeholders::_1));
  _running=false;
}
void pm::builder::collector::end()
{
  // Stop possible running thread
  PMF_DEBUG(_logCollector, "Entering end of Builder");
  if (_merger!=NULL)
    {
        PMF_INFO(_logCollector, "Existing merger "<<_running);

      if (_running)
	     _merger->stop();
    delete _merger;
    _merger=NULL;
    }
    PMF_DEBUG(_logCollector, "deleting context");
    if (_context!=NULL)
      delete _context;
    _context=NULL;
    PMF_DEBUG(_logCollector, "exiting end");
}
void pm::builder::collector::configure(http_request m)
{
  auto par = json::value::object(); 

  PMF_INFO(_logCollector, "Received CONFIGURE");
  // Parse arguments
  // auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  // for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
  //   {
  //     if (it2->first.compare("device")==0)
  //       device=std::stoi(it2->second);
  //     if (it2->first.compare("port")==0)
  //       address=std::stoi(it2->second);
  //     }

  // Store message content in paramters
  // if (params().as_object().find("port")!=params().as_object().end())
  //   { 
  //     port=params()["port"].as_integer();

  //   }
  
  
  
  // Check that needed parameters exists
  if (params().as_object().find("collectingPort")==params().as_object().end())
    { 
      PMF_ERROR(_logCollector, "Missing collectingPort,no data stream");
      par["status"]=json::value::string(U("Missing collectingPort "));
      Reply(status_codes::OK,par);
      return;  
    }
  if (params().as_object().find("processor")==params().as_object().end())
    { 
      PMF_ERROR(_logCollector, "Missing processor, list of processing pluggins");
      par["status"]=json::value::string(U("Missing processors "));
      Reply(status_codes::OK,par);
      return;  
    }

  // register data source and processors
  if (params().as_object().find("purge")!=params().as_object().end())
      _merger->setPurge(params()["purge"].as_integer() != 0);

  // Register the data source
  json::value array_keys;
  std::stringstream st("");
  st<<"tcp://*:"<<params()["collectingPort"].as_integer();
  PMF_INFO(_logCollector, "Registering " << st.str());
  _merger->registerDataSource(st.str());
  array_keys[0]=json::value::string(U(st.str()));
  PMF_INFO(_logCollector, "Now processors " << st.str());
  // Register the processors
  json::value parray_keys;uint32_t np=0;
  for (auto it = params()["processor"].as_array().begin(); it != params()["processor"].as_array().end(); it++)
  {

    PMF_INFO(_logCollector, "registering processor" << (*it).as_string());
    _merger->registerProcessor((*it).as_string());
    parray_keys[np++]=(*it);
  }

  PMF_INFO(_logCollector, " Setting parameters for processors and merger ");
  _merger->loadParameters(this->params());

  
  // Overwrite msg
  //Prepare complex answer
  PMF_INFO(_logCollector, "end of configure");
  par["status"]=json::value::string(U("OK"));
  par["sourceRegistered"] = array_keys;
  par["processorRegistered"] = parray_keys;
  Reply(status_codes::OK,par);

  return;
}

void pm::builder::collector::start(http_request m)
{
 auto par = json::value::object(); 

 PMF_INFO(_logCollector, __PRETTY_FUNCTION__ << "Received START");
 // Parse arguments
 auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
 uint32_t run=0;
 for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
   if (it2->first.compare("run")==0)
     run=std::stoi(it2->second);
 _merger->start(run);
 _running = true;
 
  PMF_INFO(_logCollector, "Builder Run " << run << " is started ");
  par["status"] = json::value::string(U("STARTED"));
  par["run"] = json::value::number(run);
  Reply(status_codes::OK,par);
  return;
}
void pm::builder::collector::stop(http_request m)
{
  auto par = json::value::object(); 
  PMF_INFO(_logCollector, "Received STOP ");
  _merger->stop();
  _running = false;
  PMF_INFO(_logCollector, "Builder is stopped \n");
  fflush(stdout);
  par["status"] = json::value::string(U("STOPPED"));
  Reply(status_codes::OK,par);
  return;

}
void pm::builder::collector::halt(http_request m)
{
  auto par = json::value::object(); 
  PMF_INFO(_logCollector, "Received HALT");
  if (_running)
    {
      _merger->stop();
      _running = false;
    }


  PMF_INFO(_logCollector, "Destroying Builder Sources");
  //stop data sources
  _merger->clear();

  par["status"] = json::value::string(U("HALTED"));
  Reply(status_codes::OK,par);
  return;

}
void pm::builder::collector::status(http_request m)
{
  auto par = json::value::object(); 

  if (_merger != NULL)
    {
      par["status"]=json::value::string(U("done"));
      par["answer"] = _merger->status();	  
    }
  else
    {
      par["status"]=json::value::string(U("FAILED"));
      par["answer"] = json::value::string(U("NO merger created yet"));
    }
  Reply(status_codes::OK,par);
}

void pm::builder::collector::purge(http_request m)
{
  auto par = json::value::object(); 
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  uint32_t active=0;
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    if (it2->first.compare("active")==0)
      active=std::stoi(it2->second);
  if (_merger != NULL)
    {
      PMF_DEBUG(_logCollector, "Setting Purge flag to "<<active);
      
      _merger->setPurge(active != 0);
      par["answer"]=json::value::number(active);
    }
  else
    par["answer"] = json::value::string(U("NO merger created yet"));
  Reply(status_codes::OK,par);
  return;
    
}
void pm::builder::collector::setheader(http_request m)
{
  auto par = json::value::object(); 
  if (_merger == NULL)
  {
    par["STATUS"] = json::value::string(U("NO merger created yet"));
    Reply(status_codes::OK,par);
    return;
  }
  int32_t nextevent=-1;std::string shead("None");
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("nextevent")==0)
	nextevent=std::stoi(it2->second);
      if (it2->first.compare("header")==0)
	shead.assign(it2->second);
    }
  if (shead.compare("None") == 0)
    {
      par["STATUS"] = json::value::string(U("NO header provided"));
      Reply(status_codes::OK,par);
      return;

    }
  std::error_code  errorCode;
  auto jdevs=web::json::value::parse(shead,errorCode);

  if (errorCode.value()>0)
  {
    par["STATUS"] = json::value::string(U("Cannot Parse"));
    par["VALUE"] = json::value::number(errorCode.value());
    Reply(status_codes::OK,par);
    return;
  }

  PMF_DEBUG(_logCollector, "Header " << jdevs);
  std::vector<uint32_t> &v = _merger->runHeader();
  v.clear();
  for (auto jt = jdevs.as_array().begin(); jt != jdevs.as_array().end(); ++jt)
    v.push_back((*jt).as_integer());


  if (nextevent != -1)
    _merger->setRunHeaderEvent(nextevent);
  _merger->processRunHeader();

  par["STATUS"] = json::value::string(U("Done"));
  par["VALUE"] = jdevs;
  Reply(status_codes::OK,par);
  return;
}

extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new  pm::builder::collector);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
