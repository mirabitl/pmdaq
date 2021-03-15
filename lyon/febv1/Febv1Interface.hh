#pragma once
#include "MpiMessageHandler.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "stdafx.hh"
#define MAX_BUFFER_LEN 0x4000
#include "pmSender.hh"
#define C3I_VERSION 145
#define MBSIZE 0x40000
#define CHBYTES 6
#define NREP 8
namespace febv1
  {
    class board;
    class Message {
    public:
      enum Fmt {HEADER=0,LEN=1,TRANS=3,CMD=4,PAYLOAD=6};
      enum Register {
	SLC_DATA=0x0,SLC_CTRL=0x201,
	ACTIVE=0x222,DEAD=0x223,
	GET_LUT=0x224,GET_LUT_STATUS=0x225,LUT_MASK=0x226,MEAS_MASK=0x230,
	ACQ_MODE=0x219,ACQ_CTRL=0x220};

      Message(): _address(0),_length(2) {;}
      inline uint64_t address(){return _address;}
      inline uint16_t length(){return _length;}
      inline uint8_t* ptr(){return _buf;}
      inline void setLength(uint16_t l){_length=l;}
      inline void setAddress(uint64_t a){_address=a;}
      inline void setAddress(std::string address,uint16_t port){_address=( (uint64_t) mpi::MpiMessageHandler::convertIP(address)<<32)|port;}
    private:
      uint64_t _address;
      uint16_t _length;
      uint8_t _buf[MAX_BUFFER_LEN];
    
    };


    /// Gere les connections aux socket et le select
    
    class Interface 
    {
    public:
      enum PORT { REGISTER=10001,DATA=10002};
      Interface();
      ~Interface(){;}
      void initialise();
      void addDevice(std::string address);
      void listen();

      inline std::map<std::string,febv1::board*>& boards(){ return _boards;}
      inline febv1::board* getBoard(std::string ip) {return _boards[ip];} 
      void close();
    private:
      void dolisten();
      std::map<std::string,febv1::board*> _boards;

      NL::SocketGroup* _group;
 
      mpi::MpiMessageHandler* _msh;
      mpi::OnRead* _onRead;
      mpi::OnAccept* _onAccept;
      mpi::OnClientDisconnect* _onClientDisconnect;
      mpi::OnDisconnect* _onDisconnect;
      boost::thread_group g_store;
      boost::thread_group g_run;
      bool _running;
    };

    // Gere chaque socket
    // processPacket est virtuel
    class socketHandler
    {
    public:
      socketHandler(std::string,uint32_t port);
      int16_t checkBuffer(uint8_t* b,uint32_t maxidx);
      uint32_t sendMessage(febv1::Message* wmsg);
      virtual void processBuffer(uint64_t id, uint16_t l,char* b);
      void purgeBuffer();
      virtual bool processPacket()=0;
      NL::Socket* socket(){return _sock;}
      uint64_t id() {return _id;}
      uint32_t ipid() {return (_id>>32)&0xFFFFFFFF;}
      uint32_t sourceid() {return (ipid()>>16)&0XFFFF;}
      //uint32_t sourceid() {return ipid();}
      inline uint8_t* answer(uint8_t tr){return _answ[tr];}
      uint32_t transaction() {return _transaction;}

      void clear();
    protected:
      uint32_t _idx;
      uint8_t _buf[MBSIZE];
    protected:
            // temporary buffer to collect reply
      uint8_t _b[MBSIZE];

    private:
      uint64_t _id;
      
      NL::Socket* _sock;
      // Command answers pointers
      std::map<uint8_t,uint8_t*> _answ;
      uint32_t _transaction;

    };
    // Gere la socket registre
    class registerHandler : public socketHandler
    {
    public:
      registerHandler(std::string);
      virtual bool processPacket();
      uint32_t writeAddress(uint16_t address,uint16_t value,bool waitreply=false);
      uint32_t writeLongWord(uint16_t address,uint64_t value,bool waitreply=false);
      void writeRam(uint16_t* address,uint16_t *buffer,uint32_t bytes);
      void processReply(uint32_t tr,uint32_t* rep=0);
      inline uint32_t slcStatus(){return _slcStatus;}
      inline void setSlcStatus(uint32_t i){_slcStatus=i;}
      inline void useTransactionId(){_noTransReply=false;}
      void dumpAnswer(uint32_t tr);
      virtual void processBuffer(uint64_t id, uint16_t l,char* b);

    private:
      uint32_t _slcStatus;
      febv1::Message* _msg;
      bool _noTransReply;
    };


    // gere la socket data et envoie les donnees a l'EVB
    class dataHandler : public socketHandler
    {
    public:
      dataHandler(std::string ip);
      virtual bool processPacket();
      inline void setTriggerId(uint8_t i) {_triggerId=i;}
      void processEventTdc();
      inline uint32_t detectorId(){return _detId;}
      inline uint32_t difId(){return ((this->id()>>48)&0xFFFF);}
      inline uint64_t abcid(){return _lastABCID;}
      inline uint32_t gtc(){return _lastGTC;}
      inline uint32_t packets(){return _nProcessed;}
      inline uint32_t event(){return _event;}
      inline uint32_t triggers(){return _ntrg;}
      void clear();
      void connect(zmq::context_t* c,std::string dest);
      void autoRegister(zmq::context_t* c,std::string,std::string appname,std::string portname);
    private:
      uint64_t _lastABCID;
      uint32_t _lastGTC,_lastBCID,_event,_ntrg,_expectedLength,_chlines;
      uint32_t _nProcessed;
      pm::pmSender* _dsData;
      uint8_t _triggerId;
      uint32_t _detId;
      uint8_t _linesbuf[0x100000];
      
      
    };

    /// Un board => 3 socket
    class board
    {
    public:
      board(std::string ip);
      inline febv1::registerHandler* reg(){return _regh;}
      inline febv1::dataHandler* data(){return _datah;}
      inline std::string ipAddress(){return _ip;}
    private:
      std::string _ip;
      febv1::registerHandler* _regh;
      febv1::dataHandler* _datah;
      
    };

  };
