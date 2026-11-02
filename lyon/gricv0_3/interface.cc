#include "Gricv0_3Interface.hh"

#include <stdexcept>

gricv0_3::Interface::Interface() : _running(false) {}

gricv0_3::Interface::~Interface()
{
  terminate();
  for (auto& entry : _boards)
    delete entry.second;
}

void gricv0_3::Interface::initialise()
{
  if (!_boards.empty())
    throw std::logic_error("gricv0_3 interface is already initialised");
  _running = false;
}

void gricv0_3::Interface::addDevice(std::string address)
{
  if (_boards.find(address) != _boards.end())
    return;

  PM_INFO(_logGricv0_3, "Creating board at address " << address);
  auto* device = new gricv0_3::board(address);
  _boards.emplace(address, device);

  const auto connect = [&address](gricv0_3::socketHandler* handler) {
    const int socket = handler->connect();
    if (socket < 0)
      throw std::runtime_error("Cannot create libhv socket for " + address +
                               ":" + std::to_string(handler->id() & 0xFFFF));
  };

  try
  {
    connect(device->reg());
    connect(device->sensor());
    connect(device->data());

    const auto timeout = std::chrono::seconds(10);
    for (auto* handler :
         {static_cast<gricv0_3::socketHandler*>(device->reg()),
          static_cast<gricv0_3::socketHandler*>(device->sensor()),
          static_cast<gricv0_3::socketHandler*>(device->data())})
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

void gricv0_3::Interface::listen()
{
  _running = true;
}

void gricv0_3::Interface::terminate()
{
  _running = false;
  close();
}

void gricv0_3::Interface::close()
{
  for (auto& entry : _boards)
  {
    entry.second->reg()->disconnect();
    entry.second->sensor()->disconnect();
    entry.second->data()->disconnect();
  }
}
