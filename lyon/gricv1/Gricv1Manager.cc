#include "Gricv1Manager.hh"
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







Gricv1Manager::Gricv1Manager() :_context(NULL),_hca(NULL),_mpi(NULL),_running(false),_run_mode(1),_running_mode(0){;}

void Gricv1Manager::initialise()
{
  // Register state

  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  
  this->addTransition("INITIALISE","CREATED","INITIALISED",std::bind(&Gricv1Manager::fsm_initialise, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","INITIALISED","CONFIGURED",std::bind(&Gricv1Manager::configure, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",std::bind(&Gricv1Manager::configure, this,std::placeholders::_1));
  
  this->addTransition("START","CONFIGURED","RUNNING",std::bind(&Gricv1Manager::start, this,std::placeholders::_1));
  this->addTransition("STOP","RUNNING","CONFIGURED",std::bind(&Gricv1Manager::stop, this,std::placeholders::_1));
  this->addTransition("DESTROY","CONFIGURED","CREATED",std::bind(&Gricv1Manager::destroy, this,std::placeholders::_1));
  this->addTransition("DESTROY","INITIALISED","CREATED",std::bind(&Gricv1Manager::destroy, this,std::placeholders::_1));
  
  
  
  //this->addCommand("JOBLOG",std::bind(&Gricv1Manager::c_joblog,this,std::placeholders::_1));
  this->addCommand("STATUS",std::bind(&Gricv1Manager::c_status,this,std::placeholders::_1));
  this->addCommand("RESET",std::bind(&Gricv1Manager::c_reset,this,std::placeholders::_1));
  
  this->addCommand("SETTHRESHOLDS",std::bind(&Gricv1Manager::c_setthresholds,this,std::placeholders::_1));
  this->addCommand("SCURVE",std::bind(&Gricv1Manager::c_scurve,this,std::placeholders::_1));
  this->addCommand("SETPAGAIN",std::bind(&Gricv1Manager::c_setpagain,this,std::placeholders::_1));
  this->addCommand("SETMASK",std::bind(&Gricv1Manager::c_setmask,this,std::placeholders::_1));
  this->addCommand("SETCHANNELMASK",std::bind(&Gricv1Manager::c_setchannelmask,this,std::placeholders::_1));
  this->addCommand("DOWNLOADDB",std::bind(&Gricv1Manager::c_downloadDB,this,std::placeholders::_1));
  this->addCommand("READREG",std::bind(&Gricv1Manager::c_readreg,this,std::placeholders::_1));
  this->addCommand("WRITEREG",std::bind(&Gricv1Manager::c_writereg,this,std::placeholders::_1));

  this->addCommand("READBME",std::bind(&Gricv1Manager::c_readbme,this,std::placeholders::_1));
  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl
  this->addCommand("GAINCURVE",std::bind(&Gricv1Manager::c_gaincurve,this,std::placeholders::_1));
  this->addCommand("CTEST",std::bind(&Gricv1Manager::c_ctest,this,std::placeholders::_1));
  this->addCommand("SETRUNMODE",std::bind(&Gricv1Manager::c_setrunmode,this,std::placeholders::_1));
 
 
  // Initialise NetLink

  _sc_running=false;
  _running=false;

}

void Gricv1Manager::end()
{
  PMF_DEBUG(_logGricv1," Entering end()"<<std::flush);

  // Stop any running process
  if (_sc_running)
    {
      PMF_DEBUG(_logGricv1,"Stopping scurve"<<std::flush);
      _sc_running=false;
      g_scurve.join();
      _running_mode=0;

    }
  //Stop listening
  if (_mpi!=NULL)
    {
      if (_running)
	{
	  PMF_DEBUG(_logGricv1," CMD: STOPPING"<<std::flush);
	  for (auto x:_mpi->boards())
	    {
	      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
	      x.second->reg()->writeRegister(gricv1::Message::Register::ACQ_CTRL,0);
	    }
	  ::sleep(1);
	  _running=false;
	}
      PMF_INFO(_logGricv1," Terminating MPI"<<std::flush);


    _mpi->terminate();
    //PMF_DEBUG(_logGricv1," Close MPI"<<std::flush);
    //_mpi->close();
    PMF_DEBUG(_logGricv1," Deleting boards "<<std::flush);
    for (auto x:_mpi->boards())
      delete x.second;
    _mpi->boards().clear();
    _mpi=0;
    }
  
}
web::json::value Gricv1Manager::build_status()
{
  web::json::value jl;  uint32_t mb=0;
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
void Gricv1Manager::c_status(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logGricv1,"Status CMD called ");
  par["STATUS"]=web::json::value::string(U("DONE"));
  auto jl=build_status();
 
  par["C3ISTATUS"]=jl;
  mqtt_publish("status",jl);
  Reply(status_codes::OK,par);
}


void Gricv1Manager::c_reset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1,"RESET CMD called ");
  for (auto x:_mpi->boards())
    {
      x.second->reg()->writeRegister(gricv1::Message::Register::ACQ_RST,1);
      ::usleep(1000);
      x.second->reg()->writeRegister(gricv1::Message::Register::ACQ_RST,0);
    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);
}

void Gricv1Manager::c_readbme(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1,"RESET CMD called ");
  web::json::value jl;uint32_t mb=0;
  for (auto x:_mpi->boards())
    {
      web::json::value r;
      r["CAL1"]=web::json::value::number(x.second->reg()->readRegister(gricv1::Message::Register::BME_CAL1));
      r["CAL2"]=web::json::value::number(x.second->reg()->readRegister(gricv1::Message::Register::BME_CAL2));
      r["CAL3"]=web::json::value::number(x.second->reg()->readRegister(gricv1::Message::Register::BME_CAL3));
      r["CAL4"]=web::json::value::number(x.second->reg()->readRegister(gricv1::Message::Register::BME_CAL4));
      r["CAL5"]=web::json::value::number(x.second->reg()->readRegister(gricv1::Message::Register::BME_CAL5));
      r["CAL6"]=web::json::value::number(x.second->reg()->readRegister(gricv1::Message::Register::BME_CAL6));
      r["CAL7"]=web::json::value::number(x.second->reg()->readRegister(gricv1::Message::Register::BME_CAL7));
      r["CAL8"]=web::json::value::number(x.second->reg()->readRegister(gricv1::Message::Register::BME_CAL8));
      r["HUM"]=web::json::value::number(x.second->reg()->readRegister(gricv1::Message::Register::BME_HUM));
      r["PRES"]=web::json::value::number(x.second->reg()->readRegister(gricv1::Message::Register::BME_PRES));
      r["TEMP"]=web::json::value::number(x.second->reg()->readRegister(gricv1::Message::Register::BME_TEMP));
      jl[mb++]=r;
    }
  std::cout<<jl<<std::endl;
  par["STATUS"]=jl;
  Reply(status_codes::OK,par);
}


void Gricv1Manager::c_setthresholds(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1,"Set6bdac called ");
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
void Gricv1Manager::c_readreg(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1,"Pulse called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  uint32_t adr=utils::queryIntValue(m,"adr",0);

  web::json::value r;
  for (auto x:_mpi->boards())
    {    
      uint32_t value=x.second->reg()->readRegister(adr);
      PMF_INFO(_logGricv1,"Read reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<value);
      r[x.second->ipAddress()]=web::json::value::number(value);
    }
  

  par["READREG"]=r;
  Reply(status_codes::OK,par);
}
void Gricv1Manager::c_writereg(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1,"Pulse called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  uint32_t adr=utils::queryIntValue(m,"adr",0);
  uint32_t val=utils::queryIntValue(m,"val",0);

  web::json::value r;
  for (auto x:_mpi->boards())
    {    
      x.second->reg()->writeRegister(adr,val);
      PMF_INFO(_logGricv1,"Write reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<val);
      r[x.second->ipAddress()]=web::json::value::number(val);
    }
  

  par["WRITEREG"]=r;
  Reply(status_codes::OK,par);
}
void Gricv1Manager::c_setpagain(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1,"Set6bdac called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  uint32_t gain=utils::queryIntValue(m,"gain",128);
  this->setGain(gain);
  par["GAIN"]=web::json::value::number(gain);
  Reply(status_codes::OK,par);
}

void Gricv1Manager::c_setmask(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1,"SetMask called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  //uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint64_t mask;
  sscanf(utils::queryStringValue(m,"mask","0XFFFFFFFFFFFFFFFF").c_str(),"%lx",&mask);
  uint32_t level=utils::queryIntValue(m,"level",0);
  PMF_INFO(_logGricv1,"SetMask called "<<std::hex<<mask<<std::dec<<" level "<<level);
  this->setMask(level,mask);
  par["MASK"]=web::json::value::number(mask);
  par["LEVEL"]=web::json::value::number(level);
  Reply(status_codes::OK,par);
}



void Gricv1Manager::c_setchannelmask(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1,"SetMask called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  //uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint32_t level=utils::queryIntValue(m,"level",0);
  uint32_t channel=utils::queryIntValue(m,"channel",0);
  bool on=utils::queryIntValue(m,"value",1)==1;
  PMF_INFO(_logGricv1,"SetMaskChannel called "<<channel<<std::dec<<" level "<<level);
  this->setChannelMask(level,channel,on);
  par["CHANNEL"]=web::json::value::number(channel);
  par["LEVEL"]=web::json::value::number(level);
  par["ON"]=web::json::value::number(on);
  Reply(status_codes::OK,par);
}

void Gricv1Manager::c_downloadDB(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1,"downloadDB called ");
  par["STATUS"]=web::json::value::string(U("DONE"));


  
  std::string dbstate=utils::queryStringValue(m,"state","NONE");
  uint32_t version=utils::queryIntValue(m,"version",0);
  web::json::value jTDC=params()["gricv1"];
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

void Gricv1Manager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1,"****** CMD: INITIALISING");
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 
  // web::json::value jtype=params()["type"];
  //_type=jtype.as_integer();
  //printf ("_type =%d\n",_type); 

  // Need a C4I tag
  if (!utils::isMember(params(),"gricv1"))
    {
      PMF_ERROR(_logGricv1," No gricv1 tag found ");
      par["status"]=web::json::value::string(U("Missing gricv1 tag"));
      Reply(status_codes::OK,par);
      return;
    }
  // Now create the Message handler
  if (_mpi==NULL)
    _mpi= new gricv1::Interface();
  _mpi->initialise();

   
  web::json::value jC4I=params()["gricv1"];
  //_msh =new MpiMessageHandler("/dev/shm");
  if (!utils::isMember(jC4I,"network"))
    {
      PMF_ERROR(_logGricv1," No gricv1:network tag found ");
      par["status"]=web::json::value::string(U("Missing gricv1:network tag"));
      Reply(status_codes::OK,par);

      return;
    }
  
  // Scan the network
    std::vector<string> *accepted=NULL;
  if (utils::isMember(jC4I,"accepted"))
    {
      accepted= new   std::vector<std::string>;
      for (auto it = jC4I["accepted"].as_array().begin(); it != jC4I["accepted"].as_array().end(); it++)
	{

	  PMF_INFO(_logGricv1, " acppeted IP" << (*it).as_string());
	  accepted->push_back((*it).as_string());
	  
	}
    }
  std::vector<string> *rejected=NULL;
  if (utils::isMember(jC4I,"rejected"))
    {
      rejected= new   std::vector<std::string>;
      for (auto it = jC4I["rejected"].as_array().begin(); it != jC4I["rejected"].as_array().end(); it++)
	{

	  PMF_INFO(_logGricv1, " rejected IP" << (*it).as_string());
	  rejected->push_back((*it).as_string());
	  
	}
    }

  std::map<uint32_t,std::string> diflist=utils::scanNetwork(jC4I["network"].as_string(),rejected,accepted);
  // Download the configuration
  if (_hca==NULL)
    {
      PMF_INFO(_logGricv1,"Create config acccess");
      _hca=new HR2ConfigAccess();
      _hca->clear();
    }
  //std::cout<< " jC4I "<<jC4I<<std::endl;
  if (utils::isMember(jC4I,"json"))
    {
      web::json::value jC4Ijson=jC4I["json"];
      if (utils::isMember(jC4Ijson,"file"))
	{
	  _hca->parseJsonFile(jC4Ijson["file"].as_string());
	}
      else
	if (utils::isMember(jC4Ijson,"url"))
	  {
	    _hca->parseJsonUrl(jC4Ijson["url"].as_string());
	  }
    }
  if (utils::isMember(jC4I,"db"))
    {
      web::json::value jC4Idb=jC4I["db"];
      PMF_INFO(_logGricv1,"Parsing:"<<jC4Idb["state"].as_string()<<jC4Idb["mode"].as_string());

              
      if (jC4Idb["mode"].as_string().compare("mongo")==0)	
	_hca->parseMongoDb(jC4Idb["state"].as_string(),jC4Idb["version"].as_integer());
      
    }
  if (_hca->asicMap().size()==0)
    {
      PMF_ERROR(_logGricv1," No ASIC found in the configuration ");
      par["status"]=web::json::value::string(U("No asic in the configuration"));
      Reply(status_codes::OK,par);

      return;
    }
  PMF_INFO(_logGricv1,"ASIC found in the configuration "<<_hca->asicMap().size() );
  // Initialise the network
  std::vector<uint32_t> vint;
  vint.clear();
  for (auto x:_hca->asicMap())
    {
      uint32_t eip= ((x.first)>>32)&0XFFFFFFFF;
      std::map<uint32_t,std::string>::iterator idif=diflist.find(eip);
      if (idif==diflist.end()) continue;
      if ( std::find(vint.begin(), vint.end(), eip) != vint.end() ) continue;
      
      PMF_INFO(_logGricv1," New C4I found in db "<<std::hex<<eip<<std::dec<<" IP address "<<idif->second);
      vint.push_back(eip);
      _mpi->addDevice(idif->second);
      PMF_INFO(_logGricv1," Registration done for "<<eip);
    }
  //std::string network=
  // Connect to the event builder
  if (_context==NULL)
    _context= new zmq::context_t(1);

  for (auto x:_mpi->boards())
    x.second->data()->autoRegister(_context,session(),"evb_builder","collectingPort");
  //x->connect(_context,params()["publish"].as_string());

  // Listen All Gricv1 sockets
  _mpi->listen();

  PMF_INFO(_logGricv1," Init done  ");
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);
}

void Gricv1Manager::configureHR2()
{
  PMF_INFO(_logGricv1," Configure the chips ");

  // Turn Off/ On SLC Mode
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(gricv1::Message::Register::SLC_CTRL,0);
  usleep(100);
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(gricv1::Message::Register::SLC_CTRL,1);

  // Now loop on slowcontrol socket
  PMF_DEBUG(_logGricv1,"Loop on socket for Sending slow control");
  for (auto x:_mpi->boards())
    {
      _hca->prepareSlowControl(x.second->ipAddress(),true);
      x.second->slc()->sendSlowControl(_hca->slcBuffer());
    }
  
  PMF_DEBUG(_logGricv1," Maintenant on charge ");
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(gricv1::Message::Register::SLC_SIZE,109);
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(gricv1::Message::Register::SLC_CTRL,2);

  usleep(1000);
  // Turn off SLC
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(gricv1::Message::Register::SLC_CTRL,0);

  // Read SLC status
  for (auto x:_mpi->boards())
    {
      uint32_t status=0,cnt=0;
      while(!(status&1))
	{status=x.second->reg()->readRegister(gricv1::Message::Register::SLC_STATUS);
	  fprintf(stderr,"::::::::::::::: %x \n",status);
	  usleep(1000);
	  cnt++;
	  if (cnt>1000)
	    {
	      PMF_ERROR(_logGricv1," DIFSTATUS NULL after 1 s ");
	    break;
	    }

	}
      x.second->reg()->setSlcStatus(status);
    }

}

void Gricv1Manager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1," CMD: CONFIGURING");

  // Now loop on slowcontrol socket


  this->configureHR2();
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);

}

void Gricv1Manager::setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif)
{

  PMF_INFO(_logGricv1," Changin thresholds: "<<b0<<","<<b1<<","<<b2);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      if (idif!=0)
	{
	  uint32_t ip=(((it->first)>>32&0XFFFFFFFF)>>16)&0xFFFF;
	  //printf("%lx %x %x \n",(it->first>>32),ip,idif);
	  if (idif!=ip) continue;
	}
      it->second.setB0(b0);
      it->second.setB1(b1);
      it->second.setB2(b2);
      //it->second.setHEADER(0x56);
      it->second.dumpBinary();
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::usleep(1);

}
void Gricv1Manager::setGain(uint16_t gain)
{

  PMF_INFO(_logGricv1," Changing Gain: "<<gain);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      for (int i=0;i<64;i++)
	it->second.setPAGAIN(i,gain);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::usleep(1);

}

void Gricv1Manager::setMask(uint32_t level,uint64_t mask)
{
  PMF_INFO(_logGricv1," Changing Mask: "<<level<<" "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASK(level,mask);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::usleep(1);

}
void Gricv1Manager::setAllMasks(uint64_t mask)
{
  PMF_INFO(_logGricv1," Changing Mask: "<<std::hex<<mask<<std::dec);
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
void Gricv1Manager::setCTEST(uint64_t mask)
{
  PMF_INFO(_logGricv1," Changing CTEST: "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      for (int i=0;i<64;i++)
	{
	  bool on=((mask>>i)&1)==1;
	it->second.setCTEST(i,on);
	PMF_INFO(_logGricv1,"CTEST: "<<std::hex<<mask<<std::dec<<" channel "<<i<<" "<<on);
	}

    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::usleep(1);

}

void Gricv1Manager::setChannelMask(uint16_t level,uint16_t channel,uint16_t val)
{
  PMF_INFO(_logGricv1," Changing Mask: "<<level<<" "<<std::hex<<channel<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASKChannel(level,channel,val==1);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::usleep(1);

}

void Gricv1Manager::start(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1," CMD: STARTING");

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
      // clear FIFO
      x.second->reg()->writeRegister(gricv1::Message::Register::ACQ_RST,2);
      ::usleep(1000);
      x.second->reg()->writeRegister(gricv1::Message::Register::ACQ_RST,0);
    }

  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , enabled (Bit 0 a 1)
      x.second->reg()->writeRegister(gricv1::Message::Register::ACQ_CTRL,_run_mode);
      //x.second->reg()->writeRegister(gricv1::Message::Register::ACQ_CTRL,5);
    }
  _running=true;
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);

}
void Gricv1Manager::stop(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1," CMD: STOPPING");
  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
      x.second->reg()->writeRegister(gricv1::Message::Register::ACQ_CTRL,0);
    }
  ::sleep(1);
  _running=false;

   if (_sc_running)
    {
      g_scurve.join();
      _sc_running=false;
    }
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);

}
void Gricv1Manager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logGricv1," CMD: Destroying");
  PMF_INFO(_logGricv1,"CLOSE called ");
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

  PMF_INFO(_logGricv1," Data sockets deleted");

  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);


  // To be done: _gricv1->clear();
}



void Gricv1Manager::ScurveStep(std::string mdccUrl,std::string builderUrl,int thmin,int thmax,int step)
{

  int ncon=_sc_win,ncoff=10000,ntrg=_sc_ntrg;
  utils::sendCommand(mdccUrl,"PAUSE",json::value::null());
  web::json::value p;
  p["nclock"]=web::json::value::number(ncon);  utils::sendCommand(mdccUrl,"SPILLON",p);
  p["nclock"]=web::json::value::number(ncoff);  utils::sendCommand(mdccUrl,"SPILLOFF",p);
  printf("Clock On %d Off %d \n",ncon, ncoff);
  p["value"]=web::json::value::number(4);  utils::sendCommand(mdccUrl,"SETSPILLREGISTER",p);
  utils::sendCommand(mdccUrl,"CALIBON",json::value::null());
  p["nclock"]=web::json::value::number(ntrg);  utils::sendCommand(mdccUrl,"SETCALIBCOUNT",p);
  int thrange=(thmax-thmin+1)/step;
  for (int vth=0;vth<=thrange;vth++)
    {
      if (!_running) break;
      utils::sendCommand(mdccUrl,"PAUSE",json::value::null());
      usleep(1000);

      uint32_t threshold=thmax-vth*step;
      //this->setThresholds(thmax-vth*step,512,512);
      if (_sc_level==0)
	this->setThresholds(threshold,512,512);
      if (_sc_level==1)
	this->setThresholds(512,threshold,512);
      if (_sc_level==2)
	this->setThresholds(512,512,threshold);
      
      _running_mode=2+((_sc_channel&0xFF)<<4)+((threshold&0XFFF)<<12);
      mqtt_publish("status",build_status());
      web::json::value h;
      h[0]=2;h[1]=web::json::value::number(thmax-vth*step);

      int firstEvent=0,firstInBoard=0;
   for (auto x : _mpi->boards())
	if (x.second->data()->event()>firstInBoard) firstInBoard=x.second->data()->event();

    auto frep = utils::sendCommand(builderUrl, "STATUS", json::value::null());
    auto jfrep = frep.extract_json();
    auto jfanswer = jfrep.get().as_object()["answer"];
    firstEvent = jfanswer["event"].as_integer();  

      web::json::value ph;
      ph["header"]=h;
      ph["nextevent"]=web::json::value::number(firstEvent+1);
      utils::sendCommand(builderUrl,"SETHEADER",ph);
      utils::sendCommand(mdccUrl,"RELOADCALIB",json::value::null());
      utils::sendCommand(mdccUrl,"RESUME",json::value::null());
      int nloop=0,lastEvent=firstEvent,lastInBoard=firstInBoard;
      while (lastInBoard < (firstInBoard + ntrg - 2))
	{
	  ::usleep(10000);
	  for (auto x : _mpi->boards())
	    if (x.second->data()->event()>lastInBoard) lastInBoard=x.second->data()->event();
	  nloop++;if (nloop > 600 || !_running)  break;
	}

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

  PMF_INFO(_logGricv1,"Step:"<<vth<<" Threshold:"<<thmax-vth*step<<" First:"<<firstEvent<<" Last:"<<lastEvent);
      //printf("Step %d Th %d First %d Last %d \n",vth,thmax-vth*step,firstEvent,lastEvent);
      utils::sendCommand(mdccUrl,"PAUSE",json::value::null());
    }
  utils::sendCommand(mdccUrl,"CALIBOFF",json::value::null());
}


void Gricv1Manager::thrd_scurve()
{
  _sc_running=true;
  this->Scurve(_sc_mode,_sc_thmin,_sc_thmax,_sc_step);
  // _sc_running=false;
}


void Gricv1Manager::Scurve(int mode,int thmin,int thmax,int step)
{
  std::string mdcc=utils::findUrl(session(),"lyon_mdcc",0);
  std::string builder=utils::findUrl(session(),"evb_builder",0);
  if (mdcc.compare("")==0)
    {
      mdcc=utils::findUrl(session(),"lyon_mbmdcc",0);
      if (mdcc.compare("")==0)
	return;
    }
  if (builder.compare("")==0) return;
  uint64_t mask=0;
  _sc_channel=mode;

  // All channel pedestal
  if (mode==255)
    {
      _sc_channel=0xFE;

      //for (int i=0;i<64;i++) mask|=(1<<i);
      mask=0xFFFFFFFFFFFFFFFF;
      this->setAllMasks(mask);
      this->setCTEST(mask);
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
	  _sc_channel=i;

	  mask=(1ULL<<i);
    PMF_INFO(_logGricv1,"Step HR2 "<<i<<"  channel"<<i<<std::dec);

	  //std::cout<<"Step HR2 "<<i<<" channel "<<i<<std::endl;
	  this->setAllMasks(mask);
	  this->setCTEST(mask);
	  this->ScurveStep(mdcc,builder,thmin,thmax,step);
	}
      _running_mode=0;
      mqtt_publish("status",build_status());

      return;
    }

  if (mode==1022)
    {
      mask=0;
      for (int i=0;i<64;i++)
	{
	  _sc_channel=i;

	  mask=(1ULL<<i);
	  PMF_INFO(_logGricv1,"Step HR2 "<<i<<"  channel"<<i<<std::dec);

	  //std::cout<<"Step HR2 "<<i<<" channel "<<i<<std::endl;
	  this->setCTEST(mask);
	  this->setAllMasks(0xFFFFFFFFFFFFFFFF);

	  this->ScurveStep(mdcc,builder,thmin,thmax,step);
	}
      _running_mode=0;
      mqtt_publish("status",build_status());

      return;
    }

  // One channel pedestal

  mask=(1ULL<<mode);
  PMF_INFO(_logGricv1,"CTEST One "<<mode<<" "<<std::hex<<mask<<std::dec);
  this->setAllMasks(mask);
  this->setCTEST(mask);
  this->ScurveStep(mdcc,builder,thmin,thmax,step);
  _running_mode=0;
  mqtt_publish("status",build_status());

  
}

void Gricv1Manager::c_scurve(http_request m)
{
  auto par = json::value::object();
  par["STATUS"]=web::json::value::string(U("DONE"));

  uint32_t first = utils::queryIntValue(m,"first",80);
  uint32_t last = utils::queryIntValue(m,"last",250);
  uint32_t step = utils::queryIntValue(m,"step",1);
  uint32_t mode = utils::queryIntValue(m,"channel",255);
  uint32_t win = utils::queryIntValue(m,"window",50000);
  uint32_t ntrg = utils::queryIntValue(m,"ntrg",50);
  _sc_level=utils::queryIntValue(m,"level",0);
  PMF_INFO(_logGricv1, " SCURVE/CTEST "<<mode<<" "<<step<<" "<<first<<" "<<last);
  
  //this->Scurve(mode,first,last,step);

  _sc_mode=mode;
  _sc_thmin=first;
  _sc_thmax=last;
  _sc_step=step;
  _sc_win=win;
  _sc_ntrg=ntrg;
    
  if (_sc_running)
    {
      par["SCURVE"]=web::json::value::string(U("ALREADY_RUNNING"));
      Reply(status_codes::OK,par);
      return;
    }

  g_scurve=std::thread(std::bind(&Gricv1Manager::thrd_scurve, this));
  par["SCURVE"]=web::json::value::string(U("RUNNING"));

  Reply(status_codes::OK,par);

}




void Gricv1Manager::GainCurveStep(std::string mdcc,std::string builder,int gmin,int gmax,int step,int threshold)
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
    
      usleep(1000);
      this->setGain(gmin+g*step);
      _running_mode=4+((_sc_channel&0xFF)<<4)+((threshold&0XFFF)<<12)+((g&0XFFF)<<24);
      mqtt_publish("status",build_status());
      int firstEvent=0,firstInBoard=0;
      for (auto x : _mpi->boards())
	if (x.second->data()->event()>firstInBoard) firstInBoard=x.second->data()->event();
      
      auto frep = utils::sendCommand(builder, "STATUS", json::value::null());
      auto jfrep = frep.extract_json();
      auto jfanswer = jfrep.get().as_object()["answer"];
      firstEvent = jfanswer["event"].as_integer();  



      
      web::json::value h;
      h[0]=3;h[1]=json::value::number(gmin+g*step);
      web::json::value ph;
      ph["header"]=h;
      ph["nextevent"]=web::json::value::number(firstEvent+1);
      utils::sendCommand(builder,"SETHEADER",ph);
      printf("SETHEADER executed\n");
      utils::sendCommand(mdcc,"RELOADCALIB",json::value::null());
   

      // Turn run type on

      utils::sendCommand(mdcc,"RESUME",json::value::null());
      int nloop=0,lastEvent=firstEvent,lastInBoard=firstInBoard;
      while (lastInBoard < (firstInBoard + ntrg - 2))
	{
	  ::usleep(10000);
	  for (auto x : _mpi->boards())
	    if (x.second->data()->event()>lastInBoard) lastInBoard=x.second->data()->event();
	  nloop++;if (nloop > 600 || !_running)  break;
	}

      while (lastEvent < (firstEvent + ntrg - 2))
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

      PMF_INFO(_logGricv1,"Step:"<<g<<" Gain:"<<gmin+g*step<<" First:"<<firstEvent<<" Last:"<<lastEvent);

      printf("Step %d Gain %d First %d Last %d loops %d \n",g,gmin+g*step,firstEvent,lastEvent,nloop);
      utils::sendCommand(mdcc,"PAUSE",json::value::null());
    }
  utils::sendCommand(mdcc,"CALIBOFF",json::value::null());
}


void Gricv1Manager::thrd_gaincurve()
{
  _sc_running=true;
  this->GainCurve(_sc_mode,_sc_gmin,_sc_gmax,_sc_step,_sc_threshold);
  _sc_running=false;
}


void Gricv1Manager::GainCurve(int mode,int gmin,int gmax,int step,int threshold)
{
  std::string mdcc=utils::findUrl(session(),"lyon_mdcc",0);
  //std::string builder=utils::findUrl(session(),"lyon_evb",0);
  std::string builder=utils::findUrl(session(),"evb_builder",0);


  if (mdcc.compare("")==0)
    {
      mdcc=utils::findUrl(session(),"lyon_mbmdcc",0);
      if (mdcc.compare("")==0)
	return;
    }

  if (builder.compare("")==0) return;

  uint64_t mask=0;
  _sc_channel=mode;

  // All channel pedestal
  if (mode==255)
    {

      _sc_channel=0XFE;

      mask=0xFFFFFFFFFFFFFFFF;
      this->setAllMasks(mask);
      this->setCTEST(mask);
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
	  _sc_channel=i;

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


void Gricv1Manager::c_gaincurve(http_request m)
{
  auto par = json::value::object();

  par["STATUS"]=web::json::value::string(U("DONE"));

  uint32_t first = utils::queryIntValue(m,"first",80);
  uint32_t last = utils::queryIntValue(m,"last",250);
  uint32_t step = utils::queryIntValue(m,"step",1);
  uint32_t mode = utils::queryIntValue(m,"channel",255);
  uint32_t thr = utils::queryIntValue(m,"threshold",255);
  PMF_INFO(_logGricv1, " GainCurve called " << mode << " first " << first << " last " << last <<" step "<<step<< " Threshold "<<thr);
  
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
 
  g_scurve=std::thread(std::bind(&Gricv1Manager::thrd_gaincurve, this));
  par["GAINCURVE"]=web::json::value::string(U("RUNNING"));
  Reply(status_codes::OK,par);


}
void Gricv1Manager::c_ctest(http_request m)
{
  auto par = json::value::object();

  par["STATUS"]=web::json::value::string(U("DONE"));
  uint32_t on = utils::queryIntValue(m,"on",1);
  uint32_t delay = utils::queryIntValue(m,"delay",80);
  uint32_t length = utils::queryIntValue(m,"length",100000);
  uint32_t number = utils::queryIntValue(m,"number",1);
  PMF_INFO(_logGricv1, " CTEST called " << on << " delay " <<delay << " length " << length <<" number "<<number);
  
  //this->Scurve(mode,first,last,step);
  for (auto x:_mpi->boards())
    {

      x.second->reg()->writeRegister(gricv1::Message::Register::CTEST_DELAY,delay);
      x.second->reg()->writeRegister(gricv1::Message::Register::CTEST_LENGTH,length);
      x.second->reg()->writeRegister(gricv1::Message::Register::CTEST_NUMBER,number);
      x.second->reg()->writeRegister(gricv1::Message::Register::CTEST_CTRL,on);
    }
  par["CTEST"]=web::json::value::string(U("DONE"));
  Reply(status_codes::OK,par);


}
void Gricv1Manager::c_setrunmode(http_request m)
{
  auto par = json::value::object();

  par["STATUS"]=web::json::value::string(U("DONE"));
  _run_mode = utils::queryIntValue(m,"mode",1);
  par["MODE"]=_run_mode;
  Reply(status_codes::OK,par);


}

extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new Gricv1Manager);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
