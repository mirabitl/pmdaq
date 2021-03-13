
#include "genesysPlugin.hh"
using namespace lydaq;
genesysPlugin::genesysPlugin(): _lv(NULL){} 
void genesysPlugin::open()
{

  PM_INFO(_logPdaq," CMD: Open ");

  std::string device;
  if (params().as_object().find("device")!=params().as_object().end())
    { 
      device=params()["device"].as_string();
    }


  uint32_t port;

  if (params().as_object().find("port")!=params().as_object().end())
    { 
    port=params()["port"].as_integer();

  }
  std::cout<<"calling open "<<std::endl;
  if (_lv!=NULL)
    delete _lv;
  
  
  
  _lv= new Genesys(device,port);
 
  _lv->INFO();

}
void genesysPlugin::close()
{
  PM_INFO(_logPdaq," CMD: closing Genesys");
  if (_lv==NULL)
    {
       PM_ERROR(_logPdaq,"No HVGenesysInterface opened");
       return;
    }
  
  delete _lv;
  _lv=NULL;
}

web::json::value genesysPlugin::status()
{
  auto r=json::value::object();
  r["name"]=json::value::string(U(this->hardware()));
  r["status"]=json::value::string(U("UNKNOWN"));
   if (_lv==NULL)
    {
      PM_ERROR(_logPdaq,"No Genesys Interface opened");
       return r;
    }
   
   float vset=_lv->ReadVoltageSet();
   float vout=_lv->ReadVoltageUsed();
   float iout=_lv->ReadCurrentUsed();
   r["vset"]=json::value::number(vset);
   r["vout"]=json::value::number(vout);
   r["iout"]=json::value::number(iout);
   if (abs(1-abs(vset-vout)/vset)<0.8)
     r["status"]=json::value::string(U("OFF"));
   else
     r["status"]=json::value::string(U("ON"));

   return r;
}


void genesysPlugin::c_status(http_request m)
{
  auto par = json::value::object();
  if (_lv==NULL)
  {
    PM_ERROR(_logPdaq,"No Genesys opened");
    par["status"]=json::value::string(U("No Device"));
    Reply(status_codes::OK,par);

    return;
  }

  par["status"] =this->status();
  Reply(status_codes::OK,par);

}
void genesysPlugin::c_on(http_request m)
{
  auto par = json::value::object();
  if (_lv==NULL)
  {
    PM_ERROR(_logPdaq,"No Genesys opened");
    par["status"]=json::value::string(U("No Device"));
    Reply(status_codes::OK,par);

    return;
  }
 _lv->ON();
  ::usleep(50000);

  par["status"] =this->status();
  Reply(status_codes::OK,par);

}

void genesysPlugin::c_off(http_request m)
{
  auto par = json::value::object();
 if (_lv==NULL)
  {
    PM_ERROR(_logPdaq,"No Genesys opened");
    par["status"]=json::value::string(U("No Device"));
    Reply(status_codes::OK,par);

    return;
  }
 _lv->OFF();
  ::usleep(50000);

 par["status"] =this->status();
 Reply(status_codes::OK,par);

}

void genesysPlugin::c_setdevice(http_request m)
{
  auto par = json::value::object(); 
  uint32_t device = 999;
  uint32_t address =999;

  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("device")==0)
        device=std::stoi(it2->second);
      if (it2->first.compare("port")==0)
        address=std::stoi(it2->second);
      }

  if (device==999 || address==999)
    {
      PM_ERROR(_logPdaq,"Invalid device or Port");
      par["status"]=json::value::string(U("Invalid Device"));
      Reply(status_codes::OK,par);
      return;
    }
  std::stringstream sdev("");
  sdev<<"/dev/ttyUSB"<<device;
  params()["device"]=json::value::string(U(sdev.str()));
  params()["port"]=json::value::number(address);



  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK,par);
}


void genesysPlugin::registerCommands() 
{

 addCommand("ON",std::bind(&genesysPlugin::c_on,this,std::placeholders::_1));
 addCommand("OFF",std::bind(&genesysPlugin::c_off,this,std::placeholders::_1));
 addCommand("SETDEVICE",std::bind(&genesysPlugin::c_setdevice,this,std::placeholders::_1));
 addCommand("GS_STATUS",std::bind(&genesysPlugin::c_status,this,std::placeholders::_1));

   
}



extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new genesysPlugin);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
