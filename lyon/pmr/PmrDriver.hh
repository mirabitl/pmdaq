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


#define PMR_TEST_REG 0x00
#define PMR_ID_REG 0x01
#define PMR_NBASIC_REG 0x02
#define PMR_SLC_DATA_REG 0x10
#define PMR_SLC_CONTROL_REG 0x11
#define PMR_SLC_STATUS_REG 0x12
#define PMR_SLC_SIZE_REG 0x13
#define PMR_RO_DATA_FORMAT_REG 0x31
#define PMR_RO_CONTROL_REG 0x30
#define PMR_RO_RESET_REG 0x32
#define PMR_PP_AN_2_DIG_REG 0x21
#define PMR_PP_DIG_2_ACQ_REG 0x22
#define PMR_PP_ACQ_2_DIG_REG 0x23
#define PMR_PP_DIG_2_AN_REG 0x24
#define PMR_PP_CONTROL_REG 0x25

#define PMR_HEADER_SIZE 20
#define PMR_FRAME_SIZE  20
#define PMR_EVENT_START 0xB0
#define PMR_EVENT_STOP 0xA0



#define PMR_ID_SHIFT 1
#define PMR_NBASIC_SHIFT 2
#define PMR_FORMAT_SHIFT 3
#define PMR_GTC_SHIFT 4
#define PMR_ABCID_SHIFT 7
#define PMR_BCID_SHIFT 13
#define PMR_LTRG_SHIFT 17
#define PMR_HEADER_SHIFT 20

#define PmrID(a) (a[PMR_ID_SHIFT])
#define PmrNbAsic(a) (a[PMR_NBASIC_SHIFT])
#define PmrFormat(a) (a[PMR_FORMAT_SHIFT])
#define PmrGTC(a) ((a[PMR_GTC_SHIFT]<<16)|(a[PMR_GTC_SHIFT+1]<<8)|(a[PMR_GTC_SHIFT+2]))
#define PmrABCID(a) (((a[PMR_ABCID_SHIFT]<<8)|(a[PMR_ABCID_SHIFT+1]))*16777216ULL+((a[PMR_ABCID_SHIFT+2]<<24)|(a[PMR_ABCID_SHIFT+3]<<16)|(a[PMR_ABCID_SHIFT+4]<<8)|(a[PMR_ABCID_SHIFT+5])))
#define PmrBCID(a) ((a[PMR_BCID_SHIFT]<<16)|(a[PMR_BCID_SHIFT+1]<<8)|(a[PMR_BCID_SHIFT+2]))


static LoggerPtr _logPmr(Logger::getLogger("PMDAQ_PMR"));

namespace pmr
{
class PmrDriver {
public:
  PmrDriver(char * deviceIdentifier ,uint32_t productid=0x6001);
  ~PmrDriver();
  int32_t open(char * deviceIdentifier, uint32_t productid );
  int32_t writeNBytes(unsigned char  *cdata, uint32_t nb);
  int32_t readNBytes(unsigned char  *cdata, uint32_t nb);
  int32_t registerWrite(uint32_t address, uint32_t data);
  int32_t registerRead(uint32_t address, uint32_t *data);
  int32_t setup();
  int32_t loadSLC(unsigned char* SLC,uint32_t size);
  int32_t setPowerPulsing(bool enable=false,uint32_t an2d=0,uint32_t d2ac=0,uint32_t ac2d=0,uint32_t d2an=0);
  int32_t setAcquisitionMode(bool active=true,bool autoreset=true,bool external=false);
  int32_t resetFSM();
  int32_t readData(unsigned char* tro,uint32_t size);
  uint32_t readOneEvent(unsigned char* cbuf);
  inline uint32_t difId(){return _difId;}
protected:
  struct ftdi_context theFtdi;
  uint32_t _productId;
  char _deviceId[12];
  uint32_t _difId;
};
};
