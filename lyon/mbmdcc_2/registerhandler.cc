#include "MBMDCCInterface.hh"

#include <arpa/inet.h>

#include <chrono>
#include <cstring>

mbmdcc_2::registerHandler::registerHandler(
    const std::string& ip)
    : socketHandler(ip, mbmdcc_2::Interface::PORT::REGISTER),
      _msg(new mbmdcc_2::Message())
{
}

mbmdcc_2::registerHandler::~registerHandler()
{
  disconnect();
  delete _msg;
}

void mbmdcc_2::registerHandler::writeRegister(uint16_t address,
                                               uint32_t value)
{
  const std::lock_guard<std::mutex> requestLock(_requestMutex);
  constexpr uint16_t length = 16;
  _msg->setAddress(id());
  _msg->setLength(length);
  uint16_t encodedLength = htons(length);
  std::memcpy(_msg->ptr() + mbmdcc_2::Message::Fmt::LEN, &encodedLength,
              sizeof(encodedLength));
  _msg->ptr()[mbmdcc_2::Message::Fmt::HEADER] = '(';
  _msg->ptr()[mbmdcc_2::Message::Fmt::CMD] =
      mbmdcc_2::Message::command::WRITEREG;
  const uint16_t encodedAddress = htons(address);
  const uint32_t encodedValue = htonl(value);
  std::memcpy(_msg->ptr() + mbmdcc_2::Message::Fmt::PAYLOAD, &encodedAddress,
              sizeof(encodedAddress));
  std::memcpy(_msg->ptr() + mbmdcc_2::Message::Fmt::PAYLOAD +
                  sizeof(encodedAddress),
              &encodedValue, sizeof(encodedValue));
  _msg->ptr()[length - 1] = ')';

  const uint32_t transaction = sendMessage(_msg, true);
  if (transaction == UINT32_MAX)
    return;

  std::vector<uint8_t> reply;
  if (!waitForReply(static_cast<uint8_t>(transaction),
                    std::chrono::milliseconds(400), reply))
    PM_ERROR(_logMbmdcc2, "No write-register reply after 400 ms for "
                              << "transaction " << transaction);
}

uint32_t mbmdcc_2::registerHandler::readRegister(uint16_t address)
{
  const std::lock_guard<std::mutex> requestLock(_requestMutex);
  constexpr uint16_t length = 16;
  _msg->setAddress(id());
  _msg->setLength(length);
  uint16_t encodedLength = htons(length);
  std::memcpy(_msg->ptr() + mbmdcc_2::Message::Fmt::LEN, &encodedLength,
              sizeof(encodedLength));
  _msg->ptr()[mbmdcc_2::Message::Fmt::HEADER] = '(';
  _msg->ptr()[mbmdcc_2::Message::Fmt::CMD] =
      mbmdcc_2::Message::command::READREG;
  const uint16_t encodedAddress = htons(address);
  std::memcpy(_msg->ptr() + mbmdcc_2::Message::Fmt::PAYLOAD, &encodedAddress,
              sizeof(encodedAddress));
  _msg->ptr()[length - 1] = ')';

  const uint32_t transaction = sendMessage(_msg, true);
  if (transaction == UINT32_MAX)
    return 0;

  std::vector<uint8_t> reply;
  if (!waitForReply(static_cast<uint8_t>(transaction),
                    std::chrono::milliseconds(400), reply))
  {
    PM_ERROR(_logMbmdcc2, "No read-register reply after 400 ms for "
                              << "transaction " << transaction);
    return 0;
  }

  constexpr size_t valueOffset =
      mbmdcc_2::Message::Fmt::PAYLOAD + sizeof(uint16_t);
  if (reply.size() < valueOffset + sizeof(uint32_t))
  {
    PM_ERROR(_logMbmdcc2, "Read-register reply is too short: "
                              << reply.size() << " bytes");
    return 0;
  }

  uint32_t value = 0;
  std::memcpy(&value, reply.data() + valueOffset, sizeof(value));
  return ntohl(value);
}

bool mbmdcc_2::registerHandler::processPacket()
{
  if (_idx < mbmdcc_2::Message::Fmt::PAYLOAD ||
      _buf[mbmdcc_2::Message::Fmt::CMD] !=
          mbmdcc_2::Message::command::ACKNOWLEDGE)
    return true;

  completeReply(_buf[mbmdcc_2::Message::Fmt::TRANS], _buf, _idx);
  return true;
}
