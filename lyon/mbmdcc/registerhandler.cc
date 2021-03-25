#include "MBMDCCInterface.hh"
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

#include <arpa/inet.h>




mbmdcc::registerHandler::registerHandler(std::string ip) : socketHandler(ip,mbmdcc::Interface::PORT::REGISTER),_noTransReply(false)

{
  _msg= new mbmdcc::Message();
}

void mbmdcc::registerHandler::writeRegister(uint16_t address,uint32_t value)
{
  uint16_t len=16;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[mbmdcc::Message::Fmt::LEN]);
  _msg->ptr()[mbmdcc::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[mbmdcc::Message::Fmt::CMD]=mbmdcc::Message::command::WRITEREG;
  uint16_t radr=htons(address);uint32_t rval=htonl(value);
  memcpy(&(_msg->ptr()[mbmdcc::Message::Fmt::PAYLOAD]),&radr,2);
  memcpy(&(_msg->ptr()[mbmdcc::Message::Fmt::PAYLOAD+2]),&rval,4);
  
  _msg->ptr()[len-1]=')';    
  uint32_t tr=this->sendMessage(_msg);
  if (_noTransReply) tr=0;
  this->processReply(tr);
}
uint32_t mbmdcc::registerHandler::readRegister(uint16_t address)
{
  uint16_t len=16;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[mbmdcc::Message::Fmt::LEN]);
  _msg->ptr()[mbmdcc::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[mbmdcc::Message::Fmt::CMD]=mbmdcc::Message::command::READREG;
  uint16_t radr=htons(address);
  memcpy(&(_msg->ptr()[mbmdcc::Message::Fmt::PAYLOAD]),&radr,2);
  _msg->ptr()[len-1]=')';

  // // For the moment the reply is in transaction 0
  // // Clear it first, to be suppressed asap Xtof return the transaction #
  // uint8_t* repa=this->answer(0);
  // memset(repa,0,0x4000);  
  uint32_t tr=this->sendMessage(_msg);
  uint32_t rep=0;
 PM_DEBUG(_logMbmdcc,"Waiting PROCESSREPLY");
 if (_noTransReply) tr=0;
  this->processReply(tr,&rep);
  return ntohl(rep);
}

void mbmdcc::registerHandler::processReply(uint32_t tr,uint32_t* reply)
{
  uint8_t b[0x4000];
  
  uint8_t* rep=this->answer(tr%255);
  if (rep==NULL)
    {
      PM_ERROR(_logMbmdcc," NULL ptr for answ "<<tr);

    }
  int cnt=0;
  while (rep[mbmdcc::Message::Fmt::CMD]!=mbmdcc::Message::ACKNOWLEDGE )
    {
      usleep(1000);
      cnt++;
      if (cnt>1000)
	{
	  PM_ERROR(_logMbmdcc," no return after "<<cnt);
	  break;
	}
    }

  // Dump returned buffer
  memcpy(b,rep,0x4000);
  uint16_t* _sBuf= (uint16_t*) &b[mbmdcc::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t trame=b[mbmdcc::Message::Fmt::TRANS];
  uint8_t command=b[mbmdcc::Message::Fmt::CMD];
  PM_DEBUG(_logMbmdcc," REPLY command ="<<(int) command<<" length="<<length<<" trame id="<<(int) trame);
  fflush(stdout);
  /*
  PM_INFO(_logMbmdcc,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  
  for (int i=mbmdcc::Message::Fmt::PAYLOAD;i<length-1;i++)
    {
      PM_INFO(_logMbmdcc,"%.2x ",(b[i]));
      
      if ((i-4)%16==15)
	{
	  PM_INFO(_logMbmdcc,"\n");
	}
    }
  PM_INFO(_logMbmdcc,"\n<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
  */
  if (reply!=0) //special case for read register
    {
      memcpy(reply,&rep[mbmdcc::Message::Fmt::PAYLOAD+2],sizeof(uint32_t));
      return;
    }

}

bool mbmdcc::registerHandler::processPacket()
{
  uint16_t* _sBuf= (uint16_t*) &_buf[mbmdcc::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t transaction=_buf[mbmdcc::Message::Fmt::TRANS];
  _sBuf=(uint16_t*) &_buf[mbmdcc::Message::Fmt::PAYLOAD];
  uint16_t address=ntohs(_sBuf[0]);
  uint32_t* lBuf=(uint32_t*) &_sBuf[1];
  PM_DEBUG(_logMbmdcc,sourceid()<<" Command answer="<<
               std::hex<<(int) address<<":"<<(int) ntohl(lBuf[0])<<std::dec
               <<" length="<<length<<" trame id="<<(int) transaction<<" buffer length "<<_idx<<std::hex<<" address of transaction "<<answer(transaction%255)<<std::dec);
  uint8_t* rep=this->answer(transaction%255);
  if (rep==NULL)
    {
      PM_ERROR(_logMbmdcc," NULL ptr for answ "<<transaction);
      
    }
  else
    memcpy(rep,_buf,length);

#undef DUMPREGREP
#ifdef DUMPREGREP
  PM_INFO(_logMbmdcc,"\n REGISTER RC ==> ");
  for (int i=0;i<_idx-1;i++)
    {
      fprintf(stderr,"%.2x ",(_buf[i]));
         
      if (i%16==15)
	{
	  PM_INFO(_logMbmdcc,"\n REGISTER RC ==> ");
	}
    }
  PM_INFO(_logMbmdcc,"\n");
#endif
  return true;
}
