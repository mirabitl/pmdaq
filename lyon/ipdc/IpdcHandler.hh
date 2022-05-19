#pragma once

#include "FtdiUsbDriver.hh"

#include "stdafx.hh"
#include "utils.hh"
static LoggerPtr _logIpdc(Logger::getLogger("PMDAQ_IPDC"));

namespace ipdc
{
  class IpdcHandler 
  {
  public:
    IpdcHandler (std::string name,uint32_t productid=0x6001);
    ~IpdcHandler();
    void open();
    void close();
    void writeRegister(uint32_t addr,uint32_t data);
    uint32_t readRegister(uint32_t addr);
    void maskTrigger();
    void unmaskTrigger();
    void maskEcal();
    void unmaskEcal();
    void resetCounter();
    void resetTDC(uint8_t b);
    uint32_t mask();
    uint32_t ecalmask();
    uint32_t spillCount();
    uint32_t busyCount(uint8_t b);
    uint32_t spillOn();
    uint32_t spillOff();
    uint32_t busyEnable();
    void setSpillOn(uint32_t nc);
    void setSpillOff(uint32_t nc);
    void setBusyEnable(uint32_t nc);
    uint32_t beam();
    void setBeam(uint32_t nc);
    uint32_t hardReset();
    void setHardReset(uint32_t nc);
    uint32_t version();
    uint32_t id();

    void calibOn();
    void calibOff();
    void reloadCalibCount();
    uint32_t calibCount();
    void setCalibCount(uint32_t nc);

    void setCalibRegister(uint32_t nc);
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
  
  private : 
    mdcc::FtdiUsbDriver* _driver;
    std::string _name;
    uint32_t _productid,_version,_id;

  };
};

