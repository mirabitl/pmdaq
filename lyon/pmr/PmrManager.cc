
#include "PmrManager.hh"
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

// using namespace Ftdi;
using namespace Pmr;

void PmrManager::prepareDevices()
{
  for (std::map<uint32_t, Pmr::FtdiDeviceInfo *>::iterator it = theFtdiDeviceInfoMap_.begin(); it != theFtdiDeviceInfoMap_.end(); it++)
    if (it->second != NULL)
      delete it->second;
  theFtdiDeviceInfoMap_.clear();
  for (std::map<uint32_t, PmrInterface *>::iterator it = _PmrInterfaceMap.begin(); it != _PmrInterfaceMap.end(); it++)
    if (it->second != NULL)
      delete it->second;
  _PmrInterfaceMap.clear();
  int ier = system("/bin/rm /var/log/pi/ftdi_devices");
  ier = system("/usr/local/bin/ListDevices.py");
  std::string line;
  std::ifstream myfile("/var/log/pi/ftdi_devices");
  std::stringstream diflist;

  if (myfile.is_open())
    {
      while (myfile.good())
	{
	  getline(myfile, line);
	  Pmr::FtdiDeviceInfo *difi = new Pmr::FtdiDeviceInfo();
	  memset(difi, 0, sizeof(Pmr::FtdiDeviceInfo));
	  sscanf(line.c_str(), "%x %x %s", &difi->vendorid, &difi->productid, difi->name);
	  if (strncmp(difi->name, "FT101", 5) == 0)
	    {
	      sscanf(difi->name, "FT101%d", &difi->id);
	      difi->type = 0;
	      std::pair<uint32_t, Pmr::FtdiDeviceInfo *> p(difi->id, difi);
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
      // std::cout << "Unable to open file"<<std::endl;
      PMF_FATAL(_logPmr, " Unable to open /var/log/pi/ftdi_devices");
    }

  for (auto it = theFtdiDeviceInfoMap_.begin(); it != theFtdiDeviceInfoMap_.end(); it++)
    PMF_INFO(_logPmr, "Device found and register " << it->first << " with info " << it->second->vendorid << " " << it->second->productid << " " << it->second->name << " " << it->second->type);
}

void PmrManager::scan(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPmr, " CMD: SCanning");
  // Fill Ftdi Map
  this->prepareDevices();
  std::map<uint32_t, Pmr::FtdiDeviceInfo *> &fm = this->getFtdiMap();
  std::map<uint32_t, PmrInterface *> dm = this->getPmrMap();
  PMF_INFO(_logPmr, " CMD: SCANDEVICE clear Maps");
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
      PMF_INFO(_logPmr, "Creating " << it->second->name);
      PmrInterface *d = new PmrInterface(it->second);
      PMF_INFO(_logPmr, "After Creating " << it->second->name);
      this->getPmrMap().insert(std::make_pair(it->first, d));
      PMF_INFO(_logPmr, " CMD: SCANDEVICE created PmrInterface @ " << std::hex << d << std::dec);
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

void PmrManager::fsm_initialise(http_request m)
{

  auto par = json::value::object();
  PMF_INFO(_logPmr, " CMD: INITIALISING");
  _vDif.clear();
  if (!utils::isMember(params(), "dif"))
    {
      PMF_ERROR(_logPmr, " No dif tag in params()");
      par["status"] = json::value::string(U("Missing dif tag"));
      Reply(status_codes::OK, par);
      return;
    }
  auto jPmr = params()["dif"];

  // Download the configuration
  /*
  if (_hca == NULL)
    {
      std::cout << "Create config acccess" << std::endl;

      _hca = new HR2ConfigAccess();
      _hca.clear();

      
    }
  */
  std::cout << " jPmr " << jPmr << std::endl;
  if (utils::isMember(jPmr, "json"))
    {
      web::json::value jPmrjson = jPmr["json"];
      if (utils::isMember(jPmrjson, "file"))
	{
	  _hca.parseJsonFile(jPmrjson["file"].as_string());
	}
      else if (utils::isMember(jPmrjson, "url"))
	{
	  _hca.parseJsonUrl(jPmrjson["url"].as_string());
	}
    }
  if (utils::isMember(jPmr, "db"))
    {
      web::json::value jPmrdb = jPmr["db"];
      PMF_ERROR(_logPmr, "Parsing:" << jPmrdb["state"].as_string() << jPmrdb["mode"].as_string());

      if (jPmrdb["mode"].as_string().compare("mongo") == 0)
	_hca.parseMongoDb2(jPmrdb["state"].as_string(), jPmrdb["version"].as_integer());

      PMF_ERROR(_logPmr, "End of parseDB " << _hca.asicMap().size());
    }


  
  if (_hca.asicMap().size() == 0)
    {
      PMF_ERROR(_logPmr, " No ASIC found in the configuration ");
      par["status"] = json::value::string(U("No ASIC found in the configuration"));
      Reply(status_codes::OK, par);
      return;
    }
  PMF_INFO(_logPmr, "ASIC found in the configuration " << _hca.asicMap().size());
  // Initialise the network
  std::map<uint32_t, PmrInterface *> dm = this->getPmrMap();
  std::vector<uint32_t> vint;

  vint.clear();
  for (auto x : _hca.asicMap())
    {
      // only MSB is used
      uint32_t eip = ((x.first) >> 56) & 0XFF;
      std::map<uint32_t, PmrInterface *>::iterator idif = dm.find(eip);
      if (idif == dm.end())
	continue;
      if (std::find(vint.begin(), vint.end(), eip) != vint.end())
	continue;

      PMF_INFO(_logPmr, " New Pmr found in db " << std::hex << eip << std::dec);
      vint.push_back(eip);

      _vDif.push_back(idif->second);
      PMF_INFO(_logPmr, " Registration done for " << eip);
    }
  // std::string network=
  //  Connect to the event builder
  if (_context == NULL)
    _context = new zmq::context_t(1);

  for (auto x : _vDif)
    {
      PMF_INFO(_logPmr, " Creating pusher for "<<x->detectorId()<<" "<< x->status()->id);
      /** Old single method
	  zmPusher* push=new zmPusher(_context,x->detectorId(),x->status()->id);
	  push->connect(params()["publish"].as_string());
      */
      //pm::pmSender *push =NULL;
      pm::pmSender *push = new pm::pmSender(_context, x->detectorId(), x->status()->id);
      // ds->connect(params()["pushdata"].as_string());
      push->autoDiscover(session(), "evb_builder", "collectingPort");
      // for (uint32_t i=0;i<_mStream.size();i++)
      //	ds->connect(_mStream[i]);
      push->collectorRegister();

      PMF_INFO(_logPmr, " Call initialise "<<x->detectorId()<<" "<< x->status()->id);
      x->initialise(push);
    }


  if (utils::isMember(jPmr, "geom"))
    {
      for (uint32_t idif=0;idif<jPmr["geom"].as_array().size();idif++)
	{
	  auto jsd=jPmr["geom"][idif].as_array();
	  uint32_t jid=jsd[0].as_integer();
	  uint32_t bp=jsd[1].as_integer();
	  
	  for (auto x : _vDif)
	      if (x->id()==jid)
		{x->leftRight(bp);break;}
	    
	}
    }
  par["status"] = json::value::string(U("initialised"));

  Reply(status_codes::OK, par);
}

void PmrManager::setThresholds(uint16_t b0, uint16_t b1, uint16_t b2, uint32_t idif)
{

  PMF_INFO(_logPmr, " Changin thresholds: " << b0 << "," << b1 << "," << b2<<","<<idif);
  for (auto it = _hca.asicMap().begin(); it != _hca.asicMap().end(); it++)
    {
      if (idif != 0)
	{
	  
	  uint32_t ip1 = (((it->first) >> 32) & 0XFFFFFFFF);
	  uint32_t ip = (ip1>>24);
	  if (idif != ip)
	    continue;
	  printf("%x %d %d %lu \n",ip1, ip, idif,it->first & 0xFF);

	}
      it->second.setB0(b0);
      it->second.setB1(b1);
      it->second.setB2(b2);
      // it->second.setHEADER(0x56);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::usleep(10);
}
void PmrManager::shiftThresholds(uint16_t b0, uint16_t b1, uint16_t b2, uint32_t idif)
{

  PMF_INFO(_logPmr, " Shifting thresholds: " << b0 << "," << b1 << "," << b2<<","<<idif);
  for (auto it = _hca.asicMap().begin(); it != _hca.asicMap().end(); it++)
    {
      if (idif != 0)
	{
	  
	  uint32_t ip1 = (((it->first) >> 32) & 0XFFFFFFFF);
	  uint32_t ip = (ip1>>24);
	  if (idif != ip)
	    continue;
	  printf("%x %d %d %lu \n",ip1, ip, idif,it->first & 0xFF);

	}
      uint16_t nb0=b0+ it->second.getB0();
      uint16_t nb1=b1+ it->second.getB1();
      uint16_t nb2=b2+ it->second.getB2();
      it->second.setB0(nb0);
      it->second.setB1(nb1);
      it->second.setB2(nb2);
      // it->second.setHEADER(0x56);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::usleep(10);
}
void PmrManager::setGain(uint16_t gain)
{

  PMF_INFO(_logPmr, " Changing Gain: " << gain);
  for (auto it = _hca.asicMap().begin(); it != _hca.asicMap().end(); it++)
    {
      for (int i = 0; i < 64; i++)
	it->second.setPAGAIN(i, gain);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::usleep(10);
}

void PmrManager::setMask(uint32_t level, uint64_t mask)
{
  //  fprintf(stderr,"\t looping on map \n");
  PMF_INFO(_logPmr, " Changing Mask: " << level << " " << std::hex << mask << std::dec);
  //fprintf(stderr,"\t looping on map \n");
  for (auto it = _hca.asicMap().begin(); it != _hca.asicMap().end(); it++)
    {
      //  fprintf(stderr,"\t ASIC %d \n",it->first);
      it->second.setMASK(level, mask);
    }
  // Now loop on slowcontrol socket
  //fprintf(stderr,"Calling configure \n");
  this->configureHR2();

  ::usleep(10);
}
void PmrManager::setChannelMask(uint16_t level, uint16_t channel, uint16_t val)
{
  PMF_INFO(_logPmr, " Changing Mask: " << level << " " << std::hex << channel << std::dec);
  for (auto it = _hca.asicMap().begin(); it != _hca.asicMap().end(); it++)
    {

      it->second.setMASKChannel(level, channel, val == 1);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();

  ::usleep(10);
}

void PmrManager::setAllMasks(uint64_t mask)
{
  PMF_INFO(_logPmr, " Changing Mask: " << std::hex << mask << std::dec);
  for (auto it = _hca.asicMap().begin(); it != _hca.asicMap().end(); it++)
    {
      //it->second.dumpBinary();
      it->second.setMASK(0, mask);
      it->second.setMASK(1, mask);
      it->second.setMASK(2, mask);
      //it->second.dumpBinary();
      //fprintf(stderr,"\t ASIC %d \n",it->first);
    }
  // Now loop on slowcontrol socket
  fprintf(stderr,"Calling configure \n");
  this->configureHR2();

  ::usleep(10);
}
void PmrManager::setCTEST(uint64_t mask)
{
  PMF_INFO(_logPmr, " Changing CTEST: " << std::hex << mask << std::dec);
  for (auto it = _hca.asicMap().begin(); it != _hca.asicMap().end(); it++)
    {
      for (int i = 0; i < 64; i++)
	{
	  bool on = ((mask >> i) & 1) == 1;
	  it->second.setCTEST(i, on);
	  PMF_DEBUG(_logPmr, "CTEST: " << std::hex << mask << std::dec << " channel " << i << " " << on);
	}
    }
  // Now loop on slowcontrol socket
  this->configureHR2();

  ::usleep(10);
}

void PmrManager::c_setthresholds(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPmr, "Set6bdac called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  uint32_t b0 = utils::queryIntValue(m, "B0", 250);
  uint32_t b1 = utils::queryIntValue(m, "B1", 250);
  uint32_t b2 = utils::queryIntValue(m, "B2", 250);
  uint32_t idif = utils::queryIntValue(m, "PMR", 0);

  this->setThresholds(b0, b1, b2, idif);
  par["THRESHOLD0"] = web::json::value::number(b0);
  par["THRESHOLD1"] = web::json::value::number(b1);
  par["THRESHOLD2"] = web::json::value::number(b2);
  par["DIF"] = web::json::value::number(idif);
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);
}
void PmrManager::c_shiftthresholds(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPmr, "shift threshold called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  uint32_t b0 = utils::queryIntValue(m, "B0", 0);
  uint32_t b1 = utils::queryIntValue(m, "B1", 0);
  uint32_t b2 = utils::queryIntValue(m, "B2", 0);
  uint32_t idif = utils::queryIntValue(m, "PMR", 0);

  this->shiftThresholds(b0, b1, b2, idif);
  par["THRESHOLD0"] = web::json::value::number(b0);
  par["THRESHOLD1"] = web::json::value::number(b1);
  par["THRESHOLD2"] = web::json::value::number(b2);
  par["DIF"] = web::json::value::number(idif);
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);
}
void PmrManager::c_setpagain(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPmr, "Set6bdac called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  uint32_t gain = utils::queryIntValue(m, "gain", 128);
  this->setGain(gain);
  par["GAIN"] = web::json::value::number(gain);

  Reply(status_codes::OK, par);
}

void PmrManager::c_setmask(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPmr, "SetMask called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  // uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint64_t mask;
  sscanf(utils::queryStringValue(m, "mask", "0XFFFFFFFFFFFFFFFF").c_str(), "%lx", &mask);
  uint32_t level = utils::queryIntValue(m, "level", 0);
  PMF_INFO(_logPmr, "SetMask called " << std::hex << mask << std::dec << " level " << level);
  this->setMask(level, mask);
  par["MASK"] = web::json::value::number(mask);
  par["LEVEL"] = web::json::value::number(level);

  Reply(status_codes::OK, par);
}

void PmrManager::c_setchannelmask(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPmr, "SetMask called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  // uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint32_t level = utils::queryIntValue(m, "level", 0);
  uint32_t channel = utils::queryIntValue(m, "channel", 0);
  bool on = utils::queryIntValue(m, "value", 1) == 1;
  PMF_INFO(_logPmr, "SetMaskChannel called " << channel << std::dec << " level " << level);
  this->setChannelMask(level, channel, on);
  par["CHANNEL"] = web::json::value::number(channel);
  par["LEVEL"] = web::json::value::number(level);
  par["ON"] = web::json::value::number(on);

  Reply(status_codes::OK, par);
}
void PmrManager::c_external(http_request m)
{
  auto par = json::value::object();
  uint32_t external = utils::queryIntValue(m, "value", 0);
  PMF_INFO(_logPmr, "EXTERNAL called " << external);

  if (external != 0)
    params()["external"] = external;
  std::map<uint32_t, PmrInterface *> dm = this->getPmrMap();
  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {
      it->second->setExternalTrigger((external == 1));
    }

  // PMF_INFO(_logPmr,"CTRLREG called "<<std::hex<<ctrlreg<<std::dec);
  par["STATUS"] = web::json::value::string(U("DONE"));
  par["TRIGGEREXT"] = web::json::value::number(external);
  Reply(status_codes::OK, par);
}
void PmrManager::c_downloadDB(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPmr, "downloadDB called ");
  par["STATUS"] = web::json::value::string(U("DONE"));

  std::string dbstate = utils::queryStringValue(m, "state", "NONE");
  uint32_t version = utils::queryIntValue(m, "version", 0);
  web::json::value jTDC = params()["dif"];
  if (utils::isMember(jTDC, "db"))
    {
	  web::json::value jTDCdb = jTDC["db"];
	  _hca.clear();
	  //delete _hca;
	  //_hca = new HR2ConfigAccess();
	  if (jTDCdb["mode"].as_string().compare("mongo") == 0)
	    _hca.parseMongoDb2(dbstate, version);

    }
  par["DBSTATE"] = web::json::value::string(dbstate);
  Reply(status_codes::OK, par);
}
web::json::value PmrManager::build_status()
{
  web::json::value array_slc;
  uint32_t nd = 0;
  std::map<uint32_t, PmrInterface *> dm = this->getPmrMap();
  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {
      
      web::json::value ds;
      ds["detid"] = json::value::number(it->second->detectorId());
      ds["state"] = json::value::string(U(it->second->state()));
      ds["id"] = json::value::number(it->second->status()->id);
      ds["status"] = json::value::number(it->second->status()->status);
      ds["slc"] = json::value::number(it->second->status()->slc);
      ds["gtc"] = json::value::number(it->second->status()->gtc);
      ds["bcid"] = json::value::number(it->second->status()->bcid);
      ds["published"] = json::value::number(it->second->status()->published);
      ds["bytes"] = json::value::number(it->second->status()->bytes);
      ds["host"] = json::value::string(U(std::string((it->second->status()->host))));
      ds["mode"]=json::value::number(_running_mode);
      array_slc[nd++] = ds;
    }
  return array_slc;
}
void PmrManager::c_status(http_request m)
{
  auto par = json::value::object();
  int32_t rc = 1;

  web::json::value array_slc=build_status();
  par["STATUS"] = web::json::value::string(U("DONE"));
  par["DIFLIST"] = array_slc;
  mqtt_publish("status",array_slc);
  Reply(status_codes::OK, par);

  return;
}

web::json::value PmrManager::configureHR2()
{
  fprintf(stderr,"On rentre Debug Configure HR2\n");
  /// A reecrire
  //uint32_t external = params()["external"].as_integer();
  //printf("TRigger EXT %x \n", external);
  int32_t rc = 1;
  std::map<uint32_t, PmrInterface *> dm = this->getPmrMap();
  web::json::value array_slc;
  uint32_t nd = 0;
#undef ONETHREAD
#ifdef ONETHREAD
  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {
      std::stringstream ips;
      // Dummy IP address for Pmrs
      ::usleep(5000);
      //fprintf(stderr,"Debug 2");
      ips << "0.0.0." << it->first;
      _hca.prepareSlowControl(ips.str(), true);
      //fprintf(stderr,"Debug 3");
      it->second->configure(_hca.slcBuffer(), _hca.slcBytes());
      //fprintf(stderr,"Debug 4");
      web::json::value ds;
      ds["id"] = json::value::number(it->first);
      //fprintf(stderr,"Debug 4");
      ds["slc"] = json::value::number(it->second->status()->slc);
      array_slc[nd++] = ds;
      //fprintf(stderr,"Debug 5");
    }
#else
  fprintf(stderr,"Debug Configure HR2\n");
  //memcpy(slowb,b,nb);

  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {
      std::stringstream ips;
      // Dummy IP address for Pmrs
      ::usleep(50000);
      //fprintf(stderr,"Debug 2");
      ips << "0.0.0." << it->first;
      //fprintf(stderr,"Debug 2 %s \n",ips.str().c_str());
      _hca.prepareSlowControl(ips.str(), true);
      //fprintf(stderr,"Debug 3 %d %d \n",it->first,_hca.slcBytes());
      nb[it->first]=_hca.slcBytes();
      memcpy(&slowb[it->first*65536],_hca.slcBuffer(),_hca.slcBytes());
      this->configureThread(it->second,&slowb[it->first*65536], nb[it->first]);
      //fprintf(stderr,"Debug 4");
    }
  ::usleep(100000);
  this->joinConfigureThreads();
  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {

      web::json::value ds;
      ds["id"] = json::value::number(it->first);
      //fprintf(stderr,"Debug 4");
      ds["slc"] = json::value::number(it->second->status()->slc);
      array_slc[nd++] = ds;
      //fprintf(stderr,"Debug 5");
    }

#endif
  return array_slc;
}

void PmrManager::configure(http_request m)
{
  //fprintf(stderr,"Debug 1");
  auto par = json::value::object();
  PMF_INFO(_logPmr, " CMD: Configuring");

  int32_t rc = 1;

  web::json::value array_slc = this->configureHR2();

  par["status"] = json::value::string(U("done"));
  par["devices"] = array_slc;
  Reply(status_codes::OK, par);

  return;
}

void PmrManager::start(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPmr, " Starting ");

  int32_t rc = 1;
  std::map<uint32_t, PmrInterface *> dm = this->getPmrMap();
  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {
      this->startReadoutThread(it->second);
      it->second->start();
    }
  _running = true;
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);

  return;
}
void PmrManager::stop(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPmr, " Stopping ");

  int32_t rc = 1;
  std::map<uint32_t, PmrInterface *> dm = this->getPmrMap();

  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {
      PMF_INFO(_logPmr, " Stopping thread of Pmr" << it->first);
      it->second->stop();
    }
  _running = false;
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);

  return;
}
void PmrManager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logPmr, " Destroying ");

  int32_t rc = 1;
  std::map<uint32_t, PmrInterface *> dm = this->getPmrMap();

  bool running = false;
  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {
      running = running || it->second->readoutStarted();
    }
  if (running)
    {
      for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
	{
	  it->second->setReadoutStarted(false);
	}

      this->joinThreads();
      // Clear thread vector
      g_d.clear();
    }
  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {
      it->second->destroy();
    }

  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);

  return;
}

PmrManager::PmrManager() : _running(false), _sc_running(false),_running_mode(0) { ; }

void PmrManager::initialise()
{

  // Zmq transport
  _context = new zmq::context_t(1);
  // Register state
  this->addState("SCANNED");
  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  this->addState("STOPPED");
  this->addTransition("SCAN", "CREATED", "SCANNED", std::bind(&PmrManager::scan, this, std::placeholders::_1));
  this->addTransition("INITIALISE", "SCANNED", "INITIALISED", std::bind(&PmrManager::fsm_initialise, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "INITIALISED", "CONFIGURED", std::bind(&PmrManager::configure, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "CONFIGURED", "CONFIGURED", std::bind(&PmrManager::configure, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "STOPPED", "CONFIGURED", std::bind(&PmrManager::configure, this, std::placeholders::_1));
  this->addTransition("START", "CONFIGURED", "RUNNING", std::bind(&PmrManager::start, this, std::placeholders::_1));
  this->addTransition("START", "STOPPED", "RUNNING", std::bind(&PmrManager::start, this, std::placeholders::_1));
  this->addTransition("STOP", "RUNNING", "STOPPED", std::bind(&PmrManager::stop, this, std::placeholders::_1));
  this->addTransition("DESTROY", "STOPPED", "CREATED", std::bind(&PmrManager::destroy, this, std::placeholders::_1));
  this->addTransition("DESTROY", "CONFIGURED", "CREATED", std::bind(&PmrManager::destroy, this, std::placeholders::_1));

  this->addCommand("STATUS", std::bind(&PmrManager::c_status, this, std::placeholders::_1));
  this->addCommand("SETTHRESHOLDS", std::bind(&PmrManager::c_setthresholds, this, std::placeholders::_1));
  this->addCommand("SHIFTTHRESHOLDS", std::bind(&PmrManager::c_shiftthresholds, this, std::placeholders::_1));
  this->addCommand("SETPAGAIN", std::bind(&PmrManager::c_setpagain, this, std::placeholders::_1));
  this->addCommand("SETMASK", std::bind(&PmrManager::c_setmask, this, std::placeholders::_1));
  this->addCommand("SETCHANNELMASK", std::bind(&PmrManager::c_setchannelmask, this, std::placeholders::_1));
  this->addCommand("DOWNLOADDB", std::bind(&PmrManager::c_downloadDB, this, std::placeholders::_1));
  this->addCommand("TRIGEXT", std::bind(&PmrManager::c_external, this, std::placeholders::_1));
  this->addCommand("SCURVE", std::bind(&PmrManager::c_scurve, this, std::placeholders::_1));
  this->addCommand("GAINCURVE", std::bind(&PmrManager::c_gaincurve, this, std::placeholders::_1));

  //_hca = NULL;
  // Initialise delays for
}

void PmrManager::end()
{
  // Stop any running process
  if (_sc_running)
    {
      _sc_running = false;
      g_scurve.join();
      _running_mode=0;
    }
  // Stop listening
  std::map<uint32_t, PmrInterface *> dm = this->getPmrMap();

  if (g_d.size() != 0)
    {

      bool running = false;
      for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
	{
	  running = running || it->second->readoutStarted();
	}
      if (running)
	{
	  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
	    {
	      it->second->setReadoutStarted(false);
	    }

	  this->joinThreads();
	}
      // Clear thread vector
      g_d.clear();
    }
  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)
    {
      delete it->second;
    }
   this->getPmrMap().clear();

   // Delet the ASICS
   //delete _hca;
}

void PmrManager::joinConfigureThreads()
  {

    int joins=0;
    do {
      ::usleep(10000);
      for (auto& i: g_c)
	{
	  if (i.joinable())
	    {
	      fprintf(stderr,"Joining thread %d \n",joins++);
	    i.join();
	    }
	}
    } while(joins<g_c.size());
    g_c.clear();
  }
void PmrManager::configureThread(PmrInterface *d,unsigned char* b,uint32_t nb)
{
  ::usleep(100000);
  //fprintf(stderr," Configuring %d  bytes\n",nb);
  g_c.push_back(std::thread(std::bind(&PmrInterface::configure, d,b,nb)));
}




void PmrManager::startReadoutThread(PmrInterface *d)
{
  if (d->readoutStarted())
    return;
  d->setReadoutStarted(true);
  g_d.push_back(std::thread(std::bind(&PmrInterface::readout, d)));
}

void PmrManager::ScurveStep(std::string mdccUrl, std::string builderUrl, int thmin, int thmax, int step)
{
  std::map<uint32_t, PmrInterface *> dm = this->getPmrMap();
  int ncon = _sc_win, ncoff = 1000, ntrg = _sc_ntrg;
  utils::sendCommand(mdccUrl, "PAUSE", json::value::null());
  web::json::value p;
  p["nclock"] = web::json::value::number(ncon);
  utils::sendCommand(mdccUrl, "SPILLON", p);
  p["nclock"] = web::json::value::number(ncoff);
  utils::sendCommand(mdccUrl, "SPILLOFF", p);
  printf("Clock On %d Off %d \n", ncon, ncoff);
  p["value"] = web::json::value::number(4);
  utils::sendCommand(mdccUrl, "SETSPILLREGISTER", p);
  utils::sendCommand(mdccUrl, "CALIBON", json::value::null());
  p["nclock"] = web::json::value::number(ntrg);
  utils::sendCommand(mdccUrl, "SETCALIBCOUNT", p);

  int thrange = (thmax - thmin + 1) / step;
  for (int vth = 0; vth <= thrange; vth++)
    {
      if (!_running)
	break;
      utils::sendCommand(mdccUrl, "PAUSE", json::value::null());

      //for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++) it->second->setRunning(false);
      usleep(100000);
      //this->setThresholds(thmax - vth * step, 512, 512);
      //::sleep(2);
      uint32_t threshold = thmax - vth * step;
      // this->setThresholds(thmax-vth*step,512,512);
      if (_sc_level == 0)
	this->setThresholds(threshold, 512, 512);
      if (_sc_level == 1)
	this->setThresholds(512, threshold, 512);
      if (_sc_level == 2)
	this->setThresholds(512, 512, threshold);
      ::usleep(2000);
      _running_mode=2+((_sc_channel&0xFF)<<4)+((threshold&0XFFF)<<12);
      mqtt_publish("status",build_status());
      web::json::value h;
      web::json::value ph;
      h[0] = json::value::number(2);
      h[1] = json::value::number(thmax - vth * step);

      int firstEvent = 0, firstInBoard = 0;

      for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)

	if (it->second->status()->gtc > firstInBoard)
	  firstInBoard = it->second->status()->gtc;

      auto frep = utils::sendCommand(builderUrl, "STATUS", json::value::null());
      auto jfrep = frep.extract_json();
      auto jfanswer = jfrep.get().as_object()["answer"];
      firstEvent = jfanswer["event"].as_integer();

      ph["header"] = h;
      ph["nextevent"] = json::value::number(firstEvent + 1);
      utils::sendCommand(builderUrl, "SETHEADER", ph);
      utils::sendCommand(mdccUrl, "RELOADCALIB", json::value::null());
      for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++) it->second->setRunning(true);

      utils::sendCommand(mdccUrl, "RESUME", json::value::null());

      int nloop = 0, lastEvent = firstEvent, lastInBoard = firstInBoard;
      while (lastInBoard < (firstInBoard + ntrg-1))
	{
	  ::usleep(10000);
	  uint32_t mingtc=0xFFFFFFFF;
	  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)

	    if (it->second->status()->gtc < mingtc)
	      mingtc = it->second->status()->gtc;

	  if (mingtc>lastInBoard) lastInBoard=mingtc;
	  nloop++;
	  if (nloop > 6000 || !_running)
	    break;
	}
      nloop=0;
      while (lastEvent < (firstEvent + ntrg - 2))
	{
	  ::usleep(100000);
	  auto rep = utils::sendCommand(builderUrl, "STATUS", json::value::null());
	  auto jrep = rep.extract_json();
	  auto janswer = jrep.get().as_object()["answer"];
	  lastEvent = janswer["event"].as_integer(); // A verifier
	  nloop++;
	  if (nloop > 100 || !_running)
	    break;
	}

      PMF_INFO(_logPmr, "Step:" << vth << " Threshold:" << thmax - vth * step << " First:" << firstEvent << " Last:" << lastEvent);
      // printf("Step %d Th %d First %d Last %d \n",vth,thmax-vth*step,firstEvent,lastEvent);
      utils::sendCommand(mdccUrl, "PAUSE", json::value::null());
      for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++) it->second->setRunning(false);

    }
  utils::sendCommand(mdccUrl, "CALIBOFF", json::value::null());
}

void PmrManager::thrd_scurve()
{
  _sc_running = true;
  this->Scurve(_sc_mode, _sc_thmin, _sc_thmax, _sc_step);
  _sc_running = false;
}

void PmrManager::Scurve(int mode, int thmin, int thmax, int step)
{
  std::string mdcc = utils::findUrl(session(), "lyon_mdcc", 0);
  std::string builder = utils::findUrl(session(), "evb_builder", 0);
  if (mdcc.compare("") == 0)
    {
      mdcc = utils::findUrl(session(), "lyon_mbmdcc", 0);
      if (mdcc.compare("") == 0)
	{
	  mdcc = utils::findUrl(session(), "lyon_ipdc", 0);
	  if (mdcc.compare("") == 0)
	    {
	      PMF_ERROR(_logPmr,"No MDCC/MBMDCC/IPDC available, exiting Scurve");

	      return;
	    }
	}
    }
  if (builder.compare("") == 0)
    return;
  uint64_t mask = 0;
  _sc_channel=mode;
  // All channel pedestal
  if (mode == 255)
    {
      _sc_channel=0xFE;
      // for (int i=0;i<64;i++) mask|=(1<<i);
      //mask = 0xFFFFFFFFFFFFFFFF;
      //this->setAllMasks(mask);
      this->setCTEST(0);
      this->ScurveStep(mdcc, builder, thmin, thmax, step);
      _running_mode=0;
      mqtt_publish("status",build_status());

      return;
    }

  // Chanel per channel pedestal (CTEST is active)
  if (mode == 1023)
    {
      mask = 0;
      for (int i = 0; i < 64; i++)
	{
	  mask = (1ULL << i);
	  PMF_INFO(_logPmr, "Step HR2 " << i << "  channel" << i << std::dec);

	  // std::cout<<"Step HR2 "<<i<<" channel "<<i<<std::endl;
	  _sc_channel=i;
	  this->setAllMasks(mask);
	  this->setCTEST(mask);
	  this->ScurveStep(mdcc, builder, thmin, thmax, step);
	}
      _running_mode=0;
      mqtt_publish("status",build_status());

      return;
    }

  if (mode == 1022)
    {
      mask = 0;
      for (int i = 0; i < 64; i++)
	{
	  mask = (1ULL << i);
	  PMF_INFO(_logPmr, "Step HR2 " << i << "  channel" << i << std::dec);

	  // std::cout<<"Step HR2 "<<i<<" channel "<<i<<std::endl;
	  this->setCTEST(mask);
	  this->setAllMasks(0xFFFFFFFFFFFFFFFF);
	  _sc_channel=i;
	  this->ScurveStep(mdcc, builder, thmin, thmax, step);
	}
      _running_mode=0;
      mqtt_publish("status",build_status());

      return;
    }

  // One channel pedestal

  mask = (1ULL << mode);
  PMF_INFO(_logPmr, "CTEST One " << mode << " " << std::hex << mask << std::dec);
  this->setAllMasks(mask);
  this->setCTEST(mask);
  this->ScurveStep(mdcc, builder, thmin, thmax, step);
  _running_mode=0;
  mqtt_publish("status",build_status());

}

void PmrManager::c_scurve(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = web::json::value::string(U("DONE"));

  uint32_t first = utils::queryIntValue(m, "first", 80);
  uint32_t last = utils::queryIntValue(m, "last", 250);
  uint32_t step = utils::queryIntValue(m, "step", 1);
  uint32_t mode = utils::queryIntValue(m, "channel", 255);
  uint32_t win = utils::queryIntValue(m, "window", 50000);
  uint32_t ntrg = utils::queryIntValue(m, "ntrg", 50);
  _sc_level = utils::queryIntValue(m, "level", 0);
  PMF_INFO(_logPmr, " SCURVE/CTEST " << mode << " " << step << " " << first << " " << last);

  // this->Scurve(mode,first,last,step);

  _sc_mode = mode;
  _sc_thmin = first;
  _sc_thmax = last;
  _sc_step = step;
  _sc_win = win;
  _sc_ntrg = ntrg;

  if (_sc_running)
    {
      par["SCURVE"] = web::json::value::string(U("ALREADY_RUNNING"));
      Reply(status_codes::OK, par);
      return;
    }

  _sc_running = true;
  g_scurve = std::thread(std::bind(&PmrManager::thrd_scurve, this));
  par["SCURVE"] = web::json::value::string(U("RUNNING"));

  Reply(status_codes::OK, par);
}

void PmrManager::GainCurveStep(std::string mdccUrl,std::string builderUrl,int gmin,int gmax,int step,int threshold)
{
  std::map<uint32_t, PmrInterface *> dm = this->getPmrMap();
  int ncon = _sc_win, ncoff = 1000, ntrg = _sc_ntrg;
  utils::sendCommand(mdccUrl, "PAUSE", json::value::null());
  web::json::value p;
  p["nclock"] = web::json::value::number(ncon);
  utils::sendCommand(mdccUrl, "SPILLON", p);
  p["nclock"] = web::json::value::number(ncoff);
  utils::sendCommand(mdccUrl, "SPILLOFF", p);
  printf("Clock On %d Off %d \n", ncon, ncoff);
  p["value"] = web::json::value::number(4);
  utils::sendCommand(mdccUrl, "SETSPILLREGISTER", p);
  utils::sendCommand(mdccUrl, "CALIBON", json::value::null());
  p["nclock"] = web::json::value::number(ntrg);
  utils::sendCommand(mdccUrl, "SETCALIBCOUNT", p);

  // Set the threshold
  if (threshold>80)
    this->setThresholds(threshold, 512, 512);

  int grange=(gmax-gmin+1)/step;
  for (int g=0;g<=grange;g++)
    {
      if (!_running) break;
      utils::sendCommand(mdccUrl,"PAUSE",json::value::null());
      //for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++) it->second->setRunning(false);
      ::usleep(100000);
      this->setGain(gmin+g*step);

      ::usleep(100000);
      if (threshold>80)
	{
	this->setThresholds(threshold, 512, 512);
	::usleep(100000);
	}
      _running_mode=4+((_sc_channel&0xFF)<<4)+((threshold&0XFFF)<<12)+((g&0XFFF)<<24);
      mqtt_publish("status",build_status());

      web::json::value h;
      web::json::value ph;
      h[0] = json::value::number(3);
      h[1] = json::value::number(gmin+g*step);
      int firstEvent = 0, firstInBoard = 0;

      for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)

	if (it->second->status()->gtc > firstInBoard)
	  firstInBoard = it->second->status()->gtc;

      auto frep = utils::sendCommand(builderUrl, "STATUS", json::value::null());
      auto jfrep = frep.extract_json();
      auto jfanswer = jfrep.get().as_object()["answer"];
      firstEvent = jfanswer["event"].as_integer();

      ph["header"] = h;
      ph["nextevent"] = json::value::number(firstEvent + 1);
      utils::sendCommand(builderUrl, "SETHEADER", ph);
      utils::sendCommand(mdccUrl, "RELOADCALIB", json::value::null());
      for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++) it->second->setRunning(true);

      utils::sendCommand(mdccUrl, "RESUME", json::value::null());

      int nloop = 0, lastEvent = firstEvent, lastInBoard = firstInBoard;
      while (lastInBoard < (firstInBoard + ntrg-1))
	{
	  ::usleep(10000);
	  uint32_t mingtc=0xFFFFFFFF;
	  for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++)

	    if (it->second->status()->gtc < mingtc)
	      mingtc = it->second->status()->gtc;

	  if (mingtc>lastInBoard) lastInBoard=mingtc;
	  nloop++;
	  if (nloop > 6000 || !_running)
	    break;
	}
      nloop=0;
      while (lastEvent < (firstEvent + ntrg - 2))
	{
	  ::usleep(100000);
	  auto rep = utils::sendCommand(builderUrl, "STATUS", json::value::null());
	  auto jrep = rep.extract_json();
	  auto janswer = jrep.get().as_object()["answer"];
	  lastEvent = janswer["event"].as_integer(); // A verifier
	  nloop++;
	  if (nloop > 100 || !_running)
	    break;
	}

      PMF_INFO(_logPmr,"Step:"<<g<<" Gain:"<<gmin+g*step<<" First:"<<firstEvent<<" Last:"<<lastEvent);

      // printf("Step %d Th %d First %d Last %d \n",vth,thmax-vth*step,firstEvent,lastEvent);
      utils::sendCommand(mdccUrl, "PAUSE", json::value::null());
      for (std::map<uint32_t, PmrInterface *>::iterator it = dm.begin(); it != dm.end(); it++) it->second->setRunning(false);

    }
  utils::sendCommand(mdccUrl, "CALIBOFF", json::value::null());

  
}


void PmrManager::thrd_gaincurve()
{
  _sc_running=true;
  this->GainCurve(_sc_mode,_sc_gmin,_sc_gmax,_sc_step,_sc_threshold);
  _sc_running=false;
}


void PmrManager::GainCurve(int mode,int gmin,int gmax,int step,int threshold)
{
  std::string mdcc=utils::findUrl(session(),"lyon_mdcc",0);
  //std::string builder=utils::findUrl(session(),"lyon_evb",0);
  std::string builder=utils::findUrl(session(),"evb_builder",0);
  if (mdcc.compare("") == 0)
    {
      mdcc = utils::findUrl(session(), "lyon_mbmdcc", 0);
      if (mdcc.compare("") == 0)
	{
	  mdcc = utils::findUrl(session(), "lyon_ipdc", 0);
	  if (mdcc.compare("") == 0)
	    {
	      PMF_ERROR(_logPmr,"No MDCC/MBMDCC/IPDC available, exiting gain curve");
	      return;
	    }
	}
    }

  if (builder.compare("")==0) return;

  uint64_t mask=0;
  _sc_channel=mode;
  // All channel pedestal
  if (mode==255)
    {
      _sc_channel=0xFE;

      //mask=0xFFFFFFFFFFFFFFFF;
      //this->setAllMasks(mask);
      this->setCTEST(0);
      this->GainCurveStep(mdcc,builder,gmin,gmax,step,threshold);
      _running_mode=0;
      mqtt_publish("status",build_status());

      return;
      
    }

  // Chanel per channel pedestal (CTEST is active)
  if (mode==1022)
    {
      mask=0;
      for (int i=0;i<64;i++)
	{
	  mask=(1ULL<<i);
	  std::cout<<"Step HR2 "<<i<<" channel "<<i<<std::endl;
	  _sc_channel=i;
	  this->setCTEST(mask);
	  this->setAllMasks(0xFFFFFFFFFFFFFFFF);
	  this->GainCurveStep(mdcc,builder,gmin,gmax,step,threshold);
	  //this->ScurveStep(mdcc,builder,thmin,thmax,step);
	}
      _running_mode=0;
      mqtt_publish("status",build_status());

      return;
    }
  if (mode==1023)
    {
      mask=0;
      for (int i=0;i<64;i++)
	{
	  mask=(1ULL<<i);
	  std::cout<<"Step HR2 CTEST "<<i<<" channel "<<i<<std::endl;
	  this->setAllMasks(mask);
	  this->setCTEST(mask);
	  _sc_channel=i;
	  this->GainCurveStep(mdcc,builder,gmin,gmax,step,threshold);
	  //this->ScurveStep(mdcc,builder,thmin,thmax,step);
	}
      return;
      _running_mode=0;
      mqtt_publish("status",build_status());

    }

  // One channel pedestal
  mask=(1ULL<<mode);
  this->setAllMasks(mask);
  this->setCTEST(mask);
  //this->ScurveStep(mdcc,builder,thmin,thmax,step);
  this->GainCurveStep(mdcc,builder,gmin,gmax,step,threshold);
  _running_mode=0;
  mqtt_publish("status",build_status());

}


void PmrManager::c_gaincurve(http_request m)
{
  auto par = json::value::object();

  par["STATUS"]=web::json::value::string(U("DONE"));

  uint32_t first = utils::queryIntValue(m,"first",2);
  uint32_t last = utils::queryIntValue(m,"last",127);
  uint32_t step = utils::queryIntValue(m,"step",2);
  uint32_t mode = utils::queryIntValue(m,"channel",255);
  uint32_t thr = utils::queryIntValue(m,"threshold",0);
  PMF_INFO(_logPmr, " GainCurve called " << mode << " first " << first << " last " << last <<" step "<<step<< " Threshold "<<thr);
  _sc_win = utils::queryIntValue(m, "window", 50000);
  _sc_ntrg = utils::queryIntValue(m, "ntrg", 50);
  //this->Scurve(mode,first,last,step);

  _sc_mode=mode;
  _sc_gmin=first;
  _sc_gmax=last;
  _sc_step=step;
  _sc_threshold=thr;
  if (_sc_running)
    {
      par["GAINCURVE"]=web::json::value::string(U("ALREADY_RUNNING"));
      return;
    }
 
  g_scurve=std::thread(std::bind(&PmrManager::thrd_gaincurve, this));
  par["GAINCURVE"]=web::json::value::string(U("RUNNING"));
  Reply(status_codes::OK,par);


}





extern "C"
{
  // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.
  handlerPlugin *loadProcessor(void)
  {
    return (new PmrManager);
  }
  // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed
  // to it.  This isn't a very safe function, since there's no
  // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin *obj)
  {
    delete obj;
  }
}
