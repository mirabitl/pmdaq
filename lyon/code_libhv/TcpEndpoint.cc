#include "TcpEndpoint.hh"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

code_libhv::TcpEndpoint::TcpEndpoint(std::string address, uint16_t port,
                                     uint64_t id,
                                     ReceiveHandler receiveHandler)
    : _address(std::move(address)),
      _port(port),
      _id(id),
      _receiveHandler(std::move(receiveHandler))
{
  if (!_receiveHandler)
    throw std::invalid_argument("libhv endpoint requires a receive handler");
}

code_libhv::TcpEndpoint::~TcpEndpoint()
{
  disconnect();
}

int code_libhv::TcpEndpoint::connect()
{
  std::lock_guard<std::mutex> lock(_mutex);
  if (_client)
    return -1;

  auto client = std::make_shared<hv::TcpClient>();
  const int socket = client->createsocket(_port, _address.c_str());
  if (socket < 0)
    return socket;

  client->onConnection = [this](const hv::SocketChannelPtr& channel) {
    if (channel->isConnected())
      _connectionChanged.notify_all();
  };
  client->onMessage = [this](const hv::SocketChannelPtr&, hv::Buffer* buffer) {
    const size_t size = buffer->size();
    auto* data = static_cast<char*>(buffer->data());
    size_t offset = 0;
    while (offset < size)
    {
      const size_t chunk =
          std::min(size - offset,
                   static_cast<size_t>(std::numeric_limits<uint16_t>::max()));
      _receiveHandler(_id, static_cast<uint16_t>(chunk), data + offset);
      offset += chunk;
    }
  };

  _client = std::move(client);
  _client->start();
  return socket;
}

bool code_libhv::TcpEndpoint::waitForConnection(
    std::chrono::milliseconds timeout)
{
  std::unique_lock<std::mutex> lock(_mutex);
  return _connectionChanged.wait_for(lock, timeout, [this] {
    return _client && _client->isConnected();
  });
}

int code_libhv::TcpEndpoint::send(const void* data, size_t length)
{
  if (data == nullptr || length == 0 ||
      length > static_cast<size_t>(std::numeric_limits<int>::max()))
    return -1;

  std::shared_ptr<hv::TcpClient> client;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    client = _client;
  }
  if (!client || !client->isConnected())
    return -1;
  return client->send(data, static_cast<int>(length));
}

void code_libhv::TcpEndpoint::disconnect()
{
  std::shared_ptr<hv::TcpClient> client;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    client = std::move(_client);
  }
  if (client)
    client->stop(true);
}
