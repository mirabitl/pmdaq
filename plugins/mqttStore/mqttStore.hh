#pragma once

#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitor.hh"
#include "stdafx.hh"
#include <mosquitto.h>

using namespace std;
#include <sstream>


class mqttStore : public monitoring::monStore
  {
  public:
    mqttStore();
    virtual void connect();
    virtual void store(std::string loc,std::string hw,uint32_t ti,web::json::value status);
    virtual  void loadParameters(web::json::value params);
    // Access to the interface
    int  mqtt_send(const char* topic,float val,uint32_t ti);
    void mqtt_setup();
  private:
    //zdaq::fsm* _fsm;
    json::value _params;
    std::string _host,_port;
    struct mosquitto *_mosq;
  };


