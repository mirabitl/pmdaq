
#include "syx27Plugin.hh"

syx27Plugin::syx27Plugin(): _hv(NULL){} 
void syx27Plugin::open()
{
  PM_INFO(_logPdaq,__PRETTY_FUNCTION__<<" CMD: Opening");

  std::string account;
  if (params().as_object().find("account")!=params().as_object().end())
    { 
      account=params()["account"].as_string();
    }
  else
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<" No account given");
      return;
    }


  std::cout<<"calling open "<<std::endl;
  if (_hv!=NULL)
    delete _hv;
  
  
  int ipass = account.find("/");
  int ipath = account.find("@");
  std::string Name,Pwd,Host;
  Name.clear();
  Name=account.substr(0,ipass); 
  Pwd.clear();
  Pwd=account.substr(ipass+1,ipath-ipass-1); 
  Host.clear();
  Host=account.substr(ipath+1,account.size()-ipath); 
  std::cout<<Name<<"|"<<std::endl;
  std::cout<<Pwd<<"|"<<std::endl;
  std::cout<<Host<<"|"<<std::endl;
  
  
  _hv= new lydaq::HVCaenInterface(Host,Name,Pwd);


  _hv->Connect();

  
  

}
void syx27Plugin::close()
{
  PM_INFO(_logPdaq,__PRETTY_FUNCTION__<<" Closing ");
  if (_hv==NULL)
    {
       PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<" No HVCaenInterface opened");
       return;
    }
  _hv->Disconnect();
  delete _hv;
  _hv=NULL;
}
web::json::value syx27Plugin::channelStatus(uint32_t channel)
{
  auto r= json::value::object();
  r["id"]=channel;
  r["status"]=json::value::string(U("notset"));
   if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No HVCaenInterface opened");
       return r;
    }
   // std::cout<<channel<<" gives "<<_hv->getOutputVoltage(channel/8,channel%8)<<std::endl;
   

   r["name"]==json::value::string(U(_hv->GetName(channel)));
   r["slot"]=json::value::number(_hv->BoardSlot(channel));
   r["channel"]=json::value::number(_hv->BoardChannel(channel));
   r["vset"]=json::value::number(_hv->GetVoltageSet(channel));
   r["iset"]=json::value::number(_hv->GetCurrentSet(channel));
   r["rampup"]=json::value::number(_hv->GetVoltageRampUp(channel));
   r["iout"]=json::value::number(_hv->GetCurrentRead(channel));
   r["vout"]=json::value::number(_hv->GetVoltageRead(channel));
   r["status"]=json::value::number(_hv->GetStatus(channel));
   
   return r;
}
web::json::value syx27Plugin::status()
{ return status(-1,-1);}
web::json::value syx27Plugin::status(int32_t first,int32_t last)
{

  auto r= json::value::object();
  r["name"]=json::value::string(U(this->hardware()));

  web::json::value c_array;uint32_t nc=0;

  if (_hv==NULL)
  {
    PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No HVCaenInterface opened");
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

void syx27Plugin::c_status(http_request m)
{

 auto par = json::value::object();

 if (_hv==NULL)
  {
    PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No HVCaenInterface opened");
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


void syx27Plugin::c_on(http_request m)
{

 auto par = json::value::object();

 if (_hv==NULL)
  {
    PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No HVCaenInterface opened");
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
    _hv->SetOn(i);
  
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}
void syx27Plugin::c_off(http_request m)
{

  auto par = json::value::object();

  if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No HVCaenInterface opened");
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
    _hv->SetOff(i);
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}
void syx27Plugin::c_clearalarm(http_request m)
{

  auto par = json::value::object();

  if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No HVCaenInterface opened");
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


  // Not implemented
  
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}
void syx27Plugin::c_vset(http_request m)
{

  auto par = json::value::object();

  if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No HVCaenInterface opened");
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
    _hv->SetVoltage(i,vset);
  
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}

void syx27Plugin::c_iset(http_request m)
{

  auto par = json::value::object();

  if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No HVCaenInterface opened");
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
    _hv->SetCurrent(i,iset);

  
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}

void syx27Plugin::c_rampup(http_request m)
{

  auto par = json::value::object();

  if (_hv==NULL)
    {
      PM_ERROR(_logPdaq,__PRETTY_FUNCTION__<<"No HVCaenInterface opened");
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
    _hv->SetVoltageRampUp(i,rup);
  par["status"] = this->status(first,last);
  Reply(status_codes::OK,par);
}








void syx27Plugin::registerCommands()
{

 this->addCommand("SY_STATUS",std::bind(&syx27Plugin::c_status,this,std::placeholders::_1));
 this->addCommand("ON",std::bind(&syx27Plugin::c_on,this,std::placeholders::_1));
 this->addCommand("OFF",std::bind(&syx27Plugin::c_off,this,std::placeholders::_1));
 this->addCommand("VSET",std::bind(&syx27Plugin::c_vset,this,std::placeholders::_1));
 this->addCommand("ISET",std::bind(&syx27Plugin::c_iset,this,std::placeholders::_1));
 this->addCommand("RAMPUP",std::bind(&syx27Plugin::c_rampup,this,std::placeholders::_1));
 this->addCommand("CLEARALARM",std::bind(&syx27Plugin::c_clearalarm,this,std::placeholders::_1));

   
}

extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new syx27Plugin);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}

