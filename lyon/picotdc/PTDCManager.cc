#include "PTDCManager.hh"
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
#include "stdafx.hh"

PTDCManager::PTDCManager() : _context(NULL), _running(false), g_mon(NULL),_dsData(NULL)
{
  ;
}

void PTDCManager::initialise()
{

  // Register state

  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");

  this->addTransition("INITIALISE", "CREATED", "INITIALISED", std::bind(&PTDCManager::fsm_initialise, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "INITIALISED", "CONFIGURED", std::bind(&PTDCManager::configure, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "CONFIGURED", "CONFIGURED", std::bind(&PTDCManager::configure, this, std::placeholders::_1));

  this->addTransition("START", "CONFIGURED", "RUNNING", std::bind(&PTDCManager::start, this, std::placeholders::_1));
  this->addTransition("STOP", "RUNNING", "CONFIGURED", std::bind(&PTDCManager::stop, this, std::placeholders::_1));
  this->addTransition("DESTROY", "CONFIGURED", "CREATED", std::bind(&PTDCManager::destroy, this, std::placeholders::_1));
  this->addTransition("DESTROY", "INITIALISED", "CREATED", std::bind(&PTDCManager::destroy, this, std::placeholders::_1));

  this->addCommand("STATUS", std::bind(&PTDCManager::c_status, this, std::placeholders::_1));
  
  this->addCommand("DOWNLOADDB", std::bind(&PTDCManager::c_downloadDB, this, std::placeholders::_1));

  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
}
void PTDCManager::end()
{
  // Stop any running process
 
  if (g_mon != NULL)
    {
      _running=false;
      g_mon->join();
      delete g_mon;
      g_mon = NULL;
    }
  
}
void PTDCManager::clearShm()
{
  std::vector<std::string> vnames;
  utils::ls(_shmPath, vnames);
  std::stringstream sc, sd;
  sc.str(std::string());
  sd.str(std::string());
  for (auto name : vnames)
    {
      sc << _shmPath << "/closed/" << name;
      sd << _shmPath << name;
      ::unlink(sc.str().c_str());
      ::unlink(sd.str().c_str());
    }
}
void PTDCManager::spy_shm()
{
  PM_INFO(_logPTDC, "Starting spy_shm");
  uint32_t nprocessed = 0, last_processed = 0;
  time_t tlast = time(0);
  uint32_t detid,sourceid,eventid;
  uint64_t bxid;
  while (_running)
    {
      std::vector<std::string> vnames;
      utils::ls(_shmPath, vnames);
      PM_INFO(_logPTDC,"Loop PATH for files "<<_shmPath<<" "<<vnames.size());	
      for (auto x : vnames)
	{
	  std::cout << x << std::endl;
	  // EUDAQ_WARN("Find file "+x);
	  // continue;
	  auto b = _dsData->buffer();
	 
	  sscanf(x.c_str(),"Event_%u_%u_%u_%lu",&detid, &sourceid, &eventid, &bxid);

	  b->setDetectorId(detid);
	  b->setDataSourceId(sourceid);
	  b->setEventId(eventid);
	  b->setBxId(bxid);
	  uint32_t pls = utils::pull(x, b->payload(), _shmPath);
	  b->setPayloadSize(pls);
	  uint64_t idx_storage = b->eventId(); // usually abcid
	  _dsData->publish(bxid,eventid,b->size());
	}

      usleep(50000);
    }
  PM_INFO(_logPTDC, "Stoping spy_shm");
}

void PTDCManager::c_status(http_request m)
{
  PM_INFO(_logPTDC, "Status CMD called ");
  auto par = json::value::object();
  http_response rep=utils::request(_ptdc_host,_ptdc_port,"STATUS",web::json::value::null());
  auto jrep = rep.extract_json();
  _detId=jrep.get().as_object()["DETID"].as_integer();
  _sourceId=jrep.get().as_object()["SOURCEID"].as_integer();
  par["STATUS"] = json::value::string(U("DONE"));
  par["DETID"]=jrep.get().as_object()["DETID"];
  par["SOURCEID"]=jrep.get().as_object()["SOURCEID"];


  Reply(status_codes::OK, par);
}

void PTDCManager::c_downloadDB(http_request m)
{
  PM_INFO(_logPTDC, "downloadDB called ");
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  std::string dbstate = "NONE";
  uint32_t version = 0;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("state") == 0)
	dbstate = it2->second;
      if (it2->first.compare("version") == 0)
	version = std::stoi(it2->second);
    }
  auto parcmd = json::value::object();
  parcmd["statename"]= json::value::string(U(dbstate));
  parcmd["version"]=json::value::number(version);
  http_response rep=utils::request(_ptdc_host,_ptdc_port,"DOWNLOADDB",parcmd);
  auto jrep = rep.extract_json();
 
  par["DBSTATE"] = json::value::string(U(dbstate));
  Reply(status_codes::OK, par);
}


void PTDCManager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PM_INFO(_logPTDC, "****** CMD: INITIALISING");
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;

  //  web::json::value jtype=params()["type"];
  // _type=jtype.as_integer();
  // printf ("_type =%d\n",_type);

  // Need a PTDC tag
  if (!utils::isMember(params(),"PTDC"))
    {
      PMF_ERROR(_logPTDC, "No PTDC tag");
      par["status"] = json::value::string(U("Missing PTDC tag "));
      Reply(status_codes::OK, par);
      return;
    }
  PM_INFO(_logPTDC, "Access PTDC");
  web::json::value jPTDC = params()["PTDC"];
  //_msh =new MpiMessageHandler("/dev/shm");
  if (!utils::isMember(jPTDC,"ptdcdaq"))
    {
      PMF_ERROR(_logPTDC, "No PTDC:ptdcdaq tag");
      par["status"] = json::value::string(U("Missing PTDC:ptdcdaq tag "));
      Reply(status_codes::OK, par);
      return;
    }
  auto jPtdcDaq=jPTDC["ptdcdaq"];
  _ptdc_host=jPtdcDaq["host"].as_string();
  _ptdc_port=jPtdcDaq["port"].as_integer();

  // Send Initialise command
  http_response rep=utils::request(_ptdc_host,_ptdc_port,"INITIALISE",jPtdcDaq["params"]);
  auto jrep = rep.extract_json();
  // A FAIRE Recupperer info sur l'initialisation
  
  
  if (_dsData!=NULL)
    {
      delete _dsData;_dsData=NULL;
    }
  PMF_INFO(_logPTDC, " Init done  ");
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);
}


void PTDCManager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPTDC, " CMD: Configuring");

  // Now Send the CONFIGURE COMMAND
  http_response rep=utils::request(_ptdc_host,_ptdc_port,"CONFIGURE",web::json::value::null());
  auto jrep = rep.extract_json();
  // A FAIRE RECUPPERER DETID ET SOURCEID
  http_response rep1=utils::request(_ptdc_host,_ptdc_port,"STATUS",web::json::value::null());
  auto jrep1 = rep1.extract_json();
  _detId=jrep1.get().as_object()["DETID"].as_integer();
  _sourceId=jrep1.get().as_object()["SOURCEID"].as_integer();
  if (_context == NULL)
    _context = new zmq::context_t(1);
  if (_dsData==NULL)
    {
      _dsData = new pm::pmSender(_context,_detId,_sourceId);
      _dsData->autoDiscover(session(),"evb_builder", "collectingPort");
      _dsData->collectorRegister(); 
    }
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);
}

/////////////////////////////////////////////////////////
void PTDCManager::start(http_request m)
{
  this->clearShm();
  _running = true;

  g_mon = new std::thread(std::bind(&PTDCManager::spy_shm, this));
  auto par = json::value::object();
  PMF_INFO(_logPTDC, " CMD: STARTING");
  // Now Send the START COMMAND
  http_response rep=utils::request(_ptdc_host,_ptdc_port,"START",web::json::value::null());
  auto jrep = rep.extract_json();
 
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);
}
void PTDCManager::stop(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPTDC, " CMD: STOPPING ");

  // Now Send the STOP COMMAND
  http_response rep=utils::request(_ptdc_host,_ptdc_port,"STOP",web::json::value::null());
  auto jrep = rep.extract_json();

  if (g_mon != NULL && _running)
    {
      _running=false;
      g_mon->join();
      delete g_mon;
      g_mon = NULL;
    }
 

  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);
}
void PTDCManager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPTDC, " CMD: DESTROYING");
  // Now Send the DESTROY COMMAND
  http_response rep=utils::request(_ptdc_host,_ptdc_port,"DESTROY",web::json::value::null());
  auto jrep = rep.extract_json();
  if (g_mon != NULL)
    {
      _running=false;
      g_mon->join();
      delete g_mon;
      g_mon = NULL;
    }
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);

  // To be done: _PTDC->clear();
}

extern "C"
{
  // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.
  handlerPlugin *loadProcessor(void)
  {
    return (new PTDCManager);
  }
  // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed
  // to it.  This isn't a very safe function, since there's no
  // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin *obj)
  {
    delete obj;
  }
}
