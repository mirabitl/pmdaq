#pragma once
#include "MessageHandler.hh"
#include <mutex>

#include <map>
#include <string>

namespace mpi
{
class MpiMessageHandler : public mpi::MessageHandler
  {
  public:
    MpiMessageHandler(std::string directory);
    virtual void processMessage(NL::Socket* socket);// throw (mpi::MpiException);
    void addHandler(uint64_t id,MPIFunctor f);
    void removeSocket(NL::Socket* socket);

  private:
    std::string _storeDir;
    std::map<uint64_t, ptrBuf> _sockMap;
    std::map<uint64_t,MPIFunctor> _handlers;
    uint64_t _npacket;
    std::mutex _sem;
  };
};

