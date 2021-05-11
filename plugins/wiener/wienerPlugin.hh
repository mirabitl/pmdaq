#pragma once
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitor.hh"
#include "stdafx.hh"

#include "WienerSnmp.hh"

using namespace std;
#include <sstream>

  class wienerPlugin : public monitoring::supervisor
  {
  public:
    wienerPlugin();
    // Transition
    virtual void open();
    virtual void close();
    virtual void registerCommands();
    // Access to the interface
    wiener::WienerDevice* getHVWienerInterface(){  //std::cout<<" get Ptr "<<_hv<<std::endl;
      return _hv;}
    // Status
    virtual web::json::value status();
    web::json::value status(int32_t f,int32_t l);
    virtual std::string hardware(){return "ISEG";}
    web::json::value channelStatus(uint32_t channel);
    
    // Commande
    void c_status(http_request m);
    void c_vset(http_request m);
    void c_iset(http_request m);
    void c_rampup(http_request m);
    void c_on(http_request m);
    void c_off(http_request m);
    void c_clearalarm(http_request m);

  private:
    //zdaq::fsm* _fsm;

    wiener::WienerDevice* _hv;
  };


