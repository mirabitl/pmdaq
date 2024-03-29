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
#include <time.h>

#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
//#include "stdafx.hh"
#include "cpprest/json.h"

//static LoggerPtr _logGenesys(Logger::getLogger("PMDAQ_GENESYS"));


using namespace std;
namespace genesys
{
  class GsDevice
  {
  public:
    void setIos();
    GsDevice(std::string device,uint32_t address);
    ~GsDevice();
    void ON();
    void OFF();
    void readCommand(std::string cmd);
    void INFO();
  
    float ReadVoltageSet();
    float ReadVoltageUsed();
    float ReadCurrentUsed();
    std::string readValue();
    web::json::value Status();
  private:


    int fd1,portstatus;
    
    unsigned char buff[1024];
    std::string _value;
    int wr,rd,nbytes,tries,_status;
    float _vSet,_vRead,_iSet,_iRead;
    time_t _lastInfo;
  };
};
