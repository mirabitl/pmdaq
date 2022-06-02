#include "Sy1527Paho.hh"
#include <unistd.h>


Sy1527Paho::Sy1527Paho(std::string name, std::string process, uint32_t instance,std::string account,uint32_t first,uint32_t last) : PahoInterface(name,process,instance),_hv(0),_first(first),_last(last),_account(account)
{
  
   if (_hv!=NULL)
    delete _hv;
  
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

  _hv= new caen::HVCaenInterface(Host,Name,Pwd);

  unlock();
  registerCommands();
}
Sy1527Paho::~Sy1527Paho()
{
   delete _hv;
  _hv=NULL;
    this->Stop();
    this->Disconnect();
}
void Sy1527Paho::registerCommands()
{
    this->addCommand("STATUS",std::bind(&Sy1527Paho::status,this,std::placeholders::_1));
    this->addCommand("BEGIN",std::bind(&Sy1527Paho::begin,this,std::placeholders::_1));
    this->addCommand("END",std::bind(&Sy1527Paho::end,this,std::placeholders::_1));
    this->addCommand("DESTROY",std::bind(&Sy1527Paho::destroy,this,std::placeholders::_1));
    this->addCommand("ON",std::bind(&Sy1527Paho::on,this,std::placeholders::_1));
    this->addCommand("OFF",std::bind(&Sy1527Paho::off,this,std::placeholders::_1));
    this->addCommand("VSET",std::bind(&Sy1527Paho::vset,this,std::placeholders::_1));
    this->addCommand("ISET",std::bind(&Sy1527Paho::iset,this,std::placeholders::_1));
    this->addCommand("RAMPUP",std::bind(&Sy1527Paho::rampup,this,std::placeholders::_1));
    this->addCommand("CLEARALARM",std::bind(&Sy1527Paho::clearalarm,this,std::placeholders::_1));
}
web::json::value Sy1527Paho::channelStatus(uint32_t channel)
{
  auto r= web::json::value::object();
  r["id"]=channel;
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

   return r;
}
web::json::value Sy1527Paho::rangeStatus(int32_t first,int32_t last)
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

void Sy1527Paho::status(web::json::value v)
{

  //PMF_INFO(_logSy1527,"Querying Status");

  auto par = web::json::value::object();

 if (_hv==NULL)
  {
    //PMF_ERROR(_logSy1527,"No Sy1527Snmp opened");
    publish("STATUS","Invalid Device");
    return;
  }
 //
 uint32_t first = _first;
 uint32_t last =_last;
 if (v.as_object().find("first")!=v.as_object().end())
   first=v["first"].as_integer();
 if (v.as_object().find("last")!=v.as_object().end())
   last=v["last"].as_integer();
 std::cout<<"querying status "<<first<<" to "<< last<<std::endl;
 publish("STATUS",this->rangeStatus(first,last).serialize());

}


void Sy1527Paho::on(web::json::value v)
{

 
 uint32_t first = _first;
 uint32_t last =_last;
 if (v.as_object().find("first")!=v.as_object().end())
   first=v["first"].as_integer();
 if (v.as_object().find("last")!=v.as_object().end())
   last=v["last"].as_integer();
  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->SetOn(i);
  ::sleep(2);
  this->status(v);
 }
void Sy1527Paho::off(web::json::value v)
{

  uint32_t first = _first;
 uint32_t last =_last;
 if (v.as_object().find("first")!=v.as_object().end())
   first=v["first"].as_integer();
 if (v.as_object().find("last")!=v.as_object().end())
   last=v["last"].as_integer();
  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->SetOff(i);
  ::sleep(2);
  this->status(v);
}
void Sy1527Paho::clearalarm(web::json::value v)
{

   uint32_t first = _first;
   uint32_t last =_last;
   if (v.as_object().find("first")!=v.as_object().end())
     first=v["first"].as_integer();
   if (v.as_object().find("last")!=v.as_object().end())
     last=v["last"].as_integer();
   // 
  // for (uint32_t i=first;i<=last;i++)
  //   _hv->setOutputSwitch(i/8,i%8,10);
  ::sleep(2);
  this->status(v);

}
void Sy1527Paho::vset(web::json::value v)
{
  if (v.as_object().find("vset")==v.as_object().end())
    return;
  uint32_t first = _first;
  uint32_t last =_last;
  
  if (v.as_object().find("first")!=v.as_object().end())
    first=v["first"].as_integer();
  if (v.as_object().find("last")!=v.as_object().end())
    last=v["last"].as_integer();
  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->SetVoltage(i,v["vset"].as_double());
  ::sleep(2);
  this->status(v);
}

void Sy1527Paho::iset(web::json::value v)
{

 if (v.as_object().find("iset")==v.as_object().end())
    return;
  uint32_t first = _first;
  uint32_t last =_last;
  
  if (v.as_object().find("first")!=v.as_object().end())
    first=v["first"].as_integer();
  if (v.as_object().find("last")!=v.as_object().end())
    last=v["last"].as_integer();
  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->SetCurrent(i,v["iset"].as_double());
  ::sleep(2);
  this->status(v);


}

void Sy1527Paho::rampup(web::json::value v)
{
if (v.as_object().find("rampup")==v.as_object().end())
    return;
  uint32_t first = _first;
  uint32_t last =_last;
  
  if (v.as_object().find("first")!=v.as_object().end())
    first=v["first"].as_integer();
  if (v.as_object().find("last")!=v.as_object().end())
    last=v["last"].as_integer();
  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->SetVoltageRampUp(i,v["rampup"].as_double());
  ::sleep(2);
  this->status(v);


  

  }




void Sy1527Paho::begin(web::json::value v)
{
  if (!isLooping())
    {
      uint32_t p=10;
      if (v.as_object().find("period")!=v.as_object().end())
	{
	  p=v["period"].as_integer();
	}
      else
	{
	  std::cout<<" period is set to 10 s\n";
	}
      std::cout<<" starting loop with period "<< p<<std::endl;
    this->startLoop(p);
    }
}
void Sy1527Paho::end(web::json::value v)
{
  if (isLooping())
    this->stopLoop();
}
void Sy1527Paho::destroy(web::json::value v)
{
  this->setListening(false);

}


int main()
{
  // Parsing /etc/paho.json
 web::json::value output;  // JSON read from input file
  
  try
    {
      // Open the file stream
      std::ifstream f("/etc/paho.json");
      // String stream for holding the JSON file
      std::stringstream strStream;
      
      // Stream file stream into string stream
      strStream << f.rdbuf();
      f.close();  // Close the filestream
      
      // Parse the string stream into a JSON object
      output = web::json::value::parse(strStream);
    }
  catch (web::json::json_exception excep)
    {
      output=web::json::value::null();
      std::cout<<"Needing /etc/paho.json file\n";
      exit(0);
	
    }
  std::cout<<output;
  //
  auto session=output["session"].as_string();
  auto host=output["broker"].as_string();
  auto port =output["port"].as_integer();
  std::cout<<"session " <<session<<" broker "<<host<<" : "<<port<<"\n";

  uint32_t first=0;
  uint32_t last=47;
  std::string account("");
  if (output.as_object().find("sy1527")!=output.as_object().end())
    {
      auto wdef=output["sy1527"];
      if (wdef.as_object().find("first")!=wdef.as_object().end())
	first=wdef["first"].as_integer();
      if (wdef.as_object().find("last")!=wdef.as_object().end())
	last=wdef["last"].as_integer();
      if (wdef.as_object().find("account")!=wdef.as_object().end())
	account.assign(wdef["account"].as_string());
      else
	{
	  std::cout<<"No address  for sy1527 \n";
	  exit(0);
	}
	
    }
  else
    {
      std::cout<<"No description for sy1527 \n";
      exit(0);
    }
  Sy1527Paho m(session,"Sy1527Paho",0,account,first,last);
  std::cout<<"Now connecting \n"<<std::flush;
  m.Connect(host,port);
  m.Start();
  //m.startLoop(10);
  while (m.isListening())
    ::sleep(5);
  m.stopLoop();
  m.Stop();
  exit(0);
}
