#include "Febv1Manager.hh"
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










Febv1Manager::Febv1Manager() : _context(NULL),_tca(NULL),_mpi(NULL) {;}

Febc1Manager::initialise()
{

  // Register state

  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  
  this->addTransition("INITIALISE","CREATED","INITIALISED",std::bind(&Febv1Manager::fsm_initialise, this,std::placeholders_1));
  this->addTransition("CONFIGURE","INITIALISED","CONFIGURED",std::bind(&Febv1Manager::configure, this,std::placeholders_1));
  this->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",std::bind(&Febv1Manager::configure, this,std::placeholders_1));
  
  this->addTransition("START","CONFIGURED","RUNNING",std::bind(&Febv1Manager::start, this,std::placeholders_1));
  this->addTransition("STOP","RUNNING","CONFIGURED",std::bind(&Febv1Manager::stop, this,std::placeholders_1));
  this->addTransition("DESTROY","CONFIGURED","CREATED",std::bind(&Febv1Manager::destroy, this,std::placeholders_1));
  this->addTransition("DESTROY","INITIALISED","CREATED",std::bind(&Febv1Manager::destroy, this,std::placeholders_1));
  
  

  this->addCommand("STATUS", std::bind(&Febv1Manager::c_status, this,std::placeholders_1));
  this->addCommand("DIFLIST", std::bind(&Febv1Manager::c_diflist, this,std::placeholders_1));
  this->addCommand("SET6BDAC", std::bind(&Febv1Manager::c_set6bdac, this,std::placeholders_1));
  this->addCommand("CAL6BDAC", std::bind(&Febv1Manager::c_cal6bdac, this,std::placeholders_1));
  this->addCommand("SETVTHTIME", std::bind(&Febv1Manager::c_setvthtime, this,std::placeholders_1));
  this->addCommand("SETONEVTHTIME", std::bind(&Febv1Manager::c_set1vthtime, this,std::placeholders_1));
  this->addCommand("SETMASK", std::bind(&Febv1Manager::c_setMask, this,std::placeholders_1));
  this->addCommand("DOWNLOADDB", std::bind(&Febv1Manager::c_downloadDB, this,std::placeholders_1));
  this->addCommand("ASICS", std::bind(&Febv1Manager::c_asics, this,std::placeholders_1));

  this->addCommand("SETMODE", std::bind(&Febv1Manager::c_setMode, this,std::placeholders_1));
  this->addCommand("SETDELAY", std::bind(&Febv1Manager::c_setDelay, this,std::placeholders_1));
  this->addCommand("SETDURATION", std::bind(&Febv1Manager::c_setDuration, this,std::placeholders_1));
  this->addCommand("GETLUT", std::bind(&Febv1Manager::c_getLUT, this,std::placeholders_1));
  this->addCommand("CALIBSTATUS", std::bind(&Febv1Manager::c_getCalibrationStatus, this,std::placeholders_1));
  this->addCommand("CALIBMASK", std::bind(&Febv1Manager::c_setCalibrationMask, this,std::placeholders_1));
  this->addCommand("TESTMASK", std::bind(&Febv1Manager::c_setMeasurementMask, this,std::placeholders_1));
  this->addCommand("SCURVE", std::bind(&Febv1Manager::c_scurve, this,std::placeholders_1));

  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;


}
void Febv1Manager::end()
{
  //Stop listening
  if (_mpi!=NULL)
    _mpi->terminate();
 
}

void Febv1Manager::c_status(http_request m)
{
  PM_INFO(_logFebv1, "Status CMD called ");
  auto par = json::value::object();

  par["STATUS"] = json::value::string(U("DONE"));

  json::value jl;
  uint32_t mb=0;
  for (auto x : _mpi->boards())
  {

    json::value jt;
    jt["detid"] = json::value::number(x.second->data()->detectorId());
    jt["sourceid"] =json::value::number(x.second->data()->difId());
    jt["gtc"] = json::value::number(x.second->data()->gtc());
    jt["abcid"] = json::value::number(x.second->data()->abcid());
    jt["event"] = json::value::number(x.second->data()->event());
    jt["triggers"] = json::value::number(x.second->data()->triggers());
    jl[mb]=jt;
  }
  par["TDCSTATUS"] = jl;
  Reply(status_codes::OK,par);

}
void Febv1Manager::c_diflist(http_request m)
{
  PM_INFO(_logFebv1, "List of source id CMD called ");
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));
  par["DIFLIST"] = json::value::string(U("EMPTY"));

  web::json::value jl;
  uint32_t mb=0;
  for (auto x : _mpi->boards())
  {
    if (x.second == NULL)
      continue;
    web::json::value jt;
    jt["detid"] = json::value::number(x.second->data()->detectorId());
    jt["sourceid"] = json::value::number(x.second->data()->difId());
    jl[mb]=jt;
  }
  par["DIFLIST"] = jl;
  Reply(status_codes::OK,par);

}

void Febv1Manager::c_set6bdac(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  uint32_t nc = 31;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    if (it2->first.compare("value")==0)
      nc=std::stoi(it2->second);
    
  PM_INFO(_logFebv1, "Set6bdac called with dac=" << nc);

  this->set6bDac(nc & 0xFF);
  par["6BDAC"] = _jControl;
  Reply(status_codes::OK,par);
}
void Febv1Manager::c_cal6bdac(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  uint32_t mask = 4294967295;
  int32_t shift = 0;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("mask")==0)  mask=std::stoi(it2->second);
      if (it2->first.compare("shift")==0)  shift=std::stoi(it2->second);
    }

  PM_INFO(_logFebv1, "cal6bdac called with mask=" << mask << " Shift:" << shift);

  this->cal6bDac(mask, shift);
  par["6BDAC"] = _jControl;
  Reply(status_codes::OK,par);
}
void Febv1Manager::c_setvthtime(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  uint32_t nc = 380.;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    if (it2->first.compare("value")==0)
      nc=std::stoi(it2->second);

  PM_INFO(_logFebv1, "set VThTime called with value=" << nc);

  this->setVthTime(nc);
  par["VTHTIME"] = _jControl;
  Reply(status_codes::OK,par);
}
void Febv1Manager::c_set1vthtime(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  uint32_t vth = 550;
  uint32_t feb = 5;
  uint32_t asic = 1;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("vth")==0)  vth=std::stoi(it2->second);
      if (it2->first.compare("feb")==0)  feb=std::stoi(it2->second);
      if (it2->first.compare("asic")==0)  asic=std::stoi(it2->second);
    }

  PM_INFO(_logFebv1, " SetOneVthTime called with vth " << vth << " feb " << feb << " asic " << asic);
  
  this->setSingleVthTime(vth, feb, asic);
  par["VTH"] = json::value::number(vth);
  par["FEB"] = json::value::number(feb);
  par["ASIC"] =json::value::number(asic);
  par["1VTH"] = _jControl;
  Reply(status_codes::OK,par);
}
void Febv1Manager::c_setMask(http_request m)
{
  PM_INFO(_logFebv1,"SetMask called ");
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  //uint32_t nc=atol(request.get("value","4294967295").c_str());
  uint32_t nc=4294967295;

  uint32_t asic = 255;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("value")==0)  nc=std::stoi(it2->second);
      if (it2->first.compare("asic")==0)  asic=std::stoi(it2->second);
    }

  PM_INFO(_logFebv1, "SetMask called  with mask" << std::hex << nc << std::dec << " and asic mask " << asic);
  this->setMask(nc, asic & 0xFF);
  par["MASK"] = _jControl;
  Reply(status_codes::OK,par);
}

void Febv1Manager::c_setCalibrationMask(http_request m)
{
  PM_INFO(_logFebv1,"SetCalibrationMask called ");
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));
  uint64_t mask = 0;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    if (it2->first.compare("value")==0)
      mask=std::stoi(it2->second);

  PM_INFO(_logFebv1, "SetCalibrationMask called  with mask" << std::hex << mask << std::dec);
  this->setCalibrationMask(mask);
  par["CMASK"] = json::value::number(mask);
  Reply(status_codes::OK,par);
}
void Febv1Manager::c_setMeasurementMask(http_request m)
{
  PM_INFO(_logFebv1, __PRETTY_FUNCTION__ << "c_setMeasurementMask called ");
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));
  uint64_t mask = 0;
  uint32_t feb = 255;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("value")==0)  mask=std::stoi(it2->second);
      if (it2->first.compare("feb")==0)  feb=std::stoi(it2->second);
    }

 
  PM_INFO(_logFebv1, "c_setMeasurementMask called  with mask" << std::hex << mask << std::dec<<" On FEB "<<feb);
  this->setMeasurementMask(mask,feb);
  par["MMASK"] = json::value::number(mask);
  Reply(status_codes::OK,par);
}
void Febv1Manager::c_setDelay(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  uint32_t delay =255;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    if (it2->first.compare("value")==0)
      delay=std::stoi(it2->second);

  _delay = delay;
  this->setDelay();
  PM_INFO(_logFebv1, "SetDelay called with " << delay << " " << _delay);
  par["DELAY"] = json::value::number(_delay);
  Reply(status_codes::OK,par);
}
void Febv1Manager::c_getLUT(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  uint32_t chan = 0;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    if (it2->first.compare("value")==0)
      chan=std::stoi(it2->second);

  this->getLUT(chan);
  PM_INFO(_logFebv1, "GETLUT called for " << chan);
  
  par["LUT"] = _jControl;
  Reply(status_codes::OK,par);
}
void Febv1Manager::c_getCalibrationStatus(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  this->getCalibrationStatus();
  PM_INFO(_logFebv1, "GetCalibrationStatus called ");
  par["CALIBRATION"] = _jControl;
  Reply(status_codes::OK,par);
}
void Febv1Manager::c_setDuration(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  uint32_t duration = 255;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    if (it2->first.compare("value")==0)
      duration=std::stoi(it2->second);

  _duration = duration;
  this->setDuration();
  PM_INFO(_logFebv1, "Setduration called with " << duration << " " << _duration);
  par["DURATION"] = json::value::number(_duration);
  Reply(status_codes::OK,par);
}

void Febv1Manager::c_setMode(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  uint32_t mode = 2;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    if (it2->first.compare("value")==0)
      mode=std::stoi(it2->second);

  if (mode != 2)
    _type = mode;
  PM_INFO(_logFebv1, "SetMode called with Mode " << mode << "  Type " << _type);
  par["MODE"] = json::value::number(_type);
  Reply(status_codes::OK,par);
}
void Febv1Manager::c_downloadDB(http_request m)
{
  PM_INFO(_logFebv1,"downloadDB called ");
  auto par = json::value::object();
  par["STATUS"]=json::value::string(U("DONE"));


  
  std::string dbstate="NONE";
  uint32_t version=0;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("state")==0)  state=it2->second;
      if (it2->first.compare("version")==0)  version=std::stoi(it2->second);
    }

  web::json::value jTDC=this->parameters()["febv1"];
  if (jTDC.isMember("db"))
    {
      web::json::value jTDCdb=jTDC["db"];
      _tca->clear();

      if (jTDCdb["mode"].as_string().compare("mongo")!=0)
	_tca->parseDb(dbstate,jTDCdb["mode"].as_string());
      else
	_tca->parseMongoDb(dbstate,version);

	 
    }
  par["DBSTATE"]=json::value::string(U(dbstate));
  Reply(status_codes::OK,par);
}



void Febv1Manager::c_scurve(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  uint32_t first = 420,last=530,step=2,mode=255;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("first")==0)  first=std::stoi(it2->second);
      if (it2->first.compare("last")==0)  last=std::stoi(it2->second);
      if (it2->first.compare("step")==0)  step=std::stoi(it2->second);
      if (it2->first.compare("channel")==0)  mode=std::stoi(it2->second);
    }

  //  PM_INFO(_logFebv1, " SetOneVthTime called with vth " << vth << " feb " << feb << " asic " << asic);
  
  //this->Scurve(mode,first,last,step);

  _sc_mode=mode;
  _sc_thmin=first;
  _sc_thmax=last;
  _sc_step=step;
  if (_sc_running)
    {
      par["SCURVE"] ="ALREADY_RUNNING";
      return;
    }
  boost::thread_group g;
  g.create_thread(boost::bind(&Febv1Manager::thrd_scurve, this));
  par["SCURVE"] =json::value::string(U("RUNNING"));
  Reply(status_codes::OK,par);
}

void Febv1Manager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PM_INFO(_logFebv1,"****** CMD: "<<m->command());
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 
  web::json::value jtype=params()["type"];
  _type=jtype.as_integer();
  printf ("_type =%d\n",_type); 

  // Need a FEBV1 tag
  if (m->content().isMember("febv1"))
    {
      printf ("found febv1/n");
      this->parameters()["febv1"]=m->content()["febv1"];
    }
  if (!this->parameters().isMember("febv1"))
    {
      PM_ERROR(_logFebv1," No febv1 tag found ");
      return;
    }
  // Now create the Message handler
  if (_mpi==NULL)
    _mpi= new febv1::Interface();
  _mpi->initialise();

   
  web::json::value jFEBV1=this->parameters()["febv1"];
  //_msh =new MpiMessageHandler("/dev/shm");
  if (!jFEBV1.isMember("network"))
    {
      PM_ERROR(_logFebv1," No febv1:network tag found ");
      return;
    }
  // Scan the network
  std::map<uint32_t,std::string> diflist=mpi::MpiMessageHandler::scanNetwork(jFEBV1["network"].as_string());
  // Download the configuration
  if (_tca==NULL)
    {
      std::cout<< "Create config acccess"<<std::endl;
      _tca=new TdcConfigAccess();
      _tca->clear();
    }
  std::cout<< " jFEBV1 "<<jFEBV1<<std::endl;
  if (jFEBV1.isMember("json"))
    {
      web::json::value jFEBV1json=jFEBV1["json"];
      if (jFEBV1json.isMember("file"))
	{
	  _tca->parseJsonFile(jFEBV1json["file"].as_string());
	}
      else
	if (jFEBV1json.isMember("url"))
	  {
	    _tca->parseJsonUrl(jFEBV1json["url"].as_string());
	  }
    }
  if (jFEBV1.isMember("db"))
    {
      web::json::value jFEBV1db=jFEBV1["db"];
      PM_ERROR(_logFebv1,"Parsing:"<<jFEBV1db["state"].as_string()<<jFEBV1db["mode"].as_string());

              
      if (jFEBV1db["mode"].as_string().compare("mongo")!=0)	
	_tca->parseDb(jFEBV1db["state"].as_string(),jFEBV1db["mode"].as_string());
      else
	_tca->parseMongoDb(jFEBV1db["state"].as_string(),jFEBV1db["version"].asUInt());
      
    }
  if (_tca->asicMap().size()==0)
    {
      PM_ERROR(_logFebv1," No ASIC found in the configuration ");
      return;
    }
  PM_INFO(_logFebv1,"ASIC found in the configuration "<<_tca->asicMap().size() );
  // Initialise the network
  std::vector<uint32_t> vint;
  vint.clear();
  for (auto x:_tca->asicMap())
    {
      uint32_t eip= ((x.first)>>32)&0XFFFFFFFF;
      std::map<uint32_t,std::string>::iterator idif=diflist.find(eip);
      if (idif==diflist.end()) continue;
      if ( std::find(vint.begin(), vint.end(), eip) != vint.end() ) continue;
      
      PM_INFO(_logFebv1," New FEBV1 found in db "<<std::hex<<eip<<std::dec<<" IP address "<<idif->second);
      vint.push_back(eip);
      _mpi->addDevice(idif->second);
      PM_INFO(_logFebv1," Registration done for "<<eip);
    }
  //std::string network=
  // Connect to the event builder
  if (_context==NULL)
    _context= new zmq::context_t(1);

  if (m->content().isMember("publish"))
    {
      this->parameters()["publish"]=m->content()["publish"];
    }

  
  for (auto x:_mpi->boards())
    x.second->data()->autoRegister(_context,this->configuration(),"BUILDER","collectingPort");
  //x->connect(_context,this->parameters()["publish"].as_string());

  // Listen All Febv1 sockets
  _mpi->listen();

  PM_INFO(_logFebv1," Init done  ");
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);
  
  
}

void Febv1Manager::configurePR2()
{
  PM_INFO(_logFebv1," COnfigure the chips ");

  
  fprintf(stderr,"Loop on socket for Sending slow control \n");
  for (auto x:_mpi->boards())
    {
      _tca->prepareSlowControl(x.second->ipAddress());
      x.second->reg()->writeRam(_tca->slcAddr(), _tca->slcBuffer(), _tca->slcBytes());
      x.second->reg()->dumpAnswer(0);
    }
}
void Febv1Manager::configure(http_request m)
{
  auto par = json::value::object();
  PM_INFO(_logFebv1," CMD: "<<m->command());

  // Now loop on slowcontrol socket


  this->configurePR2();
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);
}

void Febv1Manager::set6bDac(uint8_t dac)
{

  //::sleep(1);

  // Modify ASIC SLC
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    for (int i = 0; i < 32; i++)
    {
      it->second.set6bDac(i, dac);
    }
  }
  // Now loop on slowcontrol socket and send packet
  this->configurePR2();
}
void Febv1Manager::cal6bDac(uint32_t mask, int32_t dacShift)
{
  PM_INFO(_logFebv1, "CAL6BDAC called " << mask << " Shift " << dacShift);
  //::usleep(50000);
  std::map<uint64_t, uint16_t *> ascopy;
  // Modify ASIC SLC
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {

    if (ascopy.find(it->first) == ascopy.end())
    {
      uint16_t *b = new uint16_t[32];
      std::pair<uint64_t, uint16_t *> p(it->first, b);
      ascopy.insert(p);
    }
    auto ic = ascopy.find(it->first);
    for (int i = 0; i < 32; i++)
    {
      ic->second[i] = it->second.get6bDac(i);
      if ((mask >> i) & 1)
      {
        uint32_t dac = it->second.get6bDac(i);
        int32_t ndac = dac + dacShift;
        if (ndac < 0)
          ndac = 0;
        if (ndac > 63)
          ndac = 63;
        std::cout << "channel " << i << " DAC " << dac << " shifted to " << ndac << std::endl;

        it->second.set6bDac(i, ndac);
      }
    }
  }
  // Now loop on slowcontrol socket and send packet
  this->configurePR2();

  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {

    auto ic = ascopy.find(it->first);
    for (int i = 0; i < 32; i++)

      it->second.set6bDac(i, ic->second[i]);
  }
  for (auto it = ascopy.begin(); it != ascopy.end(); it++)
    delete it->second;

 
}
void Febv1Manager::c_asics(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));
  web::json::value jlist;
  uint32_t na=0;
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    web::json::value jasic;
    uint32_t iasic = it->first & 0xFF;
    jasic["num"] = iasic;
    uint32_t difid = ((it->first) >> 32 & 0xFFFFFFFF);
    jasic["dif"] = difid;
    it->second.toJson();
    jasic["slc"] = it->second.getJson();
    jlist[na]=jasic;
  }
  par["asics"] = jlist;
  Reply(status_codes::OK,par);
}

void Febv1Manager::setMask(uint32_t mask, uint8_t asic)
{

  //::sleep(1);
  // Change all Asics VthTime
  uint32_t umask;
  uint32_t asica = asic;
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    uint32_t iasic = it->first & 0xFF;
    fprintf(stderr, "ASIC in map %d ASIC asked %d \n", iasic, asica);
    if ((iasic & asica) == 0)
    {
      fprintf(stderr, "Skipping asic %d by masking all channels\n", iasic);
      umask = 0;
    }
    else
      umask = mask;

    for (int i = 0; i < 32; i++)
    {
      if ((umask >> i) & 1)
      {
        it->second.setMaskDiscriTime(i, 0);
      }
      else
      {
        it->second.setMaskDiscriTime(i, 1);
      }
    }
    //std::cout << "ASIC " << (int)iasic << "==========================" << std::endl;
    //it->second.Print();
  }

  // Now loop on slowcontrol socket
  this->configurePR2();
}

void Febv1Manager::setVthTime(uint32_t vth)
{

  PM_DEBUG(_logFebv1, __PRETTY_FUNCTION__ << " Debut ");
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    int iasic = it->first & 0xFF;

    it->second.setVthTime(vth);
    //it->second.Print();
    // 1 seul ASIC break;
  }
  this->configurePR2();
 
  PM_DEBUG(_logFebv1, __PRETTY_FUNCTION__ << " Fin ");
}

void Febv1Manager::setSingleVthTime(uint32_t vth, uint32_t feb, uint32_t asic)
{
  // Encode IP
  std::stringstream ip;
  ip << "192.168.10." << feb;

  PM_DEBUG(_logFebv1, __PRETTY_FUNCTION__ << " Debut ");
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    //  Change VTH time only on specified ASIC
    uint64_t eid = (((uint64_t) mpi::MpiMessageHandler::convertIP(ip.str())) << 32) | asic;
    if (eid != it->first)
      continue;
    it->second.setVthTime(vth);
  }
  this->configurePR2();

  
  PM_DEBUG(_logFebv1, __PRETTY_FUNCTION__ << " Fin ");
}

void Febv1Manager::setDelay()
{
  PM_INFO(_logFebv1, "Setting active time " << (int) _delay);
  for (auto x : _mpi->boards())
    x.second->reg()->writeAddress(0x222, _delay);
 
}
void Febv1Manager::setDuration()
{
  PM_INFO(_logFebv1, " Setting Dead time duration " << (int) _duration);
  for (auto x : _mpi->boards())
    x.second->reg()->writeAddress(0x223, _duration);
 
}
void Febv1Manager::getLUT(int chan)
{
  PM_INFO(_logFebv1, " get LUT for " << chan << " on all FEBS");
  for (auto x : _mpi->boards())
    {
      x.second->reg()->writeAddress(0x224,chan,true);
      x.second->reg()->dumpAnswer(0);
    }
}
void Febv1Manager::getCalibrationStatus()
{
  PM_INFO(_logFebv1, " get Calibration Status for on all FEBS");
  for (auto x : _mpi->boards())
    {
      x.second->reg()->writeAddress(0x225,0,true);
      x.second->reg()->dumpAnswer(0);
    }
}
void Febv1Manager::setCalibrationMask(uint64_t mask)
{
  PM_INFO(_logFebv1, " setCalibrationMask " << std::hex << mask << std::dec << " on all FEBS");
  for (auto x : _mpi->boards())
    x.second->reg()->writeLongWord(0x226, mask);
}
void Febv1Manager::setMeasurementMask(uint64_t mask,uint32_t feb)
{
  PM_INFO(_logFebv1, " setMeasurementMask " << std::hex << mask << std::dec << " on  FEBS"<<feb);
  

  
  for (auto x :  _mpi->boards())
  {
    if (feb!=255)
      {
	std::stringstream ip;
	ip <<params()["febv1"]["network"].as_string()<< feb;
	if (ip.str().compare(x.second->ipAddress())!=0)
	  {
	    PM_INFO(_logFebv1, " setMeasurementMask " <<x.second->ipAddress()<< " skipped for FEB "<<feb);
	    continue;
	  }
      }
     x.second->reg()->writeLongWord(0x230, mask);

  }
}


/////////////////////////////////////////////////////////
void Febv1Manager::start(http_request m)
{
  auto par = json::value::object();
  PM_INFO(_logFebv1," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  // Create run file
  web::json::value jc=m->content();
  _run=jc["run"].as_integer();

  // Clear buffers
  for (auto x:_mpi->boards())
    {
      x.second->data()->clear();
    }

  // Turn run type on
  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , enabled (Bit 0 a 1)
      x.second->reg()->writeAddress(0x219,_type);
      x.second->reg()->writeAddress(0x220,1);
    }
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);  
}
void Febv1Manager::stop(http_request m)
{
  auto par = json::value::object();
  PM_INFO(_logFebv1," CMD: "<<m->command());
  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
      x.second->reg()->writeAddress(0x220,0);
    }
  ::sleep(2);

  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);
}
void Febv1Manager::destroy(http_request m)
{
  auto par = json::value::object();
  PM_INFO(_logFebv1," CMD: "<<m->command());
  PM_INFO(_logFebv1,"CLOSE called ");
  
  _mpi->close();
  for (auto x:_mpi->boards())
    delete x.second;
  _mpi->boards().clear();
  delete _mpi;
  _mpi=0;

  PM_INFO(_logFebv1," Data sockets deleted");
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);


  // To be done: _febv1->clear();
}






void Febv1Manager::ScurveStep(fsmwebCaller* mdcc,fsmwebCaller* builder,int thmin,int thmax,int step)
{

  int ncon=2000,ncoff=100,ntrg=50;
  mdcc->sendCommand("PAUSE");
  web::json::value p;
  p.clear();p["nclock"]=ncon;  mdcc->sendCommand("SPILLON",p);
  p.clear();p["nclock"]=ncoff;  mdcc->sendCommand("SPILLOFF",p);
  printf("Clock On %d Off %d \n",ncon, ncoff);
  p.clear();p["value"]=4;  mdcc->sendCommand("SETSPILLREGISTER",p);
  mdcc->sendCommand("CALIBON");
  p.clear();p["nclock"]=ntrg;  mdcc->sendCommand("SETCALIBCOUNT",p);
  int thrange=(thmax-thmin+1)/step;
  for (int vth=0;vth<=thrange;vth++)
    {
      if (!_running) break;
      mdcc->sendCommand("PAUSE");
      this->setVthTime(thmax-vth*step);
      int firstEvent=0;
      for (auto x : _mpi->boards())
	if (x.second->data()->event()>firstEvent) firstEvent=x.second->data()->event();

      p.clear();
      web::json::value h;
      h.append(2);h.append(thmax-vth*step);
      p["header"]=h;
      p["nextevent"]=firstEvent+1;
      builder->sendCommand("SETHEADER",p);
      mdcc->sendCommand("RELOADCALIB");
      mdcc->sendCommand("RESUME");
      int nloop=0,lastEvent=firstEvent;
      while (lastEvent < (firstEvent + ntrg - 20))
	{
	  ::usleep(100000);
	  for (auto x : _mpi->boards())
	    if (x.second->data()->event()>lastEvent) lastEvent=x.second->data()->event();
	  nloop++;if (nloop > 20 || !_running)  break;
	}
      printf("Step %d Th %d First %d Last %d \n",vth,thmax-vth*step,firstEvent,lastEvent);
      mdcc->sendCommand("PAUSE");
    }
  mdcc->sendCommand("CALIBOFF");
}


void Febv1Manager::thrd_scurve()
{
  _sc_running=true;
  this->Scurve(_sc_mode,_sc_thmin,_sc_thmax,_sc_step);
  _sc_running=false;
}


void Febv1Manager::Scurve(int mode,int thmin,int thmax,int step)
{
  fsmwebCaller* mdcc=findMDCC("MDCCSERVER");
  fsmwebCaller* builder=findMDCC("BUILDER");
  if (mdcc==NULL) return;
  if (builder==NULL) return;
  int firmware[]={0,2,4,6,
		  8,10,12,14,
		  16,18,20,22,
		  24,26,28,30};

  int mask=0;
  if (mode==255)
    {

      //for (int i=0;i<16;i++) mask|=(1<<firmware[i]);
      //this->setMask(mask,0xFF);
      this->ScurveStep(mdcc,builder,thmin,thmax,step);
      return;
      
    }
  if (mode==1023)
    {
      int mask=0;
      for (int i=0;i<16;i++)
	{
	  mask=(1<<firmware[i]);
	  std::cout<<"Step PR2 "<<i<<" channel "<<firmware[i]<<std::endl;
	  this->setMask(mask,0xFF);
	  this->ScurveStep(mdcc,builder,thmin,thmax,step);
	}
      return;
    }
  mask=(1<<mode);
  this->setMask(mask,0xFF);
  this->ScurveStep(mdcc,builder,thmin,thmax,step);

  
}

fsmwebCaller* Febv1Manager::findMDCC(std::string appname)
{
  web::json::value cjs=this->configuration()["HOSTS"];
  //  std::cout<<cjs<<std::endl;
  std::vector<std::string> lhosts=this->configuration()["HOSTS"].getMemberNames();
  // Loop on hosts
  for (auto host:lhosts)
    {
      //std::cout<<" Host "<<host<<" found"<<std::endl;
      // Loop on processes
      const web::json::value cjsources=this->configuration()["HOSTS"][host];
      //std::cout<<cjsources<<std::endl;
      for (web::json::valueConstIterator it = cjsources.begin(); it != cjsources.end(); ++it)
	{
	  const web::json::value& process = *it;
	  std::string p_name=process["NAME"].as_string();
	  web::json::value p_param=web::json::value::null;
	  if (process.isMember("PARAMETER")) p_param=process["PARAMETER"];
	  // Loop on environenemntal variable
	  uint32_t port=0;
	  const web::json::value& cenv=process["ENV"];
	  for (web::json::valueConstIterator iev = cenv.begin(); iev != cenv.end(); ++iev)
	    {
	      std::string envp=(*iev).as_string();
	      //      std::cout<<"Env found "<<envp.substr(0,7)<<std::endl;
	      //std::cout<<"Env found "<<envp.substr(8,envp.length()-7)<<std::endl;
	      if (envp.substr(0,7).compare("WEBPORT")==0)
		{
		  port=atol(envp.substr(8,envp.length()-7).c_str());
		  break;
		}
	    }
	  if (port==0) continue;
	  if (p_name.compare(appname)==0)
	    {
	      
	      return  new fsmwebCaller(host,port); 
	    }
	}

    }
  
  return NULL;
  
}
