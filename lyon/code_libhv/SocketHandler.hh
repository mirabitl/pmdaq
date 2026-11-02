#pragma once

#include "TcpEndpoint.hh"

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace code_libhv
{

class SocketHandler
{
public:
  SocketHandler(std::string address, uint16_t port);
  virtual ~SocketHandler();

  SocketHandler(const SocketHandler&) = delete;
  SocketHandler& operator=(const SocketHandler&) = delete;

  int connect() { return _tcpEndpoint.connect(); }
  bool waitForConnection(std::chrono::milliseconds timeout)
  {
    return _tcpEndpoint.waitForConnection(timeout);
  }
  void disconnect() { _tcpEndpoint.disconnect(); }

  template <typename Message>
  uint32_t sendMessage(Message* message, bool expectReply = false)
  {
    return sendMessage(message->ptr(), message->length(), expectReply);
  }

  uint32_t sendMessage(uint8_t* data, size_t length, bool expectReply = false);
  virtual void processBuffer(uint64_t id, uint16_t length, char* data);
  void purgeBuffer();
  virtual bool processPacket() = 0;
  bool waitForReply(uint8_t transaction, std::chrono::milliseconds timeout,
                    std::vector<uint8_t>& reply);
  uint64_t id() const { return _id; }
  uint32_t ipid() const { return (_id >> 32) & 0xFFFFFFFF; }
  uint32_t sourceid() const { return (ipid() >> 16) & 0xFFFF; }
  uint32_t transaction() const { return _transaction; }
  void clear();

protected:
  static constexpr size_t BufferSize = 0x40000;
  uint32_t _idx;
  uint8_t _buf[BufferSize];
  void completeReply(uint8_t transaction, const uint8_t* data, size_t length);

private:
  struct PendingReply
  {
    bool ready = false;
    std::vector<uint8_t> data;
  };

  uint64_t _id;
  TcpEndpoint _tcpEndpoint;
  uint32_t _transaction;
  std::vector<uint8_t> _receiveBuffer;
  std::mutex _replyMutex;
  std::mutex _sendMutex;
  std::condition_variable _replyReady;
  std::map<uint8_t, PendingReply> _pendingReplies;
};

}  // namespace code_libhv
