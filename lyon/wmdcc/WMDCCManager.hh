#pragma once
#include "WMDCCInterface.hh"
#include "fsmw.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "stdafx.hh"
#include "utils.hh"

class WmdccManager : public fsmw
{
public:
  WmdccManager();
  ~WmdccManager(){;}
  virtual void initialise();
  virtual void end();
  
  void fsm_initialise(http_request m);
  void configure(http_request m);
  void destroy(http_request m);

  uint32_t version();
  uint32_t id();
  uint32_t mask();
  void maskTrigger();
  void unmaskTrigger();
  void resyncOn();
  void resyncOff();
  uint32_t spillCount();
  void resetCounter();
  uint32_t spillOn();
  uint32_t spillOff();
  void setSpillOn(uint32_t nc);
  void setSpillOff(uint32_t nc);
  uint32_t Channels();
  void setChannels(uint32_t nc);
  void calibOn();
  void calibOff();
  uint32_t calibCount();
  void setCalibCount(uint32_t nc);
  void setCalibRegister(uint32_t nc);
  uint32_t hardReset();
  void setHardReset(uint32_t nc);
  void setSpillRegister(uint32_t nc);
  uint32_t spillRegister();
  void useSPSSpill(uint32_t nc);
  void useTrigExt(bool t);
  void setTriggerDelay(uint32_t nc);
  uint32_t triggerDelay();
  void setTriggerBusy(uint32_t nc);
  uint32_t triggerBusy();
  void setExternalTrigger(uint32_t nc);
  uint32_t externalTrigger();
  void reloadCalibCount();
  void resetTDC(uint32_t b);
  void resetFSM(uint32_t b);
  uint32_t busyCount(uint32_t b);
  uint32_t readRegister(uint32_t adr);
  void writeRegister(uint32_t adr,uint32_t val);
  void c_readreg(http_request m);
  void c_writereg(http_request m);
  void c_pause(http_request m);
  void c_resume(http_request m);
  void c_calibon(http_request m);
  void c_caliboff(http_request m);
  void c_reloadcalib(http_request m);
  void c_setcalibcount(http_request m);
  void c_reset(http_request m);
  void c_spillon(http_request m);
  void c_spilloff(http_request m);
  void c_resettdc(http_request m);
  void c_resetfsm(http_request m);
  void c_channelon(http_request m);
  void c_sethardreset(http_request m);
  void c_setspillregister(http_request m);
  void c_setexternaltrigger(http_request m);
  void c_setregister(http_request m);
  void c_getregister(http_request m);
  void c_setcalibregister(http_request m);
  void c_settrigext(http_request m);
  void c_status(http_request m);
  void c_setspsspill(http_request m);
  void c_resync(http_request m);



private:
  wizcc::Controller* _mpi;
  
};
