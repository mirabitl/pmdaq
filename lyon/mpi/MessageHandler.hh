#pragma once
#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>
#include "stdafx.hh"
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
typedef std::function<void (uint64_t,uint16_t,char*)> MPIFunctor;

namespace mpi {
/**
 * @brief Exception class for mpi namespace
 * 
 */
struct MpiException : public std::exception
{
   std::string s;
   MpiException(std::string ss) : s(ss) {}
   ~MpiException() throw () {} // Updated
   const char* what() const throw() { return s.c_str(); }
};

/**
 * @brief Virtual interface to be implemented by any message processor
 * 
 * It has to be implemented and provided to the mpi::OnRead class and 
 * mpi::OnDisconnect class
 * 
 */
class MessageHandler
{
public:
/**
 * @brief Called by exec method of mpi::OnRead object
 * 
 * @param socket NL::socket
 */
  virtual void processMessage(NL::Socket* socket){;} //throw (MpiException){;}
  /**
   * @brief Called by exec of mpi::OnDisconnect object
   * 
   * @param sock NL::socket
   */
  virtual void removeSocket(NL::Socket* sock){;}
};

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
  OnAccept(MessageHandler* msh);
  /**
   * @brief callback of accept
   * 
   * @param socket NL::socket
   * @param group Socketgroup
   * @param reference void, possible callback
   */
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) ;
private:
  MessageHandler* _msh;
};

/**
 * @brief Socket Group command class called on select
 * 
 * The exec command is called when some data are to be read
 * 
 */
class OnRead: public NL::SocketGroupCmd 
{
public:
/**
 * @brief Construct a new OnRead object
 * 
 * @param msh MessageHandler pointer 
 */
  OnRead(MessageHandler* msh);
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
public:
  unsigned char _readBuffer[0x80000];
private:
  MessageHandler* _msh;
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
  * @param msh MessageHandler( currently Unused)
  */
  OnDisconnect(MessageHandler* msh);
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
  MessageHandler* _msh;
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
};

