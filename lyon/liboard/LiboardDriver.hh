#pragma once

#include <iomanip>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdint.h>

// hardware access
#include <ftdi.h>
#include <string.h>
#include "stdafx.hh"
#include "utils.hh"


#define LIBOARD_TEST_REG 0x00
#define LIBOARD_ID_REG 0x01




#define LIBOARD_SLC_DATA_REG 0x8000
#define LIBOARD_SLC_CONTROL_REG 0x8001
#define LIBOARD_SLC_STATUS_REG 0x8002





#define LIBOARD_RO_CONTROL_REG 0x2002
#define LIBOARD_RO_RESET_REG 0x2003


#define LIBOARD_MDCC_SHIFT 0x4000


#define LIBOARD_HEADER_SIZE 20
#define LIBOARD_FRAME_SIZE  11
#define LIBOARD_EVENT_START 0xB0
#define LIBOARD_EVENT_STOP 0xA0



#define LIBOARD_ID_SHIFT 1
#define LIBOARD_NBASIC_SHIFT 2
#define LIBOARD_FORMAT_SHIFT 3
#define LIBOARD_GTC_SHIFT 4
#define LIBOARD_ABCID_SHIFT 7
#define LIBOARD_BCID_SHIFT 13
#define LIBOARD_LTRG_SHIFT 17
#define LIBOARD_HEADER_SHIFT 20

#define LiboardID(a) (a[LIBOARD_ID_SHIFT])
#define LiboardNbAsic(a) (a[LIBOARD_NBASIC_SHIFT])
#define LiboardFormat(a) (a[LIBOARD_FORMAT_SHIFT])
#define LiboardGTC(a) ((a[LIBOARD_GTC_SHIFT]<<16)|(a[LIBOARD_GTC_SHIFT+1]<<8)|(a[LIBOARD_GTC_SHIFT+2]))
#define LiboardABCID(a) (((a[LIBOARD_ABCID_SHIFT]<<8)|(a[LIBOARD_ABCID_SHIFT+1]))*16777216ULL+((a[LIBOARD_ABCID_SHIFT+2]<<24)|(a[LIBOARD_ABCID_SHIFT+3]<<16)|(a[LIBOARD_ABCID_SHIFT+4]<<8)|(a[LIBOARD_ABCID_SHIFT+5])))
#define LiboardBCID(a) ((a[LIBOARD_BCID_SHIFT]<<16)|(a[LIBOARD_BCID_SHIFT+1]<<8)|(a[LIBOARD_BCID_SHIFT+2]))


static LoggerPtr _logLiboard(Logger::getLogger("PMDAQ_LIBOARD"));

namespace liboard
{
  class LiboardDriver {
  public:
    LiboardDriver(char * deviceIdentifier ,uint32_t productid=0x6001);
    ~LiboardDriver();
    int32_t open(char * deviceIdentifier, uint32_t productid );
    int32_t writeNBytes(unsigned char  *cdata, uint32_t nb);
    int32_t readNBytes(unsigned char  *cdata, uint32_t nb);
    int32_t registerWrite(uint32_t address, uint32_t data);
    int32_t registerRead(uint32_t address, uint32_t *data);
    uint32_t registerRead(uint32_t address);
    int32_t setup();
    int32_t loadSLC(uint32_t* SLC,uint32_t size);
    int32_t setAcquisitionMode(bool active=true,bool autoreset=true,bool external=false);
    int32_t resetFSM();
    int32_t readData(unsigned char* tro,uint32_t size);
    uint32_t readOneEvent(unsigned char* cbuf);
    inline uint32_t difId(){return _difId;}
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
    void setSpillOn(uint32_t nc);
    void setSpillOff(uint32_t nc);
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

  protected:
    struct ftdi_context theFtdi;
    uint32_t _productId;
    char _deviceId[12];
    uint32_t _difId;
  };
};
