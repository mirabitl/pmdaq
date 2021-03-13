
#include "wienerPlugin.hh"

wienerPlugin::wienerPlugin(): _hv(NULL){} 
void wienerPlugin::open()
{
  PM_INFO(_logPdaq,__PRETTY_FUNCTION__<<" CMD: Opening");

  std::string address;
  if (params().as_object().find("address")!=params().as_object().end())
    { 
      address=params()["address"].as_string();
    }
  else
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<" No address given");
      return;
    }


  std::cout<<"calling open "<<std::endl;
  if (_hv!=NULL)
    delete _hv;
  
  
  
  
  _hv= new lydaq::WienerSnmp(address);
  

}
void wienerPlugin::close()
{
  PM_INFO(_logPdaq,__PRETTY_FUNCTION__<<" Closing ");
  if (_hv==NULL)
    {
       PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<" No HVWienerInterface opened");
       return;
    }
  
  delete _hv;
  _hv=NULL;
}
web::json::value wienerPlugin::channelStatus(uint32_t channel)
{
  auto r= json::value::object();
  r["id"]=channel;
  r["status"]=json::value::string(U("notset"));
   if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No WienerSnmp opened");
       return r;
    }
   // std::cout<<channel<<" gives "<<_hv->getOutputVoltage(channel/8,channel%8)<<std::endl;
   r["vset"]=json::value::number(_hv->getOutputVoltage(channel/8,channel%8));
   r["iset"]=json::value::number(_hv->getOutputCurrentLimit(channel/8,channel%8));
   r["rampup"]=json::value::number(_hv->getOutputVoltageRiseRate(channel/8,channel%8));
   r["iout"]=json::value::number(_hv->getOutputMeasurementCurrent(channel/8,channel%8));
   r["vout"]=json::value::number(_hv->getOutputMeasurementSenseVoltage(channel/8,channel%8));
   r["status"]=json::value::string(U(_hv->getOutputStatus(channel/8,channel%8)));
   
   return r;
}
web::json::value wienerPlugin::status()
{ return status(-1,-1);}
web::json::value wienerPlugin::status(int32_t first,int32_t last)
{

  auto r= json::value::object();
  r["name"]=json::value::string(U(this->hardware()));

  web::json::value c_array;uint32_t nc=0;

  if (_hv==NULL)
  {
    PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No WienerSnmp opened");
    return r;
  }
  int32_t fi=0,la=0;
  if (params().as_object().find("first")==params().as_object().end() && first<0)
  {
    PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"Please define first channel");
    return r;
  }
  if (first<0)
    fi=params()["first"].as_integer();
  else
    fi=first;
  if (params().as_object().find("first")==params().as_object().end() && last<0)
  {
    PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"Please define last channel");
    return r;
  }
  if (last<0)
    la=params()["last"].as_integer();
  else
    la=last;

  for (uint32_t i=fi;i<=la;i++)
    {
      c_array[nc++]=this->channelStatus(i);
      //std::cout <<v<<std::endl;
    }
  r["channels"]=c_array;
  return r;
}

void wienerPlugin::c_status(http_request m)
{

 auto par = json::value::object();

 if (_hv==NULL)
  {
    PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No WienerSnmp opened");
    par["status"]=json::value::string(U("Invalid Device"));
    Reply(status_codes::OK,par);
    return;
  }


 
 uint32_t first = 999;
 uint32_t last =999;

 auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
 for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
   {
     if (it2->first.compare("first")==0)
       first=std::stoi(it2->second);
     if (it2->first.compare("last")==0)
       last=std::stoi(it2->second);
   }
  if (first==999 || last==999)
    {
      PM_ERROR(_logPdaq,"Invalid first or last");
      par["status"]=json::value::string(U("Invalid channels"));
      Reply(status_codes::OK,par);
      return;
    }


 
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}


void wienerPlugin::c_on(http_request m)
{

 auto par = json::value::object();

 if (_hv==NULL)
  {
    PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No WienerSnmp opened");
    par["status"]=json::value::string(U("Invalid Device"));
    Reply(status_codes::OK,par);
    return;
  }


 
 uint32_t first = 999;
 uint32_t last =999;

 auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
 for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
   {
     if (it2->first.compare("first")==0)
       first=std::stoi(it2->second);
     if (it2->first.compare("last")==0)
       last=std::stoi(it2->second);
   }
  if (first==999 || last==999)
    {
      PM_ERROR(_logPdaq,"Invalid first or last");
      par["status"]=json::value::string(U("Invalid channels"));
      Reply(status_codes::OK,par);
      return;
    }


  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputSwitch(i/8,i%8,1);
  ::sleep(2);
  
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}
void wienerPlugin::c_off(http_request m)
{

  auto par = json::value::object();

  if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No WienerSnmp opened");
      par["status"]=json::value::string(U("Invalid Device"));
      Reply(status_codes::OK,par);
      return;
    }


 
  uint32_t first = 999;
  uint32_t last =999;

  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("first")==0)
	first=std::stoi(it2->second);
      if (it2->first.compare("last")==0)
	last=std::stoi(it2->second);
    }
  if (first==999 || last==999)
    {
      PM_ERROR(_logPdaq,"Invalid first or last");
      par["status"]=json::value::string(U("Invalid channels"));
      Reply(status_codes::OK,par);
      return;
    }


  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputSwitch(i/8,i%8,0);
  ::sleep(2);
  
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}
void wienerPlugin::c_clearalarm(http_request m)
{

  auto par = json::value::object();

  if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No WienerSnmp opened");
      par["status"]=json::value::string(U("Invalid Device"));
      Reply(status_codes::OK,par);
      return;
    }


 
  uint32_t first = 999;
  uint32_t last =999;

  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("first")==0)
	first=std::stoi(it2->second);
      if (it2->first.compare("last")==0)
	last=std::stoi(it2->second);
    }
  if (first==999 || last==999)
    {
      PM_ERROR(_logPdaq,"Invalid first or last");
      par["status"]=json::value::string(U("Invalid channels"));
      Reply(status_codes::OK,par);
      return;
    }


  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputSwitch(i/8,i%8,10);
  ::sleep(2);
  
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}
void wienerPlugin::c_vset(http_request m)
{

  auto par = json::value::object();

  if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No WienerSnmp opened");
      par["status"]=json::value::string(U("Invalid Device"));
      Reply(status_codes::OK,par);
      return;
    }


 
  uint32_t first = 999;
  uint32_t last =999;
  float vset=-1.0;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("first")==0)
	first=std::stoi(it2->second);
      if (it2->first.compare("last")==0)
	last=std::stoi(it2->second);
      if (it2->first.compare("value")==0)
	vset=std::stof(it2->second);
    }
  if (first==999 || last==999 ||vset<0)
    {
      PM_ERROR(_logPdaq,"Invalid first or last or value");
      par["status"]=json::value::string(U("Invalid channels"));
      Reply(status_codes::OK,par);
      return;
    }


  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputVoltage(i/8,i%8,vset);
  ::sleep(2);
  
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}

void wienerPlugin::c_iset(http_request m)
{

  auto par = json::value::object();

  if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No WienerSnmp opened");
      par["status"]=json::value::string(U("Invalid Device"));
      Reply(status_codes::OK,par);
      return;
    }


 
  uint32_t first = 999;
  uint32_t last =999;
  float iset=-1.0;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("first")==0)
	first=std::stoi(it2->second);
      if (it2->first.compare("last")==0)
	last=std::stoi(it2->second);
      if (it2->first.compare("value")==0)
	iset=std::stof(it2->second);
    }
  if (first==999 || last==999 ||iset<0)
    {
      PM_ERROR(_logPdaq,"Invalid first or last or value");
      par["status"]=json::value::string(U("Invalid channels"));
      Reply(status_codes::OK,par);
      return;
    }


  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputCurrentLimit(i/8,i%8,iset);
  

  ::sleep(2);
  
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}

void wienerPlugin::c_rampup(http_request m)
{

  auto par = json::value::object();

  if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No WienerSnmp opened");
      par["status"]=json::value::string(U("Invalid Device"));
      Reply(status_codes::OK,par);
      return;
    }


 
  uint32_t first = 999;
  uint32_t last =999;
  float rup=-1.0;
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      if (it2->first.compare("first")==0)
	first=std::stoi(it2->second);
      if (it2->first.compare("last")==0)
	last=std::stoi(it2->second);
      if (it2->first.compare("value")==0)
	rup=std::stof(it2->second);
    }
  if (first==999 || last==999 ||rup<0)
    {
      PM_ERROR(_logPdaq,"Invalid first or last or value");
      par["status"]=json::value::string(U("Invalid channels"));
      Reply(status_codes::OK,par);
      return;
    }


  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputVoltageRiseRate(i/8,i%8,-1.*rup);
  

  ::sleep(2);
  
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}








void wienerPlugin::registerCommands()
{

 this->addCommand("WP_STATUS",std::bind(&wienerPlugin::c_status,this,std::placeholders::_1));
 this->addCommand("ON",std::bind(&wienerPlugin::c_on,this,std::placeholders::_1));
 this->addCommand("OFF",std::bind(&wienerPlugin::c_off,this,std::placeholders::_1));
 this->addCommand("VSET",std::bind(&wienerPlugin::c_vset,this,std::placeholders::_1));
 this->addCommand("ISET",std::bind(&wienerPlugin::c_iset,this,std::placeholders::_1));
 this->addCommand("RAMPUP",std::bind(&wienerPlugin::c_rampup,this,std::placeholders::_1));
 this->addCommand("CLEARALARM",std::bind(&wienerPlugin::c_clearalarm,this,std::placeholders::_1));

   
}

extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new wienerPlugin);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}

