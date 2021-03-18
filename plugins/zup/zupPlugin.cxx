
#include "zupPlugin.hh"



using namespace zup;
zupPlugin::zupPlugin(): _lv(NULL){} 
void zupPlugin::open()
{

  PMF_INFO(_logZup," CMD: Open ");

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
  
  
  
  _lv= new Zup(device,port);
 
  _lv->INFO();

}
void zupPlugin::close()
{
  PMF_INFO(_logZup," CMD: closing Zup");
  if (_lv==NULL)
    {
       PMF_ERROR(_logZup,"No HVZupInterface opened");
       return;
    }
  
  delete _lv;
  _lv=NULL;
}

web::json::value zupPlugin::status()
{
  auto r=json::value::object();
  r["name"]=json::value::string(U(this->hardware()));
  r["status"]=json::value::string(U("UNKNOWN"));
   if (_lv==NULL)
    {
      PMF_ERROR(_logZup,"No Zup Interface opened");
       return r;
    }
   auto sr=_lv->Status();

   r["vset"]=sr["vset"];
   r["iset"]=sr["iset"];
   r["vout"]=sr["vout"];
   r["iout"]=sr["iout"];
   r["pwrstatus"]=sr["status"];
   int pws=sr["status"].as_integer();
   if ((pws>>4)&1)
     r["status"]=json::value::string(U("ON"));
   else
     r["status"]=json::value::string(U("OFF"));

   return r;
}


void zupPlugin::c_status(http_request m)
{
  auto par = json::value::object();
  if (_lv==NULL)
  {
    PMF_ERROR(_logZup,"No Zup opened");
    par["status"]=json::value::string(U("No Device"));
    Reply(status_codes::OK,par);

    return;
  }

  par["status"] =this->status();
  Reply(status_codes::OK,par);

}
void zupPlugin::c_on(http_request m)
{
  auto par = json::value::object();
  if (_lv==NULL)
  {
    PMF_ERROR(_logZup,"No Zup opened");
    par["status"]=json::value::string(U("No Device"));
    Reply(status_codes::OK,par);

    return;
  }
 _lv->ON();
  ::usleep(50000);

  par["status"] =this->status();
  Reply(status_codes::OK,par);

}

void zupPlugin::c_off(http_request m)
{
  auto par = json::value::object();
 if (_lv==NULL)
  {
    PMF_ERROR(_logZup,"No Zup opened");
    par["status"]=json::value::string(U("No Device"));
    Reply(status_codes::OK,par);

    return;
  }
 _lv->OFF();
  ::usleep(50000);

 par["status"] =this->status();
 Reply(status_codes::OK,par);

}

void zupPlugin::c_setdevice(http_request m)
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
      PMF_ERROR(_logZup,"Invalid device or Port");
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


void zupPlugin::registerCommands() 
{

 addCommand("ON",std::bind(&zupPlugin::c_on,this,std::placeholders::_1));
 addCommand("OFF",std::bind(&zupPlugin::c_off,this,std::placeholders::_1));
 addCommand("SETDEVICE",std::bind(&zupPlugin::c_setdevice,this,std::placeholders::_1));
 addCommand("ZUP_STATUS",std::bind(&zupPlugin::c_status,this,std::placeholders::_1));

   
}



extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new zupPlugin);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
