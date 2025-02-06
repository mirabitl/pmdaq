#include "WizccInterface.hh"
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



wizcc::socketProcessor::socketProcessor(std::string ip,uint32_t port) : _idx(0),_transaction(0)
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

void wizcc::socketProcessor::clear()
{
  for (int i=0;i<255;i++)
    {
      memset(_answ[i],0,0x4000);
    }

}
uint32_t wizcc::socketProcessor::send_message(wizcc::Message* m,bool noreply)
{
  
  // Send the Buffer
  try
  {
    uint8_t tr=((_transaction++)%255) -1;
    m->ptr()[wizcc::Message::Fmt::TRANS]=tr;
    // Clear the ack tag for reply
    _answ[tr][wizcc::Message::Fmt::CMD]=0;
    
    _sock->send((const void*) m->ptr(),m->length()*sizeof(uint8_t));

    PM_DEBUG(_logWizcc," Address "<<std::hex<<((m->address()>>32)&0xFFFFFFFF)<<std::dec<<" Port "<<(m->address()&0XFFFF)<<" Length "<<m->length()<<" Transaction "<<tr);

    if (!noreply) wait_reply(m);
    return (tr);
  }
  catch (NL::Exception e)
  {
    PM_FATAL(_logWizcc,"Cannot send message "<<std::hex<<((m->address()>>32)&0xFFFFFFFF)<<std::dec<<" Port "<<(m->address()&0XFFFF)<<" error "<<e.msg());
  }
  //printf("Buffer sent %d bytes at Address %lx on port %ld \n",m->length(),(m->address()>>32)&0xFFFFFFF,m->address()&0XFFFF);

  return 0;

}

void  wizcc::socketProcessor::process_acknowledge(uint8_t* buffer,int len)
{
  if (buffer[wizcc::Message::Fmt::CMD]!=wizcc::Message::command::ACKNOWLEDGE) return;
  uint8_t trans=buffer[wizcc::Message::Fmt::TRANS];
  std::stringstream ss;
  ss<<"/dev/shm/"<<_id<<"_"<<(int) trans;
  int pipefifo, returnval;

  returnval = mkfifo(ss.str().c_str(), 0666);

  pipefifo = open(ss.str().c_str(), O_WRONLY);
  if (pipefifo == -1)
    {
      std::cout << "Error, cannot open fifo" << std::endl;
      return;
    }

  write(pipefifo, buffer, len);
  
  close(pipefifo);

}

void wizcc::socketProcessor::wait_reply(wizcc::Message* m)
{

  	int pipefifo, returnval;

	uint8_t trans= m->ptr()[wizcc::Message::Fmt::TRANS];
	unsigned char *buffer=_answ[trans];
	std::stringstream ss;
	ss<<"/dev/shm/"<<_id<<"_"<<(int) trans;

    // Creating the named file(FIFO)
    // mkfifo(<pathname>,<permission>)
	mkfifo(ss.str().c_str(), 0666);
	pipefifo = open(ss.str().c_str(), O_RDONLY);
	if (pipefifo == -1)
	{
		std::cout << "Error, cannot open fifo" << std::endl;
		return;
	}

	returnval = read(pipefifo, buffer, sizeof(buffer));

	fflush(stdin);

	for (int i = 0; i < sizeof(buffer); ++i)
	{
		std::cout << buffer[i];
	}

	std::cout << std::endl;

	close(pipefifo);
	remove(ss.str().c_str());
	return ;

}


int16_t wizcc::socketProcessor::check_buffer(uint8_t* b,uint32_t maxidx)
{
   uint32_t elen=0;
  if (b[0]!='(')  return -2;
 // Check TAG coherency
 if (b[0]=='(')
   {
     uint16_t* _sBuf= (uint16_t*) &b[1];
     elen=ntohs(_sBuf[0]); // Header

     PM_DEBUG(_logWizcc,"CheckBuf header ELEN "<<elen<<" MAXID "<<maxidx);
     //fprintf(stderr,"d %d %c\n",__LINE__,b[elen-1]);
     if (elen>maxidx)
       {
         PM_INFO(_logWizcc,"CheckBuf header:Not enough data ELEN "<<elen<<" MAXID "<<maxidx);
         return -1;
       }
     if (b[elen-1]==')')
       {

       return elen;
       }
     else
       {
         PM_INFO(_logWizcc,"CheckBuf header :Missing  end tag , found "<<b[elen-1] );
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

void wizcc::socketProcessor::process_buffer(uint64_t id, uint16_t l,char* bb)
{
  PM_INFO(_logWizcc,"Entering procesBuffer "<<std::hex<<id<<std::dec<<" Length "<<l);
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
     int16_t tag = check_buffer(_buf,_idx);
     if (tag>0)
       {
	 this->process_acknowledge(_buf,_idx);
	 bool ok=process_message();
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

  

  return;
  

}
void wizcc::socketProcessor::purge_buffer()
{
  PM_DEBUG(_logWizcc,"Entering PURGEBUFFER "<<_idx);
}
