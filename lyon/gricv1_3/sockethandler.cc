#include "Gricv1Interface.hh"

#include <arpa/inet.h>

#include <algorithm>
#include <cstring>

namespace {
constexpr size_t kFrameHeaderLength = 3;
constexpr size_t kMinimumFrameLength = 7;
}

gricv1_3::socketHandler::socketHandler(
    std::string ip, uint32_t port, mpi2::MpiMessageHandler* transport)
    : _idx(0),
      _id((static_cast<uint64_t>(utils::convertIP(ip)) << 32) | port),
      _transport(transport),
      _transaction(0)
{
}

void gricv1_3::socketHandler::clear()
{
  _receiveBuffer.clear();
  std::lock_guard<std::mutex> lock(_replyMutex);
  _pendingReplies.clear();
}

uint32_t gricv1_3::socketHandler::sendMessage(gricv1_3::Message* message,
                                               bool expectReply)
{
  const std::lock_guard<std::mutex> sendLock(_sendMutex);
  const uint8_t transaction =
      static_cast<uint8_t>((_transaction++ % 255) - 1);
  message->ptr()[gricv1_3::Message::Fmt::TRANS] = transaction;

  if (expectReply)
  {
    std::lock_guard<std::mutex> lock(_replyMutex);
    _pendingReplies[transaction % 255] = PendingReply{};
  }

  const int sent = _transport->send(_id, message->ptr(), message->length());
  if (sent != message->length())
  {
    if (expectReply)
    {
      std::lock_guard<std::mutex> lock(_replyMutex);
      _pendingReplies.erase(transaction % 255);
    }
    PM_ERROR(_logGricv1, "Cannot send message to endpoint " << std::hex << _id
                                                            << std::dec << " (sent "
                                                            << sent << " of "
                                                            << message->length()
                                                            << " bytes)");
    return UINT32_MAX;
  }

  PM_DEBUG(_logGricv1, " Address " << std::hex
                                   << ((message->address() >> 32) & 0xFFFFFFFF)
                                   << std::dec << " Port "
                                   << (message->address() & 0XFFFF) << " Length "
                                   << message->length() << " Transaction "
                                   << static_cast<unsigned>(transaction));
  return transaction;
}

bool gricv1_3::socketHandler::waitForReply(
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

void gricv1_3::socketHandler::completeReply(uint8_t transaction,
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

void gricv1_3::socketHandler::processBuffer(uint64_t id, uint16_t length,
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
    if (frameLength < kMinimumFrameLength || frameLength > MBSIZE)
    {
      PM_WARN(_logGricv1, "Invalid frame length " << frameLength << " from "
                                                  << std::hex << id << std::dec);
      _receiveBuffer.erase(_receiveBuffer.begin());
      continue;
    }

    if (_receiveBuffer.size() < frameLength)
      return;

    if (_receiveBuffer[frameLength - 1] != ')')
    {
      PM_WARN(_logGricv1, "Invalid frame terminator from " << std::hex << id
                                                           << std::dec);
      _receiveBuffer.erase(_receiveBuffer.begin());
      continue;
    }

    std::memcpy(_buf, _receiveBuffer.data(), frameLength);
    _idx = static_cast<uint32_t>(frameLength);
    if (!processPacket())
      PM_WARN(_logGricv1, "Frame processor rejected a frame from " << std::hex
                                                                   << id << std::dec);

    _idx = 0;
    _receiveBuffer.erase(_receiveBuffer.begin(),
                         _receiveBuffer.begin() + frameLength);
  }
}

void gricv1_3::socketHandler::purgeBuffer()
{
  _receiveBuffer.clear();
  _idx = 0;
}
