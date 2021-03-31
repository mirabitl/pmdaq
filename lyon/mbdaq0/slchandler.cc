#include "Mbdaq0Interface.hh"
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




mbdaq0::slcHandler::slcHandler(std::string ip) : socketHandler(ip,mbdaq0::Interface::PORT::SLC)
{
  _msg=new mbdaq0::Message();
}

void mbdaq0::slcHandler::sendSlowControl(uint8_t* slc,uint16_t lenbytes)
{
  uint16_t hrlen=lenbytes;
  uint16_t cpl32bit=4-hrlen%4;
  uint16_t len=hrlen+cpl32bit+mbdaq0::Message::Fmt::PAYLOAD+1;
  // Hardcode dasn le firmware
  hrlen=109;
  len=118;
  _msg->setAddress(id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[mbdaq0::Message::Fmt::LEN]);
  _msg->ptr()[mbdaq0::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[mbdaq0::Message::Fmt::CMD]=mbdaq0::Message::command::SLC;
  memcpy(&(_msg->ptr()[mbdaq0::Message::Fmt::PAYLOAD]),slc,hrlen);
  _msg->ptr()[len-1]=')';
  uint32_t tr=this->sendMessage(_msg);
}


bool mbdaq0::slcHandler::processPacket()
{
  uint16_t* _sBuf= (uint16_t*) &_buf[mbdaq0::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t transaction=_buf[mbdaq0::Message::Fmt::TRANS];

  
  PM_INFO(_logMbdaq0,this->sourceid()<<"SLC data answer="<<transaction<<" length="<<length);
#undef DEBUGSLCPACKET
#ifdef DEBUGSLCPACKET
  fprintf(stderr,"\nSLC RC ==> ");
  for (int i=0;i<_idx;i++)
    {
      fprintf(stderr,"%.2x ",(_buf[i]));
      
      if (i%16==15)
	{
	  fprintf(stderr,"\n==> ");
	}
    }
  fprintf(stderr,"\n");
#endif
  return true;
}
