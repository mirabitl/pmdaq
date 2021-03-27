#pragma once
#include <iomanip>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdint.h>

// hardware access


#include "FtdiUsbDriver.hh"
#include <ftdi.h>
#include <string.h>
#include "stdafx.hh"
#include "utils.hh"

namespace dif
{
  class driver : public mdcc::FtdiUsbDriver
  {
  public:
    driver(char * deviceIdentifier,uint32_t productid=0x6001);
    int32_t NbAsicsWrite(uint32_t tnumber,uint32_t l1,uint32_t l2,uint32_t l3,uint32_t l4 );
    int32_t SetControlRegister(int32_t tvalue);
    int32_t GetControlRegister(uint32_t *tvalue);
    int32_t HardrocGetTimerHoldRegister(uint32_t *thold);
    int32_t HardrocSetNumericalReadoutMode(int32_t tmode);
    int32_t HardrocGetNumericalReadoutMode(int32_t *tmode);
    int32_t HardrocSetNumericalReadoutStartMode(int32_t tmode);
    int32_t HardrocGetNumericalReadoutStartMode(int32_t *tmode);
    int32_t HardrocSetSCOverVoltageDefault(void);
    int32_t HardrocGetSCOverVoltage(int32_t *tmode);
    int32_t HardrocSetTestAllAsicsDefault(void);
    int32_t HardrocGetTestAllAsics(int32_t *tmode);
    int32_t HardrocSetEnablePowerPulsing(int32_t tmode);
    int32_t HardrocGetEnablePowerPulsing(int32_t *tmode);
    int32_t HardrocSetEnableTimeoutDigitalReadout(int32_t tmode);
    int32_t HardrocGetEnableTimeoutDigitalReadout(int32_t *tmode);
    int32_t HardrocGetStatusRegister(uint32_t *tstatus);
    int32_t HardrocGetMemFull(uint32_t *tstatus);
    int32_t HardrocGetRamFullCpt(uint32_t *tstatus);
    int32_t HardrocSetSCDebugRegister(int32_t tvalue);
    int32_t HardrocGetSCDebugRegister(uint32_t *tvalue);
    int32_t SetChipTypeRegister(int32_t tvalue);
    int32_t GetChipTypeRegister(uint32_t *tvalue);
    int32_t SetResetCounter(int32_t tvalue);
    int32_t GetResetCounter(uint32_t *tvalue);
    int32_t SetEventsBetweenTemperatureReadout(uint32_t tdata);
    int32_t GetEventsBetweenTemperatureReadout(uint32_t *tdata);
    int32_t SetAnalogConfigureRegister(uint32_t tdata);
    int32_t  SetPwrToPwrARegister(uint32_t tnumber);
    int32_t  SetPwrAToPwrDRegister(uint32_t tnumber);
    int32_t  SetPwrDToDAQRegister(uint32_t tnumber);
    int32_t  SetDAQToPwrDRegister(uint32_t tnumber);
    int32_t  SetPwrDToPwrARegister(uint32_t tnumber);
    int32_t  HardrocCommandAskDifTemperature(void);
    int32_t  HardrocCommandAskAsuTemperature(void);
    int32_t  GetDIFTemperature(uint32_t *tvalue)    		;
    int32_t  SetTemperatureReadoutToAuto(uint32_t tvalue)    	;
    int32_t  GetTemperatureReadoutAutoStatus(uint32_t *tvalue)    	;
    int32_t  GetASUTemperature(uint32_t *tvalue1,uint32_t *tvalue2)    						;
    int32_t UsbSetDIFID(uint32_t tnumber);
    int32_t GetDIFID(uint32_t *tnumber);
    int32_t HardrocSetGeneratorDivision(uint32_t tnumber);
    int32_t NbAsicsRead(uint32_t *tnumber);
    int32_t HardrocPwonDacDelayRead(uint32_t *tnumber);
    int32_t HardrocPwonDacDelayWrite(uint32_t tnumber);
    int32_t HardrocPwonAEndDelayRead(uint32_t *tnumber);
    int32_t HardrocPwonAEndDelayWrite(uint32_t tnumber);
    int32_t HardrocSLCStatusRead(uint32_t *tstatus);
    int32_t HardrocSLCCRCStatusRead(void);
    int32_t HardrocSLCLoadStatusRead(void);
    int32_t DIFMonitoringEnable(int32_t status);
    int32_t DIFMonitoringSetDIFGain (int32_t gain);
    int32_t DIFMonitoringSetSlabGain(int32_t gain);
    int32_t DIFMonitoringSetSequencer(int32_t status);
    int32_t DIFMonitoringSetAVDDshdn (int32_t status);
    int32_t DIFMonitoringSetDVDDshdn (int32_t status);
    int32_t DIFMonitoringSetConvertedChannels (int32_t channel);
    int32_t DIFMonitoringGetConfigRegister(uint32_t *status);
    int32_t DIFMonitoringGetDIFCurrent(uint32_t *DIFCurrent);
    int32_t DIFMonitoringGetSlabCurrent(uint32_t *SlabCurrent);
    int32_t DIFMonitoringGetChannel4Monitoring(uint32_t *Ch4Value);
    int32_t HardrocCommandSLCWrite(void);
    int32_t HardrocCommandSLCWriteLocal(void);
    int32_t HardrocCommandSLCWriteByte(unsigned char  tbyte);
    int32_t HardrocCommandSLCWriteCRC(unsigned char  *tbyte);
    int32_t HardrocCommandLemoPulse(void);
    int32_t FT245Reset(void);
    int32_t FPGAReset(void);
    int32_t HardrocReset(void);
    int32_t BCIDReset(void);
    int32_t SCReset(void);
    int32_t SCSRReset(void);
    int32_t SRReset(void);
    int32_t SCReportReset(void);
    int32_t DIFCptReset(void);
    int32_t SetPowerAnalog(int32_t tstatus);
    int32_t GetPowerAnalog(int32_t *tstatus);
    int32_t SetPowerADC(int32_t tstatus);
    int32_t GetPowerADC(int32_t *tstatus);
    int32_t SetPowerSS(int32_t tstatus);
    int32_t GetPowerSS(int32_t *tstatus);
    int32_t SetPowerDigital(int32_t tstatus);
    int32_t GetPowerDigital(int32_t *tstatus);
    int32_t SetPowerDAC(int32_t tstatus);
    int32_t GetPowerDAC(int32_t *tstatus);
    int32_t ResetCounter(int32_t tstatus);
    int32_t ClearAnalogSR(int32_t tstatus);
    int32_t SetSCChoice(int32_t tstatus);
    int32_t GetSCChoice(int32_t *tstatus);
    int32_t SetCalibrationMode(int32_t tstatus);
    int32_t GetCalibrationMode(int32_t *tstatus);
    int32_t SetSetupWithCCC(int32_t tstatus);
    int32_t GetSetupWithCCC(int32_t *tstatus);
    int32_t SetSetupWithDCC(int32_t tstatus);
    int32_t GetSetupWithDCC(int32_t *tstatus);
    int32_t SetAcqTest(int32_t tstatus);
    int32_t GetAcqTest(int32_t *tstatus);
    int32_t Set4VforSC(int32_t tstatus);
    int32_t Get4VforSC(int32_t *tstatus);
    int32_t SetMode4VforSC(int32_t tstatus);
    int32_t GetMode4VforSC(int32_t *tstatus);
    int32_t SetModeDCCCCC(int32_t tstatus);
    int32_t GetModeDCCCCC(int32_t *tstatus);
    int32_t SetHold(int32_t tstatus);
    int32_t GetHold(int32_t *tstatus);
    int32_t SetTimeoutDigitalReadout(int32_t tstatus);
    int32_t GetTimeoutDigitalReadout(int32_t *tstatus);
    int32_t SetPowerPulsing(int32_t tstatus);
    int32_t GetPowerPulsing(int32_t *tstatus);
    int32_t SetRealPowerPulsing(int32_t tstatus);
    int32_t GetRealPowerPulsing(int32_t *tstatus);
    int32_t SetDIFCommandsONOFF(int32_t tstatus);
    int32_t GetDIFCommandsONOFF(int32_t *tstatus);
    int32_t SetDROBtMode(int32_t tstatus);
    int32_t GetDROBtMode(int32_t *tstatus);
    int32_t SetClockFrequency(int32_t tstatus);
    int32_t GetClockFrequency(int32_t *tstatus);
    int32_t HardrocSetPowerPulsing(int32_t tstatus);
    int32_t HardrocGetPowerPulsing(int32_t *tstatus);
    int32_t SetSCClockFrequency(int32_t tstatus);
    int32_t GetSCClockFrequency(int32_t *tstatus);
    int32_t HardrocStartDigitalAcquisitionCommand(void);
    int32_t HardrocStopDigitalAcquisitionCommand(void);
    int32_t HardrocStartDigitalReadoutCommand(void);
    int32_t HardrocSendRamfullExtCommand(void);
    int32_t HardrocSendExternalTriggerCommand(void);
    int32_t HardrocSendMezzanine11Command(void);
    int32_t HardrocSetTimerHoldRegister(int32_t thold);
    int32_t HardrocStartAnalogAcq(void);
    int32_t HardrocSoftwareTriggerAnalogAcq(void);
    int32_t DIFMonitoringGetTemperature(uint32_t *Temperature);
    int32_t CommandSLCWriteSingleSLCFrame(unsigned char  *tbyte,uint32_t n);
    int32_t HardrocFlushDigitalFIFO(void);
    int32_t HardrocFastFlushDigitalFIFO(void);
  };
};


