#include "Gricv0_3Interface.hh"

#include <arpa/inet.h>

#include <chrono>
#include <cstring>

namespace
{
const std::chrono::milliseconds kReplyTimeout(400);
}

gricv0_3::registerHandler::registerHandler(std::string ip)
    : socketHandler(ip, gricv0_3::Interface::PORT::CTRL),
      _slcStatus(0),
      _msg(new gricv0_3::Message())
{
}

gricv0_3::registerHandler::~registerHandler()
{
  delete _msg;
}

bool gricv0_3::registerHandler::waitForCommandReply(
    uint32_t transaction, std::vector<uint8_t>& reply)
{
  if (!waitForReply(static_cast<uint8_t>(transaction), kReplyTimeout, reply))
  {
    PM_ERROR(_logGricv0_3, "No command reply after " << kReplyTimeout.count()
                                                      << " ms for transaction "
                                                      << transaction);
    return false;
  }
  if (reply.size() <= gricv0_3::Message::Fmt::CMD)
  {
    PM_ERROR(_logGricv0_3, "Command reply is too short: " << reply.size()
                                                          << " bytes");
    return false;
  }
  return true;
}

uint32_t gricv0_3::registerHandler::sendCommand(uint8_t command)
{
  const std::lock_guard<std::mutex> requestLock(_requestMutex);
  const uint16_t length = 6;
  _msg->setAddress(id());
  _msg->setLength(length);
  uint16_t encodedLength = htons(length);
  std::memcpy(_msg->ptr() + gricv0_3::Message::Fmt::LEN, &encodedLength,
              sizeof(encodedLength));
  _msg->ptr()[gricv0_3::Message::Fmt::HEADER] = '(';
  _msg->ptr()[gricv0_3::Message::Fmt::CMD] = command;
  _msg->ptr()[length - 1] = ')';

  const uint32_t transaction = sendMessage(_msg, true);
  if (transaction == UINT32_MAX)
    return 0;

  std::vector<uint8_t> reply;
  if (!waitForCommandReply(transaction, reply))
    return 0;
  return reply[gricv0_3::Message::Fmt::CMD];
}

void gricv0_3::registerHandler::sendParameter(uint8_t command,
                                              uint8_t parameter)
{
  const std::lock_guard<std::mutex> requestLock(_requestMutex);
  const uint16_t length = 7;
  _msg->setAddress(id());
  _msg->setLength(length);
  uint16_t encodedLength = htons(length);
  std::memcpy(_msg->ptr() + gricv0_3::Message::Fmt::LEN, &encodedLength,
              sizeof(encodedLength));
  _msg->ptr()[gricv0_3::Message::Fmt::HEADER] = '(';
  _msg->ptr()[gricv0_3::Message::Fmt::CMD] = command;
  _msg->ptr()[gricv0_3::Message::Fmt::PAYLOAD] = parameter;
  _msg->ptr()[length - 1] = ')';

  const uint32_t transaction = sendMessage(_msg, true);
  if (transaction == UINT32_MAX)
    return;

  std::vector<uint8_t> reply;
  waitForCommandReply(transaction, reply);
}

void gricv0_3::registerHandler::sendSlowControl(uint8_t* slc,
                                                uint16_t lengthBytes)
{
  if (slc == nullptr || lengthBytes > 109)
  {
    PM_ERROR(_logGricv0_3, "Invalid slow-control payload length "
                              << lengthBytes);
    return;
  }

  const std::lock_guard<std::mutex> requestLock(_requestMutex);
  const uint16_t length = 115;
  _msg->setAddress(id());
  _msg->setLength(length);
  uint16_t encodedLength = htons(length);
  std::memcpy(_msg->ptr() + gricv0_3::Message::Fmt::LEN, &encodedLength,
              sizeof(encodedLength));
  _msg->ptr()[gricv0_3::Message::Fmt::HEADER] = '(';
  _msg->ptr()[gricv0_3::Message::Fmt::CMD] =
      gricv0_3::Message::command::STORESC;
  std::memcpy(_msg->ptr() + gricv0_3::Message::Fmt::PAYLOAD, slc, lengthBytes);
  _msg->ptr()[length - 1] = ')';

  const uint32_t transaction = sendMessage(_msg, true);
  if (transaction == UINT32_MAX)
    return;

  std::vector<uint8_t> reply;
  waitForCommandReply(transaction, reply);
}

bool gricv0_3::registerHandler::processPacket()
{
  if (_idx <= gricv0_3::Message::Fmt::CMD)
    return false;

  const uint8_t transaction = _buf[gricv0_3::Message::Fmt::TRANS];
  completeReply(transaction, _buf, _idx);
  PM_DEBUG(_logGricv0_3, sourceid() << " Command reply "
                                   << static_cast<unsigned>(
                                          _buf[gricv0_3::Message::Fmt::CMD])
                                   << " transaction "
                                   << static_cast<unsigned>(transaction)
                                   << " length " << _idx);
  return true;
}
