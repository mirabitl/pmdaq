#pragma once

#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitor.hh"
#include "stdafx.hh"


using namespace std;
#include <sstream>


class grStore : public monitoring::monStore
  {
  public:
    grStore();
    virtual void connect();
    virtual void store(std::string loc,std::string hw,uint32_t ti,web::json::value status);
    virtual  void loadParameters(web::json::value params);
    // Access to the interface
  private:
    //zdaq::fsm* _fsm;
    json::value _params;
    std::string _graphite_host,_graphite_port;

  };


