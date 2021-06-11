
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
  for ( std::map<uint32_t,liboard::FtdiDeviceInfo*>::iterator it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    if (it->second!=NULL) delete it->second;
  theFtdiDeviceInfoMap_.clear();
  for ( std::map<uint32_t,LiboardInterface*>::iterator it=_LiboardInterfaceMap.begin();it!=_LiboardInterfaceMap.end();it++)
    if (it->second!=NULL) delete it->second;
  _LiboardInterfaceMap.clear();
  int ier=system("/bin/rm /var/log/pi/ftdi_devices");
  ier=system("/opt/dhcal/bin/ListDevices.py");
  std::string line;
  std::ifstream myfile ("/var/log/pi/ftdi_devices");
  std::stringstream diflist;



  if (myfile.is_open())
    {
      while ( myfile.good() )
	{
	  getline (myfile,line);
	  liboard::FtdiDeviceInfo* difi=new liboard::FtdiDeviceInfo();
	  memset(difi,0,sizeof(liboard::FtdiDeviceInfo));
	  sscanf(line.c_str(),"%x %x %s",&difi->vendorid,&difi->productid,difi->name);
	  if (strncmp(difi->name,"FT101",5)==0)
	    {
	      sscanf(difi->name,"FT101%d",&difi->id); 
	      difi->type=0;
	      std::pair<uint32_t,liboard::FtdiDeviceInfo*> p(difi->id,difi);
	      theFtdiDeviceInfoMap_.insert(p);
	    }
	  if (strncmp(difi->name,"DCCCCC",6)==0)
	    {sscanf(difi->name,"DCCCCC%d",&difi->id);difi->type=0x10;}


	}
      myfile.close();
    }
  else 
    {
      //std::cout << "Unable to open file"<<std::endl; 
      PMF_FATAL(_logLiboard," Unable to open /var/log/pi/ftdi_devices");
    }

  for (auto it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    PMF_INFO(_logLiboard,"Device found and register "<<it->first<<" with info "<<it->second->vendorid<<" "<<it->second->productid<<" "<<it->second->name<<" "<<it->second->type);
}


void LiboardManager::scan(http_request m) 
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard," CMD: SCanning");
  // Fill Ftdi Map
  this->prepareDevices();
  std::map<uint32_t,liboard::FtdiDeviceInfo*>& fm=this->getFtdiMap();
  std::map<uint32_t,LiboardInterface*> dm=this->getLiboardMap();
  PMF_INFO(_logLiboard," CMD: SCANDEVICE clear Maps");
  for (  auto it=dm.begin();it!=dm.end();it++)
    { if (it->second!=NULL) delete it->second;}
  dm.clear();
  // _ndif=0;
  std::vector<uint32_t> vids;
  web::json::value array;uint32_t ndev=0;
  for ( auto it=fm.begin();it!=fm.end();it++)
    {
      PMF_INFO(_logLiboard,"Creating "<<it->second->name);
      LiboardInterface* d= new LiboardInterface(it->second);
      PMF_INFO(_logLiboard,"After Creating "<<it->second->name);
      this->getLiboardMap().insert(std::make_pair(it->first,d));
      PMF_INFO(_logLiboard," CMD: SCANDEVICE created LiboardInterface @ "<<std::hex<<d<<std::dec);
      web::json::value jd;
      jd["detid"]=json::value::number(d->detectorId());
      jd["sourceid"]=json::value::number(it->first);
      vids.push_back( (d->detectorId()<<16|it->first));
      array[ndev++]=jd;
    }

    par["status"]=json::value::string(U("done"));
    par["devices"]=array;
    Reply(status_codes::OK,par);  
 

}


void LiboardManager::fsm_initialise(http_request m)
{
  
  auto par = json::value::object();
  PMF_INFO(_logLiboard," CMD: INITIALISING");
  _vDif.clear();

  if (!utils::isMember(params(),"dif"))
    {
      PMF_ERROR(_logLiboard," No dif tag in params()");
      par["status"]=json::value::string(U("Missing dif tag"));
      Reply(status_codes::OK,par);  
      return;
    }
   auto jLiboard=params()["dif"];
  
  // Download the configuration
  
  if (_hca==NULL)
    {
      std::cout<< "Create config acccess"<<std::endl;
      _hca=new LIROCConfigAccess();
      _hca->clear();
    }
  std::cout<< " jLiboard "<<jLiboard<<std::endl;
  if (utils::isMember(jLiboard,"json"))
    {
      web::json::value jLiboardjson=jLiboard["json"];
      if (utils::isMember(jLiboardjson,"file"))
	{
	  _hca->parseJsonFile(jLiboardjson["file"].as_string());
	}
      else
	if (utils::isMember(jLiboardjson,"url"))
	  {
	    _hca->parseJsonUrl(jLiboardjson["url"].as_string());
	  }
    }
  if (utils::isMember(jLiboard,"db"))
    {
      web::json::value jLiboarddb=jLiboard["db"];
      PMF_ERROR(_logLiboard,"Parsing:"<<jLiboarddb["state"].as_string()<<jLiboarddb["mode"].as_string());

              
      if (jLiboarddb["mode"].as_string().compare("mongo")==0)	
	_hca->parseMongoDb(jLiboarddb["state"].as_string(),jLiboarddb["version"].as_integer());

      PMF_ERROR(_logLiboard,"End of parseDB "<<_hca->asicMap().size());
    }
  if (_hca->asicMap().size()==0)
    {
      PMF_ERROR(_logLiboard," No ASIC found in the configuration ");
      par["status"]=json::value::string(U("No ASIC found in the configuration"));
      Reply(status_codes::OK,par);  
      return;
    }
  PMF_INFO(_logLiboard,"ASIC found in the configuration "<<_hca->asicMap().size() );
  // Initialise the network
  std::map<uint32_t,LiboardInterface*> dm=this->getLiboardMap();
  std::vector<uint32_t> vint;
   
  vint.clear();
  for (auto x:_hca->asicMap())
    {
      // only MSB is used
      uint32_t eip= ((x.first)>>56)&0XFF;
      std::map<uint32_t,LiboardInterface*>::iterator idif=dm.find(eip);
      if (idif==dm.end()) continue;
      if ( std::find(vint.begin(), vint.end(), eip) != vint.end() ) continue;

      PMF_INFO(_logLiboard," New Liboard found in db "<<std::hex<<eip<<std::dec);
      vint.push_back(eip);
      
      _vDif.push_back(idif->second);
      PMF_INFO(_logLiboard," Registration done for "<<eip);
    }
  //std::string network=
  // Connect to the event builder
  if (_context==NULL)
    _context= new zmq::context_t(1);

  for (auto x:_vDif)
    {
      PMF_INFO(_logLiboard," Creating pusher to "<<params()["publish"].as_string());
      /** Old single method
	  zmPusher* push=new zmPusher(_context,x->detectorId(),x->status()->id);
	  push->connect(params()["publish"].as_string());
      */

      pm::pmSender* push= new pm::pmSender(_context,x->detectorId(),x->status()->id);
      //ds->connect(params()["pushdata"].as_string());
      push->autoDiscover(session(),"evb_builder","collectingPort");
      //for (uint32_t i=0;i<_mStream.size();i++)
      //	ds->connect(_mStream[i]);
      push->collectorRegister();

      x->initialise(push);

    }

    par["status"]=json::value::string(U("Configured"));
    Reply(status_codes::OK,par);  







  
}







void LiboardManager::setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif)
{

  PMF_INFO(_logLiboard," Changin thresholds: "<<b0<<","<<b1<<","<<b2);
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
  this->configureLR();
  ::usleep(10);

}
void LiboardManager::setGain(uint16_t gain)
{

  PMF_INFO(_logLiboard," Changing Gain: "<<gain);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      for (int i=0;i<64;i++)
	it->second.setPAGAIN(i,gain);
    }
  // Now loop on slowcontrol socket
  this->configureLR();
  ::usleep(10);

}

void LiboardManager::setMask(uint32_t level,uint64_t mask)
{
  PMF_INFO(_logLiboard," Changing Mask: "<<level<<" "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASK(level,mask);
    }
  // Now loop on slowcontrol socket
  this->configureLR();


  ::usleep(10);

}
void LiboardManager::setChannelMask(uint16_t level,uint16_t channel,uint16_t val)
{
  PMF_INFO(_logLiboard," Changing Mask: "<<level<<" "<<std::hex<<channel<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASKChannel(level,channel,val==1);
    }
  // Now loop on slowcontrol socket
  this->configureLR();


  ::usleep(10);

}

void LiboardManager::setAllMasks(uint64_t mask)
{
  PMF_INFO(_logLiboard," Changing Mask: "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      it->second.dumpBinary();
      it->second.setMASK(0,mask);
      it->second.setMASK(1,mask);
      it->second.setMASK(2,mask);
      it->second.dumpBinary();
	    
    }
  // Now loop on slowcontrol socket
  this->configureLR();


  ::usleep(10);

}
void LiboardManager::setCTEST(uint64_t mask)
{
  PMF_INFO(_logLiboard," Changing CTEST: "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      for (int i=0;i<64;i++)
	{
	  bool on=((mask>>i)&1)==1;
	  it->second.setCTEST(i,on);
	  PMF_INFO(_logLiboard,"CTEST: "<<std::hex<<mask<<std::dec<<" channel "<<i<<" "<<on);
	}

    }
  // Now loop on slowcontrol socket
  this->configureLR();


  ::usleep(10);

}

void LiboardManager::c_setthresholds(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard,"Set6bdac called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  uint32_t b0=utils::queryIntValue(m,"B0",250);
  uint32_t b1=utils::queryIntValue(m,"B1",250);
  uint32_t b2=utils::queryIntValue(m,"B2",250);
  uint32_t idif=utils::queryIntValue(m,"LIBOARD",0);
  
  this->setThresholds(b0,b1,b2,idif);
  par["THRESHOLD0"]=web::json::value::number(b0);
  par["THRESHOLD1"]=web::json::value::number(b1);
  par["THRESHOLD2"]=web::json::value::number(b2);
  par["DIF"]=web::json::value::number(idif);
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);  

}
void LiboardManager::c_setpagain(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard,"Set6bdac called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  uint32_t gain=utils::queryIntValue(m,"gain",128);
  this->setGain(gain);
  par["GAIN"]=web::json::value::number(gain);
  
  Reply(status_codes::OK,par);  

}

void LiboardManager::c_setmask(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard,"SetMask called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  //uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint64_t mask;
  sscanf(utils::queryStringValue(m,"mask","0XFFFFFFFFFFFFFFFF").c_str(),"%lx",&mask);
  uint32_t level=utils::queryIntValue(m,"level",0);
  PMF_INFO(_logLiboard,"SetMask called "<<std::hex<<mask<<std::dec<<" level "<<level);
  this->setMask(level,mask);
  par["MASK"]=web::json::value::number(mask);
  par["LEVEL"]=web::json::value::number(level);

  Reply(status_codes::OK,par);  

}



void LiboardManager::c_setchannelmask(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard,"SetMask called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  //uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint32_t level=utils::queryIntValue(m,"level",0);
  uint32_t channel=utils::queryIntValue(m,"channel",0);
  bool on=utils::queryIntValue(m,"value",1)==1;
  PMF_INFO(_logLiboard,"SetMaskChannel called "<<channel<<std::dec<<" level "<<level);
  this->setChannelMask(level,channel,on);
  par["CHANNEL"]=web::json::value::number(channel);
  par["LEVEL"]=web::json::value::number(level);
  par["ON"]=web::json::value::number(on);

  Reply(status_codes::OK,par);  

}
void LiboardManager::c_external(http_request m)
{
  auto par = json::value::object();
  uint32_t external=utils::queryIntValue(m,"value",0);
  PMF_INFO(_logLiboard,"EXTERNAL called "<<external);


  if (external!=0)
    params()["external"]=external;
  std::map<uint32_t,LiboardInterface*> dm=this->getLiboardMap();
  for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      it->second->setExternalTrigger((external==1));
      
    }
  
  //PMF_INFO(_logLiboard,"CTRLREG called "<<std::hex<<ctrlreg<<std::dec);
  par["STATUS"]=web::json::value::string(U("DONE"));
  par["TRIGGEREXT"]=web::json::value::number( external);
  Reply(status_codes::OK,par);  

}
void LiboardManager::c_downloadDB(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard,"downloadDB called ");
  par["STATUS"]=web::json::value::string(U("DONE"));


  
  std::string dbstate=utils::queryStringValue(m,"state","NONE");
  uint32_t version=utils::queryIntValue(m,"version",0);
  web::json::value jTDC=params()["dif"];
  if (utils::isMember(jTDC,"db"))
    {
      web::json::value jTDCdb=jTDC["db"];
      _hca->clear();

      if (jTDCdb["mode"].as_string().compare("mongo")==0)
	_hca->parseMongoDb(dbstate,version);

	 
    }
  par["DBSTATE"]=web::json::value::string(dbstate);
  Reply(status_codes::OK,par);  

}


void LiboardManager::c_status(http_request m)
{
  auto par = json::value::object();
  int32_t rc=1;
  std::map<uint32_t,LiboardInterface*> dm=this->getLiboardMap();
  web::json::value array_slc;uint32_t nd=0;
 
  for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      
      web::json::value ds;
      ds["detid"]=json::value::number(it->second->detectorId());
      ds["state"]=json::value::string(U(it->second->state()));
      ds["id"]=json::value::number(it->second->status()->id);
      ds["status"]=json::value::number(it->second->status()->status);
      ds["slc"]=json::value::number(it->second->status()->slc);
      ds["gtc"]=json::value::number(it->second->status()->gtc);
      ds["bcid"]=json::value::number(it->second->status()->bcid);
      ds["bytes"]=json::value::number(it->second->status()->bytes);
      ds["host"]=json::value::string(U(std::string((it->second->status()->host))));
      array_slc[nd++]=ds;



    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  par["DIFLIST"]=array_slc;
  Reply(status_codes::OK,par);  


  return;
  
}

web::json::value LiboardManager::configureLR()
{
  /// A reecrire
  uint32_t external=params()["external"].as_integer();
  printf("TRigger EXT %x \n",external);
  int32_t rc=1;
  std::map<uint32_t,LiboardInterface*> dm=this->getLiboardMap();
  web::json::value array_slc;uint32_t nd=0;

  for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      std::stringstream ips;
      // Dummy IP address for Liboards
      ips<<"0.0.0."<<it->first;
      _hca->prepareSlowControl(ips.str(),true);
     
      it->second->configure(_hca->slcBuffer(),_hca->slcBytes());
      web::json::value ds;
      ds["id"]=json::value::number(it->first);
      ds["slc"]=json::value::number(it->second->status()->slc);
      array_slc[nd++]=ds;
      

    }
  return array_slc;
}

void LiboardManager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_DEBUG(_logLiboard," CMD: Configuring");
 
  int32_t rc=1;

  web::json::value array_slc=this->configureLR();

  par["status"]=json::value::string(U("done"));
  par["devices"]=array_slc;
  Reply(status_codes::OK,par);  

  return;
    
}

void LiboardManager::start(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard," Starting ");
 
  int32_t rc=1;
  std::map<uint32_t,LiboardInterface*> dm=this->getLiboardMap();
  for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      this->startReadoutThread(it->second);
      it->second->start();
    }
  _running=true;
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);  

  return;
  
}
void LiboardManager::stop(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard," Stopping ");
 
  int32_t rc=1;
  std::map<uint32_t,LiboardInterface*> dm=this->getLiboardMap();
 
  for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      PMF_INFO(_logLiboard," Stopping thread of Liboard"<<it->first);
      it->second->stop();
    }
  _running=false;
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);  

  return;
  
}
void LiboardManager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logLiboard," Destroying ");
  
  int32_t rc=1;
  std::map<uint32_t,LiboardInterface*> dm=this->getLiboardMap();

  bool running=false;
  for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      running=running ||it->second->readoutStarted();
    }
  if (running)
    {
      for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->setReadoutStarted(false);
	}
      
      this->joinThreads();
      // Clear thread vector
      g_d.clear();
    }
  for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      it->second->destroy();
    }

  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);  

  return;
  
}






LiboardManager::LiboardManager()  : _running(false),_sc_running(false) {;}

void LiboardManager::initialise()
{


  // Zmq transport
  _context = new zmq::context_t (1);
  // Register state
  this->addState("SCANNED");
  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  this->addState("STOPPED");
  this->addTransition("SCAN","CREATED","SCANNED",std::bind(&LiboardManager::scan, this,std::placeholders::_1));
  this->addTransition("INITIALISE","SCANNED","INITIALISED",std::bind(&LiboardManager::fsm_initialise, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","INITIALISED","CONFIGURED",std::bind(&LiboardManager::configure, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",std::bind(&LiboardManager::configure, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","STOPPED","CONFIGURED",std::bind(&LiboardManager::configure, this,std::placeholders::_1));
  this->addTransition("START","CONFIGURED","RUNNING",std::bind(&LiboardManager::start, this,std::placeholders::_1));
  this->addTransition("START","STOPPED","RUNNING",std::bind(&LiboardManager::start, this,std::placeholders::_1));
  this->addTransition("STOP","RUNNING","STOPPED",std::bind(&LiboardManager::stop, this,std::placeholders::_1));
  this->addTransition("DESTROY","STOPPED","CREATED",std::bind(&LiboardManager::destroy, this,std::placeholders::_1));
  this->addTransition("DESTROY","CONFIGURED","CREATED",std::bind(&LiboardManager::destroy, this,std::placeholders::_1));


  this->addCommand("STATUS",std::bind(&LiboardManager::c_status,this,std::placeholders::_1));
  this->addCommand("SETTHRESHOLDS",std::bind(&LiboardManager::c_setthresholds,this,std::placeholders::_1));
  this->addCommand("SETPAGAIN",std::bind(&LiboardManager::c_setpagain,this,std::placeholders::_1));
  this->addCommand("SETMASK",std::bind(&LiboardManager::c_setmask,this,std::placeholders::_1));
  this->addCommand("SETCHANNELMASK",std::bind(&LiboardManager::c_setchannelmask,this,std::placeholders::_1));
  this->addCommand("DOWNLOADDB",std::bind(&LiboardManager::c_downloadDB,this,std::placeholders::_1));
  this->addCommand("TRIGEXT",std::bind(&LiboardManager::c_external,this,std::placeholders::_1));
  this->addCommand("SCURVE",std::bind(&LiboardManager::c_scurve,this,std::placeholders::_1));

  _hca=NULL;
  // Initialise delays for 
}


void LiboardManager::end()
{
  // Stop any running process
  if (_sc_running)
    {
      _sc_running=false;
      g_scurve.join();
    }
  //Stop listening
  if (g_d.size()!=0){

    
    bool running=false;
    std::map<uint32_t,LiboardInterface*> dm=this->getLiboardMap();
    for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
      {
	running=running ||it->second->readoutStarted();
      }
    if (running)
      {
	for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
	  {
	    it->second->setReadoutStarted(false);
	  }
      
	this->joinThreads();
      }
      // Clear thread vector
    g_d.clear();
  }
  
}





void LiboardManager::startReadoutThread(LiboardInterface* d)
{
  if (d->readoutStarted()) return;
  d->setReadoutStarted(true);	
  g_d.push_back(std::thread(std::bind(&LiboardInterface::readout,d)));
  
}

void LiboardManager::ScurveStep(std::string mdcc,std::string builder,int thmin,int thmax,int step)
{
  std::map<uint32_t,LiboardInterface*> dm=this->getLiboardMap();
  int ncon=50000,ncoff=100,ntrg=50;
  utils::sendCommand(mdcc,"PAUSE",json::value::null());
  web::json::value p;
  p["nclock"]=ncon;utils::sendCommand(mdcc,"SPILLON",p);
  p["nclock"]=ncoff;utils::sendCommand(mdcc,"SPILLOFF",p);
  printf("Clock On %d Off %d \n",ncon, ncoff);
  p["value"]=4;utils::sendCommand(mdcc,"SETSPILLREGISTER",p);
  utils::sendCommand(mdcc,"CALIBON",json::value::null());
  p["nclock"]=ntrg;utils::sendCommand(mdcc,"SETCALIBCOUNT",p);
  int thrange=(thmax-thmin+1)/step;
  for (int vth=0;vth<=thrange;vth++)
    {
      if (!_running) break;
      utils::sendCommand(mdcc,"PAUSE",json::value::null());
      usleep(1000);
      this->setThresholds(thmax-vth*step,512,512);
      
      web::json::value h;
      web::json::value ph;
      h[0]=json::value::number(2);h[1]=json::value::number(thmax-vth*step);

      int firstEvent=0;
      for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
	
	if (it->second->status()->gtc>firstEvent) firstEvent=it->second->status()->gtc;

      ph["header"]=h;
      ph["nextevent"]=json::value::number(firstEvent+1);
      utils::sendCommand(builder,"SETHEADER",ph);
      utils::sendCommand(mdcc,"RELOADCALIB",json::value::null());
      utils::sendCommand(mdcc,"RESUME",json::value::null());
      int nloop=0,lastEvent=firstEvent;
      while (lastEvent < (firstEvent + ntrg - 1))
	{
	  ::usleep(10000);
	  for ( std::map<uint32_t,LiboardInterface*>::iterator it=dm.begin();it!=dm.end();it++)
	
	    if (it->second->status()->gtc>firstEvent) firstEvent=it->second->status()->gtc;

	  nloop++;if (nloop > 60000 || !_running)  break;
	}
      printf("Step %d Th %d First %d Last %d \n",vth,thmax-vth*step,firstEvent,lastEvent);
      utils::sendCommand(mdcc,"PAUSE",json::value::null());
    }
  utils::sendCommand(mdcc,"CALIBOFF",json::value::null());
}


void LiboardManager::thrd_scurve()
{
  _sc_running=true;
  this->Scurve(_sc_mode,_sc_thmin,_sc_thmax,_sc_step);
  _sc_running=false;
}


void LiboardManager::Scurve(int mode,int thmin,int thmax,int step)
{
  std::string mdccUrl=utils::findUrl(session(),"lyon_mdcc",0);
  std::string builderUrl=utils::findUrl(session(),"lyon_evb",0);
  if (mdccUrl.compare("")==0) return;
  if (builderUrl.compare("")==0) return;

  uint64_t mask=0;

  // All channel pedestal
  if (mode==255)
    {

      //for (int i=0;i<64;i++) mask|=(1<<i);
      mask=0xFFFFFFFFFFFFFFFF;
      this->setAllMasks(mask);
      this->ScurveStep(mdccUrl,builderUrl,thmin,thmax,step);
      return;
      
    }

  // Chanel per channel pedestal (CTEST is active)
  if (mode==1023)
    {
      mask=0;
      for (int i=0;i<64;i++)
	{
	  mask=(1ULL<<i);
	  std::cout<<"Step LR "<<i<<" channel "<<i<<std::endl;
	  this->setAllMasks(mask);
	  this->setCTEST(mask);
	  this->ScurveStep(mdccUrl,builderUrl,thmin,thmax,step);
	}
      return;
    }

  // One channel pedestal

  mask=(1ULL<<mode);
  PMF_INFO(_logLiboard,"CTEST One "<<mode<<" "<<std::hex<<mask<<std::dec);
  this->setAllMasks(mask);
  this->setCTEST(mask);
  this->ScurveStep(mdccUrl,builderUrl,thmin,thmax,step);

  
}

void LiboardManager::c_scurve(http_request m)
{
  auto par = json::value::object();
  par["STATUS"]=web::json::value::string(U("DONE"));

  uint32_t first = utils::queryIntValue(m,"first",80);
  uint32_t last = utils::queryIntValue(m,"last",250);
  uint32_t step = utils::queryIntValue(m,"step",1);
  uint32_t mode = utils::queryIntValue(m,"channel",255);
  PMF_INFO(_logLiboard, " SCURVE/CTEST "<<mode<<" "<<step<<" "<<first<<" "<<last);
  
  //this->Scurve(mode,first,last,step);

  _sc_mode=mode;
  _sc_thmin=first;
  _sc_thmax=last;
  _sc_step=step;
  if (_sc_running)
    {
      par["SCURVE"]=web::json::value::string(U("ALREADY_RUNNING"));
      return;
    }
  _sc_running=true;
  g_scurve=std::thread(std::bind(&LiboardManager::thrd_scurve, this));
  par["SCURVE"]=web::json::value::string(U("RUNNING"));

  Reply(status_codes::OK,par);  

}



extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new  LiboardManager);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
