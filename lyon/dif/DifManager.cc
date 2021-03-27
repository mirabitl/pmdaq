
#include "DifManager.hh"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
//#include "ftdi.hpp"
#include "pmSender.hh"
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <string.h>

//using namespace Ftdi;
using namespace dif;



void DifManager::scan(http_request m) 
{
  auto par = json::value::object();
  PMF_INFO(_logDif," CMD: SCanning");
  // Store dbcache if changed
  //
   this->prepareDevices();
   std::map<uint32_t,FtdiDeviceInfo*>& fm=this->getFtdiMap();
   std::map<uint32_t,dif::interface*> dm=this->getDifMap();
   //PMF_INFO(_logDif," CMD: SCANDEVICE clear Maps");
   for ( std::map<uint32_t,dif::interface*>::iterator it=dm.begin();it!=dm.end();it++)
     { if (it->second!=NULL) delete it->second;}
   dm.clear();
   // _ndif=0;
   std::vector<uint32_t> vids;
   web::json::value array;uint32_t nd=0;
   for ( std::map<uint32_t,FtdiDeviceInfo*>::iterator it=fm.begin();it!=fm.end();it++)
     {

       dif::interface* d= new dif::interface(it->second);
       this->getDifMap().insert(std::make_pair(it->first,d));
       PMF_INFO(_logDif," CMD: SCANDEVICE created dif::interface @ "<<std::hex<<d<<std::dec);
       web::json::value jd;
       jd["detid"]=json::value::number(d->detectorId());
       jd["sourceid"]=json::value::number(it->first);
       vids.push_back( (d->detectorId()<<16|it->first));
       array[nd++]=jd;
     }

   par["status"]=json::value::string(U("scanned"));
   par["devices"]=array;
   Reply(status_codes::OK,par);  

}


void DifManager::fsm_initialise(http_request m)
{
  auto par = json::value::object();  
  PMF_INFO(_logDif," CMD: Initialising");
  _vDif.clear();
  web::json::value jDif=params()["dif"];
  
   // Download the configuration
   if (_hca==NULL)
     {
       std::cout<< "Create config acccess"<<std::endl;
       _hca=new HR2ConfigAccess();
       _hca->clear();
     }
   //std::cout<< " jDif "<<jDif<<std::endl;
   if (utils::isMember(jDif,"json"))
     {
       web::json::value jDifjson=jDif["json"];
       if (utils::isMember(jDifjson,"file"))
	 {
	   _hca->parseJsonFile(jDifjson["file"].as_string());
	 }
       else
	 if (utils::isMember(jDifjson,"url"))
	   {
	     _hca->parseJsonUrl(jDifjson["url"].as_string());
	   }
     }
    if (utils::isMember(jDif,"db"))
     {
              web::json::value jDifdb=jDif["db"];
       PMF_INFO(_logDif,"Parsing:"<<jDifdb["state"].as_string()<<jDifdb["mode"].as_string());

              
	if (jDifdb["mode"].as_string().compare("mongo")==0)	
	  _hca->parseMongoDb(jDifdb["state"].as_string(),jDifdb["version"].as_integer());

	PMF_INFO(_logDif,"End of parseDB "<<_hca->asicMap().size());
     }
   if (_hca->asicMap().size()==0)
     {
        PMF_ERROR(_logDif," No ASIC found in the configuration ");
	par["status"]=json::value::string(U("Failed"));
	Reply(status_codes::OK,par);  
       return;
     }
   PMF_INFO(_logDif,"ASIC found in the configuration "<<_hca->asicMap().size() );
   // Initialise the network
   std::map<uint32_t,dif::interface*> dm=this->getDifMap();
   std::vector<uint32_t> vint;
   
   vint.clear();
   for (auto x:_hca->asicMap())
     {
       // only MSB is used
       uint32_t eip= ((x.first)>>56)&0XFF;
       std::map<uint32_t,dif::interface*>::iterator idif=dm.find(eip);
       if (idif==dm.end()) continue;
       if ( std::find(vint.begin(), vint.end(), eip) != vint.end() ) continue;

       PMF_INFO(_logDif," New Dif found in db "<<std::hex<<eip<<std::dec);
       vint.push_back(eip);
      
       _vDif.push_back(idif->second);
       PMF_INFO(_logDif," Registration done for "<<eip);
     }
   //std::string network=
  // Connect to the event builder
  if (_context==NULL)
    _context= new zmq::context_t(1);


  
  for (auto x:_vDif)
    {
      PMF_INFO(_logDif," Creating pusher to "<<params()["publish"].as_string());

      pm::pmSender* push= new pm::pmSender(_context,x->detectorId(),x->status()->id);
      push->autoDiscover(session(),"evb_builder","collectingPort");
      push->collectorRegister();

      x->initialise(push);

    }

  par["status"]=json::value::string(U("Initialised"));
    Reply(status_codes::OK,par);  
  
}







void DifManager::setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif)
{

  PMF_INFO(_logDif," Changin thresholds: "<<b0<<","<<b1<<","<<b2);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      if (idif!=0)
	{
	  uint32_t ip=(((it->first)>>32&0XFFFFFFFF)>>16)&0xFFFF;
	  printf("%x %x %x \n",(it->first>>32),ip,idif);
	  if (idif!=ip) continue;
	}
      it->second.setB0(b0);
      it->second.setB1(b1);
      it->second.setB2(b2);
      //it->second.setHEADER(0x56);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::usleep(1000);

}
void DifManager::setGain(uint16_t gain)
{

  PMF_INFO(_logDif," Changing Gain: "<<gain);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      for (int i=0;i<64;i++)
	it->second.setPAGAIN(i,gain);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::usleep(1000);

}

void DifManager::setMask(uint32_t level,uint64_t mask)
{
PMF_INFO(_logDif," Changing Mask: "<<level<<" "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
	it->second.setMASK(level,mask);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::usleep(1000);

}
void DifManager::setChannelMask(uint16_t level,uint16_t channel,uint16_t val)
{
PMF_INFO(_logDif," Changing Mask: "<<level<<" "<<std::hex<<channel<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASKChannel(level,channel,val==1);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::usleep(1000);

}

void DifManager::c_setthresholds(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logDif,"Set6bdac called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  uint32_t b0=utils::queryIntValue(m,"B0",250);
  uint32_t b1=utils::queryIntValue(m,"B1",250);
  uint32_t b2=utils::queryIntValue(m,"B2",250);
  uint32_t idif=utils::queryIntValue(m,"Dif",0);
  
  this->setThresholds(b0,b1,b2,idif);
  par["THRESHOLD0"]=web::json::value::number(b0);
  par["THRESHOLD1"]=web::json::value::number(b1);
  par["THRESHOLD2"]=web::json::value::number(b2);
  Reply(status_codes::OK,par);  
}
void DifManager::c_setpagain(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logDif,"Set6bdac called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  uint32_t gain=utils::queryIntValue(m,"gain",128);
  this->setGain(gain);
  par["GAIN"]=web::json::value::number(gain);
  Reply(status_codes::OK,par);  
}

void DifManager::c_setmask(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logDif,"SetMask called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  //uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint64_t mask;
  sscanf(utils::queryStringValue(m,"mask","0XFFFFFFFFFFFFFFFF").c_str(),"%lx",&mask);
  uint32_t level=utils::queryIntValue(m,"level",0);
  PMF_INFO(_logDif,"SetMask called "<<std::hex<<mask<<std::dec<<" level "<<level);
  this->setMask(level,mask);
  par["MASK"]=web::json::value::number(mask);
  par["LEVEL"]=web::json::value::number(level);
  Reply(status_codes::OK,par);  
}



void DifManager::c_setchannelmask(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logDif,"SetMask called ");
  par["STATUS"]=web::json::value::string(U("DONE"));

  
  //uint32_t nc=utils::queryIntValue(m,"value",4294967295);
  uint32_t level=utils::queryIntValue(m,"level",0);
  uint32_t channel=utils::queryIntValue(m,"channel",0);
  bool on=utils::queryIntValue(m,"value",1)==1;
  PMF_INFO(_logDif,"SetMaskChannel called "<<channel<<std::dec<<" level "<<level);
  this->setChannelMask(level,channel,on);
  par["CHANNEL"]=web::json::value::number(channel);
  par["LEVEL"]=web::json::value::number(level);
  par["ON"]=web::json::value::number(on);
  Reply(status_codes::OK,par);  
}
void DifManager::c_ctrlreg(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logDif,"CTRLREG called ");

  uint32_t  ctrlreg=0;
  //sscanf(request.get("value","0").c_str(),"%u",&ctrlreg);
  sscanf(utils::queryStringValue(m,"value","0x0").c_str(),"%u",&ctrlreg);
  if (ctrlreg!=0)
    params()["ctrlreg"]=ctrlreg;

  
  PMF_INFO(_logDif,"CTRLREG called "<<std::hex<<ctrlreg<<std::dec);
  par["STATUS"]=web::json::value::string(U("DONE"));
  par["CTRLREG"]=web::json::value::number( ctrlreg);
  Reply(status_codes::OK,par);  
}
void DifManager::c_downloadDB(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logDif,"downloadDB called ");
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
   par["DBSTATE"]=web::json::value::string(U(dbstate));
   Reply(status_codes::OK,par);  
}


void DifManager::c_status(http_request m)
{
  auto par = json::value::object();
  int32_t rc=1;
  std::map<uint32_t,dif::interface*> dm=this->getDifMap();
  web::json::value array_slc;uint32_t nd=0;
 
  for ( std::map<uint32_t,dif::interface*>::iterator it=dm.begin();it!=dm.end();it++)
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
      ds["host"]=json::value::string(U(it->second->status()->host));
      array_slc[nd++]=ds;
    }
  par["STATUS"]=web::json::value::string(U("DONE"));
  par["DifLIST"]=array_slc;
  Reply(status_codes::OK,par);  


  return;
  
}

web::json::value DifManager::configureHR2()
{
  uint32_t ctrlreg=params()["ctrlreg"].as_integer();
  printf("CTRLREG %lx \n",ctrlreg);
  int32_t rc=1;
  std::map<uint32_t,dif::interface*> dm=this->getDifMap();
  web::json::value array_slc;uint32_t nd=0;

  for ( std::map<uint32_t,dif::interface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      std::stringstream ips;
      // Dummy IP address for Difs
      ips<<"0.0.0."<<it->first;
      _hca->prepareSlowControl(ips.str(),true);
      DIFDbInfo* dbdif=it->second->dbdif();
      dbdif->id=it->first;
      dbdif->nbasic=_hca->slcBytes()/HARDROCV2_SLC_FRAME_SIZE;
      
      memcpy(dbdif->slow,_hca->slcBuffer(),_hca->slcBytes());
      it->second->configure(ctrlreg);
      web::json::value ds;
      ds["id"]=json::value::number(it->first);
      ds["slc"]=json::value::number(it->second->status()->slc);
      array_slc[nd++]=ds;
      

    }
  return array_slc;
}

void DifManager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_DEBUG(_logDif," CMD: CONFIGURING");
  uint32_t ctrlreg=utils::queryIntValue(m,"ctrlreg",0);
  if (ctrlreg!=0)
      params()["ctrlreg"]=json::value::number(ctrlreg);

  ctrlreg=params()["ctrlreg"].as_integer();
  PMF_INFO(_logDif," Configuring with  ctr "<<ctrlreg);
  int32_t rc=1;
  std::map<uint32_t,dif::interface*> dm=this->getDifMap();
  web::json::value array_slc=this->configureHR2();

  par["devices"]=array_slc;
  par["status"]=json::value::string(U("Configured"));
  Reply(status_codes::OK,par);  
  return;
    
}

void DifManager::start(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logDif," Starting ");
 
  int32_t rc=1;
  std::map<uint32_t,dif::interface*> dm=this->getDifMap();
  for ( std::map<uint32_t,dif::interface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      this->startDIFThread(it->second);
      it->second->start();
    }
  par["status"]=json::value::string(U("started"));
  Reply(status_codes::OK,par);  
  return;
  
}
void DifManager::stop(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logDif," Stopping ");
 
  int32_t rc=1;
  std::map<uint32_t,dif::interface*> dm=this->getDifMap();
 
  for ( std::map<uint32_t,dif::interface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      PMF_INFO(_logDif," Stopping thread of Dif"<<it->first);
      it->second->stop();
    }
  par["status"]=json::value::string(U("Stopped"));
  Reply(status_codes::OK,par);  
  return;
  
}
void DifManager::destroying()
{
   PMF_INFO(_logDif," Destroying ");
  
  int32_t rc=1;
  std::map<uint32_t,dif::interface*> dm=this->getDifMap();

  bool running=false;
  for ( std::map<uint32_t,dif::interface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      running=running ||it->second->readoutStarted();
    }
  if (running)
    {
      for ( std::map<uint32_t,dif::interface*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->setReadoutStarted(false);
	}
      
      this->joinThreads();
    }
  for ( std::map<uint32_t,dif::interface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      it->second->destroy();
    }

 
}
void DifManager::destroy(http_request m)
{
  auto par = json::value::object();
  this->destroying();
  return;
  
}






 DifManager::DifManager()  {;}
 
 void DifManager::initialise()
{
  // Zmq transport
  _context = new zmq::context_t (1);
  // Register state
  this->addState("SCANNED");
  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  this->addState("STOPPED");
  this->addTransition("SCAN","CREATED","SCANNED",std::bind(&DifManager::scan, this,std::placeholders::_1));
  this->addTransition("INITIALISE","SCANNED","INITIALISED",std::bind(&DifManager::fsm_initialise, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","INITIALISED","CONFIGURED",std::bind(&DifManager::configure, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",std::bind(&DifManager::configure, this,std::placeholders::_1));
  this->addTransition("CONFIGURE","STOPPED","CONFIGURED",std::bind(&DifManager::configure, this,std::placeholders::_1));
  this->addTransition("START","CONFIGURED","RUNNING",std::bind(&DifManager::start, this,std::placeholders::_1));
  this->addTransition("START","STOPPED","RUNNING",std::bind(&DifManager::start, this,std::placeholders::_1));
  this->addTransition("STOP","RUNNING","STOPPED",std::bind(&DifManager::stop, this,std::placeholders::_1));
  this->addTransition("DESTROY","STOPPED","CREATED",std::bind(&DifManager::destroy, this,std::placeholders::_1));
  this->addTransition("DESTROY","CONFIGURED","CREATED",std::bind(&DifManager::destroy, this,std::placeholders::_1));


  this->addCommand("STATUS",std::bind(&DifManager::c_status,this,std::placeholders::_1));
  this->addCommand("SETTHRESHOLDS",std::bind(&DifManager::c_setthresholds,this,std::placeholders::_1));
  this->addCommand("SETPAGAIN",std::bind(&DifManager::c_setpagain,this,std::placeholders::_1));
  this->addCommand("SETMASK",std::bind(&DifManager::c_setmask,this,std::placeholders::_1));
  this->addCommand("SETCHANNELMASK",std::bind(&DifManager::c_setchannelmask,this,std::placeholders::_1));
  this->addCommand("DOWNLOADDB",std::bind(&DifManager::c_downloadDB,this,std::placeholders::_1));
  this->addCommand("CTRLREG",std::bind(&DifManager::c_ctrlreg,this,std::placeholders::_1));

  _hca=NULL;
  g_d.clear();
  // Initialise delays for 
}

void DifManager::end()
{
  this->destroying();
}





void DifManager::prepareDevices()
{
  for ( std::map<uint32_t,FtdiDeviceInfo*>::iterator it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    if (it->second!=NULL) delete it->second;
  theFtdiDeviceInfoMap_.clear();
  for ( auto it=_interfaceMap.begin();it!=_interfaceMap.end();it++)
    if (it->second!=NULL) delete it->second;
  _interfaceMap.clear();
  system("/bin/rm /var/log/pi/ftdi_devices");
  system("/opt/dhcal/bin/ListDevices.py");
  std::string line;
  std::ifstream myfile ("/var/log/pi/ftdi_devices");
  std::stringstream diflist;



  if (myfile.is_open())
    {
      while ( myfile.good() )
	{
	  getline (myfile,line);
	  FtdiDeviceInfo* difi=new FtdiDeviceInfo();
	  memset(difi,0,sizeof(FtdiDeviceInfo));
	  sscanf(line.c_str(),"%x %x %s",&difi->vendorid,&difi->productid,difi->name);
	  if (strncmp(difi->name,"FT101",5)==0)
	    {
	      sscanf(difi->name,"FT101%d",&difi->id); 
	      difi->type=0;
	      std::pair<uint32_t,FtdiDeviceInfo*> p(difi->id,difi);
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
      PMF_FATAL(_logDif," Unable to open /var/log/pi/ftdi_devices");
    }

  for (std::map<uint32_t,FtdiDeviceInfo*>::iterator it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    PMF_INFO(_logDif,"Device found and register "<<it->first<<" with info "<<it->second->vendorid<<" "<<it->second->productid<<" "<<it->second->name<<" "<<it->second->type);
}


void DifManager::joinThreads()
{
  for (auto it=_interfaceMap.begin();it!=_interfaceMap.end();it++)
    it->second->setReadoutStarted(false);
  for (auto it=g_d.begin();it!=g_d.end();it++)
    it->join();
  g_d.clear();

}
void DifManager::startDIFThread(dif::interface* d)
{
  if (d->readoutStarted()) return;
  d->setReadoutStarted(true);	

  g_d.push_back(std::thread(std::bind(&dif::interface::readout,d)));
  
}



