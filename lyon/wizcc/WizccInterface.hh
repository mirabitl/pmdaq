#pragma once

#include "MessageHandler.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include <thread>
#include "stdafx.hh"
#include "utils.hh"
#define MAX_BUFFER_LEN 0x4000
#include "pmSender.hh"
#define C3I_VERSION 145
#define MBSIZE 0x40000
static LoggerPtr _logWizcc(Logger::getLogger("PMDAQ_WIZCC"));
/**
 * @brief pair of size, pointer
 * 
 */
typedef std::pair<uint32_t,unsigned char*> ptrBuf;
/**
 * @brief function with parameters
 * 
 * IP address| port, len,buffer
 * 
 */
typedef std::function<void (uint64_t,uint16_t,char*)> wizccFunctor;

  namespace wizcc
  {
    class board;
    //Message
    class Message {
    public:
      enum Fmt {HEADER=0,LEN=1,TRANS=3,CMD=4,PAYLOAD=6};
      enum  command { WRITEREG=1,READREG=2,SLC=4,DATA=8,ACKNOWLEDGE=16,ERROR=32};
     
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
    // Callbacks
    
/**
 * @brief Socket Group conmmand class for the accept callback
 * 
 * The exec method is called when a client socket is trying to connect 
 * 
 */
class OnAccept: public NL::SocketGroupCmd 
{

public:
/**
 * @brief Construct a new OnAccept object
 * 
 * @param msh MessageHandelr pointer (unused currently)
 */
  OnAccept();
  /**
   * @brief callback of accept
   * 
   * @param socket NL::socket
   * @param group Socketgroup
   * @param reference void, possible callback
   */
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) ;
};

/**
 * @brief Socket Group command class called on select
 * 
 * The exec command is called when some data are to be read
 * 
 */
class bufferHandler: public NL::SocketGroupCmd 
{
public:
/**
 * @brief Construct a new bufferHandler object
 * 
 * @param directroy for storage 
 */
  bufferHandler(std::string directory);
  /**
   * @brief method called when data are to be read on the socket
   * 
   * The exec method calls the processMessage of the MessageHandler object
   * 
   * @param socket NL::socket
   * @param group SocketGroup handling the socket (same select)
   * @param reference Unused
   */
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference);
  uint64_t socket_id(NL::Socket* socket);
  void process_buffer(NL::Socket* socket);
  void remove_socket(NL::Socket* socket);
  void add_handler(uint64_t id,wizccFunctor f);
public:
  unsigned char _readBuffer[0x80000];
private:
  std::string _storeDir;
  std::map<uint64_t, ptrBuf> _sockMap;
  std::map<uint64_t,wizccFunctor> _handlers;
  uint64_t _npacket;
  std::mutex _sem;
};

/**
 * @brief SocketGroup command class called on disconnection
 * 
 * The exec command is called when a socket is disconnected
 * 
 */
class OnDisconnect: public NL::SocketGroupCmd 
{
 public:
 /**
  * @brief Construct a new OnDisconnect object
  * 
  */
  OnDisconnect();
  /**
   * @brief Remove the socket from the socket group (select)
   * 
   * @param socket NL::socket
   * @param group SocketGroup
   * @param reference unused
   */
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference);
  /**
   * @brief Status of the connection
   * 
   * @return true 
   * @return false 
   */
  bool disconnected(){return _disconnect;}
private:
  bool _disconnect;
};



class OnClientDisconnect: public NL::SocketGroupCmd 
{
public:
  OnClientDisconnect();
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference);
  bool disconnected(){return _disconnect;}
private:
  bool _disconnect;
};


 
    /// Gere les connections aux socket et le select
    
    class Controller 
    {
    public:
      enum PORT { REGISTER=10002,SLC=10001,DATA=10003};
      Controller();
      ~Controller(){;}
      void initialise();
      void addDevice(std::string address);
      void listen();
      void terminate();
      inline std::map<std::string,wizcc::board*>& boards(){ return _boards;}
      inline wizcc::board* getBoard(std::string ip) {return _boards[ip];} 
      void close();
    private:
      void dolisten();
      std::map<std::string,wizcc::board*> _boards;

      NL::SocketGroup* _group;
 
      wizcc::bufferHandler* _onRead;
      wizcc::OnAccept* _onAccept;
      wizcc::OnClientDisconnect* _onClientDisconnect;
      wizcc::OnDisconnect* _onDisconnect;
      std::thread g_store;
      std::thread g_run;
      bool _running;
    };

    // Gere chaque socket
    // processPacket est virtuel
    class socketProcessor
    {
    public:
      socketProcessor(std::string,uint32_t port);
      int16_t check_buffer(uint8_t* b,uint32_t maxidx);
      uint32_t send_message(wizcc::Message* wmsg);
      void process_acknowledge(char* buffer,int len);
      void wait_reply(wizcc::Message* m);
      virtual void process_buffer(uint64_t id, uint16_t l,char* b);
      void purge_buffer();
      virtual bool process_message()=0;
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

    /// Un board => 3 socket
    class board
    {
    public:
      board(std::string ip);
      inline std::string ip_address(){return _ip;}
      inline wizcc::socketProcessor* processor(std::string name){return _sockm[name];}
      void add_processor(std::string name, wizcc::socketProcessor* p)
      {std::pair<std::string, wizcc::socketProcessor*> p1(name,p);_sockm.insert(p1); }
      inline std::map<std::string,wizcc::socketProcessor*>& processors(){return _sockm;}
    private:
      std::string _ip; 
      std::map<std::string,wizcc::socketProcessor*> _sockm;
      
    };

  };
