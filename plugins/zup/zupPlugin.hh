#pragma once
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitor.hh"
#include "stdafx.hh"
#include "Zup.hh"

using namespace std;
#include <sstream>


class zupPlugin : public monitoring::supervisor
  {
  public:
    zupPlugin();
    // Transition
    virtual void open();
    virtual void close();
    virtual void registerCommands();
    // Access to the interface
    lydaq::Zup* getZupInterface(){  //std::cout<<" get Ptr "<<_hv<<std::endl;
      return _lv;}
    // Status
    virtual web::json::value status();
    virtual std::string hardware(){return "ZUP";}

    
    // Commande
    void c_on(http_request m);
    void c_off(http_request m);
    void c_setdevice(http_request m);
    void c_status(http_request m);


  private:
    lydaq::Zup* _lv;
  };

