#pragma once
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitor.hh"
#include "stdafx.hh"

#include "bmp280.hh"


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
    bmp280* getBmpInterface(){return _bmp;}


    // Status
    virtual web::json::value status();
    virtual std::string hardware(){return "BMP";}

    
    // Commande
    void c_status(http_request m);


  private:
    bmp280* _bmp;

  };

