#ifndef _DIFInterface_h

#define _DIFInterface_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "zmSender.hh"
#include "DIFReadout.hh"
#include "DIFReadoutConstant.hh"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "ReadoutLogger.hh"



#pragma once

#include "FtdiUsbDriver.hh"
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "pmSender.hh"

#include "stdafx.hh"
#include "utils.hh"
static LoggerPtr _logDif(Logger::getLogger("PMDAQ_DIF"));
typedef struct 
{
  uint32_t vendorid;
  uint32_t productid;
  char name[12];
  uint32_t id;
  uint32_t type;
} FtdiDeviceInfo;
namespace dif {
class DIFInterface
{
public:
  DIFInterface(FtdiDeviceInfo *ftd);
  ~DIFInterface();
  void setTransport(pm::pmSender* p);
  // initialise
  void initialise(pm::pmSender* p=NULL);
  // configure
  //void difConfigure(uint32_t ctrl);
  void difConfigure(uint32_t ctrlreg,uint32_t p2pa=0x4e,uint32_t pa2pd=0x3e6,uint32_t pd2daq=0x4e,uint32_t daq2dr=0x4e,uint32_t d2ar=0x4e);


  
  void chipConfigure();
  void configure(uint32_t ctrl,uint32_t l1=0x4e,uint32_t l2=0x3e6,uint32_t l3=0x4e,uint32_t l4=0x4e,uint32_t l5=0x4e);
  //void configure(uint32_t ctrl,uint32_t l1=0,uint32_t l2=0,uint32_t l3=0,uint32_t l4=0,uint32_t l5=0);
  // Start Stop
  void start();
  void readout();
  void stop();
  // destroy
  void destroy();
  // register access
  void writeRegister(uint32_t adr,uint32_t reg);
  void readRegister(uint32_t adr,uint32_t &reg);
  // Getter and setters
  inline DIFStatus* status() const {return _status;}
  inline lydaq::DIFReadout* rd() const {return _rd;}
  inline DIFDbInfo* dbdif() const {return _dbdif;}
  void setState(std::string s){_state.assign(s);}
  inline std::string state() const {return _state;}
  inline uint32_t* data()  {return (uint32_t*) _dsData->buffer()->ptr();}
  
  // run control
  inline void setReadoutStarted(bool t){_readoutStarted=t;}
  inline bool readoutStarted() const { return _readoutStarted;}
  inline bool running() const { return _running;}
  inline uint32_t detectorId() const {return _detid;}
  inline void publishState(std::string s){setState(s);}

  static uint32_t getBufferDIF(unsigned char* cb,uint32_t idx=0);
  static uint32_t getBufferDTC(unsigned char* cb,uint32_t idx=0);
  static uint32_t getBufferGTC(unsigned char* cb,uint32_t idx=0);
  static unsigned long long getBufferABCID(unsigned char* cb,uint32_t idx=0);
private:
  FtdiDeviceInfo _ftd;
  DIFStatus* _status;
  lydaq::DIFReadout* _rd;
  std::string _state;
  DIFDbInfo* _dbdif;
  uint32_t _data[32768];

  pm::pmSender* _dsData;
  uint32_t _detid;
  bool _running,_readoutStarted,_readoutCompleted;
};
};
#endif
