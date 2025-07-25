#include "Sy1527Pico.hh"
#include <unistd.h>
#include <time.h>

Sy1527Pico::Sy1527Pico(std::string id, std::string subid,std::string account,std::vector<uint32_t> validc) : PicoInterface(id, subid, "sy1527"), _hv(0),_account(account)
{
  _valid=validc;
   if (_hv!=NULL)
    delete _hv;
   _hv=NULL;
  int ipass = _account.find("/");
  int ipath = _account.find("@");
  std::string Name,Pwd,Host;
  Name.clear();
  Name=_account.substr(0,ipass); 
  Pwd.clear();
  Pwd=_account.substr(ipass+1,ipath-ipass-1); 
  Host.clear();
  Host=_account.substr(ipath+1,_account.size()-ipath); 
  std::cout<<Name<<"|"<<std::endl;
  std::cout<<Pwd<<"|"<<std::endl;
  std::cout<<Host<<"|"<<std::endl;


  _host=Host;
  _name=Name;
  _pwd=Pwd;
  unlock();
  registerCommands();
  _last_connect=time(0);

}
void Sy1527Pico::opensocket()
{
  /** A voir
  time_t tnow=time(0);
  if (_hv!=NULL && (tnow-_last_connect)>30)
    {
      delete _hv;
      _hv=NULL;
      
    }
  **/
  if (_hv==NULL)
    {
      _hv= new caen::HVCaenInterface(_host,_name,_pwd);
      _last_connect=time(0);
      //::sleep(1);
    }
}
void Sy1527Pico::closesocket()
{
  time_t tnow=time(0);
  if (_hv!=NULL && (tnow-_last_connect)>15)
    {
      //_hv->Disconnect();
      
      delete _hv;
      _hv=NULL;
    }
  
}
Sy1527Pico::~Sy1527Pico()
{
   delete _hv;
  _hv=NULL;
    this->Stop();
    this->Disconnect();
}
void Sy1527Pico::registerCommands()
{
    this->addCommand("STATUS",std::bind(&Sy1527Pico::status,this,std::placeholders::_1));
    this->addCommand("RESET",std::bind(&Sy1527Pico::reset,this,std::placeholders::_1));
    this->addCommand("ON",std::bind(&Sy1527Pico::on,this,std::placeholders::_1));
    this->addCommand("OFF",std::bind(&Sy1527Pico::off,this,std::placeholders::_1));
    this->addCommand("VSET",std::bind(&Sy1527Pico::vset,this,std::placeholders::_1));
    this->addCommand("ISET",std::bind(&Sy1527Pico::iset,this,std::placeholders::_1));
    this->addCommand("RAMPUP",std::bind(&Sy1527Pico::rampup,this,std::placeholders::_1));
    this->addCommand("CLEARALARM",std::bind(&Sy1527Pico::clearalarm,this,std::placeholders::_1));
}
web::json::value Sy1527Pico::channelStatus(uint32_t channel)
{
  
  auto r=_hv->ChannelInfo(channel/6,channel%6);
  r["id"]=channel;
  std::cout<<channel<<" gives "<<r<<std::endl;
  return r;
  r= web::json::value::object();
  r["status"]=web::json::value::string(U("notset"));
   if (_hv==NULL)
    {
      //PMF_ERROR(_logSy1527,"No Sy1527Snmp opened");
       return r;
    }
   //PMF_INFO(_logSy1527,"Getting value for channel"<<channel);
   std::cout<<channel<<" gives "<<_hv->GetVoltageSet(channel)<<std::endl;
   r["vset"]=web::json::value::number(_hv->GetVoltageSet(channel));
   r["iset"]=web::json::value::number(_hv->GetCurrentSet(channel));
   r["rampup"]=web::json::value::number(_hv->GetVoltageRampUp(channel));
   r["iout"]=web::json::value::number(_hv->GetCurrentRead(channel));
   r["vout"]=web::json::value::number(_hv->GetVoltageRead(channel));
   r["status"]=web::json::value::number(_hv->GetStatus(channel));
   r["chname"]=web::json::value::string(U(_hv->GetName(channel)));
   r["id"]=web::json::value::number(channel);
   std::cout<<r<<std::endl;
   return r;
}
web::json::value Sy1527Pico::rangeStatus(int32_t first,int32_t last)
{

  auto r= web::json::value::object();
  r["name"]=web::json::value::string(U(this->process()));

  web::json::value c_array;uint32_t nc=0;

  if (_hv==NULL)
  {
    return r;
  }
  
  for (uint32_t i=first;i<=last;i++)
    {
      //PMF_INFO(_logSy1527,"Calling ChannelStatus "<<i);
      c_array[nc++]=this->channelStatus(i);
      //PMF_INFO(_logSy1527,"Status from "<<nc-1<<" "<<c_array[nc-1]);
      //std::cout <<v<<std::endl;
    }
  //PMF_INFO(_logSy1527,"End ");
  r["channels"]=c_array;
  return r;
}

web::json::value Sy1527Pico::validStatus()
{

  auto r= web::json::value::object();
  r["name"]=web::json::value::string(U(this->process()));

  web::json::value c_array;uint32_t nc=0;

  if (_hv==NULL)
  {
    return r;
  }
  
  for (uint32_t i=0;i<_valid.size();i++)
    {
      //PMF_INFO(_logSy1527,"Calling ChannelStatus "<<i);
      c_array[nc++]=this->channelStatus(_valid[i]);
      //PMF_INFO(_logSy1527,"Status from "<<nc-1<<" "<<c_array[nc-1]);
      //std::cout <<v<<std::endl;
    }
  //PMF_INFO(_logSy1527,"End ");
  r["channels"]=c_array;
  return r;
}
void Sy1527Pico::status(web::json::value v)
{

  //PMF_INFO(_logSy1527,"Querying Status");

  auto par = web::json::value::object();
  this->opensocket();
 if (_hv==NULL)
  {
    //PMF_ERROR(_logSy1527,"No Sy1527Snmp opened");
    publish("STATUS","Invalid Device");
    return;
  }
 //
 uint32_t first = 47;
 uint32_t last =0;
 if (v.as_object().find("first")!=v.as_object().end())
   first=v["first"].as_integer();
 if (v.as_object().find("last")!=v.as_object().end())
   last=v["last"].as_integer();
 if (last>=first)
   {
     std::cout<<"querying status "<<first<<" to "<< last<<std::endl;
     publish("",this->rangeStatus(first,last).serialize());
   }
 else
   publish("",this->validStatus().serialize());
 
}

void Sy1527Pico::ch_list(web::json::value v)
{
  _ch_list.clear();
   uint32_t first = 47;
 uint32_t last =0;
 if (v.as_object().find("first")!=v.as_object().end())
   first=v["first"].as_integer();
 if (v.as_object().find("last")!=v.as_object().end())
   last=v["last"].as_integer();
 if (last>=first)
   {
    
     for (int i=first;i<=last;i++) _ch_list.push_back(i);
     return;
   }
 else
   for (int i=0;i<_valid.size();i++) _ch_list.push_back(_valid[i]);
 return;
}

void Sy1527Pico::on(web::json::value v)
{

   this->opensocket();
   ch_list(v);
   for (uint32_t i=0;i<_ch_list.size();i++)
     _hv->SetOn(_ch_list[i]);
   ::sleep(2);

  this->status(v);

 }
void Sy1527Pico::off(web::json::value v)
{
  this->opensocket();
  //
  ch_list(v);
  for (uint32_t i=0;i<_ch_list.size();i++)
    _hv->SetOff(_ch_list[i]);
  ::sleep(2);

  this->status(v);

}
void Sy1527Pico::clearalarm(web::json::value v)
{
  this->opensocket();
  ::sleep(2);

  this->status(v);

}
void Sy1527Pico::vset(web::json::value v)
{
  this->opensocket();
  if (v.as_object().find("vset")==v.as_object().end())
    return;
  ch_list(v);
  for (uint32_t i=0;i<_ch_list.size();i++)
    _hv->SetVoltage(_ch_list[i],v["vset"].as_double());
  ::sleep(2);
  
  this->status(v);

}

void Sy1527Pico::iset(web::json::value v)
{
  this->opensocket();
  if (v.as_object().find("iset")==v.as_object().end())
    return;
  ch_list(v);
  for (uint32_t i=0;i<_ch_list.size();i++)
    _hv->SetCurrent(_ch_list[i],v["iset"].as_double());
  ::sleep(2);
  
  this->status(v);


}

void Sy1527Pico::rampup(web::json::value v)
{
  this->opensocket();
  if (v.as_object().find("rampup")==v.as_object().end())
    return;
  ch_list(v);
  for (uint32_t i=0;i<_ch_list.size();i++)
    _hv->SetVoltageRampUp(_ch_list[i],v["rampup"].as_double());
  ::sleep(2);

  this->status(v);


  

  }



void Sy1527Pico::reset(web::json::value v)
{
  this->stopLoop();
  this->Stop();
  exit(0);
}

int main()
{
  // Parsing /etc/paho.json
  web::json::value output; // JSON read from input file

  try
  {
    // Open the file stream
    std::ifstream f("/etc/pico_caen.json");
    // String stream for holding the JSON file
    std::stringstream strStream;

    // Stream file stream into string stream
    strStream << f.rdbuf();
    f.close(); // Close the filestream

    // Parse the string stream into a JSON object
    output = web::json::value::parse(strStream);
  }
  catch (web::json::json_exception excep)
  {
    output = web::json::value::null();
    std::cout << "Needing /etc/pico_caen.json file\n";
    exit(0);
  }
  std::cout << output;
  //
  auto mqtt = output["mqtt"].as_object();
  auto host = mqtt["server"].as_string();
  uint32_t port = 1883;
  auto id = output["id"].as_string();
  auto subid = output["subid"].as_string();
  std::cout << "ID " << id<<"/"<<subid<<"/sy1527 " << " broker " << host << " : " << port << "\n";

  uint32_t first = 47;
  uint32_t last = 0;
  uint32_t period=30;
  std::string ipaddress("");
  std::vector<uint32_t> valid_channels;
  if (output.as_object().find("sy1527") != output.as_object().end())
  {
    auto wdef = output["sy1527"];
    if (wdef.as_object().find("first") != wdef.as_object().end())
      first = wdef["first"].as_integer();
    if (wdef.as_object().find("last") != wdef.as_object().end())
      last = wdef["last"].as_integer();
     if (wdef.as_object().find("period") != wdef.as_object().end())
      period = wdef["period"].as_integer();
    if (wdef.as_object().find("account") != wdef.as_object().end())
      ipaddress.assign(wdef["account"].as_string());
    else
    {
      std::cout << "No account  for sy1527 \n";
      exit(0);
    }

    if (last>=first)
      {
	for (int i=first;i<=last;i++)
	  {valid_channels.push_back(i);
	    std::cout<<valid_channels[i-first]<<std::endl;}
 
      }
    else
      if (wdef.as_object().find("list") != wdef.as_object().end())
      {
	auto chlist=wdef["list"].as_array();
	for (int i=0;i<chlist.size();i++)
	  {valid_channels.push_back(chlist[i].as_integer());
	    std::cout<<valid_channels[i]<<std::endl;}

      }
    
  }
  else
  {
    std::cout << "No description for sy1527 \n";
    exit(0);
  }
  Sy1527Pico m(id,subid, ipaddress,valid_channels);
  std::cout << "Now connecting \n"
            << std::flush;
  m.Connect(host, port);
  m.Start();
  m.startLoop(period);
  while (m.isListening())
    ::sleep(5);
  m.stopLoop();
  m.Stop();
  exit(0);
}
