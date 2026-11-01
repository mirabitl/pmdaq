#include "Gricv1Interface.hh"

#include <chrono>
#include <stdexcept>
#include <utility>

gricv1_3::Interface::Interface() : _running(false) {}

gricv1_3::Interface::~Interface()
{
  terminate();
  for (auto& entry : _boards)
    delete entry.second;
}

void gricv1_3::Interface::initialise()
{
  if (!_boards.empty())
    throw std::logic_error("gricv1_3 interface is already initialised");
  _running = false;
}

void gricv1_3::Interface::addDevice(std::string address)
{
  if (_boards.find(address) != _boards.end())
    return;

  PM_INFO(_logGricv1,"Creating board at address "<<address);
  auto* device = new gricv1_3::board(address, &_transport);
  _boards.emplace(address, device);

  const auto bindAndConnect = [this, &address](gricv1_3::socketHandler* handler,
                                                uint16_t port) {
    _transport.addHandler(
        handler->id(),
        std::bind(&gricv1_3::socketHandler::processBuffer, handler,
                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    const int socket = _transport.connect(address, port);
    if (socket < 0)
      throw std::runtime_error("Cannot create libhv socket for " + address + ":" +
                               std::to_string(port));
  };

  try
  {
    bindAndConnect(device->reg(),gricv1_3::Interface::PORT::REGISTER);
    bindAndConnect(device->slc(),gricv1_3::Interface::PORT::SLC);
    bindAndConnect(device->data(),gricv1_3::Interface::PORT::DATA);

    constexpr auto timeout = std::chrono::seconds(10);
    for (auto* handler : {static_cast<gricv1_3::socketHandler*>(device->reg()),
                          static_cast<gricv1_3::socketHandler*>(device->slc()),
                          static_cast<gricv1_3::socketHandler*>(device->data())})
    {
      if (!_transport.waitForConnection(handler->id(), timeout))
        throw std::runtime_error("Timed out connecting to " + address + ":" +
                                 std::to_string(handler->id() & 0xFFFF));
    }
  }
  catch (...)
  {
    _transport.close();
    throw;
  }
}

void gricv1_3::Interface::listen()
{
  _running = true;
}

void gricv1_3::Interface::terminate()
{
  _running = false;
  close();
}

void gricv1_3::Interface::close()
{
  _transport.close();
}
