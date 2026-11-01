#include "MpiMessageHandler.hh"

#include <arpa/inet.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mpi2 {

MpiMessageHandler::~MpiMessageHandler() {
  close();
}

uint64_t MpiMessageHandler::endpointId(const std::string& ipv4, uint16_t port) {
  in_addr address{};
  if (inet_pton(AF_INET, ipv4.c_str(), &address) != 1) {
    throw std::invalid_argument("mpi2 requires a valid IPv4 address: " + ipv4);
  }
  return (static_cast<uint64_t>(address.s_addr) << 32) | port;
}

void MpiMessageHandler::addHandler(uint64_t id, MPIFunctor handler) {
  if (!handler) {
    throw std::invalid_argument("mpi2 handler must be callable");
  }
  std::lock_guard<std::mutex> lock(_mutex);
  _handlers[id] = std::move(handler);
}

int MpiMessageHandler::connect(const std::string& ipv4, uint16_t port) {
  const uint64_t id = endpointId(ipv4, port);
  std::shared_ptr<hv::TcpClient> client;
  int socket = -1;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_clients.find(id) != _clients.end()) {
      return -1;
    }

    client = std::make_shared<hv::TcpClient>();
    socket = client->createsocket(port, ipv4.c_str());
    if (socket < 0) {
      return socket;
    }

    _clients.emplace(id, client);
    client->onConnection = [this](const hv::SocketChannelPtr& channel) {
      if (channel->isConnected()) {
        _connectionChanged.notify_all();
      }
    };
    client->onMessage = [this, id](const hv::SocketChannelPtr&, hv::Buffer* buffer) {
      size_t offset = 0;
      const size_t size = buffer->size();
      auto* data = static_cast<char*>(buffer->data());
      while (offset < size) {
        const size_t chunk = std::min(
            size - offset, static_cast<size_t>(std::numeric_limits<uint16_t>::max()));
        processMessage(id, static_cast<uint16_t>(chunk), data + offset);
        offset += chunk;
      }
    };
    client->start();
  }
  return socket;
}

bool MpiMessageHandler::waitForConnection(uint64_t id,
                                          std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock(_mutex);
  const auto connected = [this, id] {
    const auto it = _clients.find(id);
    return it != _clients.end() && it->second->isConnected();
  };
  return _connectionChanged.wait_for(lock, timeout, connected);
}

int MpiMessageHandler::send(uint64_t id, const void* data, size_t length) {
  if (data == nullptr || length == 0 ||
      length > static_cast<size_t>(std::numeric_limits<int>::max())) {
    return -1;
  }

  std::shared_ptr<hv::TcpClient> client;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    const auto it = _clients.find(id);
    if (it == _clients.end()) {
      return -1;
    }
    client = it->second;
  }
  if (!client->isConnected()) {
    return -1;
  }
  return client->send(data, static_cast<int>(length));
}

void MpiMessageHandler::disconnect(uint64_t id) {
  std::shared_ptr<hv::TcpClient> client;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    const auto it = _clients.find(id);
    if (it == _clients.end()) {
      return;
    }
    client = std::move(it->second);
    _clients.erase(it);
  }
  client->stop(true);
}

void MpiMessageHandler::close() {
  std::vector<std::shared_ptr<hv::TcpClient>> clients;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto& entry : _clients) {
      clients.push_back(std::move(entry.second));
    }
    _clients.clear();
  }
  for (const auto& client : clients) {
    client->stop(true);
  }
}

void MpiMessageHandler::processMessage(uint64_t id, uint16_t length, char* data) {
  MPIFunctor handler;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    const auto it = _handlers.find(id);
    if (it == _handlers.end()) {
      std::cerr << "mpi2: no message handler registered for endpoint 0x"
                << std::hex << id << std::dec << '\n';
      return;
    }
    handler = it->second;
  }
  handler(id, length, data);
}

}  // namespace mpi2
