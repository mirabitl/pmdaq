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




#include "fsmwebCaller.hh"

using namespace zdaq;
using namespace lydaq;


lydaq::Gricv0Manager::Gricv0Manager(std::string name) : zdaq::baseApplication(name),_context(NULL),_hca(NULL),_mpi(NULL),_running(false)
{
  _fsm=this->fsm();
  // Register state

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&lydaq::Gricv0Manager::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&lydaq::Gricv0Manager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::Gricv0Manager::configure, this,_1));
  
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&lydaq::Gricv0Manager::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&lydaq::Gricv0Manager::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&lydaq::Gricv0Manager::destroy, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&lydaq::Gricv0Manager::destroy, this,_1));
  
  
  
  //_fsm->addCommand("JOBLOG",boost::bind(&lydaq::Gricv0Manager::c_joblog,this,_1,_2));
  _fsm->addCommand("STATUS",boost::bind(&lydaq::Gricv0Manager::c_status,this,_1,_2));
  _fsm->addCommand("RESET",boost::bind(&lydaq::Gricv0Manager::c_reset,this,_1,_2));
  _fsm->addCommand("STARTACQ",boost::bind(&lydaq::Gricv0Manager::c_startacq,this,_1,_2));
  _fsm->addCommand("STOPACQ",boost::bind(&lydaq::Gricv0Manager::c_stopacq,this,_1,_2));
  _fsm->addCommand("RESET",boost::bind(&lydaq::Gricv0Manager::c_reset,this,_1,_2));
  _fsm->addCommand("STORESC",boost::bind(&lydaq::Gricv0Manager::c_storesc,this,_1,_2));
  _fsm->addCommand("LOADSC",boost::bind(&lydaq::Gricv0Manager::c_loadsc,this,_1,_2));
  _fsm->addCommand("READSC",boost::bind(&lydaq::Gricv0Manager::c_readsc,this,_1,_2));
  _fsm->addCommand("LASTABCID",boost::bind(&lydaq::Gricv0Manager::c_lastabcid,this,_1,_2));
  _fsm->addCommand("LASTGTC",boost::bind(&lydaq::Gricv0Manager::c_lastgtc,this,_1,_2));
  _fsm->addCommand("SETTHRESHOLDS",boost::bind(&lydaq::Gricv0Manager::c_setthresholds,this,_1,_2));
  _fsm->addCommand("SETPAGAIN",boost::bind(&lydaq::Gricv0Manager::c_setpagain,this,_1,_2));
  _fsm->addCommand("SETMASK",boost::bind(&lydaq::Gricv0Manager::c_setmask,this,_1,_2));
  _fsm->addCommand("SETCHANNELMASK",boost::bind(&lydaq::Gricv0Manager::c_setchannelmask,this,_1,_2));
  _fsm->addCommand("DOWNLOADDB",boost::bind(&lydaq::Gricv0Manager::c_downloadDB,this,_1,_2));
  _fsm->addCommand("SCURVE",boost::bind(&lydaq::Gricv0Manager::c_scurve,this,_1,_2));
  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Service "<<name<<" is starting on "<<atoi(wp));

      
      _fsm->start(atoi(wp));
    }
    
  
 
  // Initialise NetLink


}
void lydaq::Gricv0Manager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Status CMD called ");
  response["STATUS"]="DONE";

  Json::Value jl;
  for (auto x:_mpi->boards())
    {

      Json::Value jt;
      jt["detid"]=x.second->data()->detectorId();
      std::stringstream sid;
      sid<<std::hex<<x.second->data()->difId()<<std::dec;
      jt["sourceid"]=sid.str();
      jt["SLC"]=x.second->reg()->slcStatus();
      jt["gtc"]=x.second->data()->gtc();
      jt["abcid"]=(Json::Value::UInt64)x.second->data()->abcid();
      jt["event"]=x.second->data()->event();
      jt["triggers"]=x.second->data()->triggers();
      jl.append(jt);
    }
  response["GRICSTATUS"]=jl;
}



void lydaq::Gricv0Manager::c_startacq(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"STARTACQ CMD called ");

 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(lydaq::gricv0::Message::command::STARTACQ);
    }
  response["STATUS"]="DONE";
}

void lydaq::Gricv0Manager::c_stopacq(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"STOPACQ CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(lydaq::gricv0::Message::command::STOPACQ);
    }
  response["STATUS"]="DONE";  
}

void lydaq::Gricv0Manager::c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"RESET CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(lydaq::gricv0::Message::command::RESET);
    }
  response["STATUS"]="DONE"; 
}

void lydaq::Gricv0Manager::c_storesc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"STORESC CMD called ");

  for (auto x:_mpi->boards())
    {
      _hca->prepareSlowControl(x.second->ipAddress());
      x.second->reg()->sendSlowControl(_hca->slcBuffer());
    }
  response["STATUS"]="DONE";
}

void lydaq::Gricv0Manager::c_loadsc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"LOADSC CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(lydaq::gricv0::Message::command::LOADSC);
    }
  response["STATUS"]="DONE";
}
void lydaq::Gricv0Manager::c_close(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"CLOSE CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(lydaq::gricv0::Message::command::CLOSE);
    }
  response["STATUS"]="DONE";
}

void lydaq::Gricv0Manager::c_readsc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"READSC CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(lydaq::gricv0::Message::command::READSC);
    }
  response["STATUS"]="DONE";
}

void lydaq::Gricv0Manager::c_lastabcid(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"LOADSC CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(lydaq::gricv0::Message::command::LASTABCID);
    }
  response["STATUS"]="DONE";
}

void lydaq::Gricv0Manager::c_lastgtc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"LOADSC CMD called ");
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendCommand(lydaq::gricv0::Message::command::LASTGTC);
    }
  response["STATUS"]="DONE";
}


void lydaq::Gricv0Manager::c_setthresholds(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Set6bdac called ");
  response["STATUS"]="DONE";

  
  uint32_t b0=atol(request.get("B0","250").c_str());
  uint32_t b1=atol(request.get("B1","250").c_str());
  uint32_t b2=atol(request.get("B2","250").c_str());
  uint32_t idif=atol(request.get("DIF","0").c_str());
  
  this->setThresholds(b0,b1,b2,idif);
  response["THRESHOLD0"]=b0;
  response["THRESHOLD1"]=b1;
  response["THRESHOLD2"]=b2;
}
void lydaq::Gricv0Manager::c_pulse(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Pulse called ");
  response["STATUS"]="DONE";
  uint32_t p0=atol(request.get("value","0").c_str());
 for (auto x:_mpi->boards())
    {
      x.second->reg()->sendParameter(lydaq::gricv0::Message::command::PULSE,p0);
    }
  

  response["NPULSE"]=p0&0xFF;

}

void lydaq::Gricv0Manager::c_setpagain(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Set6bdac called ");
  response["STATUS"]="DONE";

  
  uint32_t gain=atol(request.get("gain","128").c_str());
  this->setGain(gain);
  response["GAIN"]=gain;

}

void lydaq::Gricv0Manager::c_setmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMask called ");
  response["STATUS"]="DONE";

  
  //uint32_t nc=atol(request.get("value","4294967295").c_str());
  uint64_t mask;
  sscanf(request.get("mask","0XFFFFFFFFFFFFFFFF").c_str(),"%lx",&mask);
  uint32_t level=atol(request.get("level","0").c_str());
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMask called "<<std::hex<<mask<<std::dec<<" level "<<level);
  this->setMask(level,mask);
  response["MASK"]=(Json::UInt64) mask;
  response["LEVEL"]=level;
}



void lydaq::Gricv0Manager::c_setchannelmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMask called ");
  response["STATUS"]="DONE";

  
  //uint32_t nc=atol(request.get("value","4294967295").c_str());
  uint32_t level=atol(request.get("level","0").c_str());
  uint32_t channel=atol(request.get("channel","0").c_str());
  bool on=atol(request.get("value","1").c_str())==1;
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMaskChannel called "<<channel<<std::dec<<" level "<<level);
  this->setChannelMask(level,channel,on);
  response["CHANNEL"]=channel;
  response["LEVEL"]=level;
  response["ON"]=on;
}

void lydaq::Gricv0Manager::c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"downloadDB called ");
  response["STATUS"]="DONE";


  
  std::string dbstate=request.get("state","NONE");
  uint32_t version=atol(request.get("version","0").c_str());
  Json::Value jTDC=this->parameters()["gricv0"];
  if (jTDC.isMember("db"))
    {
      Json::Value jTDCdb=jTDC["db"];
      _hca->clear();

      if (jTDCdb["mode"].asString().compare("mongo")!=0)
	_hca->parseDb(dbstate,jTDCdb["mode"].asString());
      else
	_hca->parseMongoDb(dbstate,version);

	 
    }
  response["DBSTATE"]=dbstate;
}

void lydaq::Gricv0Manager::initialise(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"****** CMD: "<<m->command());
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 
  Json::Value jtype=this->parameters()["type"];
  _type=jtype.asInt();
  printf ("_type =%d\n",_type); 

  // Need a GRICV0 tag
  if (m->content().isMember("gricv0"))
    {
      printf ("found gricv0/n");
      this->parameters()["gricv0"]=m->content()["gricv0"];
    }
  if (!this->parameters().isMember("gricv0"))
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No gricv0 tag found ");
      return;
    }
  // Now create the Message handler
  if (_mpi==NULL)
    _mpi= new lydaq::gricv0::Interface();
  _mpi->initialise();

   
  Json::Value jGRICV0=this->parameters()["gricv0"];
  //_msh =new lydaq::MpiMessageHandler("/dev/shm");
  if (!jGRICV0.isMember("network"))
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No gricv0:network tag found ");
      return;
    }
  // Scan the network
  std::map<uint32_t,std::string> diflist=mpi::MpiMessageHandler::scanNetwork(jGRICV0["network"].asString());
  // Download the configuration
  if (_hca==NULL)
    {
      std::cout<< "Create config acccess"<<std::endl;
      _hca=new lydaq::HR2ConfigAccess();
      _hca->clear();
    }
  std::cout<< " jGRICV0 "<<jGRICV0<<std::endl;
  if (jGRICV0.isMember("json"))
    {
      Json::Value jGRICV0json=jGRICV0["json"];
      if (jGRICV0json.isMember("file"))
	{
	  _hca->parseJsonFile(jGRICV0json["file"].asString());
	}
      else
	if (jGRICV0json.isMember("url"))
	  {
	    _hca->parseJsonUrl(jGRICV0json["url"].asString());
	  }
    }
  if (jGRICV0.isMember("db"))
    {
      Json::Value jGRICV0db=jGRICV0["db"];
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<"Parsing:"<<jGRICV0db["state"].asString()<<jGRICV0db["mode"].asString());

              
      if (jGRICV0db["mode"].asString().compare("mongo")!=0)	
	_hca->parseDb(jGRICV0db["state"].asString(),jGRICV0db["mode"].asString());
      else
	_hca->parseMongoDb(jGRICV0db["state"].asString(),jGRICV0db["version"].asUInt());
      
    }
  if (_hca->asicMap().size()==0)
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No ASIC found in the configuration ");
      return;
    }
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"ASIC found in the configuration "<<_hca->asicMap().size() );
  // Initialise the network
  std::vector<uint32_t> vint;
  vint.clear();
  for (auto x:_hca->asicMap())
    {
      uint32_t eip= ((x.first)>>32)&0XFFFFFFFF;
      std::map<uint32_t,std::string>::iterator idif=diflist.find(eip);
      if (idif==diflist.end()) continue;
      if ( std::find(vint.begin(), vint.end(), eip) != vint.end() ) continue;
      
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" New GRICV0 found in db "<<std::hex<<eip<<std::dec<<" IP address "<<idif->second);
      vint.push_back(eip);
      _mpi->addDevice(idif->second);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Registration done for "<<eip);
    }
  //std::string network=
  // Connect to the event builder
  if (_context==NULL)
    _context= new zmq::context_t(1);

  if (m->content().isMember("publish"))
    {
      this->parameters()["publish"]=m->content()["publish"];
    }
  if (!this->parameters().isMember("publish"))
    {
      
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No publish tag found ");
      return;
    }
  for (auto x:_mpi->boards())
    x.second->data()->autoRegister(_context,this->configuration(),"BUILDER","collectingPort");
  //x->connect(_context,this->parameters()["publish"].asString());

  // Listen All Gricv0 sockets
  _mpi->listen();

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Init done  "); 
}

void lydaq::Gricv0Manager::configureHR2()
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" COnfigure the chips ");

  
  // Now loop on slowcontrol socket
  fprintf(stderr,"Loop on socket for Sending slow control \n");
  for (auto x:_mpi->boards())
    {
      _hca->prepareSlowControl(x.second->ipAddress());
      x.second->reg()->sendSlowControl(_hca->slcBuffer());
    }
  
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Maintenant on charge ");
  for (auto x:_mpi->boards())
    {
      uint32_t status=x.second->reg()->sendCommand(lydaq::gricv0::Message::command::LOADSC);
      x.second->reg()->setSlcStatus(status);
    }

}
void lydaq::Gricv0Manager::configure(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());

  // Now loop on slowcontrol socket


  this->configureHR2();

}

void lydaq::Gricv0Manager::setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif)
{

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changin thresholds: "<<b0<<","<<b1<<","<<b2);
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
void lydaq::Gricv0Manager::setAllMasks(uint64_t mask)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changing Mask: "<<std::hex<<mask<<std::dec);
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
void lydaq::Gricv0Manager::setCTEST(uint64_t mask)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changing CTEST: "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      for (int i=0;i<64;i++)
	it->second.setCTEST(i,(mask>>i)&1);

    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::usleep(1);

}

void lydaq::Gricv0Manager::setGain(uint16_t gain)
{

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changing Gain: "<<gain);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      for (int i=0;i<64;i++)
	it->second.setPAGAIN(i,gain);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::sleep(1);

}

void lydaq::Gricv0Manager::setMask(uint32_t level,uint64_t mask)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changing Mask: "<<level<<" "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASK(level,mask);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::sleep(1);

}
void lydaq::Gricv0Manager::setChannelMask(uint16_t level,uint16_t channel,uint16_t val)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changing Mask: "<<level<<" "<<std::hex<<channel<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASKChannel(level,channel,val==1);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::sleep(1);

}

void lydaq::Gricv0Manager::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  // Create run file
  Json::Value jc=m->content();
  _run=jc["run"].asInt();

  // Clear buffers
  for (auto x:_mpi->boards())
    {
      x.second->data()->clear();
    }

  // Turn run type on
  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , enabled (Bit 0 a 1)
      x.second->reg()->sendCommand(lydaq::gricv0::Message::command::STARTACQ);
    }
   _running=true;

}
void lydaq::Gricv0Manager::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
      x.second->reg()->sendCommand(lydaq::gricv0::Message::command::STOPACQ);
    }
  ::sleep(2);
 _running=false;


}
void lydaq::Gricv0Manager::destroy(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"CLOSE called ");

  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
      x.second->reg()->sendCommand(lydaq::gricv0::Message::command::CLOSE);
    }

  _mpi->close();
  for (auto x:_mpi->boards())
    delete x.second;
  _mpi->boards().clear();
  delete _mpi;
  _mpi=0;

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data sockets deleted");



  // To be done: _gricv0->clear();
}


void lydaq::Gricv0Manager::ScurveStep(fsmwebCaller* mdcc,fsmwebCaller* builder,int thmin,int thmax,int step)
{

  int ncon=2000,ncoff=100,ntrg=20;
  mdcc->sendCommand("PAUSE");
  Json::Value p;
  p.clear();p["nclock"]=ncon;  mdcc->sendCommand("SPILLON",p);
  p.clear();p["nclock"]=ncoff;  mdcc->sendCommand("SPILLOFF",p);
  printf("Clock On %d Off %d \n",ncon, ncoff);
  p.clear();p["value"]=4;  mdcc->sendCommand("SETSPILLREGISTER",p);
  //::sleep(20);
  mdcc->sendCommand("CALIBON");
  p.clear();p["nclock"]=ntrg;  mdcc->sendCommand("SETCALIBCOUNT",p);
  int thrange=(thmax-thmin+1)/step;
  for (int vth=0;vth<=thrange;vth++)
    {
      if (!_running) break;
      mdcc->sendCommand("PAUSE");
      for (auto x:_mpi->boards())
	{
	  // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
	  x.second->reg()->sendCommand(lydaq::gricv0::Message::command::STOPACQ);
	}

      usleep(1000);
      this->setThresholds(thmax-vth*step,512,512);
      p.clear();
      Json::Value h;
      h.append(2);h.append(thmax-vth*step);
      p["header"]=h;
      p["nextevent"]=1;
      builder->sendCommand("SETHEADER",p);
      printf("SETHEADER executed\n");
      int firstEvent=0;
      for (auto x : _mpi->boards())
	if (x.second->data()->event()>firstEvent) firstEvent=x.second->data()->event();
      mdcc->sendCommand("RELOADCALIB");
      for (auto x:_mpi->boards())
	{
	  x.second->data()->clear();
	}

      // Turn run type on
      for (auto x:_mpi->boards())
	{
	  // Automatic FSM (bit 1 a 0) , enabled (Bit 0 a 1)
	  x.second->reg()->sendCommand(lydaq::gricv0::Message::command::STARTACQ);
	}
      
      mdcc->sendCommand("RESUME");
      int nloop=0,lastEvent=0;firstEvent=0;
      while (lastEvent < (firstEvent + ntrg - 1))
	{
	  ::usleep(10000);
	  for (auto x : _mpi->boards())
	    if (x.second->data()->event()>lastEvent) lastEvent=x.second->data()->event();
	  nloop++;if (nloop > 600 || !_running)  break;
	}
      printf("Step %d Th %d First %d Last %d loops %d \n",vth,thmax-vth*step,firstEvent,lastEvent,nloop);
      mdcc->sendCommand("PAUSE");
    }
  mdcc->sendCommand("CALIBOFF");
}


void lydaq::Gricv0Manager::thrd_scurve()
{
  _sc_running=true;
  this->Scurve(_sc_mode,_sc_thmin,_sc_thmax,_sc_step);
  _sc_running=false;
}


void lydaq::Gricv0Manager::Scurve(int mode,int thmin,int thmax,int step)
{
  fsmwebCaller* mdcc=findMDCC("MDCCSERVER");
  fsmwebCaller* builder=findMDCC("BUILDER");
  if (mdcc==NULL) return;
  if (builder==NULL) return;
  uint64_t mask=0;

  // All channel pedestal
  if (mode==255)
    {


      mask=0xFFFFFFFFFFFFFFFF;
      this->setAllMasks(mask);
      this->ScurveStep(mdcc,builder,thmin,thmax,step);
      return;
      
    }

  // Chanel per channel pedestal (CTEST is active)
  if (mode==1023)
    {
      mask=0;
      for (int i=0;i<64;i++)
	{
	  mask=(1ULL<<i);
	  std::cout<<"Step HR2 "<<i<<" channel "<<i<<std::endl;
	  this->setAllMasks(mask);
	  this->setCTEST(mask);
	  this->ScurveStep(mdcc,builder,thmin,thmax,step);
	}
      return;
    }

  // One channel pedestal
  mask=(1ULL<<mode);
  this->setAllMasks(mask);
  this->setCTEST(mask);
  this->ScurveStep(mdcc,builder,thmin,thmax,step);

  
}

fsmwebCaller* lydaq::Gricv0Manager::findMDCC(std::string appname)
{
  Json::Value cjs=this->configuration()["HOSTS"];
  //  std::cout<<cjs<<std::endl;
  std::vector<std::string> lhosts=this->configuration()["HOSTS"].getMemberNames();
  // Loop on hosts
  for (auto host:lhosts)
    {
      //std::cout<<" Host "<<host<<" found"<<std::endl;
      // Loop on processes
      const Json::Value cjsources=this->configuration()["HOSTS"][host];
      //std::cout<<cjsources<<std::endl;
      for (Json::ValueConstIterator it = cjsources.begin(); it != cjsources.end(); ++it)
	{
	  const Json::Value& process = *it;
	  std::string p_name=process["NAME"].asString();
	  Json::Value p_param=Json::Value::null;
	  if (process.isMember("PARAMETER")) p_param=process["PARAMETER"];
	  // Loop on environenemntal variable
	  uint32_t port=0;
	  const Json::Value& cenv=process["ENV"];
	  for (Json::ValueConstIterator iev = cenv.begin(); iev != cenv.end(); ++iev)
	    {
	      std::string envp=(*iev).asString();
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
void lydaq::Gricv0Manager::c_scurve(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t first = atol(request.get("first", "80").c_str());
  uint32_t last = atol(request.get("last", "250").c_str());
  uint32_t step = atol(request.get("step", "1").c_str());
  uint32_t mode = atol(request.get("channel", "255").c_str());
  //  LOG4CXX_INFO(_logFeb, " SetOneVthTime called with vth " << vth << " feb " << feb << " asic " << asic);
  
  //this->Scurve(mode,first,last,step);

  _sc_mode=mode;
  _sc_thmin=first;
  _sc_thmax=last;
  _sc_step=step;
  if (_sc_running)
    {
      response["SCURVE"] ="ALREADY_RUNNING";
      return;
    }
  boost::thread_group g;
  g.create_thread(boost::bind(&lydaq::Gricv0Manager::thrd_scurve, this));
  response["SCURVE"] ="RUNNING";
}

