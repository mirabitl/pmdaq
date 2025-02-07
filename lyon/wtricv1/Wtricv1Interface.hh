#pragma once

#include "WizccInterface.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include <thread>
#include "stdafx.hh"
#include "utils.hh"

#include "pmSender.hh"
static LoggerPtr _logTricv1(Logger::getLogger("PMDAQ_TRICV1"));
using namespace wizcc;
  namespace tricv1
  {
    enum IDS {DETID=160};
    enum PORT { REGISTER=10002,SLC=10001,DATA=10003};
    enum Register {TEST=0x0,ID=0x1,NBASIC=0x2,
		 SLC_CTRL=0x11,SLC_STATUS=0x12,SLC_SIZE=0x13,
		 PP_PWRA_PWRD=0x21,PP_PWRD_ACQ=0x22,PP_ACQ_PWRD=0x23,PP_PWRD_PWRA=0x24,PP_CTRL=0x25,
		 ACQ_CTRL=0x30,ACQ_FMT=0x31,ACQ_RST=0x32,
		 BME_CAL1=0x40,BME_CAL2=0x41,BME_CAL3=0x42,BME_CAL4=0x43,
		 BME_CAL5=0x44,BME_CAL6=0x45,BME_CAL7=0x46,BME_CAL8=0x47,
		 BME_HUM=0x48,BME_PRES=0x49,BME_TEMP=0x4A,
		 CTEST_CTRL=0x50,CTEST_DELAY=0x51,CTEST_LENGTH=0x52,CTEST_NUMBER=0x53,CTEST_VERSION=0xFF};

    // Gere la socket registre
    class registerHandler : public wizcc::socketProcessor
    {
    public:
      registerHandler(std::string);
      virtual bool process_message();
      void writeRegister(uint16_t address,uint32_t value);
      uint32_t readRegister(uint16_t address);
      inline uint32_t slcStatus(){return _slcStatus;}
      inline void setSlcStatus(uint32_t i){_slcStatus=i;}
      inline void useTransactionId(){_noTransReply=false;}
    private:
      uint32_t _slcStatus;
      wizcc::Message* _msg;
      bool _noTransReply;
    };

    // Gere la socket Slow control
    class slcHandler : public wizcc::socketProcessor
    {
    public:
      slcHandler(std::string);
      virtual bool process_message();
      void sendSlowControl(uint8_t* slc,uint16_t len_bytes=109);
    private:
      wizcc::Message* _msg;
    };

    // gere la socket data et envoie les donnees a l'EVB
    class dataHandler : public wizcc::socketProcessor
    {
    public:
      dataHandler(std::string ip);
      virtual bool process_message();
      virtual void processBuffer(uint64_t id, uint16_t l,char* b);
      inline void setTriggerId(uint8_t i) {_triggerId=i;}

      inline uint32_t detectorId(){return _detId;}
      inline uint32_t difId(){return ((this->id()>>48)&0xFFFF);}
      inline uint64_t abcid(){return _lastABCID;}
      inline uint32_t gtc(){return _lastGTC;}
      inline uint32_t packets(){return _nProcessed;}
      inline uint32_t event(){return _event;}
      inline uint32_t triggers(){return _ntrg;}
      void clear();
      void connect(zmq::context_t* c,std::string dest);
      void autoRegister(zmq::context_t* c,std::string session,std::string appname,std::string portname);
    private:
      uint64_t _lastABCID;
      uint32_t _lastGTC,_lastBCID,_event,_ntrg,_expectedLength;
      uint32_t _nProcessed;
      pm::pmSender* _dsData;
      uint8_t _triggerId;
      uint32_t _detId;

      
    };


  };
