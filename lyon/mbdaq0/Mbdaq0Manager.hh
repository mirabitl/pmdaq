#pragma once
#include "Mbdaq0Interface.hh"
#include "fsmw.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>

class Mbdaq0Manager : public fsmw
{
public:
  Mbdaq0Manager();
  ~Mbdaq0Manager(){;}
  virtual void initialise();
  virtual void end();


    
  void fsm_initialise(http_request m);
  void configure(http_request m);
  void destroy(http_request m);

  uint32_t version();
  uint32_t id();
  uint32_t Channels();
  void setChannels(uint32_t nc);
  void resetTDC(uint8_t b);
  void resetFSM(uint8_t b);
  uint32_t readRegister(uint32_t adr);
  void writeRegister(uint32_t adr,uint32_t val);
  void c_readreg(http_request m);
  void c_writereg(http_request m);
  void c_resetfsm(http_request m);
  void c_resettdc(http_request m);
  void c_channelon(http_request m);
  void c_status(http_request m);
  
private:

  mbdaq0::Interface* _mpi;
  
};
