#pragma once
#include <iostream>

#include <string.h>
#include <stdio.h>


#include "interface.hh"
#include "fsmw.hh"

using namespace sdcc;
#include <sstream>
class SdccManager : public fsmw
  {
  public:
    SdccManager();
    virtual void initialise();
    virtual void end();
    void open(http_request m);
    void fsm_initialise(http_request m);
    void configure(http_request m);
    void start(http_request m);
    void stop(http_request m);
    void cmd(http_request m);
    void Open(std::string s);

    // getters

    sdcc::interface* getInterface(){return _manager;}
    // Commands
    void pause(http_request m);
    void resume(http_request m);
    void difreset(http_request m);
    void cccreset(http_request m);
    void readreg(http_request m);
    void writereg(http_request m);


  private:
    std::string _state;
    //zdaq::fsm* _fsm;

    sdcc::interface* _manager;
  };

