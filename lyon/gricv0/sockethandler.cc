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

gricv0::socketHandler::socketHandler(std::string ip,uint32_t port) : _idx(0),_transaction(0)
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
  _id=( (uint64_t) mpi::MpiMessageHandler::convertIP(ip)<<32)|port;

  _sock=new NL::Socket(ip,port);
}

void gricv0::socketHandler::clear()
{
  for (int i=0;i<256;i++)
    {
      memset(_answ[i],0,0x4000);
    }

}
uint32_t gricv0::socketHandler::sendMessage(gricv0::Message* m)
{
  
  // Send the Buffer
  try
  {
    uint8_t tr= ((_transaction++)%255)-1;
    m->ptr()[gricv0::Message::Fmt::TRANS]=tr;
    // Clear the ack tag for reply
    _answ[_transaction-1][gricv0::Message::Fmt::CMD]=0;
    
    _sock->send((const void*) m->ptr(),m->length()*sizeof(uint8_t));

    LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Address "<<std::hex<<((m->address()>>32)&0xFFFFFFFF)<<std::dec<<" Port "<<(m->address()&0XFFFF)<<" Length "<<m->length()<<" Transaction "<<tr);

    return (tr);
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }
  printf("Buffer sent %d bytes at Address %lx on port %ld \n",m->length(),(m->address()>>32)&0xFFFFFFF,m->address()&0XFFFF);


}
int16_t gricv0::socketHandler::checkBuffer(uint8_t* b,uint32_t maxidx)
{
   uint32_t elen=0;
  if (b[0]!='(')  return -2;
 // Check TAG coherency
 if (b[0]=='(')
   {
     uint16_t* _sBuf= (uint16_t*) &b[1];
     elen=ntohs(_sBuf[0]); // Header

     LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<"CheckBuf header ELEN "<<elen<<" MAXID "<<maxidx);
     //fprintf(stderr,"d %d %c\n",__LINE__,b[elen-1]);
     if (elen>maxidx)
       {
         LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<"CheckBuf header:Not enough data ELEN "<<elen<<" MAXID "<<maxidx);
         return -1;
       }
     if (b[elen-1]==')')
       {

       return elen;
       }
     else
       {
         LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<"CheckBuf header :Missing  end tag ");
         return -1;
       }
   }
 return -3; 
 
}

void gricv0::socketHandler::processBuffer(uint64_t id, uint16_t l,char* bb)
{
  LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<"Entering procesBuffer "<<std::hex<<id<<std::dec<<" Length "<<l);
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

  

     //DEBUG_PRINT("Exiting procesBuffer %llx %d  IDX %d \n",id,l,_idx);
  return;
  

}
void gricv0::socketHandler::purgeBuffer()
{
  fprintf(stderr,"Entering PURGEBUFFER %d \n",_idx);
}
