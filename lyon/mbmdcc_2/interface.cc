#include "MBMDCCInterface.hh"

#include <stdexcept>
#include <utility>

mbmdcc_2::Interface::Interface() : _running(false) {}

mbmdcc_2::Interface::~Interface()
{
  terminate();
  for (auto& entry : _boards)
    delete entry.second;
}

void mbmdcc_2::Interface::initialise()
{
  if (!_boards.empty())
    throw std::logic_error("mbmdcc_2 interface is already initialised");
  _running = false;
}

void mbmdcc_2::Interface::addDevice(const std::string& address)
{
  if (_boards.find(address) != _boards.end())
    return;

  PM_INFO(_logMbmdcc2, "Creating MBMDCC board at address " << address);
  auto* device = new mbmdcc_2::board(address);
  _boards.emplace(address, device);

  const auto connect = [&address](mbmdcc_2::socketHandler* handler) {
    const int socket = handler->connect();
    if (socket < 0)
      throw std::runtime_error("Cannot create libhv socket for " + address +
                               ":" +
                               std::to_string(handler->id() & 0xFFFF));
  };

  try
  {
    connect(device->reg());
    if (!device->reg()->waitForConnection(std::chrono::seconds(10)))
      throw std::runtime_error("Timed out connecting to " + address + ":" +
                               std::to_string(device->reg()->id() & 0xFFFF));
  }
  catch (...)
  {
    _boards.erase(address);
    delete device;
    throw;
  }
}

void mbmdcc_2::Interface::listen()
{
  _running = true;
}

void mbmdcc_2::Interface::terminate()
{
  _running = false;
  close();
}

void mbmdcc_2::Interface::close()
{
  for (auto& entry : _boards)
    entry.second->reg()->disconnect();
}
