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
#include <string>
#include "stdafx.hh"
#include "utils.hh"
static LoggerPtr _logFTDI(Logger::getLogger("PMDAQ_FTDI"));

namespace mdcc
{
  class FtdiUsbDriver {
  public:
    enum RC {OK=0,TESTREG=1,CLOSE=2,PURGE=3,UNAVAILABLE=4,BULKERROR=5,TOOMANYTRIALS=6,TIMEOUT=7,EEPROMREAD=8,EEPROMWRITE=9,EEPROMERASE=10,RESETBUS=11,OPENFAILED=12};
    FtdiUsbDriver(char * deviceIdentifier ,uint32_t productid=0x6001);
    ~FtdiUsbDriver();
    void checkReadWrite(uint32_t start,uint32_t count);
    void FT245Purge( void );
    int32_t read( unsigned char  *resultPtr );
    void readNb( unsigned char  *resultPtr,int32_t nbbytes );
    void write( unsigned char  data);
    void writeNb( unsigned char  *cdata, uint32_t nb);
    void readEEPROM( uint32_t address, 	uint32_t *resultPtr );
    void writeEEPROM( 	uint32_t address,	uint32_t data);
    void resetEEPROM( 	void);
    void resetBus( );
    void readStatus( uint32_t *RXQueue, uint32_t *TXQueue, uint32_t *Event);
    int32_t UsbGetFirmwareRevision(uint32_t *version) ;
    int32_t SetTestRegister(int32_t tvalue)   ;
    int32_t GetTestRegister(uint32_t *tvalue)   ;
    int32_t UsbReadByte(unsigned char  *tbyte);
    int32_t UsbRead4Bytes(uint32_t *data);
    int32_t UsbRead16Bytes(unsigned char  *data);
    int32_t UsbRead22Bytes(unsigned char  *data);
    int32_t UsbReadnBytes(unsigned char  *data, int32_t nbbytes);
    int32_t UsbRead3Bytes(uint32_t *data);
    int32_t UsbRead2Bytes(uint32_t *data);
    int32_t UsbRegisterRead(uint32_t address, uint32_t *data);
    int32_t UsbRegisterWrite(uint32_t address, uint32_t data);
    int32_t UsbRegisterWrite2(uint32_t address, uint32_t data);
    int32_t UsbCommandWrite(uint32_t command);
    int32_t  FT245GetStatus(int32_t *RXQueue,int32_t *TXQueue ,int32_t *Event)   ;
    int32_t FT245Reset(void);
    std::string rcMessage();
    bool isOk(){return _rc==RC::OK;}
    static uint16_t CrcTable[256];
  protected:
    struct ftdi_context theFtdi;
    uint32_t timeOut;
    uint32_t theProduct_;
    char theName_[12];
    uint32_t _rc;
  };
};

