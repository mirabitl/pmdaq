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

gricv0::sensorHandler::sensorHandler(std::string ip) : socketHandler(ip,gricv0::Interface::PORT::SENSOR)
{
  _msg=new gricv0::Message();
}



bool gricv0::sensorHandler::processPacket()
{
  uint16_t* _sBuf= (uint16_t*) &_buf[gricv0::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t transaction=_buf[gricv0::Message::Fmt::TRANS];

  
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<this->sourceid()<<"SENSOR data answer="<<transaction<<" length="<<length);
  fprintf(stderr,"\nSENSOR RC ==> ");
  for (int i=0;i<_idx;i++)
    {
      fprintf(stderr,"%.2x ",(_buf[i]));
      
      if (i%16==15)
	{
	  fprintf(stderr,"\n==> ");
	}
    }
  fprintf(stderr,"\n");
  return true;
}
