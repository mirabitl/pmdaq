#pragma once
/**
 * @file FtdiUsbDriver.hh
 * @author L.Mirabito
 * @brief FTDI driver
 * @version 1.0
 * @date 2024-09-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */
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
// Logger for FTDI functions
static LoggerPtr _logFTDI(Logger::getLogger("PMDAQ_FTDI"));
/**
 * @brief Namespace for mdcc
 * 
 */
namespace mdcc
{
  /**
   * @brief Class to access FTDI USB chips
   * 
   */
  class FtdiUsbDriver {
  public:
    /**
     * @brief Construct a new Ftdi Usb Driver object
     * 
     * @param deviceIdentifier 
     * @param productid 
     */
    FtdiUsbDriver(char * deviceIdentifier ,uint32_t productid=0x6001);
    /**
     * @brief Destroy the Ftdi Usb Driver object
     * 
     */
    ~FtdiUsbDriver();
    /**
     * @brief Check the USB access
     * 
     * @param start Start register address
     * @param count Length of register scans
     */
    void checkReadWrite(uint32_t start,uint32_t count);
    /**
     * @brief Purge the FTDI FIFOs
     * 
     */
    void FT245Purge( void );
    /**
     * @brief Read 1 byte 
     * 
     * @verbatim 
     * RC 
     * -666 No USB available
     * <0   USB bulk erro
     * 0   Too many tries
     * 1 Ok
     * @endverbatim
     * 
     * @param resultPtr Destination buffer
     * @return int32_t Number of bytes or error code
     */
    int32_t read( unsigned char  *resultPtr );
    /**
     * @brief Read N bytes 
     * 
     * @param resultPtr Destination buffer
     * @param nbbytes Number of bytes to read
     */
    void readNb( unsigned char  *resultPtr,int32_t nbbytes );
    /**
     * @brief Write 1 byte 
     * 
     * @param data Byte to write
     */
    void write( unsigned char  data);
    /**
     * @brief Write N bytes
     * 
     * @param cdata source buffer
     * @param nb Number of bytes to write
     */
    void writeNb( unsigned char  *cdata, uint32_t nb);
    /**
     * @brief Read  one short fromEEPROM
     * 
     * @param address Address to read
     * @param resultPtr destination buffer
     */
    void readEEPROM( uint32_t address, 	uint32_t *resultPtr );
    /**
     * @brief Write data to the eeprom
     * 
     * @param address Eeprom address
     * @param data data to write
     */
    void writeEEPROM( 	uint32_t address,	uint32_t data);
    /**
     * @brief Reset EEPROM
     * 
     */
    void resetEEPROM( 	void);
    /**
     * @brief Reset USB bus
     * 
     */
    void resetBus( );
    /**
     * @brief Read FTDI status
     * 
     * @param RXQueue 
     * @param TXQueue 
     * @param Event 
     * @deprecated Always 1 in RxQueue
     */
    void readStatus( uint32_t *RXQueue, uint32_t *TXQueue, uint32_t *Event);
    /**
     * @brief Read MDCC version at address 0x100
     * 
     * @param version destination address
     * @return int32_t error code -1 if read error
     */
    int32_t UsbGetFirmwareRevision(uint32_t *version) ;
    /**
     * @brief Set the Test register value at address 0x2
     * 
     * @param tvalue value to write
     * @return int32_t error code -2 if read error
     */
    int32_t SetTestRegister(int32_t tvalue)   ;
    /**
     * @brief Get the Test Register value
     * 
     * @param tvalue destination buffer
     * @return int32_t error code -2 if read error
     */
    int32_t GetTestRegister(uint32_t *tvalue)   ;
    /**
     * @brief Read one byte
     * 
     * @param tbyte destination buffer
     * @return int32_t 1 or error 0
     */
    int32_t UsbReadByte(unsigned char  *tbyte);
    /**
     * @brief Read 4 bytes
     * 
     * @param data destination buffer
     * @return int32_t 4 or error 0
     */
    int32_t UsbRead4Bytes(uint32_t *data);
    /**
     * @brief Read 16 bytes
     * 
     * @param data destination buffer
     * @return int32_t 16 or error 0
     */
    int32_t UsbRead16Bytes(unsigned char  *data);
    /**
     * @brief Read 22 bytes
     * 
     * @param data destination buffer
     * @return int32_t 22 or error 0
     */
    int32_t UsbRead22Bytes(unsigned char  *data);
    /**
     * @brief Read N bytes
     * 
     * @param data destination buffer
     * @param nbbytes Number of bytes to read
     * @return int32_t nbbytes or error 0
     */
    int32_t UsbReadnBytes(unsigned char  *data, int32_t nbbytes);
    /**
     * @brief Read 3 bytes
     * 
     * @param data destination buffer
     * @return int32_t 3 or error 0
     */
    int32_t UsbRead3Bytes(uint32_t *data);
    /**
     * @brief Read 2 bytes
     * 
     * @param data destination buffer
     * @return int32_t 2 or error 0
     */
    int32_t UsbRead2Bytes(uint32_t *data);
    /**
     * @brief read register value (32 bits) at address
     * 
     * @param address register address
     * @param data Destination buffer
     * @return int32_t 0 Ok negative error
     */
    int32_t UsbRegisterRead(uint32_t address, uint32_t *data);
    /**
     * @brief Write data in a register
     * 
     * @param address Register address
     * @param data Data to write
     * @return int32_t 0 Ok negative error
     * #deprecated proxy to UsbRegisterWrite2
     */
    int32_t UsbRegisterWrite(uint32_t address, uint32_t data);
    /**
     * @brief Write data in a register
     * 
     * @param address Register address
     * @param data Data to write
     * @return int32_t 0 Ok negative error
     */
    int32_t UsbRegisterWrite2(uint32_t address, uint32_t data);
    /**
     * @brief Write a MDCC command
     * 
     * @param command Command value
     * @return int32_t 0 Ok negative error
     */
    int32_t UsbCommandWrite(uint32_t command);
    /**
     * @brief Get FTDI status
     * 
     * @param RXQueue 
     * @param TXQueue 
     * @param Event 
     * @return int32_t 0 and RxQueue set to 1
     * @deprecated Allways 1 in RxQueue
     */
    int32_t  FT245GetStatus(int32_t *RXQueue,int32_t *TXQueue ,int32_t *Event)   ;
    /**
     * @brief Reset the bus
     * 
     * @return int32_t 0 Ok negative error
     */
    int32_t FT245Reset(void);
    /**
     * @brief Encode error message from return code
     * 
     * @return std::string 
     */
    std::string rcMessage();
    /**
     * @brief Check good return code
     * 
     * @return true 
     * @return false 
     */
    bool isOk(){return _rc==RC::OK;}
    static uint16_t CrcTable[256];
    enum RC {OK=0,TESTREG=1,CLOSE=2,PURGE=3,UNAVAILABLE=4,BULKERROR=5,TOOMANYTRIALS=6,TIMEOUT=7,EEPROMREAD=8,EEPROMWRITE=9,EEPROMERASE=10,RESETBUS=11,OPENFAILED=12};

  protected:
    struct ftdi_context theFtdi;
    uint32_t timeOut;
    uint32_t theProduct_;
    char theName_[12];
    uint32_t _rc;
  };
};

