#include "Gricv1Interface.hh"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <map>
#include <bitset>
#include <boost/format.hpp>
#include <arpa/inet.h>
#include <cstring>




gricv1_2::registerHandler::registerHandler(std::string ip)
    : socketHandler(ip, gricv1_2::Interface::PORT::REGISTER),
      _slcStatus(0),
      _msg(new gricv1_2::Message())
{
}

gricv1_2::registerHandler::~registerHandler()
{
  disconnect();
  delete _msg;
}

void gricv1_2::registerHandler::writeRegister(uint16_t address,uint32_t value)
{
  const std::lock_guard<std::mutex> requestLock(_requestMutex);
  uint16_t len=16;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[gricv1_2::Message::Fmt::LEN]);
  _msg->ptr()[gricv1_2::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[gricv1_2::Message::Fmt::CMD]=gricv1_2::Message::command::WRITEREG;
  uint16_t radr=htons(address);uint32_t rval=htonl(value);
  memcpy(&(_msg->ptr()[gricv1_2::Message::Fmt::PAYLOAD]),&radr,2);
  memcpy(&(_msg->ptr()[gricv1_2::Message::Fmt::PAYLOAD+2]),&rval,4);
  
  _msg->ptr()[len-1]=')';    
  const uint32_t transaction = sendMessage(_msg, true);
  if (transaction == UINT32_MAX)
    return;
  std::vector<uint8_t> reply;
  if (!waitForReply(static_cast<uint8_t>(transaction),
                    std::chrono::milliseconds(400), reply))
    PM_ERROR(_logGricv1, "No write-register reply after 400 ms for "
                             << "transaction " << transaction);
}
uint32_t gricv1_2::registerHandler::readRegister(uint16_t address)
{
  const std::lock_guard<std::mutex> requestLock(_requestMutex);
  uint16_t len=16;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[gricv1_2::Message::Fmt::LEN]);
  _msg->ptr()[gricv1_2::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[gricv1_2::Message::Fmt::CMD]=gricv1_2::Message::command::READREG;
  uint16_t radr=htons(address);
  memcpy(&(_msg->ptr()[gricv1_2::Message::Fmt::PAYLOAD]),&radr,2);
  _msg->ptr()[len-1]=')';

  const uint32_t transaction = sendMessage(_msg, true);
  if (transaction == UINT32_MAX)
    return 0;
  std::vector<uint8_t> reply;
  if (!waitForReply(static_cast<uint8_t>(transaction),
                    std::chrono::milliseconds(400), reply))
  {
    PM_ERROR(_logGricv1, "No read-register reply after 400 ms for "
                             << "transaction " << transaction);
    return 0;
  }
  constexpr size_t valueOffset =
      gricv1_2::Message::Fmt::PAYLOAD + sizeof(uint16_t);
  if (reply.size() < valueOffset + sizeof(uint32_t))
  {
    PM_ERROR(_logGricv1, "Read-register reply is too short: "
                             << reply.size() << " bytes");
    return 0;
  }
  uint32_t value = 0;
  std::memcpy(&value, reply.data() + valueOffset, sizeof(value));
  return ntohl(value);
}

bool gricv1_2::registerHandler::processPacket()
{
  if (_idx < gricv1_2::Message::Fmt::PAYLOAD ||
      _buf[gricv1_2::Message::Fmt::CMD] !=
          gricv1_2::Message::command::ACKNOWLEDGE)
    return true;

  const uint8_t transaction = _buf[gricv1_2::Message::Fmt::TRANS];
  PM_DEBUG(_logGricv1, sourceid() << " Register acknowledgement transaction "
                                  << static_cast<unsigned>(transaction)
                                  << " length " << _idx);
  completeReply(transaction, _buf, _idx);

#define DUMPREGREPN
#ifdef DUMPREGREP
  fprintf(stderr,"\n REGISTER RC ==> ");
  for (int i=0;i<_idx-1;i++)
    {
      fprintf(stderr,"%.2x ",(_buf[i]));
         
      if (i%16==15)
	{
	  fprintf(stderr,"\n REGISTER RC ==> ");
	}
    }
  fprintf(stderr,"\n");
#endif
  return true;
}
