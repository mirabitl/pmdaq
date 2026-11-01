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
#include <chrono>
#include <cstring>




gricv1_3::registerHandler::registerHandler(std::string ip,mpi2::MpiMessageHandler* transport) : socketHandler(ip,gricv1_3::Interface::PORT::REGISTER,transport)

{
  _msg= new gricv1_3::Message();
}

void gricv1_3::registerHandler::writeRegister(uint16_t address,uint32_t value)
{
  const std::lock_guard<std::mutex> requestLock(_requestMutex);
  uint16_t len=16;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[gricv1_3::Message::Fmt::LEN]);
  _msg->ptr()[gricv1_3::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[gricv1_3::Message::Fmt::CMD]=gricv1_3::Message::command::WRITEREG;
  uint16_t radr=htons(address);uint32_t rval=htonl(value);
  memcpy(&(_msg->ptr()[gricv1_3::Message::Fmt::PAYLOAD]),&radr,2);
  memcpy(&(_msg->ptr()[gricv1_3::Message::Fmt::PAYLOAD+2]),&rval,4);
  
  _msg->ptr()[len-1]=')';    
  uint32_t tr=this->sendMessage(_msg,true);
  if (tr==UINT32_MAX)
    return;
  std::vector<uint8_t> reply;
  if (!waitForReply(static_cast<uint8_t>(tr),std::chrono::milliseconds(400),reply))
    PM_ERROR(_logGricv1,"No write-register reply after 400 ms for transaction "<<tr);
}
uint32_t gricv1_3::registerHandler::readRegister(uint16_t address)
{
  const std::lock_guard<std::mutex> requestLock(_requestMutex);
  uint16_t len=16;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[gricv1_3::Message::Fmt::LEN]);
  _msg->ptr()[gricv1_3::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[gricv1_3::Message::Fmt::CMD]=gricv1_3::Message::command::READREG;
  uint16_t radr=htons(address);
  memcpy(&(_msg->ptr()[gricv1_3::Message::Fmt::PAYLOAD]),&radr,2);
  _msg->ptr()[len-1]=')';

  uint32_t tr=this->sendMessage(_msg,true);
  if (tr==UINT32_MAX)
    return 0;
  std::vector<uint8_t> reply;
  if (!waitForReply(static_cast<uint8_t>(tr),std::chrono::milliseconds(400),reply))
  {
    PM_ERROR(_logGricv1,"No read-register reply after 400 ms for transaction "<<tr);
    return 0;
  }
  constexpr size_t valueOffset=gricv1_3::Message::Fmt::PAYLOAD+2;
  if (reply.size()<valueOffset+sizeof(uint32_t))
  {
    PM_ERROR(_logGricv1,"Read-register reply is too short: "<<reply.size()<<" bytes");
    return 0;
  }
  uint32_t value=0;
  std::memcpy(&value,reply.data()+valueOffset,sizeof(value));
  return ntohl(value);
}

bool gricv1_3::registerHandler::processPacket()
{
  uint8_t transaction=_buf[gricv1_3::Message::Fmt::TRANS];
  const uint8_t command=_buf[gricv1_3::Message::Fmt::CMD];
  if (_idx<gricv1_3::Message::Fmt::PAYLOAD ||
      command!=gricv1_3::Message::command::ACKNOWLEDGE)
    return true;

  completeReply(transaction,_buf,_idx);
  return true;
}
