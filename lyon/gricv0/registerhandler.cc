#include "Gricv0Interface.hh"
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


using namespace lydaq;

gricv0::registerHandler::registerHandler(std::string ip) : socketHandler(ip,gricv0::Interface::PORT::CTRL),_noTransReply(true)

{
  _msg= new gricv0::Message();
}
uint32_t  gricv0::registerHandler::sendCommand(uint8_t command)
{
  uint16_t len=6;
  _msg->setAddress(id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[gricv0::Message::Fmt::LEN]);
  _msg->ptr()[0]='(';
  sp[0]=htons(len);
  _msg->ptr()[gricv0::Message::Fmt::CMD]=command;
  _msg->ptr()[len-1]=')';    
  uint32_t tr=this->sendMessage(_msg);
  uint32_t rep=0;
  this->processReply(tr,&rep);
  return rep;
}
void  gricv0::registerHandler::sendParameter(uint8_t command,uint8_t par)
{
  uint16_t len=7;
  _msg->setAddress(id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[gricv0::Message::Fmt::LEN]);
  _msg->ptr()[0]='(';
  sp[0]=htons(len);
  _msg->ptr()[gricv0::Message::Fmt::CMD]=command;
  _msg->ptr()[gricv0::Message::Fmt::PAYLOAD]=par;
  _msg->ptr()[len-1]=')';

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" SENDING ="<<(int) command<<" length="<<len<<" parameter="<<(int) par<<" address="<<id());
  uint32_t tr=this->sendMessage(_msg);
  this->processReply(tr);
}


void gricv0::registerHandler::sendSlowControl(uint8_t* slc,uint16_t lenbytes)
{
 
  uint32_t len=115;
  _msg->setAddress(id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[gricv0::Message::Fmt::LEN]);
  _msg->ptr()[gricv0::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[gricv0::Message::Fmt::CMD]=lydaq::gricv0::Message::command::STORESC;
  memcpy(&(_msg->ptr()[gricv0::Message::Fmt::PAYLOAD]),slc,lenbytes);
  _msg->ptr()[len-1]=')';
  uint32_t tr=this->sendMessage(_msg);
  this->processReply(tr);
}


void gricv0::registerHandler::processReply(uint32_t tr,uint32_t* reply)
{
  uint8_t b[0x4000];
  
  uint8_t* rep=this->answer(tr%255);
  if (rep==NULL)
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" NULL ptr for answ "<<tr);

    }
  int cnt=0;
  //  while (rep[gricv0::Message::Fmt::CMD]!=lydaq::gricv0::Message::ACKNOWLEDGE )
    while (rep[gricv0::Message::Fmt::CMD]==0 )
    {
      usleep(1000);
      cnt++;
      if (cnt>1000)
	{
	  LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" no return after "<<cnt);
	  break;
	}
    }

  // Dump returned buffer
  memcpy(b,rep,0x4000);
  uint16_t* _sBuf= (uint16_t*) &b[gricv0::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t trame=b[gricv0::Message::Fmt::TRANS];
  uint8_t command=b[gricv0::Message::Fmt::CMD];
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" REPLY command ="<<(int) command<<" length="<<length<<" trame id="<<(int) trame);
  fflush(stdout);
  /*
  fprintf(stderr,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  
  for (int i=gricv0::Message::Fmt::PAYLOAD;i<length-1;i++)
    {
      fprintf(stderr,"%.2x ",(b[i]));
      
      if ((i-4)%16==15)
	{
	  fprintf(stderr,"\n");
	}
    }
  fprintf(stderr,"\n<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
  */
  if (reply!=0) //special case for read register
    {
      (*reply)=rep[gricv0::Message::Fmt::CMD];
      //memcpy(reply,&rep[gricv0::Message::Fmt::CMD],1);
      return;
    }

}

bool gricv0::registerHandler::processPacket()
{
  uint16_t* _sBuf= (uint16_t*) &_buf[gricv0::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t transaction=_buf[gricv0::Message::Fmt::TRANS];
  _sBuf=(uint16_t*) &_buf[gricv0::Message::Fmt::PAYLOAD];
  uint16_t address=ntohs(_sBuf[0]);
  uint32_t* lBuf=(uint32_t*) &_sBuf[1];
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<sourceid()<<" Command answer="<<
               std::hex<<(int) address<<":"<<(int) ntohl(lBuf[0])<<std::dec
               <<" length="<<length<<" trame id="<<(int) transaction<<" buffer length "<<_idx<<std::hex<<" address of transaction "<<answer(transaction%255)<<std::dec);
  uint8_t* rep=this->answer(transaction%255);
  if (rep==NULL)
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" NULL ptr for answ "<<transaction);
      
    }
  else
    memcpy(rep,_buf,length);

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
