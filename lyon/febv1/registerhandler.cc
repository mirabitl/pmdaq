#include "Febv1Interface.hh"
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
#include "stdafx.hh"
#include <arpa/inet.h>



febv1::registerHandler::registerHandler(std::string ip) : socketHandler(ip,febv1::Interface::PORT::REGISTER),_noTransReply(true)

{
  _msg= new febv1::Message();
}

void febv1::registerHandler::writeRam(uint16_t* slcAddr,uint16_t *slcBuffer,uint32_t slcBytes)
{
    if (slcBuffer[1] < 2)
      return;

    uint16_t* sockbuf= (uint16_t*) _msg->ptr();
    sockbuf[0] = htons(0xFF00);
    sockbuf[1] = htons(slcBytes);
    int idx = 2;
    for (int i = 0; i < slcBytes; i++)
      {
	sockbuf[idx] = htons(slcAddr[i]);
	sockbuf[idx + 1] = htons(slcBuffer[i]);
	idx += 2;
      }
  //
  // Creating file  with name = SLC
    std::stringstream sb;
    for (int i = 0; i < slcBytes - 1; i++)
      {
	
	sb << std::hex << (int)(slcBuffer[i] & 0xFF);
      }
    char name[2512];
    memset(name, 0, 2512);
    sprintf(name, "/dev/shm/%s/%d/%s", socket()->hostTo().c_str(), socket()->portTo(), sb.str().c_str());
    int fd = ::open(name, O_CREAT | O_RDWR | O_NONBLOCK, S_IRWXU);
    PM_DEBUG(_logFebv1, " Creating SHM area " << name);

    ::close(fd);
    _msg->setAddress(this->id());
    _msg->setLength(idx*sizeof(uint16_t));

    uint32_t tr=this->sendMessage(_msg);
    if (_noTransReply) tr=0;
    this->processReply(tr);
}
uint32_t febv1::registerHandler::writeAddress(uint16_t address,uint16_t value,bool waitreply)
{

  
  uint16_t len=8;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sb= (uint16_t*) _msg->ptr();
  sb[0] = htons(0xFF00);
  sb[1] = htons(1);
  sb[2] = htons(address);
  sb[3] = htons(value);
  uint32_t tr=this->sendMessage(_msg);
  uint32_t rep=0;
  if (waitreply)
    {
      fprintf(stderr,"Waiting PROCESSREPLY\n");
      this->processReply(0,&rep);
    }
  return ntohl(rep);
}
uint32_t febv1::registerHandler::writeLongWord(uint16_t addr,uint64_t val,bool waitreply)
{
  uint16_t len=20;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sb= (uint16_t*) _msg->ptr();
  sb[0] = htons(0xFF00);
  sb[1] = htons(4);
  sb[2] = htons(addr);
  sb[3] = htons(val & 0XFFFF);
  sb[4] = htons(addr + 1);
  sb[5] = htons((val >> 16) & 0XFFFF);
  sb[6] = htons(addr + 2);
  sb[7] = htons((val >> 32) & 0xFFFF);
  sb[8] = htons(addr + 3);
  sb[9] = htons((val >> 48) & 0xFFFF);

  //PM_INFO(_logFebv1," Sending message "<<this->id());
  
  uint32_t tr=this->sendMessage(_msg);
  uint32_t rep=0;
  if (waitreply)
    {
      fprintf(stderr,"Waiting PROCESSREPLY\n");
      this->processReply(0,&rep);
    }
  return ntohl(rep);
}

void febv1::registerHandler::processReply(uint32_t tr,uint32_t* reply)
{
  uint8_t b[0x4000];
  
  uint8_t* rep=this->answer(tr%NREP);
  if (rep==NULL)
    {
      PM_ERROR(_logFebv1," NULL ptr for answ "<<tr);

    }
  int cnt=0;
  while (rep[0]==0&&rep[1]==0 ) /// Hope one of the two first bytes are non 0 or loose 1 s
    {
      usleep(1000);
      cnt++;
      if (cnt>1000)
	{
	  PM_ERROR(_logFebv1," no return after "<<cnt);
	  break;
	}
    }

  // Dump returned buffer
  if (reply!=0) //special case for read register
    {
      memcpy(reply,&rep[3],sizeof(uint32_t));
      return;
    }

}

bool febv1::registerHandler::processPacket()
{
  uint8_t* rep=this->answer(0);
  if (rep==NULL)
    {
      PM_ERROR(_logFebv1," NULL ptr for answ ");
      
    }
  else
    {
      uint16_t len=_idx+4;
      rep[0]='(';
      uint16_t* sb=(uint16_t*) &rep[1];
      sb[0]=len&0XFFFF;
      rep[len-1]=')';
	
      memcpy(&rep[3],_buf,_idx);
    }
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
void febv1::registerHandler::dumpAnswer(uint32_t tr)
{
   uint8_t* rep=this->answer(tr);
   uint16_t* sb=(uint16_t*) &rep[1];
   fprintf(stderr,"Answer Length ==> %d \n",sb[0]);
   for (int i=0;i<sb[0];i++)
    {
      fprintf(stderr,"%.2x ",(rep[i+3]));
      if (i%16==15) fprintf(stderr,"\n");
    }
  fprintf(stderr,"\n");
}

void febv1::registerHandler::processBuffer(uint64_t id, uint16_t l,char* bb)
{
  //if (l>16) getchar();
  //memcpy(_b,bb,l);
  memcpy(&_buf[_idx],bb,l);
  _idx+=l;
#define DEBUGPACKETN
#ifdef DEBUGPACKET
  fprintf(stderr,"\n DEBUG PACKET IDX %d L %d  ID %lx \n",_idx,l,id);
     for (int i=0;i<_idx;i++)
       {
	 fprintf(stderr,"%.2x ",((uint8_t) _buf[i]));
	 
	 if (i%16==15)
	   {
	     fprintf(stderr,"\n");
	   }
       }
     fprintf(stderr,"\n END PACKET \n");
#endif
     uint32_t a=l;
 checkpoint:
     int16_t tag = checkBuffer(_buf,_idx);
     if (tag>0)
       {
	 bool ok=processPacket();
	 if (tag<_idx)
	   {
	     memcpy(_b,&_buf[tag],_idx-tag);
	     _idx=_idx-tag;
	     memset(_buf,0,MBSIZE);
	     memcpy(_buf,_b,_idx);
	     goto checkpoint;
	   }
	 if (tag==_idx) _idx=0;
	   
	  return;
       }
     else
       if (tag==0)
	 {
	   processPacket();
	   _idx=0;
	 }
	 
  

     //DEBUG_PRINT("Exiting procesBuffer %llx %d  IDX %d \n",id,l,_idx);
  return;
  
}
