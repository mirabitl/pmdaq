#include "Wtricv1Interface.hh"
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




wtricv1::registerHandler::registerHandler(std::string ip) : socketProcessor(ip,wtricv1::PORT::REGISTER),_noTransReply(false)

{
  _msg= new wizcc::Message();
}

void wtricv1::registerHandler::writeRegister(uint16_t address,uint32_t value)
{
  uint16_t len=16;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[wizcc::Message::Fmt::LEN]);
  _msg->ptr()[wizcc::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[wizcc::Message::Fmt::CMD]=wizcc::Message::command::WRITEREG;
  uint16_t radr=htons(address);uint32_t rval=htonl(value);
  memcpy(&(_msg->ptr()[wizcc::Message::Fmt::PAYLOAD]),&radr,2);
  memcpy(&(_msg->ptr()[wizcc::Message::Fmt::PAYLOAD+2]),&rval,4);
  
  _msg->ptr()[len-1]=')';    
  int32_t tr=this->send_message(_msg);
}
uint32_t wtricv1::registerHandler::readRegister(uint16_t address)
{
  uint16_t len=16;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[wizcc::Message::Fmt::LEN]);
  _msg->ptr()[wizcc::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[wizcc::Message::Fmt::CMD]=wizcc::Message::command::READREG;
  uint16_t radr=htons(address);
  memcpy(&(_msg->ptr()[wizcc::Message::Fmt::PAYLOAD]),&radr,2);
  _msg->ptr()[len-1]=')';

  // // For the moment the reply is in transaction 0
  // // Clear it first, to be suppressed asap Xtof return the transaction #
  // uint8_t* repa=this->answer(0);
  // memset(repa,0,0x4000);  
  int32_t tr=this->send_message(_msg);
  uint32_t rep=0;
  if (!_noTransReply &&tr>=0)
    {
      memcpy(&rep,&(this->answer(tr)[wizcc::Message::Fmt::PAYLOAD+2]),sizeof(uint32_t));
    }

  return ntohl(rep);
}


bool wtricv1::registerHandler::process_message()
{
  return true;
}
