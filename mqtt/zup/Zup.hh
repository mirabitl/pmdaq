#pragma once
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include <string.h>

#include <fcntl.h>

#include <errno.h>

#include <termios.h>

#include <unistd.h>
#include <string>
#include <iostream>
//#include "stdafx.hh"
//#include "utils.hh"
//static LoggerPtr _logZup(Logger::getLogger("PMDAQ_ZUP"));
#include "cpprest/json.h"
namespace zup
{
  class Zup
  {
  public:
    Zup(std::string device,uint32_t address);
    ~Zup();
    void ON();
    void OFF();
    void readCommand(std::string cmd);
    void INFO();
    float ReadVoltageSet();
    float ReadVoltageUsed();
    float ReadCurrentUsed();
    web::json::value Status();
  private:


    int fd1,portstatus;
    std::string _value;

    char buff[1024];
    float _vSet,_vRead,_iSet,_iRead;
    int _status;
    int wr,rd,nbytes,tries;
  };
};
