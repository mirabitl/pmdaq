#include "PahoInterface.hh"
#include <sstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <unistd.h>

const auto TIMEOUT = std::chrono::seconds(10);
PahoInterface::PahoInterface(std::string name, std::string process, uint32_t instance) : _cli(NULL), _listening(false),_looping(false),_period(20)
{

  std::stringstream ss;
  ss << name << "/" << process << "/" << instance;
  _id = ss.str();
  std::cout<<"Creating interface "<<_id<<"\n";
}

PahoInterface::~PahoInterface()
{
  this->Disconnect();
}
void PahoInterface::Disconnect()
{
  if (_cli == NULL)
    return;
  if (_cli->is_connected())
  {
    std::stringstream ss;
    ss << _id << "/CMD";
    std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
    _cli->unsubscribe(ss.str())->wait();
    _cli->stop_consuming();
    _cli->disconnect()->wait();
    std::cout << "OK" << std::endl;
  }
}
void PahoInterface::Start()
{
  _listening = true;
  g_listen = std::thread(std::bind(&PahoInterface::Listen, this));
}
void PahoInterface::Stop()
{
  if (_listening)
  {
    _listening = false;
    g_listen.join();
  }
}
void PahoInterface::startLoop(uint32_t p)
{
  _looping = true;
  _period=p;
  g_loop = std::thread(std::bind(&PahoInterface::loop, this));
}
void PahoInterface::stopLoop()
{
  if (_looping)
  {
    _looping = false;
    g_loop.join();
  }
}
void PahoInterface::Connect(std::string host, uint32_t port)
{
  _serverHost = host;
  _serverPort = port;
  std::stringstream ss;
  ss << "tcp://" << host << ":" << port;
  std::cout<<"Connecting to "<<ss.str()<<"\n";
  _cli = std::make_shared<mqtt::async_client>(ss.str(), _id);
  int keepalive = 0;

  auto connOpts = mqtt::connect_options_builder()
                      .keep_alive_interval(std::chrono::seconds(0))
                      .automatic_reconnect(std::chrono::seconds(2), std::chrono::seconds(30))
                      .clean_session(true)
                      .finalize();
  _cli->start_consuming();

  // Connect to the server

  std::cout << "Connecting to the MQTT server..." << std::flush;
  auto tok = _cli->connect(connOpts);

  std::cout << "Waiting for the connection..." << std::endl;
  tok->wait();
  std::cout << "  ...OK" << std::endl;
  // Getting the connect response will block waiting for the
  // connection to complete.
  auto rsp = tok->get_connect_response();

  // If there is no session present, then we need to subscribe, but if
  // there is a session, then the server remembers us and our
  // subscriptions.
  if (!rsp.is_session_present())
  {
    // subscribe to any command
    std::stringstream ss;
    ss << _id << "/CMD";
    _cli->subscribe(ss.str(), 2)->wait();
    // publish Infos
    std::stringstream si;
    si << _id << "/INFOS";
    auto r = web::json::value::object();
    auto rc = web::json::value::array();
    uint32_t nc=0;
    for (auto x:_commands)
    {rc[nc++] = web::json::value::string(U(x.first));}
    r["commands"] = rc;
    std::string sm = r.serialize();
    std::cout << "\nSending message..." << std::endl;
    mqtt::message_ptr pubmsg = mqtt::make_message(mqtt::string_ref(si.str().c_str()), mqtt::binary_ref(sm.c_str()));
    pubmsg->set_qos(0);
    pubmsg->set_retained(true);
    _cli->publish(pubmsg)->wait_for(TIMEOUT);
    std::cout << "  ...OK" << std::endl;
  }
  std::cout << "OK" << std::endl;
}
void PahoInterface::Listen()
{
  while (_listening)
  {
    auto msg = _cli->consume_message();
    if (!msg)
      break;
    std::cout << msg->get_topic() << ": " << msg->to_string() << std::endl;
    this->processMessage(msg);
  }
}
void PahoInterface::loop()
{
  while (_looping)
  {
    auto v= web::json::value::object();
    status(v);
    ::sleep(_period);
  }
}
void PahoInterface::addCommand(std::string s, MqttCmdFunctor f)
{
  _commands.insert(std::pair<std::string, MqttCmdFunctor>(s, f));
}

void PahoInterface::processMessage(mqtt::const_message_ptr msg)
{
  std::stringstream ss;
    ss << _id << "/CMD";
  if (msg->get_topic().compare(ss.str()) != 0)
    return;
  web::json::value ret = web::json::value::parse(msg->to_string());
  std::cout << ret << std::endl;
  if(!ret.has_field("name"))
    return;
  if(!ret.has_field("params"))
    return;

auto icf = _commands.find(ret["name"].as_string());
if (icf != _commands.end())
  {
    icf->second(ret["params"]);
     }


}
void PahoInterface::publish(std::string topic,std::string value)
{
   std::stringstream si;
   si << id() << "/"<<topic;
   
   std::cout << "\nSending message..." << std::endl;
   mqtt::message_ptr pubmsg = mqtt::make_message(mqtt::string_ref(si.str().c_str()), mqtt::binary_ref(value.c_str()));
    pubmsg->set_qos(0);
    _cli->publish(pubmsg)->wait_for(std::chrono::seconds(4));
}
void PahoInterface::publish(std::string topic,float value)
{
   std::stringstream si;
   si << id() << "/"<<topic;
    std::stringstream sv;
    sv << value;
    
   std::cout << "\nSending message..." << std::endl;
   
   mqtt::message_ptr pubmsg = mqtt::make_message(mqtt::string_ref(si.str().c_str()), mqtt::binary_ref(sv.str().c_str()));
    pubmsg->set_qos(0);
    _cli->publish(pubmsg)->wait_for(std::chrono::seconds(4));
}
void PahoInterface::publish(std::string topic,int32_t value)
{
   std::stringstream si;
   si << id() << "/"<<topic;
    std::stringstream sv;
    sv << value;
    
   std::cout << "\nSending message..." << std::endl;
   
   mqtt::message_ptr pubmsg = mqtt::make_message(mqtt::string_ref(si.str().c_str()), mqtt::binary_ref(sv.str().c_str()));
    pubmsg->set_qos(0);
    _cli->publish(pubmsg)->wait_for(std::chrono::seconds(4));
}
