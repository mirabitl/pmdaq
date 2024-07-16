#include "Gricv0Manager.hh"
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
#include <boost/format.hpp>




using namespace gricv0;


Gricv0Manager::Gricv0Manager() : _context(NULL),_hca(NULL),_mpi(NULL),_running(false),_running_mode(0)
{;}
void Gricv0Manager::initialise()
{

  // Register state

  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  
  this->addTransition("INITIALISE","CREATED","INITIALISED",std::bind(&Gricv0Manager::fsm_initialise, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","INITIALISED","CONFIGURED",std::bind(&Gricv0Manager::configure, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",std::bind(&Gricv0Manager::configure, this,std::placeholders::_1));
  
  this->addTransition("START","CONFIGURED","RUNNING",std::bind(&Gricv0Manager::start, this,std::placeholders::_1));
  this->addTransition("STOP","RUNNING","CONFIGURED",std::bind(&Gricv0Manager::stop, this,std::placeholders::_1));
  this->addTransition("DESTROY","CONFIGURED","CREATED",std::bind(&Gricv0Manager::destroy, this,std::placeholders::_1));
  this->addTransition("DESTROY","INITIALISED","CREATED",std::bind(&Gricv0Manager::destroy, this,std::placeholders::_1));
  
  
  
  //this->addCommand("JOBLOG",std::bind(&Gricv0Manager::c_joblog,this,std::placeholders::_1));
  this->addCommand("STATUS",std::bind(&Gricv0Manager::c_status,this,std::placeholders::_1));
  this->addCommand("RESET",std::bind(&Gricv0Manager::c_reset,this,std::placeholders::_1));
  this->addCommand("STARTACQ",std::bind(&Gricv0Manager::c_startacq,this,std::placeholders::_1));
  this->addCommand("STOPACQ",std::bind(&Gricv0Manager::c_stopacq,this,std::placeholders::_1));
  this->addCommand("RESET",std::bind(&Gricv0Manager::c_reset,this,std::placeholders::_1));
  this->addCommand("STORESC",std::bind(&Gricv0Manager::c_storesc,this,std::placeholders::_1));
  this->addCommand("LOADSC",std::bind(&Gricv0Manager::c_loadsc,this,std::placeholders::_1));
  this->addCommand("READSC",std::bind(&Gricv0Manager::c_readsc,this,std::placeholders::_1));
  this->addCommand("LASTABCID",std::bind(&Gricv0Manager::c_lastabcid,this,std::placeholders::_1));
  this->addCommand("LASTGTC",std::bind(&Gricv0Manager::c_lastgtc,this,std::placeholders::_1));
  this->addCommand("SETTHRESHOLDS",std::bind(&Gricv0Manager::c_setthresholds,this,std::placeholders::_1));
  this->addCommand("SHIFTTHRESHOLDS",std::bind(&Gricv0Manager::c_shiftthresholds,this,std::placeholders::_1));
  this->addCommand("SETPAGAIN",std::bind(&Gricv0Manager::c_setpagain,this,std::placeholders::_1));
  this->addCommand("SETMASK",std::bind(&Gricv0Manager::c_setmask,this,std::placeholders::_1));
  this->addCommand("SETCHANNELMASK",std::bind(&Gricv0Manager::c_setchannelmask,this,std::placeholders::_1));
  this->addCommand("DOWNLOADDB",std::bind(&Gricv0Manager::c_downloadDB,this,std::placeholders::_1));
  this->addCommand("SCURVE",std::bind(&Gricv0Manager::c_scurve,this,std::placeholders::_1));
  this->addCommand("GAINCURVE",std::bind(&Gricv0Manager::c_gaincurve,this,std::placeholders::_1));
  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
 
  
 
  // Initialise NetLink


}
void Gricv0Manager::end()
{
  // Stop any running process
  if (_sc_running)
    {
      _sc_running=false;
      g_scurve.join();
      _running_mode=0;
    }
  //Stop listening
  if (_mpi!=NULL)
    {
      if (_running)
	{
	  PMF_INFO(_logGricv0," CMD: STOPPING");
	  
	  for (auto x:_mpi->boards())
	    {
	      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
	      x.second->reg()->sendCommand(gricv0::Message::command::STOPACQ);
	    }
	  ::sleep(1);
	  _running=false;
	}
  // Close sockets
          // Terminate listening

      PMF_INFO(_logGricv0,"CLOSE CMD called ");
      for (auto x=_mpi->boards().begin();x!=_mpi->boards().end();x++)
      {
	(*x).second->reg()->sendCommand(gricv0::Message::command::CLOSE);
      }
      PMF_INFO(_logGricv0,"CLOSE DONE called ");
    PMF_INFO(_logGricv0,"TERMINATE CMD called ");
    _mpi->terminate();
    PMF_INFO(_logGricv0,"TERMINATE DONE called ");
    _mpi=NULL;
    }
  
}
web::json::value Gricv0Manager::build_status()
{
   web::json::value jl;uint32_t mb=0;
  for (auto x:_mpi->boards())
    {

      web::json::value jt;
      jt["detid"]=web::json::value::number(x.second->data()->detectorId());
      std::stringstream sid;
      sid<<std::hex<<x.second->data()->difId()<<std::dec;
      jt["sourceid"]=web::json::value::string(U(sid.str()));
      jt["SLC"]=web::json::value::number(x.second->reg()->slcStatus());
      jt["gtc"]=web::json::value::number(x.second->data()->gtc());
      jt["abcid"]=web::json::value::number(x.second->data()->abcid());
      jt["event"]=web::json::value::number(x.second->data()->event());
      jt["triggers"]=web::json::value::number(x.second->data()->triggers());
      jt["mode"]=web::json::value::number(_running_mode);
      jl[mb++]=jt;
    }
  return jl;
}

void Gricv0Manager::c_status(http_request m)
{

  auto par = json::value::object();

  PMF_INFO(_logGricv0,"Status CMD called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  auto jl=build_status();
  par["GRICSTATUS"]=jl;
  mqtt_publish("status",jl);  
  Reply(status_codes::OK,par);

}



void Gricv0Manager::c_startacq(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logGricv0,"STARTACQ CMD called ");

 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(gricv0::Message::command::STARTACQ);
    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}

void Gricv0Manager::c_stopacq(http_request m)
{
  auto par = json::value::object();
    
  PMF_INFO(_logGricv0,"STOPACQ CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(gricv0::Message::command::STOPACQ);
    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}

void Gricv0Manager::c_reset(http_request m)
{
  auto par = json::value::object();
    
  PMF_INFO(_logGricv0,"RESET CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(gricv0::Message::command::RESET);
    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}

void Gricv0Manager::c_storesc(http_request m)
{
  auto par = json::value::object();
    
  PMF_INFO(_logGricv0,"STORESC CMD called ");

  for (auto x:_mpi->boards())
    {
      _hca->prepareSlowControl(x.second->ipAddress());
      x.second->reg()->sendSlowControl(_hca->slcBuffer());
    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}

void Gricv0Manager::c_loadsc(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logGricv0,"LOADSC CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(gricv0::Message::command::LOADSC);
    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}
void Gricv0Manager::c_close(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logGricv0,"CLOSE CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(gricv0::Message::command::CLOSE);
    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}

void Gricv0Manager::c_readsc(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logGricv0,"READSC CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(gricv0::Message::command::READSC);
    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}

void Gricv0Manager::c_lastabcid(http_request m)
{
  auto par = json::value::object();
    
  PMF_INFO(_logGricv0,"LOADSC CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(gricv0::Message::command::LASTABCID);
    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}

void Gricv0Manager::c_lastgtc(http_request m)
{
  auto par = json::value::object();
    
  PMF_INFO(_logGricv0,"LOADSC CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(gricv0::Message::command::LASTGTC);
    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}


void Gricv0Manager::c_setthresholds(http_request m)
{
    auto par = json::value::object();

  PMF_INFO(_logGricv0,"Set6bdac called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  uint32_t b0=utils::queryIntValue(m,"B0",250);
  uint32_t b1=utils::queryIntValue(m,"B1",250);
  uint32_t b2=utils::queryIntValue(m,"B2",250);
  uint32_t idif=utils::queryIntValue(m,"DIF",0);
  
  this->setThresholds(b0,b1,b2,idif);
  par["THRESHOLD0"]=web::json::value::number(b0);
  par["THRESHOLD1"]=web::json::value::number(b1);
  par["THRESHOLD2"]=web::json::value::number(b2);
  Reply(status_codes::OK,par);
}
void Gricv0Manager::c_shiftthresholds(http_request m)
{
    auto par = json::value::object();

  PMF_INFO(_logGricv0,"Set6bdac called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  uint32_t b0=utils::queryIntValue(m,"B0",0);
  uint32_t b1=utils::queryIntValue(m,"B1",0);
  uint32_t b2=utils::queryIntValue(m,"B2",0);
  uint32_t idif=utils::queryIntValue(m,"DIF",0);
  
  this->shiftThresholds(b0,b1,b2,idif);
  par["SHIFTTHRESHOLD0"]=web::json::value::number(b0);
  par["SHIFTTHRESHOLD1"]=web::json::value::number(b1);
  par["SHIFTTHRESHOLD2"]=web::json::value::number(b2);
  Reply(status_codes::OK,par);
}
void Gricv0Manager::c_pulse(http_request m)
{
    auto par = json::value::object();

  PMF_INFO(_logGricv0,"Pulse called ");
  par["STATUS"]=web::json::value::string(U("DONE"));
  uint32_t p0=utils::queryIntValue(m,"value",0);
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendParameter(gricv0::Message::command::PULSE,p0);
    }
  

  par["NPULSE"]=web::json::value::number(p0&0xFF);
  Reply(status_codes::OK,par);
}

void Gricv0Manager::c_setpagain(http_request m)
{
    auto par = json::value::object();

  PMF_INFO(_logGricv0,"Set6bdac called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  uint32_t gain=utils::queryIntValue(m,"gain",128);
  this->setGain(gain);
  par["GAIN"]=web::json::value::number(gain);
  Reply(status_codes::OK,par);
}

void Gricv0Manager::c_setmask(http_request m)
{
    auto par = json::value::object();

  PMF_INFO(_logGricv0,"SetMask called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  //uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint64_t mask;
  sscanf(utils::queryStringValue(m,"mask","0XFFFFFFFFFFFFFFFF").c_str(),"%lx",&mask);
  uint32_t level=utils::queryIntValue(m,"level",0);
  PMF_INFO(_logGricv0,"SetMask called "<<std::hex<<mask<<std::dec<<" level "<<level);
  this->setMask(level,mask);
  par["MASK"]=web::json::value::number(mask);
  par["LEVEL"]=web::json::value::number(level);
    Reply(status_codes::OK,par);
}



void Gricv0Manager::c_setchannelmask(http_request m)
{
    auto par = json::value::object();

  PMF_INFO(_logGricv0,"SetMask called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  //uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint32_t level=utils::queryIntValue(m,"level",0);
  uint32_t channel=utils::queryIntValue(m,"channel",0);
  bool on=utils::queryIntValue(m,"value",1)==1;
  PMF_INFO(_logGricv0,"SetMaskChannel called "<<channel<<std::dec<<" level "<<level);
  this->setChannelMask(level,channel,on);
  par["CHANNEL"]=web::json::value::number(channel);
  par["LEVEL"]=web::json::value::number(level);
  par["ON"]=web::json::value::number(on);
    Reply(status_codes::OK,par);
}

void Gricv0Manager::c_downloadDB(http_request m)
{
    auto par = json::value::object();

  PMF_INFO(_logGricv0,"downloadDB called ");
  par["STATUS"]=web::json::value::string(U("DONE"));


  std::string dbstate=utils::queryStringValue(m,"state","NONE");


  uint32_t version=utils::queryIntValue(m,"version",0);
  web::json::value jTDC=params()["gricv0"];
  if (utils::isMember(jTDC,"db"))
    {
      web::json::value jTDCdb=jTDC["db"];
      _hca->clear();

      if (jTDCdb["mode"].as_string().compare("mongo")==0)
	_hca->parseMongoDb(dbstate,version);

	 
    }
  par["DBSTATE"]=web::json::value::string(U(dbstate));
  Reply(status_codes::OK,par);
}

void Gricv0Manager::fsm_initialise(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logGricv0,"****** CMD: INITIALISING");
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 
  // Need a GRICV0 tag
  if (!utils::isMember(params(),"gricv0"))
    {
      PMF_ERROR(_logGricv0," No gricv0 tag found ");
      par["STATUS"]=web::json::value::string(U("Missing gricv0 tag"));
      Reply(status_codes::OK,par);
      return;
    }
  // Now create the Message handler
  PMF_INFO(_logGricv0,"Starting MPI ");
  if (_mpi==NULL)
    _mpi= new gricv0::Interface();
  _mpi->initialise();

  PMF_INFO(_logGricv0,"Go thru parameters ");
  web::json::value jGRICV0=params()["gricv0"];
  //_msh =new MpiMessageHandler("/dev/shm");
  if (!utils::isMember(jGRICV0,"network"))
    {
      PMF_ERROR(_logGricv0," No gricv0:network tag found ");
      par["STATUS"]=web::json::value::string(U("Missing network tag"));
      Reply(status_codes::OK,par);
      return;
    }
  // Download the configuration
  if (_hca==NULL)
    {
      std::cout<< "Create config acccess"<<std::endl;
      _hca=new HR2ConfigAccess();
      _hca->clear();
    }
  std::cout<< " jGRICV0 "<<jGRICV0<<std::endl;

  std::vector<string> *accepted=NULL;
  if (utils::isMember(jGRICV0,"accepted"))
    {
      accepted= new   std::vector<std::string>;
      for (auto it = jGRICV0["accepted"].as_array().begin(); it != jGRICV0["accepted"].as_array().end(); it++)
	{

	  PMF_INFO(_logGricv0, " acppeted IP" << (*it).as_string());
	  accepted->push_back((*it).as_string());
	  
	}
    }
  std::vector<string> *rejected=NULL;
  if (utils::isMember(jGRICV0,"rejected"))
    {
      rejected= new   std::vector<std::string>;
      for (auto it = jGRICV0["rejected"].as_array().begin(); it != jGRICV0["rejected"].as_array().end(); it++)
	{

	  PMF_INFO(_logGricv0, " rejected IP" << (*it).as_string());
	  rejected->push_back((*it).as_string());
	  
	}
    }

  // Scan the network
  PMF_INFO(_logGricv0," Scanning gricv0:network  "<<jGRICV0["network"].as_string());
  std::map<uint32_t,std::string> diflist=utils::scanNetwork(jGRICV0["network"].as_string(),rejected,accepted);

  //DB stuff
  if (utils::isMember(jGRICV0,"json"))
    {
      web::json::value jGRICV0json=jGRICV0["json"];
      if (utils::isMember(jGRICV0json,"file"))
	{
	  _hca->parseJsonFile(jGRICV0json["file"].as_string());
	}
      else
	if (utils::isMember(jGRICV0json,"url"))
	  {
	    _hca->parseJsonUrl(jGRICV0json["url"].as_string());
	  }
    }
  if (utils::isMember(jGRICV0,"db"))
    {
      web::json::value jGRICV0db=jGRICV0["db"];
      PMF_ERROR(_logGricv0,"Parsing:"<<jGRICV0db["state"].as_string()<<jGRICV0db["mode"].as_string());

              
      if (jGRICV0db["mode"].as_string().compare("mongo")==0)	
	_hca->parseMongoDb(jGRICV0db["state"].as_string(),jGRICV0db["version"].as_integer());
      
    }
  if (_hca->asicMap().size()==0)
    {
      PMF_ERROR(_logGricv0," No ASIC found in the configuration ");
      return;
    }
  PMF_INFO(_logGricv0,"ASIC found in the configuration "<<_hca->asicMap().size() );
  // Initialise the network
  std::vector<uint32_t> vint;
  vint.clear();
  for (auto x:_hca->asicMap())
    {
      uint32_t eip= ((x.first)>>32)&0XFFFFFFFF;
      std::map<uint32_t,std::string>::iterator idif=diflist.find(eip);
      if (idif==diflist.end()) continue;
      if (std::find(vint.begin(), vint.end(), eip) != vint.end() ) continue;
      
      PMF_INFO(_logGricv0," New GRICV0 found in db "<<std::hex<<eip<<std::dec<<" IP address "<<idif->second);
      vint.push_back(eip);
      _mpi->addDevice(idif->second);
      PMF_INFO(_logGricv0," Registration done for "<<eip);
    }
  //std::string network=
  // Connect to the event builder
  if (_context==NULL)
    _context= new zmq::context_t(1);

  for (auto x:_mpi->boards())
    x.second->data()->autoRegister(_context,session(),"evb_builder","collectingPort");
  //x->connect(_context,params()["publish"].as_string());

  // Listen All Gricv0 sockets
  _mpi->listen();

  PMF_INFO(_logGricv0," Init done  ");
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);
}

void Gricv0Manager::configureHR2()
{
  PMF_INFO(_logGricv0," COnfigure the chips ");

  
  // Now loop on slowcontrol socket
  fprintf(stderr,"Loop on socket for Sending slow control \n");
  for (auto x:_mpi->boards())
    {
      _hca->prepareSlowControl(x.second->ipAddress());
      x.second->reg()->sendSlowControl(_hca->slcBuffer());
    }
  
  PMF_INFO(_logGricv0," Maintenant on charge ");
  for (auto x:_mpi->boards())
    {
      uint32_t status=x.second->reg()->sendCommand(gricv0::Message::command::LOADSC);
      x.second->reg()->setSlcStatus(status);
    }

}
void Gricv0Manager::configure(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logGricv0," CMD: CONFIGURING");

  // Now loop on slowcontrol socket


  this->configureHR2();
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);
}

void Gricv0Manager::setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif)
{

  PMF_INFO(_logGricv0," Changin thresholds: "<<b0<<","<<b1<<","<<b2);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      if (idif!=0)
	{
	  uint32_t ip=(((it->first)>>32&0XFFFFFFFF)>>16)&0xFFFF;
	  printf("%lx %x %x \n",(it->first>>32),ip,idif);
	  if (idif!=ip) continue;
	}
      it->second.setB0(b0);
      it->second.setB1(b1);
      it->second.setB2(b2);
      //it->second.setHEADER(0x56);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::usleep(10000);

}
void Gricv0Manager::shiftThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif)
{

  PMF_INFO(_logGricv0," shifting thresholds: "<<b0<<","<<b1<<","<<b2);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      if (idif!=0)
	{
	  uint32_t ip=(((it->first)>>32&0XFFFFFFFF)>>16)&0xFFFF;
	  printf("%lx %x %x \n",(it->first>>32),ip,idif);
	  if (idif!=ip) continue;
	}
      it->second.setB0( it->second.getB0()+b0);
      it->second.setB1(it->second.getB1()+b1);
      it->second.setB2(it->second.getB2()+b2);
      //it->second.setHEADER(0x56);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::usleep(10000);

}
void Gricv0Manager::setAllMasks(uint64_t mask)
{
  PMF_INFO(_logGricv0," Changing Mask: "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      it->second.dumpBinary();
      it->second.setMASK(0,mask);
      it->second.setMASK(1,mask);
      it->second.setMASK(2,mask);
      it->second.dumpBinary();
	    
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::usleep(1);

}
void Gricv0Manager::setCTEST(uint64_t mask)
{
  PMF_INFO(_logGricv0," Changing CTEST: "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      for (int i=0;i<64;i++)
	it->second.setCTEST(i,(mask>>i)&1);

    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::usleep(1);

}

void Gricv0Manager::setGain(uint16_t gain)
{

  PMF_INFO(_logGricv0," Changing Gain: "<<gain);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      for (int i=0;i<64;i++)
	it->second.setPAGAIN(i,gain);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::sleep(1);

}

void Gricv0Manager::setMask(uint32_t level,uint64_t mask)
{
  PMF_INFO(_logGricv0," Changing Mask: "<<level<<" "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASK(level,mask);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::sleep(1);

}
void Gricv0Manager::setChannelMask(uint16_t level,uint16_t channel,uint16_t val)
{
  PMF_INFO(_logGricv0," Changing Mask: "<<level<<" "<<std::hex<<channel<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASKChannel(level,channel,val==1);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::sleep(1);

}

void Gricv0Manager::start(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logGricv0," CMD: STARTING");
 
  // Create run file
  _run=utils::queryIntValue(m,"run",0);

  // Clear buffers
  for (auto x:_mpi->boards())
    {
      x.second->data()->clear();
    }

  // Turn run type on
  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , enabled (Bit 0 a 1)
      x.second->reg()->sendCommand(gricv0::Message::command::STARTACQ);
    }
   _running=true;
 par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);

   
}
void Gricv0Manager::stop(http_request m)
{

  auto par = json::value::object();
  PMF_INFO(_logGricv0," CMD: STOPPING");

  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
      x.second->reg()->sendCommand(gricv0::Message::command::STOPACQ);
    }
  ::sleep(2);
 _running=false;
 par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);


}
void Gricv0Manager::destroy(http_request m)
{
  
  auto par = json::value::object();

  PMF_INFO(_logGricv0,"CLOSE called ");

  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
      x.second->reg()->sendCommand(gricv0::Message::command::CLOSE);
    }
if (_mpi!=NULL)
    {
      _mpi->terminate();

  _mpi->close();
  for (auto x:_mpi->boards())
    delete x.second;
  _mpi->boards().clear();
  delete _mpi;
  _mpi=0;
    }

  PMF_INFO(_logGricv0," Data sockets deleted");
    par["status"]=json::value::string(U("done"));

  Reply(status_codes::OK,par);


  // To be done: _gricv0->clear();
}


void Gricv0Manager::ScurveStep(std::string mdcc,std::string builder,int thmin,int thmax,int step)
{

  
  int ncon = _sc_win, ncoff = 1000, ntrg = _sc_ntrg;
  utils::sendCommand(mdcc,"PAUSE",json::value::null());
  web::json::value p;
  p["nclock"]=ncon;  utils::sendCommand(mdcc,"SPILLON",p);
  p["nclock"]=ncoff;  utils::sendCommand(mdcc,"SPILLOFF",p);
  PMF_DEBUG(_logGricv0,"Clock "<<ncon<<"/"<< ncoff);
  p["value"]=4;  utils::sendCommand(mdcc,"SETSPILLREGISTER",p);
  //::sleep(20);
  utils::sendCommand(mdcc,"CALIBON",json::value::null());
  p["nclock"]=ntrg;  utils::sendCommand(mdcc,"SETCALIBCOUNT",p);
  int thrange=(thmax-thmin+1)/step;
  for (int vth=0;vth<=thrange;vth++)
    {
      if (!_running) break;
      utils::sendCommand(mdcc,"PAUSE",json::value::null());
      for (auto x:_mpi->boards())
	{
	  // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
	  x.second->reg()->sendCommand(gricv0::Message::command::STOPACQ);
	}

      usleep(1000);
      //this->setThresholds(thmax-vth*step,512,512);
      if (_sc_gmin!=0)
	this->setGain(_sc_gmin);
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
      int firstEvent=0;
#define USEFEBS
#ifdef USEFEBS
      for (auto x : _mpi->boards())
	if (x.second->data()->event()>firstEvent) firstEvent=x.second->data()->event();
#else
      auto frep = utils::sendCommand(builder, "STATUS", json::value::null());
      auto jfrep = frep.extract_json();
      auto jfanswer = jfrep.get().as_object()["answer"];
      firstEvent = jfanswer["event"].as_integer();  
#endif
      firstEvent=0;
      web::json::value h;
      h[0]=2;h[1]=json::value::number(thmax-vth*step);
      web::json::value ph;
      ph["header"]=h;
      //ph["nextevent"]=web::json::value::number(firstEvent+1);
      ph["nextevent"]=web::json::value::number(2);
      utils::sendCommand(builder,"SETHEADER",ph);
      printf("SETHEADER executed\n");
      utils::sendCommand(mdcc,"RELOADCALIB",json::value::null());
      for (auto x:_mpi->boards())
	{
	  x.second->data()->clear();
	}

      // Turn run type on
      for (auto x:_mpi->boards())
	{
	  // Automatic FSM (bit 1 a 0) , enabled (Bit 0 a 1)
	  x.second->reg()->sendCommand(gricv0::Message::command::STARTACQ);
	}
      
      utils::sendCommand(mdcc,"RESUME",json::value::null());
      firstEvent=0;
      int nloop=0,lastEvent=firstEvent;
#ifdef USEFEBS
      while (lastEvent < (firstEvent + ntrg - 1))
	{
	  ::usleep(10000);
	  for (auto x : _mpi->boards())
	    if (x.second->data()->event()>lastEvent) lastEvent=x.second->data()->event();
	  nloop++;if (nloop > 60000 || !_running)  break;
	}
      lastEvent=0;
      while (lastEvent < (firstEvent + ntrg - 2))
	{
	  ::usleep(100000);
	  auto rep = utils::sendCommand(builder, "STATUS", json::value::null());
	  auto jrep = rep.extract_json();
	  auto janswer = jrep.get().as_object()["answer"];
	  lastEvent = janswer["event"].as_integer(); // A verifier
	  std::cout<<lastEvent<<" CMP "<<(firstEvent + ntrg - 1)<<std::endl;
	  nloop++;
	  if (nloop > 100 || !_running)
	    break;
	}
#else
      while (lastEvent < (firstEvent + ntrg - 1))
	{
	  ::usleep(100000);
	  auto rep = utils::sendCommand(builder, "STATUS", json::value::null());
	  auto jrep = rep.extract_json();
	  auto janswer = jrep.get().as_object()["answer"];
	  lastEvent = janswer["event"].as_integer(); // A verifier
	  nloop++;
	  if (nloop > 100 || !_running)
	    break;
	}
#endif



      PMF_INFO(_logGricv0, "Step:" << vth << " Threshold:" << thmax - vth * step << " First:" << firstEvent << " Last:" << lastEvent);
      //printf("Step %d Th %d First %d Last %d loops %d \n",vth,thmax-vth*step,firstEvent,lastEvent,nloop);
      utils::sendCommand(mdcc,"PAUSE",json::value::null());
    }
  utils::sendCommand(mdcc,"CALIBOFF",json::value::null());
}


void Gricv0Manager::thrd_scurve()
{
  _sc_running=true;
  this->Scurve(_sc_mode,_sc_thmin,_sc_thmax,_sc_step);
  _sc_running=false;
}


void Gricv0Manager::Scurve(int mode,int thmin,int thmax,int step)
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
	      PMF_ERROR(_logGricv0,"No MDCC/MBMDCC/IPDC available, exiting Scurve");

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

      mask=0xFFFFFFFFFFFFFFFF;
      this->setAllMasks(mask);
      this->setCTEST(0);
      this->ScurveStep(mdcc,builder,thmin,thmax,step);
       _running_mode=0;
       mqtt_publish("status",build_status());
      return;
      
    }

  // Chanel per channel pedestal (CTEST is active)
  if (mode==1023)
    {
      mask=0;
      for (int i=0;i<64;i++)
	{
	  mask=(1ULL<<i);
	  PMF_INFO(_logGricv0, "Step HR2 " << i << "  channel" << i << std::dec);
	  _sc_channel=i;
	  this->setAllMasks(mask);
	  this->setCTEST(mask);
	  this->ScurveStep(mdcc,builder,thmin,thmax,step);
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
	  _sc_channel=i;
	  mask = (1ULL << i);
	  PMF_INFO(_logGricv0, "Step HR2 " << i << "  channel" << i << std::dec);

	  // std::cout<<"Step HR2 "<<i<<" channel "<<i<<std::endl;
	  this->setCTEST(mask);
	  this->setAllMasks(0xFFFFFFFFFFFFFFFF);

	  this->ScurveStep(mdcc, builder, thmin, thmax, step);
	}
       _running_mode=0;
       mqtt_publish("status",build_status());
      return;
    }
  // One channel pedestal
  mask=(1ULL<<mode);
  PMF_INFO(_logGricv0, "CTEST One " << mode << " " << std::hex << mask << std::dec);
  this->setAllMasks(mask);
  this->setCTEST(mask);
  this->ScurveStep(mdcc,builder,thmin,thmax,step);
 _running_mode=0;
 mqtt_publish("status",build_status());
  
}



void Gricv0Manager::GainCurveStep(std::string mdcc,std::string builder,int gmin,int gmax,int step,int threshold)
{

  int ncon=2000,ncoff=100,ntrg=20;
  utils::sendCommand(mdcc,"PAUSE",json::value::null());
  web::json::value p;
  p["nclock"]=ncon;  utils::sendCommand(mdcc,"SPILLON",p);
  p["nclock"]=ncoff;  utils::sendCommand(mdcc,"SPILLOFF",p);
  printf("Clock On %d Off %d \n",ncon, ncoff);
  p["value"]=4;  utils::sendCommand(mdcc,"SETSPILLREGISTER",p);
  //::sleep(20);
  utils::sendCommand(mdcc,"CALIBON",json::value::null());
  p["nclock"]=ntrg;  utils::sendCommand(mdcc,"SETCALIBCOUNT",p);
  
  int grange=(gmax-gmin+1)/step;
  for (int g=0;g<=grange;g++)
    {
      if (!_running) break;
      utils::sendCommand(mdcc,"PAUSE",json::value::null());
      for (auto x:_mpi->boards())
	{
	  // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
	  x.second->reg()->sendCommand(gricv0::Message::command::STOPACQ);
	}

      usleep(1000);
      this->setThresholds(threshold,512,512);
      this->setGain(gmin+g*step);
      _running_mode=4+((_sc_channel&0xFF)<<4)+((threshold&0XFFF)<<12)+((g&0XFFF)<<24);
      mqtt_publish("status",build_status());
      int firstEvent=0;
#define USEFEBS
#ifdef USEFEBS
      for (auto x : _mpi->boards())
	if (x.second->data()->event()>firstEvent) firstEvent=x.second->data()->event();
#else
      auto frep = utils::sendCommand(builder, "STATUS", json::value::null());
      auto jfrep = frep.extract_json();
      auto jfanswer = jfrep.get().as_object()["answer"];
      firstEvent = jfanswer["event"].as_integer();  
#endif
      firstEvent=0;
      web::json::value h;
      h[0]=3;h[1]=json::value::number(gmin+g*step);
      web::json::value ph;
      ph["header"]=h;
      ph["nextevent"]=web::json::value::number(firstEvent+1);
      utils::sendCommand(builder,"SETHEADER",ph);
      printf("SETHEADER executed\n");
      utils::sendCommand(mdcc,"RELOADCALIB",json::value::null());
      for (auto x:_mpi->boards())
	{
	  x.second->data()->clear();
	}

      // Turn run type on
      for (auto x:_mpi->boards())
	{
	  // Automatic FSM (bit 1 a 0) , enabled (Bit 0 a 1)
	  x.second->reg()->sendCommand(gricv0::Message::command::STARTACQ);
	}
      
      utils::sendCommand(mdcc,"RESUME",json::value::null());
      firstEvent=0;
      int firstInBoard=0;
      int nloop=0,lastEvent=firstEvent,lastInBoard=firstInBoard;

      while (lastInBoard< (firstInBoard + ntrg - 1))
	{
	  ::usleep(10000);
	  for (auto x : _mpi->boards())
	    if (x.second->data()->event()>lastInBoard) lastInBoard=x.second->data()->event();
	  nloop++;if (nloop > 600 || !_running)  break;
	}
      while (lastEvent < (firstEvent + ntrg - 1))
	{
	  ::usleep(100000);
	  auto rep = utils::sendCommand(builder, "STATUS", json::value::null());
	  auto jrep = rep.extract_json();
	  auto janswer = jrep.get().as_object()["answer"];
	  lastEvent = janswer["event"].as_integer(); // A verifier
	  nloop++;
	  if (nloop > 100 || !_running)
	    break;
	}




      
      printf("Step %d Gain %d First %d Last %d loops %d \n",g,gmin+g*step,firstEvent,lastEvent,nloop);
      utils::sendCommand(mdcc,"PAUSE",json::value::null());
    }
  utils::sendCommand(mdcc,"CALIBOFF",json::value::null());
}


void Gricv0Manager::thrd_gaincurve()
{
  _sc_running=true;
  this->GainCurve(_sc_mode,_sc_gmin,_sc_gmax,_sc_step,_sc_threshold);
  _sc_running=false;
}


void Gricv0Manager::GainCurve(int mode,int gmin,int gmax,int step,int threshold)
{
  std::string mdcc=utils::findUrl(session(),"lyon_mdcc",0);
  //std::string builder=utils::findUrl(session(),"lyon_evb",0);
  std::string builder=utils::findUrl(session(),"evb_builder",0);

  if (mdcc.compare("")==0) return;
  if (builder.compare("")==0) return;

  uint64_t mask=0;
  _sc_channel=mode;
  // All channel pedestal
  if (mode==255)
    {

      _sc_channel=0XFE;

      mask=0xFFFFFFFFFFFFFFFF;
      this->setAllMasks(mask);
      this->GainCurveStep(mdcc,builder,gmin,gmax,step,threshold);
      _running_mode=0;
      mqtt_publish("status",build_status());

      return;
      
    }

  // Chanel per channel pedestal (CTEST is active)
  if (mode==1023)
    {
      mask=0;
      for (int i=0;i<64;i++)
	{
	  _sc_channel=i;
	  mask=(1ULL<<i);
	  std::cout<<"Step HR2 "<<i<<" channel "<<i<<std::endl;
	  this->setAllMasks(mask);
	  this->setCTEST(mask);
	  this->GainCurveStep(mdcc,builder,gmin,gmax,step,threshold);
	  //this->ScurveStep(mdcc,builder,thmin,thmax,step);
	}
       _running_mode=0;
       mqtt_publish("status",build_status());
 
      return;
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




void Gricv0Manager::c_scurve(http_request m)
{
  auto par = json::value::object();

  par["STATUS"]=web::json::value::string(U("DONE"));
  _sc_thmin = utils::queryIntValue(m, "first", 80);
  _sc_thmax = utils::queryIntValue(m, "last", 250);
  _sc_step = utils::queryIntValue(m, "step", 1);
  _sc_mode = utils::queryIntValue(m, "channel", 255);
  _sc_win = utils::queryIntValue(m, "window", 50000);
  _sc_ntrg = utils::queryIntValue(m, "ntrg", 50);
  _sc_level = utils::queryIntValue(m, "level", 0);
  _sc_gmin = utils::queryIntValue(m, "gain", 0);
  
  PMF_INFO(_logGricv0, " SCurve called  mode: " << _sc_mode << " TH_Min " << _sc_thmin << " TH_Max " << _sc_thmax <<" Step "<<_sc_step <<" Level "<<_sc_level);
  
  //this->Scurve(mode,first,last,step);

  if (_sc_running)
    {
      par["SCURVE"]=web::json::value::string(U("ALREADY_RUNNING"));
      return;
    }
 
  g_scurve=std::thread(std::bind(&Gricv0Manager::thrd_scurve, this));
  par["SCURVE"]=web::json::value::string(U("RUNNING"));
  Reply(status_codes::OK,par);


}
void Gricv0Manager::c_gaincurve(http_request m)
{
  auto par = json::value::object();

  par["STATUS"]=web::json::value::string(U("DONE"));

  uint32_t first = utils::queryIntValue(m,"first",80);
  uint32_t last = utils::queryIntValue(m,"last",250);
  uint32_t step = utils::queryIntValue(m,"step",1);
  uint32_t mode = utils::queryIntValue(m,"channel",255);
  uint32_t thr = utils::queryIntValue(m,"threshold",255);
  PMF_INFO(_logGricv0, " GainCurve called " << mode << " first " << first << " last " << last <<" step "<<step<< " Threshold "<<thr);
  
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
 
  g_scurve=std::thread(std::bind(&Gricv0Manager::thrd_gaincurve, this));
  par["GAINCURVE"]=web::json::value::string(U("RUNNING"));
  Reply(status_codes::OK,par);


}
extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new Gricv0Manager);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}

