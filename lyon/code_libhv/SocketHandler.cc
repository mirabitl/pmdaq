#include "SocketHandler.hh"

#include <arpa/inet.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
constexpr size_t kFrameHeaderLength = 3;
constexpr size_t kMinimumFrameLength = 6;
constexpr size_t kMaximumFrameLength = 0xFFFF;
constexpr size_t kTransactionOffset = 3;
}

code_libhv::SocketHandler::SocketHandler(std::string address, uint16_t port)
    : _idx(0),
      _id((static_cast<uint64_t>(inet_addr(address.c_str())) << 32) | port),
      _tcpEndpoint(address, port, _id,
                   [this](uint64_t id, uint16_t length, char* data) {
                     processBuffer(id, length, data);
                   }),
      _transaction(0)
{
  std::memset(_buf, 0, sizeof(_buf));
}

code_libhv::SocketHandler::~SocketHandler()
{
  disconnect();
}

uint32_t code_libhv::SocketHandler::sendMessage(uint8_t* data, size_t length,
                                                bool expectReply)
{
  if (data == nullptr || length <= kTransactionOffset ||
      length > static_cast<size_t>(std::numeric_limits<uint16_t>::max()))
    return UINT32_MAX;

  std::lock_guard<std::mutex> sendLock(_sendMutex);
  const uint8_t transaction =
      static_cast<uint8_t>((_transaction++ % 255) - 1);
  data[kTransactionOffset] = transaction;

  if (expectReply)
  {
    std::lock_guard<std::mutex> lock(_replyMutex);
    _pendingReplies[transaction % 255] = PendingReply{};
  }

  const int sent = _tcpEndpoint.send(data, length);
  if (sent != static_cast<int>(length))
  {
    if (expectReply)
    {
      std::lock_guard<std::mutex> lock(_replyMutex);
      _pendingReplies.erase(transaction % 255);
    }
    std::cerr << "code_libhv: failed to send " << length
              << " bytes to endpoint 0x" << std::hex << _id << std::dec
              << " (sent " << sent << ")\n";
    return UINT32_MAX;
  }
  return transaction;
}

bool code_libhv::SocketHandler::waitForReply(
    uint8_t transaction, std::chrono::milliseconds timeout,
    std::vector<uint8_t>& reply)
{
  std::unique_lock<std::mutex> lock(_replyMutex);
  const auto ready = [this, transaction] {
    const auto it = _pendingReplies.find(transaction % 255);
    return it != _pendingReplies.end() && it->second.ready;
  };

  if (!_replyReady.wait_for(lock, timeout, ready))
  {
    _pendingReplies.erase(transaction % 255);
    return false;
  }

  const uint8_t key = transaction % 255;
  reply = std::move(_pendingReplies.at(key).data);
  _pendingReplies.erase(key);
  return true;
}

void code_libhv::SocketHandler::completeReply(uint8_t transaction,
                                              const uint8_t* data,
                                              size_t length)
{
  {
    std::lock_guard<std::mutex> lock(_replyMutex);
    const auto it = _pendingReplies.find(transaction % 255);
    if (it == _pendingReplies.end())
      return;
    it->second.data.assign(data, data + length);
    it->second.ready = true;
  }
  _replyReady.notify_all();
}

void code_libhv::SocketHandler::processBuffer(uint64_t id, uint16_t length,
                                              char* data)
{
  if (length == 0)
    return;

  _receiveBuffer.insert(_receiveBuffer.end(), data, data + length);
  while (!_receiveBuffer.empty())
  {
    const auto start = std::find(_receiveBuffer.begin(), _receiveBuffer.end(),
                                 static_cast<uint8_t>('('));
    if (start == _receiveBuffer.end())
    {
      _receiveBuffer.clear();
      return;
    }
    if (start != _receiveBuffer.begin())
      _receiveBuffer.erase(_receiveBuffer.begin(), start);

    if (_receiveBuffer.size() < kFrameHeaderLength)
      return;

    uint16_t encodedLength = 0;
    std::memcpy(&encodedLength, _receiveBuffer.data() + 1,
                sizeof(encodedLength));
    const size_t frameLength = ntohs(encodedLength);
    if (frameLength < kMinimumFrameLength ||
        frameLength > kMaximumFrameLength)
    {
      std::cerr << "code_libhv: invalid frame length " << frameLength
                << " from endpoint 0x" << std::hex << id << std::dec << '\n';
      _receiveBuffer.erase(_receiveBuffer.begin());
      continue;
    }

    if (_receiveBuffer.size() < frameLength)
      return;

    if (_receiveBuffer[frameLength - 1] != ')')
    {
      std::cerr << "code_libhv: invalid frame terminator from endpoint 0x"
                << std::hex << id << std::dec << '\n';
      _receiveBuffer.erase(_receiveBuffer.begin());
      continue;
    }

    std::memcpy(_buf, _receiveBuffer.data(), frameLength);
    _idx = static_cast<uint32_t>(frameLength);
    if (!processPacket())
      std::cerr << "code_libhv: frame processor rejected endpoint 0x"
                << std::hex << id << std::dec << '\n';
    _idx = 0;
    _receiveBuffer.erase(_receiveBuffer.begin(),
                         _receiveBuffer.begin() + frameLength);
  }
}

void code_libhv::SocketHandler::purgeBuffer()
{
  _receiveBuffer.clear();
  _idx = 0;
}

void code_libhv::SocketHandler::clear()
{
  purgeBuffer();
  std::lock_guard<std::mutex> lock(_replyMutex);
  _pendingReplies.clear();
}
