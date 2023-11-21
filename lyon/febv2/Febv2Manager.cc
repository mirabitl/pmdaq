#include "Febv2Manager.hh"
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

Febv2Manager::Febv2Manager() : _context(NULL), _running(false), g_mon(NULL), _dsData(NULL)
{
  ;
}

void Febv2Manager::initialise()
{

  // Register state
  this->addState("PREINITIALISED");
  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  this->addTransition("CREATEFEB", "CREATED", "PREINITIALISED", std::bind(&Febv2Manager::createfeb, this, std::placeholders::_1));

  this->addTransition("INITIALISE", "PREINITIALISED", "INITIALISED", std::bind(&Febv2Manager::fsm_initialise, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "INITIALISED", "CONFIGURED", std::bind(&Febv2Manager::configure, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "CONFIGURED", "CONFIGURED", std::bind(&Febv2Manager::configure, this, std::placeholders::_1));

  this->addTransition("START", "CONFIGURED", "RUNNING", std::bind(&Febv2Manager::start, this, std::placeholders::_1));
  this->addTransition("STOP", "RUNNING", "CONFIGURED", std::bind(&Febv2Manager::stop, this, std::placeholders::_1));
  this->addTransition("DESTROY", "CONFIGURED", "PREINITIALISED", std::bind(&Febv2Manager::destroy, this, std::placeholders::_1));
  this->addTransition("DESTROY", "INITIALISED", "PREINITIALISED", std::bind(&Febv2Manager::destroy, this, std::placeholders::_1));

  this->addCommand("STATUS", std::bind(&Febv2Manager::c_status, this, std::placeholders::_1));

  this->addCommand("DOWNLOADDB", std::bind(&Febv2Manager::c_downloadDB, this, std::placeholders::_1));

  this->addCommand("VTHSHIFT", std::bind(&Febv2Manager::c_vthshift, this, std::placeholders::_1));

  // std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
}
void Febv2Manager::end()
{
  // Stop any running process

  if (g_mon != NULL)
    {
      _running = false;
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
  uint32_t detid, sourceid, eventid;
  uint64_t bxid;
  while (_running)
    {
      std::vector<std::string> vnames;
      utils::ls(_shmPath, vnames);
      PM_DEBUG(_logFebv2, "Loop PATH for files " << _shmPath << " " << vnames.size());
      for (auto x : vnames)
	{
	  std::cout << x << std::endl;
	  // EUDAQ_WARN("Find file "+x);
	  // continue;
	  auto b = _dsData->buffer();

	  sscanf(x.c_str(), "Event_%u_%u_%u_%lu", &detid, &sourceid, &eventid, &bxid);

	  b->setDetectorId(detid);
	  b->setDataSourceId(sourceid);
	  b->setEventId(eventid);
	  b->setBxId(bxid);
	  uint32_t pls = utils::pull(x, b->payload(), _shmPath);
	  b->setPayloadSize(pls);
	  uint64_t idx_storage = b->eventId(); // usually abcid
	  _dsData->publish(bxid, eventid, b->size());
	}

      usleep(50000);
    }
  PM_INFO(_logFebv2, "Stoping spy_shm");
}
web::json::value Febv2Manager::decode_spyne_answer(web::json::value v,std::string c)
{
  std::string s1=c+"Response";
  std::string s2=c+"Result";
  auto s_v=v.as_object()[s1][s2][0].as_string();
  web::json::value ret = json::value::parse(s_v);
  return ret;

}
void Febv2Manager::c_status(http_request m)
{
  PM_INFO(_logFebv2, "Status CMD called ");
  auto par = json::value::object();

  auto jrep = this->post("STATUS");
  auto w_rep=decode_spyne_answer(jrep,"STATUS");

  _detId = w_rep.as_object()["DETID"].as_integer();
  _sourceId = w_rep.as_object()["SOURCEID"].as_integer();
  par["STATUS"] = json::value::string(U("DONE"));
  par["DETID"] = _detId;
  par["SOURCEID"] = _sourceId;
  par["EVENT"]=w_rep.as_object()["STATUS"];
  mqtt_publish("status",par);
  Reply(status_codes::OK, par);
}

void Febv2Manager::c_downloadDB(http_request m)
{
  PM_INFO(_logFebv2, "downloadDB called ");
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));
  std::string dbstate = utils::queryStringValue(m, "state", "NONE");
  uint32_t version = utils::queryIntValue(m, "version", 0);

  auto parcmd = json::value::object();
  parcmd["statename"] = json::value::string(U(dbstate));
  parcmd["version"] = json::value::number(version);
  auto jrep=this->post("DOWNLOADDB",parcmd);

  par["DBSTATE"] = json::value::string(U(dbstate));
  Reply(status_codes::OK, par);
}
void Febv2Manager::c_vthshift(http_request m)
{

  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));
  uint32_t shift = utils::queryIntValue(m, "shift", 50);
  PM_INFO(_logFebv2, "VTHSHIFT called "<<shift);
  auto parcmd = json::value::object();
  parcmd["shift"] = json::value::number(shift);
  auto jrep=this->post("SHIFTVTH",parcmd);

  par["SHIFT"] = json::value::number(shift);
  Reply(status_codes::OK, par);
}

web::json::value Febv2Manager::post(std::string cmd, web::json::value v )
{
  std::cerr<<"post command "<<_feb_host<<":"<<_feb_port<<"/"<<cmd<<" "<<v<<std::endl;
  http_response rep = utils::request(_feb_host, _feb_port, cmd, v);
  
  auto jrep = rep.extract_json();
  std::cerr<<" Returned response:"<<jrep.get()<<std::endl;
  return jrep.get();
}
void Febv2Manager::createfeb(http_request m)
{
  auto par = json::value::object();
  PM_INFO(_logFebv2, "****** CMD: PREINITIALISED , creating FEB object");
  // Need a Febv2 tag
  if (!utils::isMember(params(), "febv2"))
    {
      PMF_ERROR(_logFebv2, "No febv2 tag");
      par["status"] = json::value::string(U("Missing Febv2 tag "));
      Reply(status_codes::OK, par);
      return;
    }
  PM_INFO(_logFebv2, "Access Febv2");
  web::json::value jFebv2 = params()["febv2"];
  //_msh =new MpiMessageHandler("/dev/shm");
  if (!utils::isMember(jFebv2, "daq"))
    {
      PMF_ERROR(_logFebv2, "No Febv2:febdaq tag");
      par["status"] = json::value::string(U("Missing Febv2:febdaq tag "));
      Reply(status_codes::OK, par);
      return;
    }
  auto jFebDaq = jFebv2["daq"];
  _feb_host = jFebDaq["host"].as_string();
  _feb_port = jFebDaq["port"].as_integer();
  _shmPath=jFebv2["shm_path"].as_string();

  auto jrep = this->post("/CREATE",jFebDaq["params"]);
  auto w_rep=decode_spyne_answer(jrep,"CREATE");
  //  A FAIRE Recupperer info sur l'initialisation

  if (_dsData != NULL)
    {
      delete _dsData;
      _dsData = NULL;
    }
  PMF_INFO(_logFebv2, " Create done  ");
  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;
  Reply(status_codes::OK, par);
}

void Febv2Manager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PM_INFO(_logFebv2, "****** CMD: INITIALISING");
  // Send Initialise command
  auto jrep = this->post("/INITIALISE");
  auto w_rep=decode_spyne_answer(jrep,"INITIALISE");
  if (_dsData != NULL)
    {
      delete _dsData;
      _dsData = NULL;
    }
  PMF_INFO(_logFebv2, " Init done  ");
  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;

  Reply(status_codes::OK, par);
}

void Febv2Manager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logFebv2, " CMD: Configuring");

  // Now Send the CONFIGURE COMMAND

  auto jrep = this->post("/CONFIGURE");

  // A FAIRE RECUPPERER DETID ET SOURCEID

  auto jrepl = this->post("/STATUS");
  auto w_rep=decode_spyne_answer(jrepl,"STATUS");

  _detId = w_rep.as_object()["DETID"].as_integer();
  _sourceId = w_rep.as_object()["SOURCEID"].as_integer();


  
  //std::cout<<_detId<<" "<<_sourceId<<std::endl;
  if (_context == NULL)
    _context = new zmq::context_t(1);
  if (_dsData == NULL)
    {
      _dsData = new pm::pmSender(_context, _detId, _sourceId);
      _dsData->autoDiscover(session(), "evb_builder", "collectingPort");
      _dsData->collectorRegister();
    }
  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;

  Reply(status_codes::OK, par);
}

/////////////////////////////////////////////////////////
void Febv2Manager::start(http_request m)
{
  this->clearShm();
  _running = true;

  g_mon = new std::thread(std::bind(&Febv2Manager::spy_shm, this));
  auto par = json::value::object();
  PMF_INFO(_logFebv2, " CMD: STARTING");
  // Now Send the START COMMAND

  auto jrep = this->post("/START");
  auto w_rep=decode_spyne_answer(jrep,"START");
  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;
  Reply(status_codes::OK, par);
}
void Febv2Manager::stop(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logFebv2, " CMD: STOPPING ");

  // Now Send the STOP COMMAND
  auto jrep = this->post("/STOP");
  auto w_rep=decode_spyne_answer(jrep,"STOP");
  if (g_mon != NULL && _running)
    {
      _running = false;
      g_mon->join();
      delete g_mon;
      g_mon = NULL;
    }

  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;

  Reply(status_codes::OK, par);
}
void Febv2Manager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logFebv2, " CMD: DESTROYING");
  // Now Send the DESTROY COMMAND
  auto jrep = this->post("/DESTROY");
  auto w_rep=decode_spyne_answer(jrep,"DESTROY");

  if (g_mon != NULL)
    {
      _running = false;
      g_mon->join();
      delete g_mon;
      g_mon = NULL;
    }
  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;

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
