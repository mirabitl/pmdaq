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

gricv0::dataHandler::dataHandler(std::string ip) : socketHandler(ip,gricv0::Interface::PORT::DATA),_ntrg(0),_dsData(NULL),_triggerId(0),_detId(140)
{
  
}
void gricv0::dataHandler::connect(zmq::context_t* c,std::string dest)
{
  if (_dsData!=NULL)
    delete _dsData;
  _dsData = new zdaq::zmSender(c,_detId,this->sourceid());
  _dsData->connect(dest);
}
void gricv0::dataHandler::clear()
{
  _nProcessed=0;
  _lastGTC=0;
  _lastABCID=0;
  _lastBCID=0;
  _event=0;
  for (int i=0;i<255;i++)
    {
      memset(this->answer(i),0,MAX_BUFFER_LEN);
    }
}


void gricv0::dataHandler::autoRegister(zmq::context_t* c,Json::Value config,std::string appname,std::string portname)
{
  if (_dsData!=NULL)
    delete _dsData;
  _dsData = new zdaq::zmSender(c,_detId,this->sourceid());
  _dsData->autoDiscover(config,appname,portname);//
  //for (uint32_t i=0;i<_mStream.size();i++)
  //        ds->connect(_mStream[i]);
  _dsData->collectorRegister();

}

bool gricv0::dataHandler::processPacket()
{

  uint16_t* _sBuf= (uint16_t*) &_buf[gricv0::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint16_t trame=_buf[gricv0::Message::Fmt::TRANS];
  uint16_t command=_buf[gricv0::Message::Fmt::CMD];

  uint32_t* ib=(uint32_t*) &_buf[gricv0::Message::Fmt::PAYLOAD];
  
  _lastGTC=((uint32_t) _buf[5] <<24)|((uint32_t) _buf[6] <<16)|((uint32_t) _buf[7] <<8)|((uint32_t) _buf[8]);
  _lastABCID = ((uint64_t) _buf[9] <<48)|((uint64_t) _buf[10] <<32)|((uint64_t) _buf[11] <<24)|((uint64_t) _buf[12] <<16)|((uint64_t) _buf[13] <<8)|((uint64_t) _buf[14]);

  LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<id()<<" Command answer="<<command<<" length="<<length<<" trame id="<<trame<<" buffer length "<<_idx);

#define DEBUGEVENTN
#ifdef DEBUGEVENT  
  fprintf(stderr,"Length %d \n==> ",_idx);
  for (int i=4;i<_idx;i++)
       {
      fprintf(stderr,"%.2x ",(_buf[i]));
      
      if (i%16==15)
	{
	  fprintf(stderr,"\n==> ");
	}
    }
  fprintf(stderr,"\n");
#endif
   uint8_t temp[0x100000];
  uint32_t* itemp=(uint32_t*) temp;
  uint64_t* ltemp=(uint64_t*) temp;
  itemp[0]=_event;

  itemp[1]=_lastGTC;
  ltemp[1]=_lastABCID;
  itemp[4]= _event;
  itemp[5]=ipid();
  itemp[6]=length;
  uint32_t idx=28; // 4 x5 int + 1 int64
  uint32_t trbcid=0;
  memcpy(&temp[idx],&_buf[14],length-14);
  idx+=(length-14);
  if (_dsData!=NULL)
    {
      //memcpy((unsigned char*) _dsData->payload(),temp,idx);
      memcpy((unsigned char*) _dsData->payload(),_buf,length);
      _dsData->publish(_lastABCID,_lastGTC,idx);
      if (_event%100==0)
	LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<id()<<"Publish  Event="<<_event<<" GTC="<<_lastGTC<<" ABCID="<<_lastABCID<<" size="<<idx);
    }
  _event++;
  return true;
}
