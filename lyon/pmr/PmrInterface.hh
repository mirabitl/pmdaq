#pragma once


#include <iostream>

#include <string.h>
#include<stdio.h>
//#include "zmPusher.hh"
#include "pmSender.hh"
using namespace std;
#include <sstream>
#include <map>
#include <vector>


#include "PmrDriver.hh"

///< Local definition of struct
namespace Pmr {
  
typedef struct 
{
  uint32_t vendorid;
  uint32_t productid;
  char name[12];
  uint32_t id;
  uint32_t type;
} FtdiDeviceInfo;

typedef struct
{
  uint32_t id;
  uint32_t status;
  uint32_t slc;
  uint32_t gtc;
  uint32_t published;
  uint64_t bcid;
  uint64_t bytes;
  char host[80];
} DIFStatus;

};

namespace Pmr {
class PmrInterface
{
public:
  PmrInterface(Pmr::FtdiDeviceInfo *ftd);
  ~PmrInterface();
  void setTransport(pm::pmSender* p);
  // initialise
  void initialise(pm::pmSender* p=NULL);
  // configure
  void setRunning(bool t);
  void configure(unsigned char* b, uint32_t nb);
  // Start Stop
  void start();
  void readout();
  void stop();
  // destroy
  void destroy();
  // Getter and setters
  inline Pmr::DIFStatus* status() const {return _status;}
  inline Pmr::PmrDriver* rd() const {return _rd;}
  void setState(std::string s){_state.assign(s);}
  inline std::string state() const {return _state;}
  inline uint32_t* data()  {return (uint32_t*) _dsData->buffer()->ptr();}
  
  // run control
  inline void setReadoutStarted(bool t){_readoutStarted=t;}
  inline bool readoutStarted() const { return _readoutStarted;}
  inline bool running() const { return _running;}
  inline uint32_t detectorId() const {return _detid;}
  inline void publishState(std::string s){setState(s);}

  // Exteranl trigger used
  void setExternalTrigger(bool t);
private:
  Pmr::FtdiDeviceInfo _ftd;
  Pmr::DIFStatus* _status;
  std::string _state;
  uint8_t _cbuf[262144];
  Pmr::PmrDriver* _rd;
  pm::pmSender* _dsData;
  uint32_t _detid;
  bool _running,_readoutStarted,_readoutCompleted;
  bool _external;
    std::mutex _sem;


};
};

