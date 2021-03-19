#ifndef _GRICV0_INTERFACE_HH
#define _GRICV0_INTERFACE_HH

#include "MpiMessageHandler.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "ReadoutLogger.hh"
#define MAX_BUFFER_LEN 0x4000
#include "zmSender.hh"
#define C3I_VERSION 145
#define MBSIZE 0x40000

namespace lydaq
{
  namespace gricv0
  {
    class board;
    class Message {
    public:
      enum Fmt {HEADER=0,LEN=1,TRANS=3,CMD=4,PAYLOAD=5};
      enum  command { STARTACQ=0,STOPACQ=1,RESET=2,READSC=5,LOADSC=6,STORESC=7,LASTABCID=3,LASTGTC=4,CLOSE=10,PULSE=11};
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
      enum PORT {CTRL=9760,DATA=9761,SENSOR=9762};
      Interface();
      ~Interface(){;}
      void initialise();
      void addDevice(std::string address);
      void listen();

      inline std::map<std::string,gricv0::board*>& boards(){ return _boards;}
      inline gricv0::board* getBoard(std::string ip) {return _boards[ip];} 
      void close();
    private:
      void dolisten();
      std::map<std::string,gricv0::board*> _boards;

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
      uint32_t sendMessage(gricv0::Message* wmsg);
      void processBuffer(uint64_t id, uint16_t l,char* b);
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
    private:
      uint64_t _id;
      
      NL::Socket* _sock;
      // temporary buffer to collect reply
      uint8_t _b[MBSIZE];
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
      uint32_t sendCommand(uint8_t command);
      void sendParameter(uint8_t command,uint8_t par);
      void sendSlowControl(uint8_t* slc,uint16_t len_bytes=109);
      void processReply(uint32_t tr,uint32_t* rep=0);
      inline uint32_t slcStatus(){return _slcStatus;}
      inline void setSlcStatus(uint32_t i){_slcStatus=i;}
      inline void useTransactionId(){_noTransReply=false;}
    private:
      uint32_t _slcStatus;
      gricv0::Message* _msg;
      bool _noTransReply;
    };

    // Gere la socket Sensor
    class sensorHandler : public socketHandler
    {
    public:
      sensorHandler(std::string);
      virtual bool processPacket();
    private:
      gricv0::Message* _msg;
    };

    // gere la socket data et envoie les donnees a l'EVB
    class dataHandler : public socketHandler
    {
    public:
      dataHandler(std::string ip);
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
      void autoRegister(zmq::context_t* c,Json::Value config,std::string appname,std::string portname);
    private:
      uint64_t _lastABCID;
      uint32_t _lastGTC,_lastBCID,_event,_detid,_ntrg,_expectedLength;
      uint32_t _nProcessed;
      zdaq::zmSender* _dsData;
      uint8_t _triggerId;
      uint32_t _detId;

      
    };

    /// Un board => 3 socket
    class board
    {
    public:
      board(std::string ip);
      inline gricv0::registerHandler* reg(){return _regh;}
      inline gricv0::sensorHandler* sensor(){return _sensorh;}
      inline gricv0::dataHandler* data(){return _datah;}
      inline std::string ipAddress(){return _ip;}
    private:
      std::string _ip;
      gricv0::registerHandler* _regh;
      gricv0::sensorHandler* _sensorh;
      gricv0::dataHandler* _datah;
      
    };

  };
};
#endif
