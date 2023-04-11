#include "PicoInterface.hh"
#include <sstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
//Topic:pico_test/RUNNING| Message : {"location": "pico_test", "subsystem": "Bureau", "devices": ["hih", "rp2040", "bme"]}
const auto TIMEOUT = std::chrono::seconds(10);
const int QOS = 1;

// How many to buffer while off-line
const int MAX_BUFFERED_MESSAGES = 1200;

PicoInterface::PicoInterface(std::string id, std::string subid, std::string hardware) : _id(id), _subid(id), _hw(hardware), _cli(NULL), _listening(false), _looping(false), _period(20)
{

  std::stringstream ss;
  ss << id << "/" << subid << "/" << hardware;
  _path = ss.str();
  std::stringstream ss1;
  ss1 << id << "/" << subid << "/CMD";
  _cmdpath = ss1.str();
  std::cout << "Creating interface " << _path << "\n";

   std::stringstream slist;
    slist<<_id<<"/LIST";
    _listpath=slist.str();
    std::stringstream sreset;
    sreset<<_id<<"/RESET";
    _resetpath=sreset.str();

  // Client id
  std::stringstream ssc;
  ssc << "pico_client_" << rand() % 32768;
  _clientid = ssc.str();
}

PicoInterface::~PicoInterface()
{
  this->Disconnect();
}
void PicoInterface::Disconnect()
{
  if (_cli == NULL)
    return;
  if (_cli->is_connected())
  {

    std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
    _cli->unsubscribe(_cmdpath)->wait();
    _cli->stop_consuming();
    _cli->disconnect()->wait();
    std::cout << "OK" << std::endl;
  }
}
void PicoInterface::Start()
{
  _listening = true;
  g_listen = std::thread(std::bind(&PicoInterface::Listen, this));
}
void PicoInterface::Stop()
{
  if (_listening)
  {
    _listening = false;
    g_listen.join();
  }
}
void PicoInterface::startLoop(uint32_t p)
{
  _looping = true;
  _period = p;
  g_loop = std::thread(std::bind(&PicoInterface::loop, this));
}
void PicoInterface::stopLoop()
{
  if (_looping)
  {
    _looping = false;
    g_loop.join();
  }
}
void PicoInterface::Connect(std::string host, uint32_t port)
{
  
  _serverHost = host;
  _serverPort = port;
  std::stringstream ss;
  ss << "tcp://" << host << ":" << port;
  std::cout << "Connecting to " << ss.str() << "\n with id " << _id << std::endl;
  auto createOpts = mqtt::create_options_builder()
                        .send_while_disconnected(true, true)
                        .max_buffered_messages(MAX_BUFFERED_MESSAGES)
                        .delete_oldest_messages()
                        .finalize();

  _cli = std::make_shared<mqtt::async_client>(ss.str(), "", createOpts);
  mqtt::async_client *pc = _cli.get();
  _cli->set_connected_handler([pc](const std::string &)
                              { std::cout << "*** Connected ("
                                          << time(0) << ") ***" << std::endl; });

  _cli->set_connection_lost_handler([pc](const std::string &)
                                    { std::cout << "*** Connection Lost ("
                                                << time(0) << ") ***" << std::endl; });

  int keepalive = 0;

  // auto connOpts = mqtt::connect_options_builder()
  //   .keep_alive_interval(std::chrono::seconds(0))
  //   .automatic_reconnect(std::chrono::seconds(2), std::chrono::seconds(30))
  //   .clean_session(true)
  //   .finalize();

  auto willMsg = mqtt::message("test/events", "Time publisher disconnected", 1, true);
  auto connOpts = mqtt::connect_options_builder()
                      .clean_session()
                      .will(willMsg)
                      .keep_alive_interval(std::chrono::seconds(120))
                      .automatic_reconnect(std::chrono::seconds(1), std::chrono::seconds(10))
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

    _cli->subscribe(_cmdpath, 2)->wait();   
    _cli->subscribe(_listpath, 2)->wait();
    _cli->subscribe(_resetpath, 2)->wait();
    // publish Infos
    std::stringstream si;
    si << _path << "/INFOS";
    auto r = web::json::value::object();
    auto rc = web::json::value::array();
    uint32_t nc = 0;
    for (auto x : _commands)
    {
      rc[nc++] = web::json::value::string(U(x.first));
    }
    r["commands"] = rc;
    std::string sm = r.serialize();
    std::cout << "\nSending message..." << std::endl;
    mqtt::message_ptr pubmsg = mqtt::make_message(mqtt::string_ref(si.str().c_str()), mqtt::binary_ref(sm.c_str()));
    pubmsg->set_qos(1);
    pubmsg->set_retained(true);
    _cli->publish(pubmsg)->wait_for(TIMEOUT);
    std::cout << "  ...OK" << std::endl;
  }
  std::cout << "OK" << std::endl;
}
void PicoInterface::Listen()
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
void PicoInterface::loop()
{
  while (_looping)
  {
    auto v = web::json::value::object();
    status(v);
    for (int i = 0; i < _period; i++)
    {
      if (!_looping)
        return;
      ::sleep(1);
    }
  }
}
void PicoInterface::addCommand(std::string s, MqttCmdFunctor f)
{
  _commands.insert(std::pair<std::string, MqttCmdFunctor>(s, f));
}

void PicoInterface::processMessage(mqtt::const_message_ptr msg)
{
  if (msg->get_topic().compare(_resetpath) == 0)
    {
      this->stopLoop();
      this->Stop();
      exit(0);
    }
  if (msg->get_topic().compare(_listpath) == 0)
    {
      web::json::value jdev;
      auto actives = web::json::value::object();
      actives["location"]= web::json::value::string(U(_id));
      actives["subsystem"]= web::json::value::string(U(_subid));
      jdev[0]= web::json::value::string(U(_hw));
      actives["devices"]=jdev;
      std::stringstream s_top;
      s_top<<_id<<"/RUNNING";
      this->publish(s_top.str(),actives);

      return;
    }

  
  if (msg->get_topic().compare(_cmdpath) != 0)
    return;
  web::json::value ret = web::json::value::parse(msg->to_string());
  std::cout << ret << std::endl;
  if (!ret.has_field("device"))
    return;
  if (ret["device"].as_string().compare(_hw)!=0)
     return;
  if (!ret.has_field("params"))
    return;
  if (!ret.has_field("command"))
    return;
  auto icf = _commands.find(ret["command"].as_string());
  if (icf != _commands.end())
  {
    icf->second(ret["params"]);
  }
}
void PicoInterface::publish(std::string topic, web::json::value value)
{
  std::stringstream si;
  if (topic.size() > 0)
    si << _path << "/" << topic;
  else
    si << _path;
  std::cout << "\nSending message..." << std::endl;
  mqtt::message_ptr pubmsg = mqtt::make_message(mqtt::string_ref(si.str().c_str()), mqtt::binary_ref(value.serialize().c_str()));
  pubmsg->set_qos(0);
  _cli->publish(pubmsg)->wait_for(std::chrono::seconds(4));
}
void PicoInterface::publish(std::string topic, std::string value)
{
  std::stringstream si;
  if (topic.size() > 0)
    si << _path << "/" << topic;
  else
    si << _path;
  std::cout << "\nSending message..." << std::endl;
  mqtt::message_ptr pubmsg = mqtt::make_message(mqtt::string_ref(si.str().c_str()), mqtt::binary_ref(value.c_str()));
  pubmsg->set_qos(0);
  _cli->publish(pubmsg)->wait_for(std::chrono::seconds(4));
}
void PicoInterface::publish(std::string topic, float value)
{
  std::stringstream si;
  if (topic.size() > 0)
    si << _path << "/" << topic;
  else
    si << _path;
  std::stringstream sv;
  sv << value;
  std::cout << "\nSending message..." << std::endl;

  mqtt::message_ptr pubmsg = mqtt::make_message(mqtt::string_ref(si.str().c_str()), mqtt::binary_ref(sv.str().c_str()));
  pubmsg->set_qos(0);
  _cli->publish(pubmsg)->wait_for(std::chrono::seconds(4));
}
void PicoInterface::publish(std::string topic, int32_t value)
{
  std::stringstream si;
  if (topic.size() > 0)
    si << _path << "/" << topic;
  else
    si << _path;
  std::stringstream sv;
  sv << value;

  std::cout << "\nSending message..." << std::endl;

  mqtt::message_ptr pubmsg = mqtt::make_message(mqtt::string_ref(si.str().c_str()), mqtt::binary_ref(sv.str().c_str()));
  pubmsg->set_qos(0);
  _cli->publish(pubmsg)->wait_for(std::chrono::seconds(4));
}
