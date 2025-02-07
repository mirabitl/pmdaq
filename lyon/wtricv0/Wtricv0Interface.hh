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


static LoggerPtr _logWtricv0(Logger::getLogger("PMDAQ_WTRICV0"));
namespace wtricv0
  {
    enum IDS {DETID=140};
    enum PORT {CTRL=9760,DATA=9761,SENSOR=9762};
    enum  command { STARTACQ=0,STOPACQ=1,RESET=2,READSC=5,LOADSC=6,STORESC=7,LASTABCID=3,LASTGTC=4,CLOSE=10,PULSE=11};
    // Gere la socket registre
    class registerHandler : public socketProcessor
    {
    public:
      registerHandler(std::string);
      virtual bool process_message();
      uint32_t sendCommand(uint8_t command);
      void sendParameter(uint8_t command,uint8_t par);
      void sendSlowControl(uint8_t* slc,uint16_t len_bytes=109);
      inline uint32_t slcStatus(){return _slcStatus;}
      inline void setSlcStatus(uint32_t i){_slcStatus=i;}
      inline void useTransactionId(){_noTransReply=false;}
    private:
      uint32_t _slcStatus;
      wizcc::Message* _msg;
      bool _noTransReply;
    };

    // Gere la socket Sensor
    class sensorHandler : public socketProcessor
    {
    public:
      sensorHandler(std::string);
      virtual bool process_message();
    private:
      wizcc::Message* _msg;
    };

    // gere la socket data et envoie les donnees a l'EVB
    class dataHandler : public socketProcessor
    {
    public:
      dataHandler(std::string ip);
      virtual bool process_message();
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
      uint32_t _lastGTC,_lastBCID,_event,_detid,_ntrg,_expectedLength;
      uint32_t _nProcessed;
      pm::pmSender* _dsData;
      uint8_t _triggerId;
      uint32_t _detId;

      
    };


  };
