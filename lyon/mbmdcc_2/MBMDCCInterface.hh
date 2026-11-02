#pragma once

#include "SocketHandler.hh"
#include "stdafx.hh"
#include "utils.hh"

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#define MAX_BUFFER_LEN 0x4000
#define MBSIZE 0x40000

static LoggerPtr _logMbmdcc2(Logger::getLogger("PMDAQ_MBMDCC_2"));

namespace mbmdcc_2
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
      PAYLOAD = 6
    };

    enum command
    {
      WRITEREG = 1,
      READREG = 2,
      SLC = 4,
      DATA = 8,
      ACKNOWLEDGE = 16,
      ERROR = 32
    };

    enum Register
    {
      TEST = 0x0,
      ID = 0x1,
      MASK = 0x2,
      SPILL_CNT = 0x3,
      ACQ_CTRL = 0x4,
      SPILL_ON = 0x5,
      SPILL_OFF = 0x6,
      CALIB_CTRL = 0x8,
      CALIB_NWIN = 0xA,
      RESET_FE = 0xC,
      WIN_CTRL = 0xD,
      TRG_EXT_DELAY = 0xE,
      TRG_EXT_LEN = 0xF,
      TRG_EXT_CONFIG = 0x10,
      TRG_EXT_COUNT = 0x11,
      CLK_ENABLE = 0x1D,
      MIN_BUSY_LENGTH = 0x20,
      CHANNEL_ENABLE = 0x21,
      TRG_EXT_NB = 0x22,
      EN_BUSY_TRG = 0x20,
      DEBOUNCE_BUSY = 0x21,
      TDC_CTRL = 0x30,
      TDC_COARSE = 0x31,
      TDC_T1 = 0x32,
      TDC_CNT1 = 0x33,
      TDC_T2 = 0x34,
      TDC_CNT2 = 0x35,
      TDC_T3 = 0x36,
      TDC_CNT3 = 0x37,
      TDC_T4 = 0x38,
      TDC_CNT4 = 0x39,
      TDC_T5 = 0x3A,
      TDC_CNT5 = 0x3B,
      TDC_T6 = 0x3C,
      TDC_CNT6 = 0x3D,
      TDC_CAL1 = 0x40,
      TDC_CAL2 = 0x41,
      BUSY_0 = 0x50,
      RESET_FSM = 0x60,
      SPS_SPILL_DURATION = 0x70,
      SPS_SPILL_CTRL = 0x71,
      LEMO_MASK = 0xA0,
      RESYNC_MASK = 0x200,
      VERSION = 0x100
    };

    Message() : _address(0), _length(2) {}
    uint64_t address() const { return _address; }
    uint16_t length() const { return _length; }
    uint8_t* ptr() { return _buf; }
    void setLength(uint16_t length) { _length = length; }
    void setAddress(uint64_t address) { _address = address; }
    void setAddress(const std::string& address, uint16_t port)
    {
      _address = (static_cast<uint64_t>(utils::convertIP(address)) << 32) | port;
    }

  private:
    uint64_t _address;
    uint16_t _length;
    uint8_t _buf[MAX_BUFFER_LEN];
  };

  class Interface
  {
  public:
    enum PORT
    {
      REGISTER = 10002
    };

    Interface();
    ~Interface();
    void initialise();
    void addDevice(const std::string& address);
    void listen();
    void terminate();
    std::map<std::string, board*>& boards() { return _boards; }
    board* getBoard(const std::string& ip) { return _boards[ip]; }
    void close();

  private:
    std::map<std::string, board*> _boards;
    bool _running;
  };

  using socketHandler = code_libhv::SocketHandler;

  class registerHandler : public socketHandler
  {
  public:
    explicit registerHandler(const std::string& ip);
    ~registerHandler() override;
    bool processPacket() override;
    void writeRegister(uint16_t address, uint32_t value);
    uint32_t readRegister(uint16_t address);

  private:
    Message* _msg;
    std::mutex _requestMutex;
  };

  class board
  {
  public:
    explicit board(const std::string& ip);
    ~board();
    registerHandler* reg() { return _regh; }
    std::string ipAddress() const { return _ip; }

  private:
    std::string _ip;
    registerHandler* _regh;
  };
}
