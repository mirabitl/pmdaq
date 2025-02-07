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




tricv1::slcHandler::slcHandler(std::string ip) : socketHandler(ip,tricv1::PORT::SLC)
{
  _msg=new wizcc::Message();
}

void tricv1::slcHandler::sendSlowControl(uint8_t* slc,uint16_t lenbytes)
{
  uint16_t hrlen=lenbytes;
  uint16_t cpl32bit=4-hrlen%4;
  uint16_t len=hrlen+cpl32bit+wizcc::Message::Fmt::PAYLOAD+1;
  // Hardcode dasn le firmware
  hrlen=109;
  len=118;
  _msg->setAddress(id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[wizcc::Message::Fmt::LEN]);
  _msg->ptr()[wizcc::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[wizcc::Message::Fmt::CMD]=wizcc::Message::command::SLC;
  memcpy(&(_msg->ptr()[wizcc::Message::Fmt::PAYLOAD]),slc,hrlen);
  _msg->ptr()[len-1]=')';
  uint32_t tr=this->send_message(_msg);
}


bool tricv1::slcHandler::process_message()
{
  return true;
}
