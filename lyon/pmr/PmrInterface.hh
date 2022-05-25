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
namespace pmr {
  
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
  uint64_t bcid;
  uint64_t bytes;
  char host[80];
} DIFStatus;

};

namespace pmr {
class PmrInterface
{
public:
  PmrInterface(pmr::FtdiDeviceInfo *ftd);
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
  inline pmr::DIFStatus* status() const {return _status;}
  inline pmr::PmrDriver* rd() const {return _rd;}
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
  pmr::FtdiDeviceInfo _ftd;
  pmr::DIFStatus* _status;
  std::string _state;
  uint32_t _data[32768];
  pmr::PmrDriver* _rd;
  pm::pmSender* _dsData;
  uint32_t _detid;
  bool _running,_readoutStarted,_readoutCompleted;
  bool _external;
  std::mutex _sem;


};
};

