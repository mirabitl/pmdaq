#include "BmpPaho.hh"
#include <unistd.h>


BmpPaho::BmpPaho(std::string name, std::string process, uint32_t instance) : PahoInterface(name,process,instance),_bmp(0)
{
  
   if (_bmp!=NULL)
    delete _bmp;
  
  
  _bmp= new bmp280();

  unlock();
  registerCommands();
}
BmpPaho::~BmpPaho()
{
   delete _bmp;
  _bmp=NULL;
    this->Stop();
    this->Disconnect();
}
void BmpPaho::registerCommands()
{
    this->addCommand("STATUS",std::bind(&BmpPaho::status,this,std::placeholders::_1));
    this->addCommand("BEGIN",std::bind(&BmpPaho::begin,this,std::placeholders::_1));
    this->addCommand("END",std::bind(&BmpPaho::end,this,std::placeholders::_1));
    this->addCommand("DESTROY",std::bind(&BmpPaho::destroy,this,std::placeholders::_1));
}
void BmpPaho::begin(web::json::value v)
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
void BmpPaho::end(web::json::value v)
{
  if (isLooping())
    this->stopLoop();
}
void BmpPaho::destroy(web::json::value v)
{
  this->setListening(false);

}
void BmpPaho::status(web::json::value v)
{
 
 
  
    std::cout<<v<<std::endl;
   
    auto r = web::json::value::object();
    lock();
    float t,p;
    _bmp->TemperaturePressionRead(&t,&p);
    ::usleep(100000);
    unlock();
    r["pressure"]=web::json::value::number(p);
    r["temperature"]=web::json::value::number(t);
    std::string sm = r.serialize();
    publish("STATUS",sm);
    publish("P",p);
    publish("T",t);
   
    std::cout << "\nSend done message..." <<time(0)<< std::endl;
	

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
  BmpPaho m(session,"BmpPaho",0);
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
