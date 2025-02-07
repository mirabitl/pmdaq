
#include "Wtricv0Interface.hh"
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




wtricv0::registerHandler::registerHandler(std::string ip) : socketProcessor(ip,wtricv0::PORT::CTRL),_noTransReply(true)

{
  _msg= new wizcc::Message();
}
uint32_t  wtricv0::registerHandler::sendCommand(uint8_t command)
{
  uint16_t len=6;
  _msg->setAddress(id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[wizcc::Message::Fmt::LEN]);
  _msg->ptr()[0]='(';
  sp[0]=htons(len);
  _msg->ptr()[wizcc::Message::Fmt::CMD]=command;
  _msg->ptr()[len-1]=')';    
  int32_t tr=this->sendMessage(_msg);
  uint32_t rep=0;
  //this->processReply(tr,&rep);
  if (!_noTransReply &&tr>=0)
   {
     uint8_t c=this->answer(tr)[wizcc::Message::Fmt::CMD];
     rep=c;
   }

  return rep;
}
void  wtricv0::registerHandler::sendParameter(uint8_t command,uint8_t par)
{
  uint16_t len=7;
  _msg->setAddress(id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[wizcc::Message::Fmt::LEN]);
  _msg->ptr()[0]='(';
  sp[0]=htons(len);
  _msg->ptr()[wizcc::Message::Fmt::CMD]=command;
  _msg->ptr()[wizcc::Message::Fmt::PAYLOAD]=par;
  _msg->ptr()[len-1]=')';

  PM_INFO(_logWtricv0," SENDING ="<<(int) command<<" length="<<len<<" parameter="<<(int) par<<" address="<<id());
  uint32_t tr=this->sendMessage(_msg);
  //this->processReply(tr);
}


void wtricv0::registerHandler::sendSlowControl(uint8_t* slc,uint16_t lenbytes)
{
 
  uint32_t len=115;
  _msg->setAddress(id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[wizcc::Message::Fmt::LEN]);
  _msg->ptr()[wizcc::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[wizcc::Message::Fmt::CMD]=wtricv0::command::STORESC;
  memcpy(&(_msg->ptr()[wizcc::Message::Fmt::PAYLOAD]),slc,lenbytes);
  _msg->ptr()[len-1]=')';
  uint32_t tr=this->sendMessage(_msg);
  //this->processReply(tr);
}


bool wtricv0::registerHandler::process_message()
{
  return true;
}
