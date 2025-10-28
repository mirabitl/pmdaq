#include "shm_data_source.hh"
#include "stdafx.hh"
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <unistd.h>

shm_data_source::shm_data_source()
    : _context(NULL), _running(false), g_mon(NULL), _dsData(NULL), _udpUrl("") {
  ;
}

void shm_data_source::initialise() {

  // Register state
  this->addState("PREINITIALISED");
  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  this->addTransition(
      "CREATEBOARD", "CREATED", "PREINITIALISED",
      std::bind(&shm_data_source::createboard, this, std::placeholders::_1));

  this->addTransition(
      "INITIALISE", "PREINITIALISED", "INITIALISED",
      std::bind(&shm_data_source::fsm_initialise, this, std::placeholders::_1));
  this->addTransition(
      "CONFIGURE", "INITIALISED", "CONFIGURED",
      std::bind(&shm_data_source::configure, this, std::placeholders::_1));
  this->addTransition(
      "CONFIGURE", "CONFIGURED", "CONFIGURED",
      std::bind(&shm_data_source::configure, this, std::placeholders::_1));

  this->addTransition(
      "START", "CONFIGURED", "RUNNING",
      std::bind(&shm_data_source::start, this, std::placeholders::_1));
  this->addTransition(
      "STOP", "RUNNING", "CONFIGURED",
      std::bind(&shm_data_source::stop, this, std::placeholders::_1));
  this->addTransition(
      "DESTROY", "CONFIGURED", "PREINITIALISED",
      std::bind(&shm_data_source::destroy, this, std::placeholders::_1));
  this->addTransition(
      "DESTROY", "INITIALISED", "PREINITIALISED",
      std::bind(&shm_data_source::destroy, this, std::placeholders::_1));

  this->addCommand("STATUS", std::bind(&shm_data_source::c_status, this,
                                       std::placeholders::_1));
  this->addCommand("DSLIST", std::bind(&shm_data_source::c_dslist, this,
                                       std::placeholders::_1));

  this->addCommand("ADDTOKENS", std::bind(&shm_data_source::c_add_tokens, this,
                                          std::placeholders::_1));

  // std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
}
void shm_data_source::end() {
  // Stop any running process

  if (g_mon != NULL) {
    _running = false;
    g_mon->join();
    delete g_mon;
    g_mon = NULL;
  }
}
void shm_data_source::clearShm() {
  std::vector<std::string> vnames;
  utils::ls(_shmPath, vnames);
  std::stringstream sc, sd;
  sc.str(std::string());
  sd.str(std::string());
  for (auto name : vnames) {
    sc << _shmPath << "/closed/" << name;
    sd << _shmPath << name;
    ::unlink(sc.str().c_str());
    ::unlink(sd.str().c_str());
  }
}
void shm_data_source::spy_shm() {
  PM_INFO(_logShmDS, "Starting spy_shm");
  uint32_t nprocessed = 0, last_processed = 0;
  time_t tlast = time(0);
  uint32_t detid, sourceid, eventid;
  uint64_t bxid;
  auto parcmd = json::value::object();
  int id, offset, elen, gtc, tokens;
  while (_running) {
    std::vector<std::string> vnames;
    utils::ls(_shmPath, vnames);
    PM_DEBUG(_logShmDS,
             "Loop PATH for files " << _shmPath << " " << vnames.size());
    uint32_t nproc = 0;
    for (auto x : vnames) {
      // std::cout << x << std::endl;
      //  EUDAQ_WARN("Find file "+x);
      //  continue;
      auto b = _dsData->buffer();

      sscanf(x.c_str(), "Event_%u_%u_%u_%lu", &detid, &sourceid, &eventid,
             &bxid);

      b->setDetectorId(detid);
      b->setDataSourceId(sourceid);
      b->setEventId(eventid);
      b->setBxId(bxid);
      uint32_t pls = utils::pull(x, b->payload(), _shmPath);

      uint32_t *l = (uint32_t *)b->payload();
      id = ntohl(l[0]);
      offset = ntohl(l[1]);
      elen = ntohl(l[2]);
      gtc = ntohl(l[3]);
      tokens = ntohl(l[4]);

      PM_DEBUG(_logShmDS, "NPROC " << nproc << " Id " << id << " offset "
                                   << offset << " elen " << elen << " gtc "
                                   << gtc << " tokens " << tokens);

      b->setPayloadSize(pls);
      uint64_t idx_storage = b->eventId(); // usually abcid
      _dsData->publish(bxid, eventid, b->size());
      nproc += pls / 1472;
    }
    int nt = 100000 - tokens;
    if (nt > 40000 && vnames.size() > 1) {
      PM_INFO(_logShmDS, "resend " << nt);
      //::sleep(10);
      parcmd["tokens"] = json::value::number(nt);
      auto jrep = this->post("ADDTOKENS", parcmd);
      PM_INFO(_logShmDS, "NPROC " << nt << " Id " << id << " offset " << offset
                                  << " elen " << elen << " gtc " << gtc
                                  << " tokens " << tokens);

      if (_udpUrl.length() > 1)
        utils::sendCommand(_udpUrl, "ADDTOKENS", parcmd);
    }
    usleep(50000);
  }
  PM_INFO(_logShmDS, "Stoping spy_shm");
}
web::json::value shm_data_source::decode_spyne_answer(web::json::value v,
                                                      std::string c) {
  std::string s1 = c + "Response";
  std::string s2 = c + "Result";
  auto s_v = v.as_object()[s1][s2][0].as_string();
  web::json::value ret = json::value::parse(s_v);
  return ret;
}
void shm_data_source::c_dslist(http_request m) {
  PM_INFO(_logShmDS, "DSLIST CMD called ");
  auto par = json::value::object();

  auto jrep = this->post("STATUS");
  auto w_rep = decode_spyne_answer(jrep, "STATUS");

  _detId = w_rep.as_object()["DETID"].as_integer();
  _sourceId = w_rep.as_object()["SOURCEID"].as_integer();

  web::json::value array_slc;
  web::json::value ds;
  ds["detid"] = _detId;
  ds["id"] = _sourceId;
  ds["gtc"] = w_rep.as_object()["STATUS"];
  array_slc[0] = ds;
  par["STATUS"] = web::json::value::string(U("DONE"));
  par["DSLIST"] = array_slc;
  Reply(status_codes::OK, par);
}
void shm_data_source::c_status(http_request m) {
  PM_DEBUG(_logShmDS, "STATUS CMD called ");
  auto par = json::value::object();

  auto jrep = this->post("STATUS");
  auto w_rep = decode_spyne_answer(jrep, "STATUS");

  _detId = w_rep.as_object()["DETID"].as_integer();
  _sourceId = w_rep.as_object()["SOURCEID"].as_integer();
  par["STATUS"] = json::value::string(U("DONE"));
  par["DETID"] = _detId;
  par["SOURCEID"] = _sourceId;
  par["EVENT"] = w_rep.as_object()["STATUS"];
  mqtt_publish("status", par);
  Reply(status_codes::OK, par);
}
void shm_data_source::c_add_tokens(http_request m) {
  PM_INFO(_logShmDS, "Add tokens called ");
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));
  uint32_t tokens = utils::queryIntValue(m, "tokens", 10000);

  auto parcmd = json::value::object();
  parcmd["tokens"] = json::value::number(tokens);
  auto jrep = this->post("ADDTOKENS", parcmd);
  if (_udpUrl.length() > 1)
    utils::sendCommand(_udpUrl, "ADDTOKENS", parcmd);
  par["TOKENS"] = json::value::number(tokens);
  Reply(status_codes::OK, par);
}

web::json::value shm_data_source::post(std::string cmd, web::json::value v) {
  // std::cerr<<"post command "<<_board_host<<":"<<_board_port<<"/"<<cmd<<"
  // "<<v<<std::endl;
  http_response rep = utils::request(_board_host, _board_port, cmd, v);

  auto jrep = rep.extract_json();
  std::cerr << " Returned response:" << jrep.get() << std::endl;
  return jrep.get();
}
void shm_data_source::createboard(http_request m) {
  auto par = json::value::object();
  PM_INFO(_logShmDS, "****** CMD: PREINITIALISED , creating BOARD object");
  // Need a ShmDS tag
  if (!utils::isMember(params(), "shm_data_source")) {
    PMF_ERROR(_logShmDS, "No shm_data_source tag");
    par["status"] = json::value::string(U("Missing ShmDS tag "));
    Reply(status_codes::OK, par);
    return;
  }
  PM_INFO(_logShmDS, "Access ShmDS");
  web::json::value jShmDS = params()["shm_data_source"];
  //_msh =new MpiMessageHandler("/dev/shm");
  if (!utils::isMember(jShmDS, "daq")) {
    PMF_ERROR(_logShmDS, "No ShmDS:boarddaq tag");
    par["status"] = json::value::string(U("Missing ShmDS:boarddaq tag "));
    Reply(status_codes::OK, par);
    return;
  }
  auto jBoardDaq = jShmDS["daq"];
  _board_host = jBoardDaq["host"].as_string();
  _board_port = jBoardDaq["port"].as_integer();
  _shmPath = jShmDS["shm_path"].as_string();

  auto jrep = this->post("/CREATE", jBoardDaq["params"]);
  auto w_rep = decode_spyne_answer(jrep, "CREATE");
  //  A FAIRE Recupperer info sur l'initialisation

  if (_dsData != NULL) {
    delete _dsData;
    _dsData = NULL;
  }
  this->clearShm();
  PMF_INFO(_logShmDS, " Create done  ");
  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;
  Reply(status_codes::OK, par);
}

void shm_data_source::fsm_initialise(http_request m) {
  auto par = json::value::object();
  PM_INFO(_logShmDS, "****** CMD: INITIALISING");
  _udpUrl = utils::findUrl(session(), "lyon_udp_data_source", 0);
  // Send Initialise command
  auto jrep = this->post("/INITIALISE");
  auto w_rep = decode_spyne_answer(jrep, "INITIALISE");
  if (_dsData != NULL) {
    delete _dsData;
    _dsData = NULL;
  }
  PMF_INFO(_logShmDS, " Init done  ");
  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;

  Reply(status_codes::OK, par);
}

void shm_data_source::configure(http_request m) {
  auto par = json::value::object();
  PMF_INFO(_logShmDS, " CMD: Configuring");

  // Now Send the CONFIGURE COMMAND

  auto jrep = this->post("/CONFIGURE");

  // A FAIRE RECUPPERER DETID ET SOURCEID

  auto jrepl = this->post("/STATUS");
  auto w_rep = decode_spyne_answer(jrepl, "STATUS");

  _detId = w_rep.as_object()["DETID"].as_integer();
  _sourceId = w_rep.as_object()["SOURCEID"].as_integer();

  // std::cout<<_detId<<" "<<_sourceId<<std::endl;
  if (_context == NULL)
    _context = new zmq::context_t(1);
  if (_dsData == NULL) {
    _dsData = new pm::pmSender(_context, _detId, _sourceId);
    _dsData->autoDiscover(session(), "evb_builder", "collectingPort");
    _dsData->collectorRegister();
  }
  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;

  Reply(status_codes::OK, par);
}

/////////////////////////////////////////////////////////
void shm_data_source::start(http_request m) {
  this->clearShm();
  _running = true;

  g_mon = new std::thread(std::bind(&shm_data_source::spy_shm, this));
  auto par = json::value::object();
  PMF_INFO(_logShmDS, " CMD: STARTING");
  // Now Send the START COMMAND

  auto jrep = this->post("/START");
  auto w_rep = decode_spyne_answer(jrep, "START");
  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;
  Reply(status_codes::OK, par);
}
void shm_data_source::stop(http_request m) {
  auto par = json::value::object();
  PMF_INFO(_logShmDS, " CMD: STOPPING ");

  // Now Send the STOP COMMAND
  auto jrep = this->post("/STOP");
  auto w_rep = decode_spyne_answer(jrep, "STOP");
  if (g_mon != NULL && _running) {
    _running = false;
    g_mon->join();
    delete g_mon;
    g_mon = NULL;
  }

  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;

  Reply(status_codes::OK, par);
}
void shm_data_source::destroy(http_request m) {
  auto par = json::value::object();
  PMF_INFO(_logShmDS, " CMD: DESTROYING");
  // Now Send the DESTROY COMMAND
  auto jrep = this->post("/DESTROY");
  auto w_rep = decode_spyne_answer(jrep, "DESTROY");

  if (g_mon != NULL) {
    _running = false;
    g_mon->join();
    delete g_mon;
    g_mon = NULL;
  }
  par["status"] = json::value::string(U("done"));
  par["answer"] = w_rep;

  Reply(status_codes::OK, par);

  // To be done: _ShmDS->clear();
}

extern "C" {
// loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and
// returns it.
handlerPlugin *loadProcessor(void) { return (new shm_data_source); }
// The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is
// passed to it.  This isn't a very safe function, since there's no way to
// ensure that the object provided is indeed a LowPassDHCALAnalyzer.
void deleteProcessor(handlerPlugin *obj) { delete obj; }
}
