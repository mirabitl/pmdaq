#ifndef _MBMDCC_MANAGER_HH
#define _MBMDCC_MANAGER_HH
#include "MBMDCCInterface.hh"
#include "baseApplication.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "ReadoutLogger.hh"

namespace lydaq
{

class MbmdccManager : public zdaq::baseApplication
{
public:
  MbmdccManager(std::string name);
  ~MbmdccManager(){;}

  void initialise(zdaq::fsmmessage* m);
  void configure(zdaq::fsmmessage* m);
  void destroy(zdaq::fsmmessage* m);

  uint32_t version();
  uint32_t id();
  uint32_t mask();
  void maskTrigger();
  void unmaskTrigger();
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
  void useSPSSpill(bool t);
  void useTrigExt(bool t);
  void setTriggerDelay(uint32_t nc);
  uint32_t triggerDelay();
  void setTriggerBusy(uint32_t nc);
  uint32_t triggerBusy();
  void setExternalTrigger(uint32_t nc);
  uint32_t externalTrigger();
  void reloadCalibCount();
  void resetTDC(uint8_t b);
  uint32_t busyCount(uint8_t b);
  uint32_t readRegister(uint32_t adr);
  void writeRegister(uint32_t adr,uint32_t val);
  void c_readreg(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_writereg(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_pause(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_resume(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_calibon(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_caliboff(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_reloadcalib(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setcalibcount(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_spillon(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_spilloff(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_resettdc(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_channelon(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_sethardreset(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setspillregister(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setexternaltrigger(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setregister(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_getregister(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setcalibregister(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_settrigext(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);




private:
  lydaq::mbmdcc::Interface* _mpi;



  zdaq::fsmweb* _fsm;




  
};
};
#endif
