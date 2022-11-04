#include "Febv2Manager.hh"
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
#include "stdafx.hh"

Febv2Manager::Febv2Manager() : _context(NULL), _running(false), g_mon(NULL),_dsData(NULL)
{
  ;
}

void Febv2Manager::initialise()
{

  // Register state

  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");

  this->addTransition("INITIALISE", "CREATED", "INITIALISED", std::bind(&Febv2Manager::fsm_initialise, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "INITIALISED", "CONFIGURED", std::bind(&Febv2Manager::configure, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "CONFIGURED", "CONFIGURED", std::bind(&Febv2Manager::configure, this, std::placeholders::_1));

  this->addTransition("START", "CONFIGURED", "RUNNING", std::bind(&Febv2Manager::start, this, std::placeholders::_1));
  this->addTransition("STOP", "RUNNING", "CONFIGURED", std::bind(&Febv2Manager::stop, this, std::placeholders::_1));
  this->addTransition("DESTROY", "CONFIGURED", "CREATED", std::bind(&Febv2Manager::destroy, this, std::placeholders::_1));
  this->addTransition("DESTROY", "INITIALISED", "CREATED", std::bind(&Febv2Manager::destroy, this, std::placeholders::_1));

  this->addCommand("STATUS", std::bind(&Febv2Manager::c_status, this, std::placeholders::_1));
  
  this->addCommand("DOWNLOADDB", std::bind(&Febv2Manager::c_downloadDB, this, std::placeholders::_1));

  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
}
void Febv2Manager::end()
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
void Febv2Manager::clearShm()
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
void Febv2Manager::spy_shm()
{
	PM_INFO(_logFebv2, "Starting spy_shm");
	uint32_t nprocessed = 0, last_processed = 0;
	time_t tlast = time(0);
	while (_running)
	{
		std::vector<std::string> vnames;
		utils::ls(_shmPath, vnames);
		PM_INFO(_logFebv2,"Loop PATH for files "<<_shmPath<<" "<<vnames.size());	
		for (auto x : vnames)
		{
			std::cout << x << std::endl;
			// EUDAQ_WARN("Find file "+x);
			// continue;
			auto b = std::make_shared<pm::buffer>(0x80000);
			uint32_t pls = utils::pull(x, b->ptr(), _shmPath);
			b->setPayloadSize(pls);
			
			uint64_t idx_storage = b->eventId(); // usually abcid
			
			
		}

		usleep(50000);
	}
	PM_INFO(_logFebv2, "Stoping run_monitor");
}

void Febv2Manager::c_status(http_request m)
{
  PM_INFO(_logFebv2, "Status CMD called ");
  auto par = json::value::object();

  par["STATUS"] = json::value::string(U("DONE"));

  json::value jl;
  uint32_t mb = 0;
  for (auto x : _mpi->boards())
  {

    json::value jt;
    jt["detid"] = json::value::number(x.second->data()->detectorId());
    jt["sourceid"] = json::value::number(x.second->data()->difId());
    jt["gtc"] = json::value::number(x.second->data()->gtc());
    jt["abcid"] = json::value::number(x.second->data()->abcid());
    jt["event"] = json::value::number(x.second->data()->event());
    jt["triggers"] = json::value::number(x.second->data()->triggers());
    jl[mb++] = jt;
  }
  par["TDCSTATUS"] = jl;
  Reply(status_codes::OK, par);
}

void Febv2Manager::c_downloadDB(http_request m)
{
  PM_INFO(_logFebv2, "downloadDB called ");
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
 http_response rep=utils::request(_feb_host,_feb_port,"DOWNLOADDB",parcmd);
  auto jrep = rep.extract_json();
 
  par["DBSTATE"] = json::value::string(U(dbstate));
  Reply(status_codes::OK, par);
}


void Febv2Manager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PM_INFO(_logFebv2, "****** CMD: INITIALISING");
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;

  //  web::json::value jtype=params()["type"];
  // _type=jtype.as_integer();
  // printf ("_type =%d\n",_type);

  // Need a Febv2 tag
  if (!utils::isMember(params(),"Febv2"))
  {
    PMF_ERROR(_logFebv2, "No Febv2 tag");
    par["status"] = json::value::string(U("Missing Febv2 tag "));
    Reply(status_codes::OK, par);
    return;
  }
  PM_INFO(_logFebv2, "Access Febv2");
  web::json::value jFebv2 = params()["Febv2"];
  //_msh =new MpiMessageHandler("/dev/shm");
  if (!utils::isMember(jFebv2,"febdaq"))
  {
    PMF_ERROR(_logFebv2, "No Febv2:febdaq tag");
    par["status"] = json::value::string(U("Missing Febv2:febdaq tag "));
    Reply(status_codes::OK, par);
    return;
  }
  auto jFebDaq=jFebv2["febdaq"];
  _feb_host=jFebDaq["host"].as_string();
  _feb_port=jFebDaq["port"].as_integer();

  // Send Initialise command
  http_response rep=utils::request(_feb_host,_feb_port,"INITIALISE",jFebDaq["params"]);
  auto jrep = rep.extract_json();
  // A FAIRE Recupperer info sur l'initialisation

  
if (_dsData!=NULL)
    {
      delete _dsData;_dsData=NULL;
    }
  PMF_INFO(_logFebv2, " Init done  ");
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);
}


void Febv2Manager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logFebv2, " CMD: Configuring");

  // Now Send the CONFIGURE COMMAND
  http_response rep=utils::request(_feb_host,_feb_port,"CONFIGURE",web::json::value::null());
  auto jrep = rep.extract_json();
  // A FAIRE RECUPPERER DETID ET SOURCEID
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
void Febv2Manager::start(http_request m)
{
  this->clearShm();
  _running = true;

	g_mon = std::thread(std::bind(&Febv2Manager::spy_shm, this));
  auto par = json::value::object();
  PMF_INFO(_logFebv2, " CMD: STARTING");
 // Now Send the START COMMAND
  http_response rep=utils::request(_feb_host,_feb_port,"START",web::json::value::null());
  auto jrep = rep.extract_json();
 
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);
}
void Febv2Manager::stop(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logFebv2, " CMD: STOPPING ");

// Now Send the STOP COMMAND
  http_response rep=utils::request(_feb_host,_feb_port,"STOP",web::json::value::null());
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
void Febv2Manager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logFebv2, " CMD: DESTROYING");
  // Now Send the DESTROY COMMAND
  http_response rep=utils::request(_feb_host,_feb_port,"DESTROY",web::json::value::null());
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

  // To be done: _Febv2->clear();
}

extern "C"
{
  // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.
  handlerPlugin *loadProcessor(void)
  {
    return (new Febv2Manager);
  }
  // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed
  // to it.  This isn't a very safe function, since there's no
  // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin *obj)
  {
    delete obj;
  }
}
