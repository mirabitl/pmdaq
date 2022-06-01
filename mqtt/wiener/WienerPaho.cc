#include "WienerPaho.hh"
#include <unistd.h>


WienerPaho::WienerPaho(std::string name, std::string process, uint32_t instance,std::string ipadr,uint32_t first,uint32_t last) : PahoInterface(name,process,instance),_hv(0),_first(first),_last(last),_ipaddress(ipadr)
{
  
   if (_hv!=NULL)
    delete _hv;
  
  
  _hv= new wiener::WienerDevice(_ipaddress);

  unlock();
  registerCommands();
}
WienerPaho::~WienerPaho()
{
   delete _hv;
  _hv=NULL;
    this->Stop();
    this->Disconnect();
}
void WienerPaho::registerCommands()
{
    this->addCommand("STATUS",std::bind(&WienerPaho::status,this,std::placeholders::_1));
    this->addCommand("BEGIN",std::bind(&WienerPaho::begin,this,std::placeholders::_1));
    this->addCommand("END",std::bind(&WienerPaho::end,this,std::placeholders::_1));
    this->addCommand("DESTROY",std::bind(&WienerPaho::destroy,this,std::placeholders::_1));
    this->addCommand("ON",std::bind(&WienerPaho::on,this,std::placeholders::_1));
 this->addCommand("OFF",std::bind(&WienerPaho::off,this,std::placeholders::_1));
 this->addCommand("VSET",std::bind(&WienerPaho::vset,this,std::placeholders::_1));
 this->addCommand("ISET",std::bind(&WienerPaho::iset,this,std::placeholders::_1));
 this->addCommand("RAMPUP",std::bind(&WienerPaho::rampup,this,std::placeholders::_1));
 this->addCommand("CLEARALARM",std::bind(&WienerPaho::clearalarm,this,std::placeholders::_1));
}
web::json::value WienerPaho::channelStatus(uint32_t channel)
{
  auto r= web::json::value::object();
  r["id"]=channel;
  r["status"]=web::json::value::string(U("notset"));
   if (_hv==NULL)
    {
      //PMF_ERROR(_logWiener,"No WienerSnmp opened");
       return r;
    }
   //PMF_INFO(_logWiener,"Getting value for channel"<<channel);
   std::cout<<channel<<" gives "<<_hv->getOutputVoltage(channel/8,channel%8)<<std::endl;
   r["vset"]=web::json::value::number(_hv->getOutputVoltage(channel/8,channel%8));
   r["iset"]=web::json::value::number(_hv->getOutputCurrentLimit(channel/8,channel%8));
   r["rampup"]=web::json::value::number(_hv->getOutputVoltageRiseRate(channel/8,channel%8));
   r["iout"]=web::json::value::number(_hv->getOutputMeasurementCurrent(channel/8,channel%8));
   r["vout"]=web::json::value::number(_hv->getOutputMeasurementSenseVoltage(channel/8,channel%8));
   r["status"]=web::json::value::string(U(_hv->getOutputStatus(channel/8,channel%8)));
   
   return r;
}
web::json::value WienerPaho::rangeStatus(int32_t first,int32_t last)
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
      //PMF_INFO(_logWiener,"Calling ChannelStatus "<<i);
      c_array[nc++]=this->channelStatus(i);
      //PMF_INFO(_logWiener,"Status from "<<nc-1<<" "<<c_array[nc-1]);
      //std::cout <<v<<std::endl;
    }
  //PMF_INFO(_logWiener,"End ");
  r["channels"]=c_array;
  return r;
}

void WienerPaho::status(web::json::value v)
{

  //PMF_INFO(_logWiener,"Querying Status");

  auto par = web::json::value::object();

 if (_hv==NULL)
  {
    //PMF_ERROR(_logWiener,"No WienerSnmp opened");
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


void WienerPaho::on(web::json::value v)
{

 
 uint32_t first = _first;
 uint32_t last =_last;
 if (v.as_object().find("first")!=v.as_object().end())
   first=v["first"].as_integer();
 if (v.as_object().find("last")!=v.as_object().end())
   last=v["last"].as_integer();
  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputSwitch(i/8,i%8,1);
  ::sleep(2);
  this->status(v);
 }
void WienerPaho::off(web::json::value v)
{

  uint32_t first = _first;
 uint32_t last =_last;
 if (v.as_object().find("first")!=v.as_object().end())
   first=v["first"].as_integer();
 if (v.as_object().find("last")!=v.as_object().end())
   last=v["last"].as_integer();
  // 
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputSwitch(i/8,i%8,0);
  ::sleep(2);
  this->status(v);
}
void WienerPaho::clearalarm(web::json::value v)
{

   uint32_t first = _first;
   uint32_t last =_last;
   if (v.as_object().find("first")!=v.as_object().end())
     first=v["first"].as_integer();
   if (v.as_object().find("last")!=v.as_object().end())
     last=v["last"].as_integer();
   // 
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputSwitch(i/8,i%8,10);
  ::sleep(2);
  this->status(v);

}
void WienerPaho::vset(web::json::value v)
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
    _hv->setOutputVoltage(i/8,i%8,v["vset"].as_double());
  ::sleep(2);
  this->status(v);
}

void WienerPaho::iset(web::json::value v)
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
    _hv->setOutputCurrentLimit(i/8,i%8,v["iset"].as_double());
  ::sleep(2);
  this->status(v);


}

void WienerPaho::rampup(web::json::value v)
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
    _hv->setOutputVoltageRiseRate(i/8,i%8,-1.*v["rampup"].as_double());
  ::sleep(2);
  this->status(v);


  

  }




void WienerPaho::begin(web::json::value v)
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
void WienerPaho::end(web::json::value v)
{
  if (isLooping())
    this->stopLoop();
}
void WienerPaho::destroy(web::json::value v)
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
  std::string ipaddress("");
  if (output.as_object().find("wiener")!=output.as_object().end())
    {
      auto wdef=output["wiener"];
      if (wdef.as_object().find("first")!=wdef.as_object().end())
	first=wdef["first"].as_integer();
      if (wdef.as_object().find("last")!=wdef.as_object().end())
	last=wdef["last"].as_integer();
      if (wdef.as_object().find("address")!=wdef.as_object().end())
	ipaddress.assign(wdef["address"].as_string());
      else
	{
	  std::cout<<"No address  for wiener \n";
	  exit(0);
	}
	
    }
  else
    {
      std::cout<<"No description for wiener \n";
      exit(0);
    }
  WienerPaho m(session,"WienerPaho",0,ipaddress,first,last);
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
