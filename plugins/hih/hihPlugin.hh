#pragma once
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitor.hh"
#include "stdafx.hh"
#include "hih8000.hh"

using namespace std;
#include <sstream>


class hihPlugin : public monitoring::supervisor
  {
  public:
    hihPlugin();
    // Transition
    virtual void open();
    virtual void close();
    virtual void registerCommands();
    // Access to the interface
    hih8000* getHih8000Interface(){  //std::cout<<" get Ptr "<<_hih8000<<std::endl;
      return _hih;}

    // Status
    virtual web::json::value status();
    virtual std::string hardware(){return "HIH";}

    
    // Commande
    void c_status(http_request m);


  private:
    hih8000* _hih;
  };

