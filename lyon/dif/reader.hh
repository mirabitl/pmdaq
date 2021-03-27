#pragma once
#include "driver.hh"
#include "DIFReadoutConstant.hh"

namespace dif
{
  class reader : public dif::driver
  {
  public:

    reader (std::string name,uint32_t productid=0x6001);
    ~reader ();
    void setPowerManagment(uint32_t P2PAReg, uint32_t PA2PDReg,uint32_t PD2DAQReg, uint32_t DAQ2DReg,uint32_t D2AReg);
    void setAsicType(uint32_t asicType);
    void setNumberOfAsics(uint32_t NumberOfAsics);
    void setControlRegister(uint32_t ControlRegister);
    void initialise(uint32_t difid,uint32_t asicType,uint32_t NumberOfAsics,uint32_t ctrlreg,uint32_t P2PAReg, uint32_t PA2PDReg,uint32_t PD2DAQReg, uint32_t DAQ2DReg,uint32_t D2AReg);
    void start();
    void stop();
    void DoRefreshNbOfASICs();
    int32_t DoReadSLCStatus();
    void configureRegisters();
    void DoSendDIFTemperatureCommand();
    void DoSendASUTemperatureCommand();
    void DoGetASUTemperatureCommand(uint32_t *ttemp1,uint32_t *ttemp2);
    void DoGetDIFTemperatureCommand(uint32_t *ttemp);
    void DoSetTemperatureReadoutToAuto(uint32_t tvalue);
    void DoGetTemperatureReadoutAutoStatus(uint32_t *tvalue);
    void DoSetEventsBetweenTemperatureReadout(uint32_t tvalue);
    void DoSetAnalogConfigureRegister(uint32_t tdata);
    int32_t configureChips(SingleHardrocV2ConfigurationFrame* slow) ;
    uint32_t DoHardrocV2ReadoutDigitalData(unsigned char* CurrentDIFDigitalData);
  protected:
    std::string theName_;
    uint32_t theDIFId_,theAsicType_,theNumberOfAsics_,theControlRegister_,theCurrentSLCStatus_,thePwrToPwrARegister_,thePwrAToPwrDRegister_,thePwrDToDAQRegister_,theDAQToPwrDRegister_,thePwrDToPwrARegister_;
    float theTemperatureBuffer_[2];
  };
};


