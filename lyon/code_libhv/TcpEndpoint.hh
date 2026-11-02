#pragma once

#include <hv/TcpClient.h>

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace code_libhv
{

class TcpEndpoint
{
public:
  using ReceiveHandler = std::function<void(uint64_t, uint16_t, char*)>;

  TcpEndpoint(std::string address, uint16_t port, uint64_t id,
              ReceiveHandler receiveHandler);
  ~TcpEndpoint();

  TcpEndpoint(const TcpEndpoint&) = delete;
  TcpEndpoint& operator=(const TcpEndpoint&) = delete;

  int connect();
  bool waitForConnection(std::chrono::milliseconds timeout);
  int send(const void* data, size_t length);
  void disconnect();

private:
  std::string _address;
  uint16_t _port;
  uint64_t _id;
  ReceiveHandler _receiveHandler;
  std::shared_ptr<hv::TcpClient> _client;
  std::mutex _mutex;
  std::condition_variable _connectionChanged;
};

}  // namespace code_libhv
