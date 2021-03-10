#pragma once
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitor.hh"
#include "stdafx.hh"

#undef BMP183
#ifdef BMP183
#include "BMP183.hh"
#else
#include "BMP280.hh"
#endif

using namespace std;
#include <sstream>


class bmpPlugin : public monitoring::supervisor
  {
  public:
    bmpPlugin();
    // Transition
    virtual void open();
    virtual void close();
    virtual void registerCommands();
    // Access to the interface
#ifdef BMP183
    lydaq::BMP183* getBmpInterface(){return _bmp;}
#else
    lydaq::BMP280* getBmpInterface(){return _bmp;}
#endif

    // Status
    virtual web::json::value status();
    virtual std::string hardware(){return "BMP";}

    
    // Commande
    void c_status(http_request m);


  private:
#ifdef BMP183 
    lydaq::BMP183* _bmp;
#else
    lydaq::BMP280* _bmp;
#endif
  };

