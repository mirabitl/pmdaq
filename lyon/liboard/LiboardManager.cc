
#include "LiboardManager.hh"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/param.h>
//#include "ftdi.hpp"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <string.h>

//using namespace Ftdi;
using namespace liboard;

void LiboardManager::prepareDevices()
{
  for (std::map<uint32_t, liboard::FtdiDeviceInfo *>::iterator it = theFtdiDeviceInfoMap_.begin(); it != theFtdiDeviceInfoMap_.end(); it++)
    if (it->second != NULL)
      delete it->second;
  theFtdiDeviceInfoMap_.clear();
  for (std::map<uint32_t, LiboardInterface *>::iterator it = _LiboardInterfaceMap.begin(); it != _LiboardInterfaceMap.end(); it++)
    if (it->second != NULL)
      delete it->second;
  _LiboardInterfaceMap.clear();
  int ier = system("/bin/rm /var/log/pi/ftdi_devices");
  ier = system("/opt/dhcal/bin/ListDevices.py");
  std::string line;
  std::ifstream myfile("/var/log/pi/ftdi_devices");
  std::stringstream diflist;

  if (myfile.is_open())
  {
    while (myfile.good())
    {
      getline(myfile, line);
      liboard::FtdiDeviceInfo *difi = new liboard::FtdiDeviceInfo();
      memset(difi, 0, sizeof(liboard::FtdiDeviceInfo));
      sscanf(line.c_str(), "%x %x %s", &difi->vendorid, &difi->productid, difi->name);
      if (strncmp(difi->name, "FT101", 5) == 0)
      {
        sscanf(difi->name, "FT101%d", &difi->id);
        difi->type = 0;
        std::pair<uint32_t, liboard::FtdiDeviceInfo *> p(difi->id, difi);
        theFtdiDeviceInfoMap_.insert(p);
      }
      if (strncmp(difi->name, "LI_", 3) == 0)
      {
        sscanf(difi->name, "LI_%d", &difi->id);
        difi->type = 0;
        std::pair<uint32_t, liboard::FtdiDeviceInfo *> p(difi->id, difi);
        theFtdiDeviceInfoMap_.insert(p);
      }
      if (strncmp(difi->name, "DCCCCC", 6) == 0)
      {
        sscanf(difi->name, "DCCCCC%d", &difi->id);
        difi->type = 0x10;
      }
    }
    myfile.close();
  }
  else
  {
    //std::cout << "Unable to open file"<<std::endl;
    PMF_FATAL(_logLiboard, " Unable to open /var/log/pi/ftdi_devices");
  }

  for (auto it = theFtdiDeviceInfoMap_.begin(); it != theFtdiDeviceInfoMap_.end(); it++)
    PMF_INFO(_logLiboard, "Device found and register " << it->first << " with info " << it->second->vendorid << " " << it->second->productid << " " << it->second->name << " " << it->second->type);
}

void LiboardManager::scan(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " CMD: SCanning");
  // Fill Ftdi Map
  _mdcc = NULL;
  this->prepareDevices();
  std::map<uint32_t, liboard::FtdiDeviceInfo *> &fm = this->getFtdiMap();
  std::map<uint32_t, LiboardInterface *> dm = this->getLiboardMap();
  PMF_INFO(_logLiboard, " CMD: SCANDEVICE clear Maps");
  for (auto it = dm.begin(); it != dm.end(); it++)
  {
    if (it->second != NULL)
      delete it->second;
  }
  dm.clear();
  // _ndif=0;
  std::vector<uint32_t> vids;
  web::json::value array;
  uint32_t ndev = 0;
  for (auto it = fm.begin(); it != fm.end(); it++)
  {
    PMF_INFO(_logLiboard, "Creating " << it->second->name);
    LiboardInterface *d = new LiboardInterface(it->second);
    if (_mdcc == NULL)
      _mdcc = d->rd();
    PMF_INFO(_logLiboard, "After Creating " << it->second->name << _mdcc);
    this->getLiboardMap().insert(std::make_pair(it->first, d));
    PMF_INFO(_logLiboard, " CMD: SCANDEVICE created LiboardInterface @ " << std::hex << d << std::dec);
    web::json::value jd;
    jd["detid"] = json::value::number(d->detectorId());
    jd["sourceid"] = json::value::number(it->first);
    vids.push_back((d->detectorId() << 16 | it->first));
    array[ndev++] = jd;
  }

  par["status"] = json::value::string(U("done"));
  par["devices"] = array;
  Reply(status_codes::OK, par);
}

void LiboardManager::fsm_initialise(http_request m)
{
  // _mdcc->maskTrigger();

  auto par = json::value::object();
  PMF_INFO(_logLiboard, " CMD: INITIALISING");
  _vDif.clear();

  if (!utils::isMember(params(), "dif"))
  {
    PMF_ERROR(_logLiboard, " No dif tag in params()");
    par["status"] = json::value::string(U("Missing dif tag"));
    Reply(status_codes::OK, par);
    return;
  }
  auto jLiboard = params()["dif"];

  // Download the configuration

  if (_hca == NULL)
  {
    std::cout << "Create config acccess" << std::endl;
    _hca = new LIROCConfigAccess();
    _hca->clear();
  }
  std::cout << " jLiboard " << jLiboard << std::endl;
  if (utils::isMember(jLiboard, "json"))
  {
    web::json::value jLiboardjson = jLiboard["json"];
    if (utils::isMember(jLiboardjson, "file"))
    {
      _hca->parseJsonFile(jLiboardjson["file"].as_string());
    }
    else if (utils::isMember(jLiboardjson, "url"))
    {
      _hca->parseJsonUrl(jLiboardjson["url"].as_string());
    }
  }
  if (utils::isMember(jLiboard, "db"))
  {
    web::json::value jLiboarddb = jLiboard["db"];
    PMF_ERROR(_logLiboard, "Parsing:" << jLiboarddb["state"].as_string() << jLiboarddb["mode"].as_string());

    if (jLiboarddb["mode"].as_string().compare("mongo") == 0)
      _hca->parseMongoDb(jLiboarddb["state"].as_string(), jLiboarddb["version"].as_integer());

    PMF_ERROR(_logLiboard, "End of parseDB " << _hca->asicMap().size());
  }
  if (_hca->asicMap().size() == 0)
  {
    PMF_ERROR(_logLiboard, " No ASIC found in the configuration ");
    par["status"] = json::value::string(U("No ASIC found in the configuration"));
    Reply(status_codes::OK, par);
    return;
  }
  PMF_INFO(_logLiboard, "ASIC found in the configuration " << _hca->asicMap().size());
  // Initialise the network
  std::map<uint32_t, LiboardInterface *> dm = this->getLiboardMap();
  std::vector<uint32_t> vint;

  vint.clear();
  for (auto x : _hca->asicMap())
  {
    // only MSB is used
    uint32_t eip = ((x.first) >> 56) & 0XFF;
    std::map<uint32_t, LiboardInterface *>::iterator idif = dm.find(eip);
    if (idif == dm.end())
      continue;
    if (std::find(vint.begin(), vint.end(), eip) != vint.end())
      continue;

    PMF_INFO(_logLiboard, " New Liboard found in db " << std::hex << eip << std::dec);
    vint.push_back(eip);

    _vDif.push_back(idif->second);
    PMF_INFO(_logLiboard, " Registration done for " << eip);
  }
  //std::string network=
  // Connect to the event builder
  if (_context == NULL)
    _context = new zmq::context_t(1);

  for (auto x : _vDif)
  {
    PMF_INFO(_logLiboard, " Creating pusher to ");
    /** Old single method
	  zmPusher* push=new zmPusher(_context,x->detectorId(),x->status()->id);
	  push->connect(params()["publish"].as_string());
      */

    pm::pmSender *push = new pm::pmSender(_context, x->detectorId(), x->status()->id);
    //ds->connect(params()["pushdata"].as_string());
    push->autoDiscover(session(), "evb_builder", "collectingPort");
    //for (uint32_t i=0;i<_mStream.size();i++)
    //	ds->connect(_mStream[i]);
    push->collectorRegister();

    x->initialise(push);
    _mdcc = x->rd();
    PMF_INFO(_logLiboard, " Done pusher to ");
  }

  par["status"] = json::value::string(U("Initialised"));
  Reply(status_codes::OK, par);
}

void LiboardManager::setThreshold(uint16_t b0, uint32_t idif)
{

  PMF_INFO(_logLiboard, " Changin thresholds: " << b0);
  for (auto it = _hca->asicMap().begin(); it != _hca->asicMap().end(); it++)
  {
    if (idif != 0)
    {
      uint32_t ip = (((it->first) >> 32 & 0XFFFFFFFF) >> 16) & 0xFFFF;
      printf("%lx %x %x \n", (it->first >> 32), ip, idif);
      if (idif != ip)
        continue;
    }
    it->second.setDAC_threshold(b0);
    //it->second.setHEADER(0x56);
  }
  // Now loop on slowcontrol socket
  this->configureLR();
  ::usleep(10);
}
void LiboardManager::setDC_pa(uint8_t gain)
{

  PMF_INFO(_logLiboard, " Changing Gain: " << gain);
  for (auto it = _hca->asicMap().begin(); it != _hca->asicMap().end(); it++)
  {
    for (int i = 0; i < 64; i++)
      it->second.setDC_pa(i, gain);
  }
  // Now loop on slowcontrol socket
  this->configureLR();
  ::usleep(10);
}
void LiboardManager::setDAC_local(uint8_t dac)
{

  PMF_INFO(_logLiboard, " Changing DAC local: " << dac);
  for (auto it = _hca->asicMap().begin(); it != _hca->asicMap().end(); it++)
  {
    for (int i = 0; i < 64; i++)
      it->second.setDAC_local(i, dac);
  }
  // Now loop on slowcontrol socket
  this->configureLR();
  ::usleep(10);
}

void LiboardManager::setMask(uint64_t mask)
{
  PMF_INFO(_logLiboard, " Changing Mask: " << std::hex << mask << std::dec);
  for (auto it = _hca->asicMap().begin(); it != _hca->asicMap().end(); it++)
  {
    for (int i = 0; i < 64; i++)
      it->second.setMask(i, (mask >> i) & 1);
    it->second.Print();
  }
  // Now loop on slowcontrol socket
  this->configureLR();

  ::usleep(10);
}
void LiboardManager::setChannelMask(uint16_t channel, uint16_t val)
{
  PMF_INFO(_logLiboard, " Changing Mask: " << std::hex << channel << std::dec);
  for (auto it = _hca->asicMap().begin(); it != _hca->asicMap().end(); it++)
  {

    it->second.setMask(channel, val == 1);
  }
  // Now loop on slowcontrol socket
  this->configureLR();

  ::usleep(10);
}

void LiboardManager::setCtest(uint64_t mask)
{

  PMF_INFO(_logLiboard, " Changing Ctest: " << std::hex << mask << std::dec);
  for (auto it = _hca->asicMap().begin(); it != _hca->asicMap().end(); it++)
  {
    for (int i = 0; i < 64; i++)
      it->second.setCtest(i, (mask >> i) & 1);
  }
  // Now loop on slowcontrol socket
  this->configureLR();

  ::usleep(10);

  // Now loop on slowcontrol socket
  this->configureLR();

  ::usleep(10);
}

void LiboardManager::c_setthreshold(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "SetThreshold called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  uint32_t b0 = utils::queryIntValue(m, "B0", 470);
  uint32_t idif = utils::queryIntValue(m, "LIBOARD", 0);

  this->setThreshold(b0, idif);
  par["THRESHOLD0"] = web::json::value::number(b0);
  par["DIF"] = web::json::value::number(idif);
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);
}
void LiboardManager::c_setdcpa(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "SetDC_pa called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  uint32_t gain = utils::queryIntValue(m, "value", 0);
  this->setDC_pa(gain);
  par["DC_pa"] = web::json::value::number(gain);

  Reply(status_codes::OK, par);
}
void LiboardManager::c_setdaclocal(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "SetDACLocal called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  uint32_t gain = utils::queryIntValue(m, "value", 0);
  this->setDAC_local(gain);
  par["DC_pa"] = web::json::value::number(gain);

  Reply(status_codes::OK, par);
}

void LiboardManager::c_setmask(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "SetMask called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  //uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint64_t mask;
  sscanf(utils::queryStringValue(m, "mask", "0XFFFFFFFFFFFFFFFF").c_str(), "%lx", &mask);
  PMF_INFO(_logLiboard, "SetMask called " << std::hex << mask << std::dec);
  this->setMask(mask);
  par["MASK"] = web::json::value::number(mask);

  Reply(status_codes::OK, par);
}
void LiboardManager::c_masktdcchannels(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "mask TDC channels called ");
  par["STATUS"] = web::json::value::string(U("DONE"));
  if (_mdcc == NULL)
  {
    PMF_ERROR(_logLiboard, "Please open MDC01 first");
    par["STATUS"] = web::json::value::string(U("Please open MDC01 first"));
    Reply(status_codes::OK, par);
    return;
  }

  //uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint64_t mask;
  sscanf(utils::queryStringValue(m, "mask", "0XFFFFFFFFFFFFFFFF").c_str(), "%lx", &mask);
  PMF_INFO(_logLiboard, "mask TDC channels called " << std::hex << mask << std::dec);
  _mdcc->maskTdcChannels(mask);
  par["MASK"] = web::json::value::number(mask);

  Reply(status_codes::OK, par);
}
void LiboardManager::c_setlatchdelay(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "Set Latch delay called ");
  par["STATUS"] = web::json::value::string(U("DONE"));
  if (_mdcc == NULL)
  {
    PMF_ERROR(_logLiboard, "Please open MDC01 first");
    par["STATUS"] = web::json::value::string(U("Please open MDC01 first"));
    Reply(status_codes::OK, par);
    return;
  }

  uint32_t nc = utils::queryIntValue(m, "value", 0x10);
  PMF_INFO(_logLiboard, "Latch delay is " << std::hex << nc << std::dec);
  _mdcc->setLatchDelay(nc);
  par["DELAY"] = web::json::value::number(nc);

  Reply(status_codes::OK, par);
}
void LiboardManager::c_setlatchduration(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "Set Latch Duration called ");
  par["STATUS"] = web::json::value::string(U("DONE"));
  if (_mdcc == NULL)
  {
    PMF_ERROR(_logLiboard, "Please open MDC01 first");
    par["STATUS"] = web::json::value::string(U("Please open MDC01 first"));
    Reply(status_codes::OK, par);
    return;
  }

  uint32_t nc = utils::queryIntValue(m, "value", 0x10);
  PMF_INFO(_logLiboard, "Latch duration is " << std::hex << nc << std::dec);
  _mdcc->setLatchDelay(nc);
  par["DELAY"] = web::json::value::number(nc);

  Reply(status_codes::OK, par);
}

void LiboardManager::c_setchannelmask(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "SetMask called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  //uint32_t nc=utils::queryIntValue(m,"value",4294967295);

  uint32_t channel = utils::queryIntValue(m, "channel", 0);
  bool on = utils::queryIntValue(m, "value", 1) == 1;
  PMF_INFO(_logLiboard, "SetMaskChannel called " << channel << std::dec);
  this->setChannelMask(channel, on);
  par["CHANNEL"] = web::json::value::number(channel);
  par["ON"] = web::json::value::number(on);

  Reply(status_codes::OK, par);
}
void LiboardManager::c_external(http_request m)
{
  auto par = json::value::object();
  uint32_t external = utils::queryIntValue(m, "value", 0);
  PMF_INFO(_logLiboard, "EXTERNAL called " << external);

  if (external != 0)
    params()["external"] = external;
  std::map<uint32_t, LiboardInterface *> dm = this->getLiboardMap();
  for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
  {
    it->second->setExternalTrigger((external == 1));
  }

  //PMF_INFO(_logLiboard,"CTRLREG called "<<std::hex<<ctrlreg<<std::dec);
  par["STATUS"] = web::json::value::string(U("DONE"));
  par["TRIGGEREXT"] = web::json::value::number(external);
  Reply(status_codes::OK, par);
}
void LiboardManager::c_downloadDB(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "downloadDB called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  std::string dbstate = utils::queryStringValue(m, "state", "NONE");
  uint32_t version = utils::queryIntValue(m, "version", 0);
  web::json::value jTDC = params()["dif"];
  if (utils::isMember(jTDC, "db"))
  {
    web::json::value jTDCdb = jTDC["db"];
    _hca->clear();

    if (jTDCdb["mode"].as_string().compare("mongo") == 0)
      _hca->parseMongoDb(dbstate, version);
  }
  par["DBSTATE"] = web::json::value::string(dbstate);
  Reply(status_codes::OK, par);
}

void LiboardManager::c_status(http_request m)
{
  auto par = json::value::object();
  int32_t rc = 1;
  std::map<uint32_t, LiboardInterface *> dm = this->getLiboardMap();
  web::json::value array_slc;
  uint32_t nd = 0;

  for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
  {

    web::json::value ds;
    ds["detid"] = json::value::number(it->second->detectorId());
    ds["state"] = json::value::string(U(it->second->state()));
    ds["id"] = json::value::number(it->second->status()->id);
    ds["status"] = json::value::number(it->second->status()->status);
    ds["slc"] = json::value::number(it->second->status()->slc);
    ds["gtc"] = json::value::number(it->second->status()->gtc);
    ds["bcid"] = json::value::number(it->second->status()->bcid);
    ds["bytes"] = json::value::number(it->second->status()->bytes);
    ds["host"] = json::value::string(U(std::string((it->second->status()->host))));
    array_slc[nd++] = ds;
  }
  par["STATUS"] = web::json::value::string(U("DONE"));
  par["DIFLIST"] = array_slc;
  Reply(status_codes::OK, par);

  return;
}

web::json::value LiboardManager::configureLR()
{
  /// A reecrire
  // uint32_t external=params()["external"].as_integer();
  // printf("TRigger EXT %x \n",external);
  int32_t rc = 1;
  std::map<uint32_t, LiboardInterface *> dm = this->getLiboardMap();
  web::json::value array_slc;
  uint32_t nd = 0;

  for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
  {
    std::stringstream ips;
    // Dummy IP address for Liboards
    ips << "0.0.0." << it->first;
    PMF_INFO(_logLiboard, "Configuring" << it->first << " " << it->second);

    _hca->prepareSlowControl(ips.str());

    it->second->configure(_hca->slcBuffer(), _hca->slcWords());
    web::json::value ds;
    ds["id"] = json::value::number(it->first);
    ds["slc"] = json::value::number(it->second->status()->slc);
    array_slc[nd++] = ds;
  }
  return array_slc;
}

void LiboardManager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " CMD: Configuring");

  int32_t rc = 1;

  web::json::value array_slc = this->configureLR();

  par["status"] = json::value::string(U("done"));
  par["devices"] = array_slc;
  Reply(status_codes::OK, par);

  return;
}

void LiboardManager::start(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Starting ");

  int32_t rc = 1;
  std::map<uint32_t, LiboardInterface *> dm = this->getLiboardMap();
  for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
  {
    this->startReadoutThread(it->second);
    it->second->start();
  }
  _running = true;
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);

  return;
}
void LiboardManager::stop(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Stopping ");

  int32_t rc = 1;
  _running = false;
  if (_sc_running)
  {
    _sc_running = false;
    g_scurve.join();
  }
  _sc_running = false;
  ::usleep(100000);
  std::map<uint32_t, LiboardInterface *> dm = this->getLiboardMap();

  for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
  {
    PMF_INFO(_logLiboard, " Stopping thread of Liboard" << it->first);
    it->second->stop();
  }

  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);

  return;
}
void LiboardManager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Destroying ");

  int32_t rc = 1;
  std::map<uint32_t, LiboardInterface *> dm = this->getLiboardMap();

  bool running = false;
  for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
  {
    running = running || it->second->readoutStarted();
  }
  if (running)
  {
    for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {
      it->second->setReadoutStarted(false);
    }

    this->joinThreads();
    // Clear thread vector
    g_d.clear();
  }
  for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
  {
    it->second->destroy();
  }

  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);

  return;
}

LiboardManager::LiboardManager() : _running(false), _sc_running(false) { ; }

void LiboardManager::initialise()
{

  // Zmq transport
  _context = new zmq::context_t(1);
  // Register state
  this->addState("SCANNED");
  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  this->addState("STOPPED");
  this->addTransition("SCAN", "CREATED", "SCANNED", std::bind(&LiboardManager::scan, this, std::placeholders::_1));
  this->addTransition("INITIALISE", "SCANNED", "INITIALISED", std::bind(&LiboardManager::fsm_initialise, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "INITIALISED", "CONFIGURED", std::bind(&LiboardManager::configure, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "CONFIGURED", "CONFIGURED", std::bind(&LiboardManager::configure, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "STOPPED", "CONFIGURED", std::bind(&LiboardManager::configure, this, std::placeholders::_1));
  this->addTransition("START", "CONFIGURED", "RUNNING", std::bind(&LiboardManager::start, this, std::placeholders::_1));
  this->addTransition("START", "STOPPED", "RUNNING", std::bind(&LiboardManager::start, this, std::placeholders::_1));
  this->addTransition("STOP", "RUNNING", "STOPPED", std::bind(&LiboardManager::stop, this, std::placeholders::_1));
  this->addTransition("DESTROY", "STOPPED", "CREATED", std::bind(&LiboardManager::destroy, this, std::placeholders::_1));
  this->addTransition("DESTROY", "CONFIGURED", "CREATED", std::bind(&LiboardManager::destroy, this, std::placeholders::_1));

  this->addCommand("STATUS", std::bind(&LiboardManager::c_status, this, std::placeholders::_1));
  this->addCommand("SETTHRESHOLD", std::bind(&LiboardManager::c_setthreshold, this, std::placeholders::_1));
  this->addCommand("SETDCPA", std::bind(&LiboardManager::c_setdcpa, this, std::placeholders::_1));
  this->addCommand("SETDACLOCAL", std::bind(&LiboardManager::c_setdaclocal, this, std::placeholders::_1));
  this->addCommand("SETMASK", std::bind(&LiboardManager::c_setmask, this, std::placeholders::_1));
  this->addCommand("SETCHANNELMASK", std::bind(&LiboardManager::c_setchannelmask, this, std::placeholders::_1));
  this->addCommand("DOWNLOADDB", std::bind(&LiboardManager::c_downloadDB, this, std::placeholders::_1));
  this->addCommand("TRIGEXT", std::bind(&LiboardManager::c_external, this, std::placeholders::_1));
  this->addCommand("SCURVE", std::bind(&LiboardManager::c_scurve, this, std::placeholders::_1));
  this->addCommand("SCURVE1", std::bind(&LiboardManager::c_scurve1, this, std::placeholders::_1));

  this->addCommand("MASKTDCCHANNELS", std::bind(&LiboardManager::c_masktdcchannels, this, std::placeholders::_1));
  this->addCommand("SETLATCHDELAY", std::bind(&LiboardManager::c_setlatchdelay, this, std::placeholders::_1));
  this->addCommand("SETLATCHDURATION", std::bind(&LiboardManager::c_setlatchduration, this, std::placeholders::_1));

  // MDCC stuff

  this->addCommand("PAUSE", std::bind(&LiboardManager::c_pause, this, std::placeholders::_1));
  this->addCommand("RESUME", std::bind(&LiboardManager::c_resume, this, std::placeholders::_1));
  this->addCommand("RESET", std::bind(&LiboardManager::c_reset, this, std::placeholders::_1));
  this->addCommand("WRITEREG", std::bind(&LiboardManager::c_writereg, this, std::placeholders::_1));
  this->addCommand("READREG", std::bind(&LiboardManager::c_readreg, this, std::placeholders::_1));
  this->addCommand("MDCCSTATUS", std::bind(&LiboardManager::c_mdccstatus, this, std::placeholders::_1));
  this->addCommand("SPILLON", std::bind(&LiboardManager::c_spillon, this, std::placeholders::_1));
  this->addCommand("SPILLOFF", std::bind(&LiboardManager::c_spilloff, this, std::placeholders::_1));
  this->addCommand("BEAMON", std::bind(&LiboardManager::c_beamon, this, std::placeholders::_1));

  this->addCommand("RESETTDC", std::bind(&LiboardManager::c_resettdc, this, std::placeholders::_1));
  this->addCommand("CALIBON", std::bind(&LiboardManager::c_calibon, this, std::placeholders::_1));
  this->addCommand("CALIBOFF", std::bind(&LiboardManager::c_caliboff, this, std::placeholders::_1));
  this->addCommand("RELOADCALIB", std::bind(&LiboardManager::c_reloadcalib, this, std::placeholders::_1));
  this->addCommand("SETCALIBCOUNT", std::bind(&LiboardManager::c_setcalibcount, this, std::placeholders::_1));
  this->addCommand("SETSPILLREGISTER", std::bind(&LiboardManager::c_setspillregister, this, std::placeholders::_1));
  this->addCommand("SETCALIBREGISTER", std::bind(&LiboardManager::c_setcalibregister, this, std::placeholders::_1));
  this->addCommand("SETHARDRESET", std::bind(&LiboardManager::c_sethardreset, this, std::placeholders::_1));
  this->addCommand("SETTRIGEXT", std::bind(&LiboardManager::c_settrigext, this, std::placeholders::_1));
  this->addCommand("SETEXTERNAL", std::bind(&LiboardManager::c_setexternaltrigger, this, std::placeholders::_1));

  _hca = NULL;
  // Initialise delays for
}

void LiboardManager::end()
{
  // Stop any running process
  if (_sc_running)
  {
    _sc_running = false;
    g_scurve.join();
  }
  //Stop listening
  if (g_d.size() != 0)
  {

    bool running = false;
    std::map<uint32_t, LiboardInterface *> dm = this->getLiboardMap();
    for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {
      running = running || it->second->readoutStarted();
    }
    if (running)
    {
      for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
      {
        it->second->setReadoutStarted(false);
      }

      this->joinThreads();
    }
    // Clear thread vector
    g_d.clear();
  }
}

void LiboardManager::startReadoutThread(LiboardInterface *d)
{
  if (d->readoutStarted())
    return;
  d->setReadoutStarted(true);
  g_d.push_back(std::thread(std::bind(&LiboardInterface::readout, d)));
}

void LiboardManager::ScurveStep(std::string builder, int thmin, int thmax, int step)
{
  std::map<uint32_t, LiboardInterface *> dm = this->getLiboardMap();
  //int ncon = 1500, ncoff = 100000, ntrg = 5;
  _mdcc->maskTrigger();
  web::json::value p;
  _mdcc->setSpillOn(_sc_spillon);
  _mdcc->setSpillOff(_sc_spilloff);
  _mdcc->setSpillRegister(4); //4 normalement
  _mdcc->calibOn();
  _mdcc->setCalibCount(_sc_ntrg);
  int thrange = (thmax - thmin + 1) / step;
  for (int vth = 0; vth <= thrange; vth++)
  {
  debut:
    if (!_sc_running)
      break;
    if (!_running)
      break;

    _mdcc->maskTrigger();

    usleep(100000);
    this->setThreshold(thmax - vth * step);
    usleep(100000);

    web::json::value h;
    web::json::value ph;
    h[0] = json::value::number(2);
    h[1] = json::value::number(thmax - vth * step);

    int firstEvent = 0;

#define USEBOARDS
#ifdef USEBOARDS
    for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)

      if (it->second->rd()->last_read() > firstEvent)
        firstEvent = it->second->rd()->last_read();
#else
    auto frep = utils::sendCommand(builder, "STATUS", json::value::null());
    auto jfrep = frep.extract_json();
    auto jfanswer = jfrep.get().as_object()["answer"];
    firstEvent = jfanswer["event"].as_integer();
#endif

    ph["header"] = h;
    ph["nextevent"] = json::value::number(firstEvent + 1);
    utils::sendCommand(builder, "SETHEADER", ph);
    _mdcc->reloadCalibCount();
    //::usleep(500000);
    _mdcc->unmaskTrigger();
    int nloop = 0, lastEvent = firstEvent;
#ifdef USEBOARDS
    while (lastEvent < (firstEvent + _sc_ntrg -1))
    {
      ::usleep(10000);
      for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)

        if (it->second->rd()->last_read() > lastEvent)
          lastEvent = it->second->rd()->last_read();
      nloop++;
      if (nloop > 1000 || !_running || !_sc_running)
        break;
    }
#else
    while (lastEvent < (firstEvent + _sc_ntrg - 2) && _sc_running)
    {
      ::usleep(10000);
      auto rep = utils::sendCommand(builder, "STATUS", json::value::null());
      auto jrep = rep.extract_json();
      auto janswer = jrep.get().as_object()["answer"];
      lastEvent = janswer["event"].as_integer(); // A verifier
      nloop++;
      if (nloop > 1000 || !_running || !_sc_running)
        break;
    }
#endif
    PMF_ERROR(_logLiboard, "Calibration Step " << vth << " " << firstEvent << " " << lastEvent << " SC RUNNING " << (int)_sc_running << " " << (int)_running);
    printf("Step %d Th %d First %d Last %d \n", vth, thmax - vth * step, firstEvent, lastEvent);
    _mdcc->maskTrigger();
    if ((lastEvent - firstEvent) < 3)
      goto debut;
  }
  _mdcc->calibOff();
}

void LiboardManager::ScurveStandalone(uint32_t mode, int thmin, int thmax, int step, bool usectest)
{
  std::map<uint32_t, LiboardInterface *> dm = this->getLiboardMap();

  uint64_t mask = 0;

  // All channel pedestal
  if (mode == 255)
  {

    //for (int i=0;i<64;i++) mask|=(1<<i);
    mask = 0;
    this->setMask(mask);
  }

  // Chanel per channel pedestal (CTEST is active)
  else if (mode == 1023)
  {
    mask = 0;
    for (int i = 0; i < 64; i++)
    {
      uint64_t maskctest=(1ULL<<i);
      mask = ~(1ULL << i);
      std::cout << "Step LR " << i << " channel " << i << std::endl;
      this->setMask(mask);
      if (usectest)
        this->setCtest(maskctest);
    }
  }

  // One channel pedestal
  else
  {
    uint64_t maskctest=(1ULL<<mode);
    mask = ~(1ULL << mode);
    PMF_INFO(_logLiboard, "CTEST One " << mode << " " << std::hex << mask << std::dec);
    this->setMask(mask);
    if (usectest)
      this->setCtest(maskctest);
  }
  int ncon = 150, ncoff = 10000, ntrg = 10;
  _mdcc->maskTrigger();
  web::json::value p;
  _mdcc->setSpillOn(ncon);
  _mdcc->setSpillOff(ncoff);
  _mdcc->setSpillRegister(4); //4 normalement
  _mdcc->calibOn();
  _mdcc->setCalibCount(ntrg);
  int thrange = (thmax - thmin + 1) / step;
  int firstEvent = 0;

  for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
  {
    it->second->rd()->analyze_init();
    it->second->start();
    it->second->rd()->resetFSM();
  }
  for (int vth = 0; vth <= thrange; vth++)
  {
    _mdcc->maskTrigger();

    usleep(10000);
    this->setThreshold(thmax - vth * step);
    for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
      it->second->rd()->set_vth(thmax - vth * step);
    usleep(10000);
    _mdcc->reloadCalibCount();
    _mdcc->unmaskTrigger();
    usleep(10000);

    int nloop = 0, lastEvent = firstEvent;
    uint8_t cbuf[0x4000000];
    for (int iev = 0; iev < ntrg; iev++)
    {
    debut:
      int nr = 0, nl = 0;
      while (nr == 0)
      {
        for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
        {
          nr = it->second->rd()->readOneEvent(cbuf);
          if (nr != 0)
          {
            PMF_ERROR(_logLiboard, "Calibration " << it->second->rd()->last_read() << " " << it->second->rd()->vth_set());
          }
          else
            ::usleep(10000);
        }
        nl++;
        if (nl > 100)
          break;
      }
    }

    _mdcc->maskTrigger();
  }
  _mdcc->calibOff();
  for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
  {
    it->second->stop();
  }
}

void LiboardManager::thrd_scurve()
{
  _sc_running = true;
  this->Scurve(_sc_channel, _sc_thmin, _sc_thmax, _sc_step);
  _sc_running = false;
}

void LiboardManager::Scurve(int mode, int thmin, int thmax, int step)
{
  std::string builderUrl = utils::findUrl(session(), "evb_builder", 0);
  if (builderUrl.compare("") == 0)
    return;

  uint64_t mask = 0;

  // All channel pedestal
  if (mode== 255)
  {

    // Do not apply any mask but the one in the DB
    this->ScurveStep(builderUrl, thmin, thmax, step);
    return;
  }

  // Chanel per channel pedestal (CTEST is active)
  if (mode == 1023)
  {
    mask = 0;
    for (int i = 0; i < 64; i++)
    {
      mask = (1ULL << i);
      PMF_INFO(_logLiboard, "SCURVE Channel" << i << " " << std::hex << mask << std::dec);
      this->setMask(mask);
      if (_sc_ctest) this->setCtest(mask);
      this->ScurveStep(builderUrl, thmin, thmax, step);
    }
    return;
  }

  // One channel pedestal

  mask = ~(1ULL << mode);
  PMF_INFO(_logLiboard, "CTEST One " << mode << " " << std::hex << mask << std::dec);
  this->setMask(mask);
  if (_sc_ctest)  this->setCtest(mask);
  this->ScurveStep(builderUrl, thmin, thmax, step);
}

void LiboardManager::c_scurve(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = web::json::value::string(U("DONE"));

  _sc_thmin = utils::queryIntValue(m, "first", 80);
  _sc_thmax = utils::queryIntValue(m, "last", 250);
  _sc_step = utils::queryIntValue(m, "step", 1);
  _sc_channel = utils::queryIntValue(m, "channel", 255);
  _sc_ctest = (utils::queryIntValue(m, "ctest", 0) == 1);
  _sc_spillon = utils::queryIntValue(m, "spillon", 150);
  _sc_spilloff= utils::queryIntValue(m, "spilloff", 10000);
  _sc_ntrg= utils::queryIntValue(m, "ntrg", 5);

  PMF_INFO(_logLiboard, " SCURVE/CTEST " << _sc_channel << " " << _sc_step << " " << _sc_thmin << " " << _sc_thmax);

  //this->Scurve(mode,first,last,step);

  
  if (_sc_running)
  {
    par["SCURVE"] = web::json::value::string(U("ALREADY_RUNNING"));
    return;
  }
  _sc_running = true;
  g_scurve = std::thread(std::bind(&LiboardManager::thrd_scurve, this));
  par["SCURVE"] = web::json::value::string(U("RUNNING"));

  Reply(status_codes::OK, par);
}
void LiboardManager::c_scurve1(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = web::json::value::string(U("DONE"));

  uint32_t first = utils::queryIntValue(m, "first", 80);
  uint32_t last = utils::queryIntValue(m, "last", 250);
  uint32_t step = utils::queryIntValue(m, "step", 1);
  uint32_t mode = utils::queryIntValue(m, "channel", 255);
  bool ctest = (utils::queryIntValue(m, "ctest", 0) == 1);

  PMF_INFO(_logLiboard, " SCURVE1/CTEST " << mode << " " << step << " " << first << " " << last << " " << ctest);

  this->ScurveStandalone(mode, first, last, step, ctest);

  par["SCURVE"] = web::json::value::string(U("RUNNING"));
  auto res = json::value();
  for (std::map<uint32_t, LiboardInterface *>::iterator it = this->getLiboardMap().begin(); it != this->getLiboardMap().end(); it++)
  {
    uint16_t *scb = it->second->rd()->scurve();
    int ch = mode;
    if (mode == 255 || mode == 1023)
      ch = 39;
    for (int i = first; i <= last;)
    {
      fprintf(stderr, "%d %d ", i, scb[1024 * ch + i]);
      i += 1;
    }
    //for (int i=0;i<64*1024;i++)
    //res[i]= web::json::value::number(scb[i]);
  }
  par["RESULTS"] = res;
  Reply(status_codes::OK, par);
}
void LiboardManager::c_pause(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Pause called ");

  if (_mdcc == NULL)
  {
    PMF_ERROR(_logLiboard, "Please open MDC01 first");
    par["STATUS"] = web::json::value::string(U("Please open MDC01 first"));
    Reply(status_codes::OK, par);
    return;
  }
  _mdcc->maskTrigger();
  ::usleep(10000);
  auto dm = this->getLiboardMap();
  for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
  {
    it->second->pause();
  }
  par["STATUS"] = web::json::value::string(U("DONE"));

  Reply(status_codes::OK, par);
}
void LiboardManager::c_resume(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Resume called ");

  if (_mdcc == NULL)
  {
    PMF_ERROR(_logLiboard, "Please open MDC01 first");
    par["STATUS"] = web::json::value::string(U("Please open MDC01 first"));
    Reply(status_codes::OK, par);
    return;
  }
  _mdcc->unmaskTrigger();
  ::usleep(10000);
  auto dm = this->getLiboardMap();
  for (std::map<uint32_t, LiboardInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
  {
    it->second->resume();
  }

  par["STATUS"] = web::json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}
void LiboardManager::c_calibon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Calib On called ");
  if (_mdcc == NULL)
  {
    PMF_ERROR(_logLiboard, "Please open MDC01 first");
    par["STATUS"] = web::json::value::string(U("Please open MDC01 first"));
    Reply(status_codes::OK, par);
    return;
  }
  _mdcc->calibOn();
  par["STATUS"] = web::json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}
void LiboardManager::c_caliboff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Calib Off called ");
  if (_mdcc == NULL)
  {
    PMF_ERROR(_logLiboard, "Please open MDC01 first");
    par["STATUS"] = web::json::value::string(U("Please open MDC01 first"));
    Reply(status_codes::OK, par);
    return;
  }
  _mdcc->calibOff();
  par["STATUS"] = web::json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}
void LiboardManager::c_reloadcalib(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Calib reload called ");
  if (_mdcc == NULL)
  {
    PMF_ERROR(_logLiboard, "Please open MDC01 first");
    par["STATUS"] = web::json::value::string(U("Please open MDC01 first"));
    Reply(status_codes::OK, par);
    return;
  }
  _mdcc->reloadCalibCount();
  par["STATUS"] = web::json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}

void LiboardManager::c_setcalibcount(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Calib count called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t nc = utils::queryIntValue(m, "nclock", 5000000);
  _mdcc->setCalibCount(nc);

  par["STATUS"] = web::json::value::string(U("DONE"));
  par["NCLOCK"] = web::json::value::number(nc);
  Reply(status_codes::OK, par);
}

void LiboardManager::c_reset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " RESET called ");
  if (_mdcc == NULL)
  {
    PMF_ERROR(_logLiboard, "Please open MDC01 first");
    par["STATUS"] = web::json::value::string(U("Please open MDC01 first"));
    Reply(status_codes::OK, par);
    return;
  }
  _mdcc->resetCounter();
  par["STATUS"] = web::json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}

void LiboardManager::c_readreg(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "Read Register called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t adr = utils::queryIntValue(m, "address", 2);
  uint32_t val = _mdcc->registerRead(adr);

  par["STATUS"] = web::json::value::string(U("DONE"));
  par["ADDRESS"] = web::json::value::number(adr);
  par["VALUE"] = web::json::value::number(val);
  Reply(status_codes::OK, par);
}
void LiboardManager::c_writereg(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Write Register called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t adr = utils::queryIntValue(m, "address", 2);
  uint32_t value = utils::queryIntValue(m, "value", 1234);
  _mdcc->registerWrite(adr, value);
  PMF_INFO(_logLiboard, " Write Register called " << std::hex << adr << " " << value << std::dec);

  par["STATUS"] = web::json::value::string(U("DONE"));
  par["ADDRESS"] = web::json::value::number(adr);
  par["VALUE"] = web::json::value::number(value);
  Reply(status_codes::OK, par);
}
void LiboardManager::c_spillon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Spill ON called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t nc = utils::queryIntValue(m, "nclock", 50);
  _mdcc->setSpillOn(nc);

  par["STATUS"] = web::json::value::string(U("DONE"));
  par["NCLOCK"] = web::json::value::number(nc);
  Reply(status_codes::OK, par);
}
void LiboardManager::c_spilloff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Spill Off called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t nc = utils::queryIntValue(m, "nclock", 5000);
  _mdcc->setSpillOff(nc);

  par["STATUS"] = web::json::value::string(U("DONE"));
  par["NCLOCK"] = web::json::value::number(nc);
  Reply(status_codes::OK, par);
}
void LiboardManager::c_resettdc(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Reset TDC called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t nc = utils::queryIntValue(m, "value", 0);
  _mdcc->resetTDC(nc & 0xF);

  par["STATUS"] = web::json::value::string(U("DONE"));
  //par["NCLOCK"]=web::json::value::number(nc);
  Reply(status_codes::OK, par);
}

void LiboardManager::c_beamon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " beam on time called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t nc = utils::queryIntValue(m, "nclock", 5000000);
  _mdcc->setBeam(nc);

  par["STATUS"] = web::json::value::string(U("DONE"));
  par["NCLOCK"] = web::json::value::number(nc);
  Reply(status_codes::OK, par);
}

void LiboardManager::c_sethardreset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Hard reset called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t nc = utils::queryIntValue(m, "value", 0);
  _mdcc->setHardReset(nc);

  par["STATUS"] = web::json::value::string(U("DONE"));
  par["VALUE"] = web::json::value::number(nc);
  Reply(status_codes::OK, par);
}

void LiboardManager::c_setspillregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "Spill register called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t nc = utils::queryIntValue(m, "value", 0);
  _mdcc->setSpillRegister(nc);

  par["STATUS"] = web::json::value::string(U("DONE"));
  par["VALUE"] = web::json::value::number(nc);
  Reply(status_codes::OK, par);
}
void LiboardManager::c_setexternaltrigger(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "Spill register called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t nc = utils::queryIntValue(m, "value", 0);
  _mdcc->setExternalTrigger(nc);

  par["STATUS"] = web::json::value::string(U("DONE"));
  par["VALUE"] = web::json::value::number(nc);
  Reply(status_codes::OK, par);
}
void LiboardManager::c_setcalibregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, "Calib register called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t nc = utils::queryIntValue(m, "value", 0);
  PMF_INFO(_logLiboard, "Calib register called " << nc);
  _mdcc->setCalibRegister(nc);

  par["STATUS"] = web::json::value::string(U("DONE"));
  par["VALUE"] = web::json::value::number(nc);
  Reply(status_codes::OK, par);
}

void LiboardManager::c_settrigext(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Trig ext setting called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  uint32_t delay = utils::queryIntValue(m, "delay", 20);
  uint32_t busy = utils::queryIntValue(m, "busy", 20);
  _mdcc->setTriggerDelay(delay);
  _mdcc->setTriggerBusy(busy);

  par["STATUS"] = web::json::value::string(U("DONE"));
  par["DELAY"] = web::json::value::number(delay);
  par["BUSY"] = web::json::value::number(busy);
  Reply(status_codes::OK, par);
}

void LiboardManager::c_mdccstatus(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard, " Status called ");
  if (_mdcc == NULL)
  {
    par["STATUS"] = web::json::value::string(U("NO Mdcc created"));
    Reply(status_codes::OK, par);
    return;
  }
  web::json::value rc;
  rc["version"] = json::value::number(_mdcc->version());
  rc["mask"] = json::value::number(_mdcc->mask());
  rc["hard"] = json::value::number(_mdcc->hardReset());
  rc["spill"] = json::value::number(_mdcc->spillCount());
  rc["busy0"] = json::value::number(_mdcc->busyCount(0));
  rc["busy1"] = json::value::number(_mdcc->busyCount(1));
  rc["busy2"] = json::value::number(_mdcc->busyCount(2));
  rc["spillon"] = json::value::number(_mdcc->spillOn());
  rc["spilloff"] = json::value::number(_mdcc->spillOff());
  rc["ecalmask"] = json::value::number(_mdcc->ecalmask());
  rc["beam"] = json::value::number(_mdcc->beam());
  rc["calib"] = json::value::number(_mdcc->calibCount());
  rc["spillreg"] = json::value::number(_mdcc->spillRegister());
  rc["trigdelay"] = json::value::number(_mdcc->triggerDelay());
  rc["external"] = json::value::number(_mdcc->externalTrigger());
  par["COUNTERS"] = rc;
  par["STATUS"] = web::json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}

extern "C"
{
  // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.
  handlerPlugin *loadProcessor(void)
  {
    return (new LiboardManager);
  }
  // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed
  // to it.  This isn't a very safe function, since there's no
  // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin *obj)
  {
    delete obj;
  }
}
