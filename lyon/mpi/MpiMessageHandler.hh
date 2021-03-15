#pragma once
#include "MessageHandler.hh"


#include <map>
#include <string>
typedef std::pair<uint32_t,unsigned char*> ptrBuf;
typedef std::function<void (uint64_t,uint16_t,char*)> MPIFunctor;

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
  };
};

