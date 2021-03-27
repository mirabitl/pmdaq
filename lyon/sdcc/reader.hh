#pragma once
//#include "UsbCCCDriver.h"
#include "driver.hh"
#define USBDRIVER sdcc::driver

namespace sdcc
{
  class reader 
  {
  public:
    reader (std::string name);
    virtual int open() ;//throw( LocalHardwareException ) ;
    virtual int close() ;//throw( LocalHardwareException );

  private : 
    USBDRIVER* _driver;
    std::string _name;
  public : 
    virtual void DoSendDIFReset();
    virtual void DoSendBCIDReset();
    virtual void DoSendStartAcquisitionAuto();
    virtual void DoSendRamfullExt();
    virtual void DoSendTrigExt();
    virtual void DoSendStopAcquisition();
    virtual void DoSendDigitalReadout();
    virtual void DoSendClearMemory();
    virtual void DoSendStartSingleAcquisition();
    virtual void DoSendPulseLemo();
    virtual void DoSendRazChannel();
    virtual void DoSendTrigger();
    virtual void DoSendCCCReset();
    virtual void DoSendSpillOn();
    virtual void DoSendSpillOff();
    virtual void DoWriteRegister(uint32_t addr,uint32_t data);
    virtual uint32_t DoReadRegister(uint32_t addr);
    virtual void DoSendPauseTrigger();
    virtual void DoSendResumeTrigger();
  };
};

