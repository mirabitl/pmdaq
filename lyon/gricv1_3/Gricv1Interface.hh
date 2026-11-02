#pragma once

#include "SocketHandler.hh"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include <map>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <mutex>
#include <thread>
#include "stdafx.hh"
#include "utils.hh"
#define MAX_BUFFER_LEN 0x4000
#include "pmSender.hh"
#define C3I_VERSION 145
#define MBSIZE 0x40000
static LoggerPtr _logGricv1(Logger::getLogger("PMDAQ_GRICV1_3"));

namespace gricv1_3
  {
    class board;
    class Message {
    public:
      enum Fmt {HEADER=0,LEN=1,TRANS=3,CMD=4,PAYLOAD=6};
      enum  command { WRITEREG=1,READREG=2,SLC=4,DATA=8,ACKNOWLEDGE=16,ERROR=32};
      enum Register {TEST=0x0,ID=0x1,NBASIC=0x2,
		 SLC_CTRL=0x11,SLC_STATUS=0x12,SLC_SIZE=0x13,
		 PP_PWRA_PWRD=0x21,PP_PWRD_ACQ=0x22,PP_ACQ_PWRD=0x23,PP_PWRD_PWRA=0x24,PP_CTRL=0x25,
		 ACQ_CTRL=0x30,ACQ_FMT=0x31,ACQ_RST=0x32,
		 BME_CAL1=0x40,BME_CAL2=0x41,BME_CAL3=0x42,BME_CAL4=0x43,
		 BME_CAL5=0x44,BME_CAL6=0x45,BME_CAL7=0x46,BME_CAL8=0x47,
		 BME_HUM=0x48,BME_PRES=0x49,BME_TEMP=0x4A,
		 CTEST_CTRL=0x50,CTEST_DELAY=0x51,CTEST_LENGTH=0x52,CTEST_NUMBER=0x53,CTEST_VERSION=0xFF};
        
                                
                        

      Message(): _address(0),_length(2) {;}
      inline uint64_t address(){return _address;}
      inline uint16_t length(){return _length;}
      inline uint8_t* ptr(){return _buf;}
      inline void setLength(uint16_t l){_length=l;}
      inline void setAddress(uint64_t a){_address=a;}
      inline void setAddress(std::string address,uint16_t port){_address=( (uint64_t) utils::convertIP(address)<<32)|port;}
    private:
      uint64_t _address;
      uint16_t _length;
      uint8_t _buf[MAX_BUFFER_LEN];
    
    };

    class Interface 
    {
    public:
      enum PORT { REGISTER=10002,SLC=10001,DATA=10003};
      Interface();
      ~Interface();
      void initialise();
      void addDevice(std::string address);
      void listen();
      void terminate();
      inline std::map<std::string,gricv1_3::board*>& boards(){ return _boards;}
      inline gricv1_3::board* getBoard(std::string ip) {return _boards[ip];}
      void close();
    private:
      std::map<std::string,gricv1_3::board*> _boards;
      bool _running;
    };

    using socketHandler = code_libhv::SocketHandler;
    // Gere la socket registre
    class registerHandler : public socketHandler
    {
    public:
      explicit registerHandler(std::string ip);
      ~registerHandler() override;
      virtual bool processPacket();
      void writeRegister(uint16_t address,uint32_t value);
      uint32_t readRegister(uint16_t address);
      inline uint32_t slcStatus(){return _slcStatus;}
      inline void setSlcStatus(uint32_t i){_slcStatus=i;}
    private:
      uint32_t _slcStatus;
      gricv1_3::Message* _msg;
      std::mutex _requestMutex;
    };

    // Gere la socket Slow control
    class slcHandler : public socketHandler
    {
    public:
      explicit slcHandler(std::string ip);
      ~slcHandler() override;
      virtual bool processPacket();
      void sendSlowControl(uint8_t* slc,uint16_t len_bytes=109);
    private:
      gricv1_3::Message* _msg;
    };

    // gere la socket data et envoie les donnees a l'EVB
    class dataHandler : public socketHandler
    {
    public:
      explicit dataHandler(std::string ip);
      ~dataHandler() override;
      virtual bool processPacket();
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

    /// Un board => 3 socket
    class board
    {
    public:
      explicit board(std::string ip);
      ~board();
      inline gricv1_3::registerHandler* reg(){return _regh;}
      inline gricv1_3::slcHandler* slc(){return _slch;}
      inline gricv1_3::dataHandler* data(){return _datah;}
      inline std::string ipAddress(){return _ip;}
    private:
      std::string _ip;
      gricv1_3::registerHandler* _regh;
      gricv1_3::slcHandler* _slch;
      gricv1_3::dataHandler* _datah;
      
    };

  };
