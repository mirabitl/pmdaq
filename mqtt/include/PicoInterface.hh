#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include "mqtt/async_client.h"
#include "mqtt/message.h"

#include "cpprest/json.h"

typedef std::function<void (web::json::value)> MqttCmdFunctor;

class PicoInterface 
{
private:
  std::map<std::string,MqttCmdFunctor> _commands;
  std::string _setup;
  std::string _process;
  std::string _serverHost;
  uint32_t _serverPort;
  std::string _clientid;
  std::string _id,_subid,_hw,_path,_cmdpath;
  std::thread g_listen;
  bool _listening;

  std::thread g_loop;
  uint32_t _period;
  bool _looping;
protected:
  std::shared_ptr<mqtt::async_client> _cli;


public:


  PicoInterface(std::string id,std::string subid,std::string hw);  
 ~PicoInterface();
  void Connect(std::string host, uint32_t port);
  void Disconnect();
  void addCommand(std::string s,MqttCmdFunctor f);
  void Subscribe();
  void Start();
  void Stop();
  void Listen();
  virtual void status(web::json::value v){;}
  void startLoop(uint32_t p);
  void stopLoop();
  void loop();
  void setListening(bool t) {_listening=t;}
  bool isListening(){return _listening;}
  bool isLooping(){return _looping;}
  std::shared_ptr<mqtt::async_client> client(){return _cli;}
  void processMessage(mqtt::const_message_ptr msg);
  inline std::string id() {return _id;}
  inline std::string process() {return _process;}
  void publish(std::string topic,web::json::value v);
  void publish(std::string topic,std::string);
  void publish(std::string topic,float t);
  void publish(std::string topic,int32_t t);
  
  
};
