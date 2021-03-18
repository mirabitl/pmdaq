#pragma once
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitor.hh"
#include "stdafx.hh"
#include "Genesys.hh"

using namespace std;
#include <sstream>


class genesysPlugin : public monitoring::supervisor
  {
  public:
    genesysPlugin();
    // Transition
    virtual void open();
    virtual void close();
    virtual void registerCommands();
    // Access to the interface
    genesys::Genesys* getGenesysInterface(){  //std::cout<<" get Ptr "<<_hv<<std::endl;
      return _lv;}
    // Status
    virtual web::json::value status();
    virtual std::string hardware(){return "GENESYS";}

    
    // Commande
    void c_on(http_request m);
    void c_off(http_request m);
    void c_setdevice(http_request m);
    void c_status(http_request m);


  private:
    genesys::Genesys* _lv;
  };

