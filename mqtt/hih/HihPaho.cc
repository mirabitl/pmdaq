#include "HihPaho.hh"
#include <unistd.h>

HihPaho::HihPaho(std::string name, std::string process, uint32_t instance) : PahoInterface(name, process, instance), _Hih(0)
{

  if (_Hih != NULL)
    delete _Hih;

  _Hih = new hihdriver();

  unlock();
  registerCommands();
}
HihPaho::~HihPaho()
{
  delete _Hih;
  _Hih = NULL;
  this->Stop();
  this->Disconnect();
}
void HihPaho::registerCommands()
{
  this->addCommand("STATUS", std::bind(&HihPaho::status, this, std::placeholders::_1));
  this->addCommand("BEGIN", std::bind(&HihPaho::begin, this, std::placeholders::_1));
  this->addCommand("END", std::bind(&HihPaho::end, this, std::placeholders::_1));
  this->addCommand("DESTROY", std::bind(&HihPaho::destroy, this, std::placeholders::_1));
}
void HihPaho::begin(web::json::value v)
{
  if (!isLooping())
  {
    uint32_t p = 10;
    if (v.as_object().find("period") != v.as_object().end())
    {
      p = v["period"].as_integer();
    }
    else
    {
      std::cout << " period is set to 10 s\n";
    }
    this->startLoop(p);
  }
}
void HihPaho::end(web::json::value v)
{
  if (isLooping())
    this->stopLoop();
}
void HihPaho::destroy(web::json::value v)
{
  this->setListening(false);
}
void HihPaho::status(web::json::value v)
{

  std::cout << v << std::endl;

  auto r = web::json::value::object();
  lock();
  _Hih->Read();

  float h0 = _Hih->humidity(0), t0 = _Hih->temperature(0), h1 = _Hih->humidity(1), t1 = _Hih->temperature(1);
  ::usleep(100000);
  unlock();
  r["t0"] = web::json::value::number(t0);
  r["h0"] = web::json::value::number(h0);
  r["t1"] = web::json::value::number(t1);
  r["h1"] = web::json::value::number(h1);
  std::string sm = r.serialize();
  publish("STATUS", sm);
  publish("H0", h0);
  publish("T0", t0);
  publish("H0", h1);
  publish("T0", t1);
  std::cout << "\nSend done message..." << time(0) << std::endl;
}

int main()
{
  // Parsing /etc/paho.json
  web::json::value output; // JSON read from input file

  try
  {
    // Open the file stream
    std::ifstream f("/etc/paho.json");
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
    std::cout << "Needing /etc/paho.json file\n";
    exit(0);
  }
  std::cout << output;
  //
  auto session = output["session"].as_string();
  auto host = output["broker"].as_string();
  auto port = output["port"].as_integer();
  std::cout << "session " << session << " broker " << host << " : " << port << "\n";
  HihPaho m(session, "HihPaho", 0);
  std::cout << "Now connecting \n"
            << std::flush;
  m.Connect(host, port);
  m.Start();
  // m.startLoop(10);
  while (m.isListening())
    ::sleep(5);
  m.stopLoop();
  m.Stop();
  exit(0);
}
