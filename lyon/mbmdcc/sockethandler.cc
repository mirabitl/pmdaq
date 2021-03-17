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




mbmdcc::socketHandler::socketHandler(std::string ip,uint32_t port) : _idx(0),_transaction(0)
{
  memset(_buf,0,MBSIZE);
  memset(_b,0,MBSIZE);
  // initialise answer storage
  for (int i=0;i<256;i++)
    {
      uint8_t* b=new uint8_t[0x4000];
      std::pair<uint8_t,uint8_t*> p(i,b);
      _answ.insert(p);
    }
  // Now create the socket
  _id=( (uint64_t) utils::convertIP(ip)<<32)|port;

  _sock=new NL::Socket(ip,port);
}

void mbmdcc::socketHandler::clear()
{
  for (int i=0;i<255;i++)
    {
      memset(_answ[i],0,0x4000);
    }

}
uint32_t mbmdcc::socketHandler::sendMessage(mbmdcc::Message* m)
{
  
  // Send the Buffer
  try
  {
    uint8_t tr=((_transaction++)%255) -1;
    m->ptr()[mbmdcc::Message::Fmt::TRANS]=tr;
    // Clear the ack tag for reply
    _answ[tr][mbmdcc::Message::Fmt::CMD]=0;
    
    _sock->send((const void*) m->ptr(),m->length()*sizeof(uint8_t));

    PM_INFO(_logMbmdcc," Address "<<std::hex<<((m->address()>>32)&0xFFFFFFFF)<<std::dec<<" Port "<<(m->address()&0XFFFF)<<" Length "<<m->length()<<" Transaction "<<tr);

    return (tr);
  }
  catch (NL::Exception e)
  {
    PM_FATAL(_logMbmdcc,"Cannot send message "<<std::hex<<((m->address()>>32)&0xFFFFFFFF)<<std::dec<<" Port "<<(m->address()&0XFFFF)<<" error "<<e.msg());
  }
  printf("Buffer sent %d bytes at Address %lx on port %ld \n",m->length(),(m->address()>>32)&0xFFFFFFF,m->address()&0XFFFF);

  return 0;
}
int16_t mbmdcc::socketHandler::checkBuffer(uint8_t* b,uint32_t maxidx)
{
   uint32_t elen=0;
  if (b[0]!='(')  return -2;
 // Check TAG coherency
 if (b[0]=='(')
   {
     uint16_t* _sBuf= (uint16_t*) &b[1];
     elen=ntohs(_sBuf[0]); // Header

     PM_WARN(_logMbmdcc,"CheckBuf header ELEN "<<elen<<" MAXID "<<maxidx);
     //fprintf(stderr,"d %d %c\n",__LINE__,b[elen-1]);
     if (elen>maxidx)
       {
         PM_INFO(_logMbmdcc,"CheckBuf header:Not enough data ELEN "<<elen<<" MAXID "<<maxidx);
         return -1;
       }
     if (b[elen-1]==')')
       {

       return elen;
       }
     else
       {
         PM_INFO(_logMbmdcc,"CheckBuf header :Missing  end tag , found "<<b[elen-1] );
	      for (int i=0;i<elen;i++)
       {
	 fprintf(stderr,"%.2x ",((uint8_t) b[i]));
	 
	 if (i%16==15)
	   {
	     fprintf(stderr,"\n");
	   }
       }
         return -1;
       }
   }
 return -3; 
 
}

void mbmdcc::socketHandler::processBuffer(uint64_t id, uint16_t l,char* bb)
{
  PM_DEBUG(_logMbmdcc,"Entering procesBuffer "<<std::hex<<id<<std::dec<<" Length "<<l);
  //if (l>16) getchar();
  //memcpy(_b,bb,l);
  memcpy(&_buf[_idx],bb,l);
  _idx+=l;
#define DEBUGPACKET
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

  

     //DEBUG_PRINT("Exiting procesBuffer %llx %d  IDX %d \n",id,l,_idx);
  return;
  

}
void mbmdcc::socketHandler::purgeBuffer()
{
  fprintf(stderr,"Entering PURGEBUFFER %d \n",_idx);
}
