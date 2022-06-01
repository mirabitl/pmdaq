#include "ZupPaho.hh"
#include <unistd.h>


ZupPaho::ZupPaho(std::string name, std::string process, uint32_t instance,std::string device,uint32_t port) : PahoInterface(name,process,instance),_lv(0),_port(port),_device(device)
{
  
   if (_lv!=NULL)
    delete _lv;
  
  
   _lv= new zup::Zup(_device,_port);

  unlock();
  registerCommands();
}
ZupPaho::~ZupPaho()
{
   delete _lv;
  _lv=NULL;
    this->Stop();
    this->Disconnect();
}
void ZupPaho::registerCommands()
{
    this->addCommand("STATUS",std::bind(&ZupPaho::status,this,std::placeholders::_1));
    this->addCommand("BEGIN",std::bind(&ZupPaho::begin,this,std::placeholders::_1));
    this->addCommand("END",std::bind(&ZupPaho::end,this,std::placeholders::_1));
    this->addCommand("DESTROY",std::bind(&ZupPaho::destroy,this,std::placeholders::_1));
    this->addCommand("ON",std::bind(&ZupPaho::on,this,std::placeholders::_1));
    this->addCommand("OFF",std::bind(&ZupPaho::off,this,std::placeholders::_1));
}
void ZupPaho::status(web::json::value v)
{

  //PMF_INFO(_logZup,"Querying Status");

  auto par = web::json::value::object();

 if (_lv==NULL)
  {
    //PMF_ERROR(_logZup,"No ZupSnmp opened");
    publish("STATUS","Invalid Device");
    return;
  }
 publish("STATUS",_lv->Status().serialize());

}


void ZupPaho::on(web::json::value v)
{

 

  _lv->ON();
  ::sleep(1);
  this->status(v);
 }
void ZupPaho::off(web::json::value v)
{

  _lv->OFF();
  ::sleep(1);
  this->status(v);
}


void ZupPaho::begin(web::json::value v)
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
void ZupPaho::end(web::json::value v)
{
  if (isLooping())
    this->stopLoop();
}
void ZupPaho::destroy(web::json::value v)
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
  if (output.as_object().find("zup")!=output.as_object().end())
    {
      auto wdef=output["zup"];
      if (wdef.as_object().find("port")!=wdef.as_object().end())
	zport=wdef["port"].as_integer();

      if (wdef.as_object().find("device")!=wdef.as_object().end())
	zdevice.assign(wdef["device"].as_string());
      else
	{
	  std::cout<<"No address  for zup \n";
	  exit(0);
	}
	
    }
  else
    {
      std::cout<<"No description for zup \n";
      exit(0);
    }
  ZupPaho m(session,"ZupPaho",0,zdevice,zport);
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
