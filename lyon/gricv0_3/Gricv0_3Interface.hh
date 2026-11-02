#pragma once

#include "SocketHandler.hh"

#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "stdafx.hh"
#include "utils.hh"
#include "pmSender.hh"

#define MAX_BUFFER_LEN 0x4000
#define MBSIZE 0x40000

static LoggerPtr _logGricv0_3(Logger::getLogger("PMDAQ_GRICV0_3"));

namespace gricv0_3
{
  class board;

  class Message
  {
  public:
    enum Fmt
    {
      HEADER = 0,
      LEN = 1,
      TRANS = 3,
      CMD = 4,
      PAYLOAD = 5
    };

    enum command
    {
      STARTACQ = 0,
      STOPACQ = 1,
      RESET = 2,
      LASTABCID = 3,
      LASTGTC = 4,
      READSC = 5,
      LOADSC = 6,
      STORESC = 7,
      CLOSE = 10,
      PULSE = 11
    };

    Message() : _address(0), _length(2) {}
    uint64_t address() const { return _address; }
    uint16_t length() const { return _length; }
    uint8_t* ptr() { return _buf; }
    void setLength(uint16_t length) { _length = length; }
    void setAddress(uint64_t address) { _address = address; }
    void setAddress(const std::string& address, uint16_t port)
    {
      _address =
          (static_cast<uint64_t>(utils::convertIP(address)) << 32) | port;
    }

  private:
    uint64_t _address;
    uint16_t _length;
    uint8_t _buf[MAX_BUFFER_LEN];
  };

  using socketHandler = code_libhv::SocketHandler;

  class Interface
  {
  public:
    enum PORT
    {
      CTRL = 9760,
      DATA = 9761,
      SENSOR = 9762
    };

    Interface();
    ~Interface();
    void initialise();
    void terminate();
    void addDevice(std::string address);
    void listen();
    std::map<std::string, board*>& boards() { return _boards; }
    board* getBoard(const std::string& ip) { return _boards[ip]; }
    void close();

  private:
    std::map<std::string, board*> _boards;
    bool _running;
  };

  class registerHandler : public socketHandler
  {
  public:
    explicit registerHandler(std::string ip);
    ~registerHandler() override;
    bool processPacket() override;
    uint32_t sendCommand(uint8_t command);
    void sendParameter(uint8_t command, uint8_t parameter);
    void sendSlowControl(uint8_t* slc, uint16_t lengthBytes = 109);
    uint32_t slcStatus() const { return _slcStatus; }
    void setSlcStatus(uint32_t status) { _slcStatus = status; }

  private:
    bool waitForCommandReply(uint32_t transaction, std::vector<uint8_t>& reply);
    uint32_t _slcStatus;
    Message* _msg;
    std::mutex _requestMutex;
  };

  class sensorHandler : public socketHandler
  {
  public:
    explicit sensorHandler(std::string ip);
    ~sensorHandler() override;
    bool processPacket() override;
  };

  class dataHandler : public socketHandler
  {
  public:
    explicit dataHandler(std::string ip);
    ~dataHandler() override;
    bool processPacket() override;
    void setTriggerId(uint8_t triggerId) { _triggerId = triggerId; }
    uint32_t detectorId() const { return _detId; }
    uint32_t difId() const { return (id() >> 48) & 0xFFFF; }
    uint64_t abcid() const { return _lastABCID; }
    uint32_t gtc() const { return _lastGTC; }
    uint32_t packets() const { return _nProcessed; }
    uint32_t event() const { return _event; }
    uint32_t triggers() const { return _ntrg; }
    void clear();
    void connect(zmq::context_t* context, std::string destination);
    void autoRegister(zmq::context_t* context, std::string session,
                      std::string appName, std::string portName);

  private:
    uint64_t _lastABCID;
    uint32_t _lastGTC;
    uint32_t _lastBCID;
    uint32_t _event;
    uint32_t _detid;
    uint32_t _ntrg;
    uint32_t _expectedLength;
    uint32_t _nProcessed;
    pm::pmSender* _dsData;
    uint8_t _triggerId;
    uint32_t _detId;
  };

  class board
  {
  public:
    explicit board(std::string ip);
    ~board();
    registerHandler* reg() { return _regh; }
    sensorHandler* sensor() { return _sensorh; }
    dataHandler* data() { return _datah; }
    std::string ipAddress() const { return _ip; }

  private:
    std::string _ip;
    registerHandler* _regh;
    sensorHandler* _sensorh;
    dataHandler* _datah;
  };
}
