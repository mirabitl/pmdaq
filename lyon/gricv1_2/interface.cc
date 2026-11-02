#include "Gricv1Interface.hh"

#include <chrono>
#include <stdexcept>
#include <utility>

gricv1_2::Interface::Interface() : _running(false) {}

gricv1_2::Interface::~Interface()
{
  terminate();
  for (auto& entry : _boards)
    delete entry.second;
}

void gricv1_2::Interface::initialise()
{
  if (!_boards.empty())
    throw std::logic_error("gricv1_2 interface is already initialised");
  _running = false;
}

void gricv1_2::Interface::addDevice(std::string address)
{
  if (_boards.find(address) != _boards.end())
    return;

  PM_INFO(_logGricv1,"Creating board at address "<<address);
  auto* device = new gricv1_2::board(address);
  _boards.emplace(address, device);

  const auto connect = [&address](gricv1_2::socketHandler* handler) {
    const int socket = handler->connect();
    if (socket < 0)
      throw std::runtime_error("Cannot create libhv socket for " + address +
                               ":" + std::to_string(handler->id() & 0xFFFF));
  };

  try
  {
    connect(device->reg());
    connect(device->slc());
    connect(device->data());

    constexpr auto timeout = std::chrono::seconds(10);
    for (auto* handler : {static_cast<gricv1_2::socketHandler*>(device->reg()),
                          static_cast<gricv1_2::socketHandler*>(device->slc()),
                          static_cast<gricv1_2::socketHandler*>(device->data())})
    {
      if (!handler->waitForConnection(timeout))
        throw std::runtime_error("Timed out connecting to " + address + ":" +
                                 std::to_string(handler->id() & 0xFFFF));
    }
  }
  catch (...)
  {
    _boards.erase(address);
    delete device;
    throw;
  }
}

void gricv1_2::Interface::listen()
{
  _running = true;
}

void gricv1_2::Interface::terminate()
{
  _running = false;
  close();
}

void gricv1_2::Interface::close()
{
  for (auto& entry : _boards)
  {
    entry.second->reg()->disconnect();
    entry.second->slc()->disconnect();
    entry.second->data()->disconnect();
  }
}
