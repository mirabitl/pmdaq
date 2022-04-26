#pragma once
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitor.hh"
#include "stdafx.hh"


#include "HVCaenInterface.hh"
using namespace std;
#include <sstream>

  class syx27Plugin : public monitoring::supervisor
  {
  public:
    syx27Plugin();
    // Transition
    virtual void open();
    virtual void close();
    virtual void registerCommands();
    // Access to the interface
    void lock() {_bsem.lock();}
    void unlock() {_bsem.unlock();}

    caen::HVCaenInterface* getHVCaenInterface(){return _hv;}
    // Status
    virtual web::json::value status();
    web::json::value status(int32_t f,int32_t l);
    virtual std::string hardware(){return "SYX27";}
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
    caen::HVCaenInterface* _hv;
    std::mutex _bsem;

  };


