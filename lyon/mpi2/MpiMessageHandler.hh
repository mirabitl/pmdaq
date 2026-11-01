#pragma once

#include "MessageHandler.hh"

#include <hv/TcpClient.h>

#ifdef __FILENAME__
#undef __FILENAME__
#endif

#include <cstddef>
#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace mpi2 {

class MpiMessageHandler : public MessageHandler {
public:
  MpiMessageHandler() = default;
  ~MpiMessageHandler() override;

  MpiMessageHandler(const MpiMessageHandler&) = delete;
  MpiMessageHandler& operator=(const MpiMessageHandler&) = delete;

  static uint64_t endpointId(const std::string& ipv4, uint16_t port);

  void addHandler(uint64_t id, MPIFunctor handler);
  int connect(const std::string& ipv4, uint16_t port);
  bool waitForConnection(uint64_t id, std::chrono::milliseconds timeout);
  int send(uint64_t id, const void* data, size_t length);
  void disconnect(uint64_t id);
  void close();

  void processMessage(uint64_t id, uint16_t length, char* data) override;

private:
  std::mutex _mutex;
  std::condition_variable _connectionChanged;
  std::map<uint64_t, MPIFunctor> _handlers;
  std::map<uint64_t, std::shared_ptr<hv::TcpClient>> _clients;
};

}  // namespace mpi2
