#include "GenesysPaho.hh"
#include <unistd.h>


GenesysPaho::GenesysPaho(std::string name, std::string process, uint32_t instance,std::string device,uint32_t port) : PahoInterface(name,process,instance),_lv(0),_port(port),_device(device)
{
  
   if (_lv!=NULL)
    delete _lv;
  
  
   _lv= new genesys::GsDevice(_device,_port);

  unlock();
  registerCommands();
}
GenesysPaho::~GenesysPaho()
{
   delete _lv;
  _lv=NULL;
    this->Stop();
    this->Disconnect();
}
void GenesysPaho::registerCommands()
{
    this->addCommand("STATUS",std::bind(&GenesysPaho::status,this,std::placeholders::_1));
    this->addCommand("BEGIN",std::bind(&GenesysPaho::begin,this,std::placeholders::_1));
    this->addCommand("END",std::bind(&GenesysPaho::end,this,std::placeholders::_1));
    this->addCommand("DESTROY",std::bind(&GenesysPaho::destroy,this,std::placeholders::_1));
    this->addCommand("ON",std::bind(&GenesysPaho::on,this,std::placeholders::_1));
    this->addCommand("OFF",std::bind(&GenesysPaho::off,this,std::placeholders::_1));
}
void GenesysPaho::status(web::json::value v)
{

  //PMF_INFO(_logGenesys,"Querying Status");

  auto par = web::json::value::object();

 if (_lv==NULL)
  {
    //PMF_ERROR(_logGenesys,"No GenesysSnmp opened");
    publish("STATUS","Invalid Device");
    return;
  }
 publish("STATUS",_lv->Status().serialize());

}


void GenesysPaho::on(web::json::value v)
{

 

  _lv->ON();
  ::sleep(1);
  this->status(v);
 }
void GenesysPaho::off(web::json::value v)
{

  _lv->OFF();
  ::sleep(1);
  this->status(v);
}


void GenesysPaho::begin(web::json::value v)
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
    this->startLoop(p);
    }
}
void GenesysPaho::end(web::json::value v)
{
  if (isLooping())
    this->stopLoop();
}
void GenesysPaho::destroy(web::json::value v)
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

  uint32_t zport=1;
  std::string zdevice("/dev/ttyUSB0");
  if (output.as_object().find("genesys")!=output.as_object().end())
    {
      auto wdef=output["genesys"];
      if (wdef.as_object().find("port")!=wdef.as_object().end())
	zport=wdef["port"].as_integer();

      if (wdef.as_object().find("device")!=wdef.as_object().end())
	zdevice.assign(wdef["device"].as_string());
      else
	{
	  std::cout<<"No address  for genesys \n";
	  exit(0);
	}
	
    }
  else
    {
      std::cout<<"No description for genesys \n";
      exit(0);
    }
  GenesysPaho m(session,"GenesysPaho",0,zdevice,zport);
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
