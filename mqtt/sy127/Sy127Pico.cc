#include "Sy127Pico.hh"
#include <unistd.h>
#include <time.h>

Sy127Pico::Sy127Pico(std::string id, std::string subid,std::string device,uint32_t mode) : PicoInterface(id, subid, "sy127"), _hv(0),_device(device),_mode(mode)
{
   if (_hv!=NULL)
    delete _hv;
   _hv=NULL;
  unlock();
  registerCommands();
  _last_connect=time(0);
   if (_hv==NULL)
    {
      _hv= new Sy127Access(_device,9600,_mode);
      _last_connect=time(0);
      //::sleep(1);
    }
}
Sy127Pico::~Sy127Pico()
{
   delete _hv;
  _hv=NULL;
    this->Stop();
    this->Disconnect();
}
void Sy127Pico::registerCommands()
{
    this->addCommand("STATUS",std::bind(&Sy127Pico::status,this,std::placeholders::_1));
    this->addCommand("RESET",std::bind(&Sy127Pico::reset,this,std::placeholders::_1));
    this->addCommand("ON",std::bind(&Sy127Pico::on,this,std::placeholders::_1));
    this->addCommand("OFF",std::bind(&Sy127Pico::off,this,std::placeholders::_1));
    this->addCommand("VSET",std::bind(&Sy127Pico::vset,this,std::placeholders::_1));
    this->addCommand("ISET",std::bind(&Sy127Pico::iset,this,std::placeholders::_1));
    this->addCommand("RAMPUP",std::bind(&Sy127Pico::rampup,this,std::placeholders::_1));
    this->addCommand("RAMPDOWN",std::bind(&Sy127Pico::rampdown,this,std::placeholders::_1));
    this->addCommand("CLEARALARM",std::bind(&Sy127Pico::clearalarm,this,std::placeholders::_1));
}
void Sy127Pico::status(web::json::value v)
{

  //PMF_INFO(_logSy127,"Querying Status");

  auto par = web::json::value::object();

 if (_hv==NULL)
  {
    //PMF_ERROR(_logSy127,"No Sy127Snmp opened");
    publish("STATUS","Invalid Device");
    return;
  }
 lock();
 _hv->status();
 unlock();
 //
   publish("",_hv->json_status());
 
}


void Sy127Pico::on(web::json::value v)
{
  if (_hv==NULL)  return;
 if (v.as_object().find("channel")==v.as_object().end())
   return;
 auto s_ch=v["channel"].as_string();
 std::cout<<_hv->status(s_ch)<<"|||"<<std::endl;
  if (_hv->status(s_ch).compare("ON")==0) return;
  lock();
  _hv->toggle(s_ch);
  unlock();
 }
void Sy127Pico::off(web::json::value v)
{
  if (_hv==NULL)  return;
 if (v.as_object().find("channel")==v.as_object().end())
   return;
 auto s_ch=v["channel"].as_string();
 std::cout<<_hv->status(s_ch)<<"|||"<<std::endl;
  if (_hv->status(s_ch).compare("OFF")==0) return;
    lock();
  _hv->toggle(s_ch);
  unlock();

}
void Sy127Pico::clearalarm(web::json::value v)
{
  return;
}
void Sy127Pico::vset(web::json::value v)
{
  if (_hv==NULL)  return;
  if (v.as_object().find("value")==v.as_object().end())
    return;
  if (v.as_object().find("channel")==v.as_object().end())
    return;
  lock();
  _hv->setV0(v["channel"].as_string(),v["value"].as_double());
  unlock();
}

void Sy127Pico::iset(web::json::value v)
{
  if (_hv==NULL)  return;
  if (v.as_object().find("value")==v.as_object().end())
    return;
  if (v.as_object().find("channel")==v.as_object().end())
    return;
    lock();
  _hv->setI0(v["channel"].as_string(),v["value"].as_double());
  unlock();
}

void Sy127Pico::rampup(web::json::value v)
{
  if (_hv==NULL)  return;
  if (v.as_object().find("value")==v.as_object().end())
    return;
  if (v.as_object().find("channel")==v.as_object().end())
    return;
    lock();
  _hv->setRup(v["channel"].as_string(),v["value"].as_double());
    unlock();
  }

void Sy127Pico::rampdown(web::json::value v)
{
  if (_hv==NULL)  return;
  if (v.as_object().find("value")==v.as_object().end())
    return;
  if (v.as_object().find("channel")==v.as_object().end())
    return;
    lock();
  _hv->setRdw(v["channel"].as_string(),v["value"].as_double());
    unlock();
}



void Sy127Pico::reset(web::json::value v)
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
  std::cout << "ID " << id<<"/"<<subid<<"/sy127 " << " broker " << host << " : " << port << "\n";


  uint32_t mode = 0;
  uint32_t period=30;
  std::string device("");
  std::vector<uint32_t> valid_channels;
  if (output.as_object().find("sy127") != output.as_object().end())
  {
    auto wdef = output["sy127"];
    if (wdef.as_object().find("mode") != wdef.as_object().end())
      mode = wdef["mode"].as_integer();
     if (wdef.as_object().find("period") != wdef.as_object().end())
      period = wdef["period"].as_integer();
    if (wdef.as_object().find("device") != wdef.as_object().end())
      device.assign(wdef["device"].as_string());
    else
    {
      std::cout << "No account  for sy127 \n";
      exit(0);
    }

  }
  else
  {
    std::cout << "No description for sy127 \n";
    exit(0);
  }
  Sy127Pico m(id,subid,device,mode);
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
