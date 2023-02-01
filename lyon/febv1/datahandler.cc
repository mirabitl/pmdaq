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
#include <boost/format.hpp>
#include <arpa/inet.h>



febv1::dataHandler::dataHandler(std::string ip) : socketHandler(ip,febv1::Interface::PORT::DATA),_ntrg(0),_dsData(NULL),_triggerId(0),_detId(130),_lastGTC(0),_lastABCID(0),_chlines(0)
{
  
}
void febv1::dataHandler::connect(zmq::context_t* c,std::string dest)
{
  if (_dsData!=NULL)
    delete _dsData;
  _dsData = new pm::pmSender(c,_detId,this->sourceid());
  _dsData->connect(dest);
}
void febv1::dataHandler::clear()
{
  _nProcessed=0;
  _lastGTC=0;
  _lastABCID=0;
  _lastBCID=0;
  _event=0;
}


void febv1::dataHandler::autoRegister(zmq::context_t* c,std::string session,std::string appname,std::string portname)
{
  if (_dsData!=NULL)
    delete _dsData;
  _dsData = new pm::pmSender(c,_detId,this->sourceid());
  _dsData->autoDiscover(session,appname,portname);//
  _dsData->collectorRegister();

}

bool febv1::dataHandler::processPacket()
{
 
  uint32_t *_lBuf = (uint32_t *)&_buf[0];
  uint16_t *_sBuf = (uint16_t *)&_buf[0];

  int16_t tag = checkBuffer(_buf, _idx);
  if (tag < 0)
    return false;

  if (ntohl(_lBuf[0]) == 0xcafedade)
    {
      if (_lastABCID != 0)
	{
	  // Write Event
	  PM_INFO(_logFebv1,  "Writing completed Event" << _event << " GTC " << _lastGTC << " ABCID " << _lastABCID << " Lines " << _chlines << " ID " << id());
	  // To be done
	  this->processEventTdc();
	  // Reset lines number
	  _chlines = 0;
	}

      // Find new gtc and absolute bcid
      uint32_t gtc = (_buf[7] | (_buf[6] << 8) | (_buf[5] << 16) | (_buf[4] << 24));
      uint64_t abcid = ((uint64_t)_buf[13] | ((uint64_t)_buf[12] << 8) | ((uint64_t)_buf[11] << 16) | ((uint64_t)_buf[10] << 24) | ((uint64_t)_buf[9] << 32));
      if (abcid == _lastABCID)
	{
	  PM_ERROR(_logFebv1,  id() << " ABCID HEADER ERROR " << abcid << "  last " << _lastABCID);
	}
      if (gtc == _lastGTC)
	{
	  PM_ERROR(_logFebv1,  id() << " GTC HEADER ERROR " << gtc << "  last " << _lastGTC);
	}
      _nProcessed++;
      _event++;
      _lastGTC = gtc;
      _lastABCID = abcid;
      //INFO_PRINT(" New Event Header  %d Packets %d GTC %d ABCID %llu Size %d\n", _event, _nProcessed, gtc, abcid, _idx);
#define DEBUGPACKETN
#ifdef DEBUGPACKET
      printf("\n==> ");
      for (int i = 0; i < _idx; i++)
	{
	  printf("%.2x ", (_buf[i]));

	  if (i % 16 == 15)
	    {
	      printf("\n==> ");
	    }
	}
      printf("\n");
#endif
      return true;
    }

  // Channel packet processing
  _nProcessed++;

  uint16_t channel = ntohs(_sBuf[2]);
  uint16_t *tmp = (uint16_t *)&_buf[7];
  uint32_t gtc = (_buf[9] | (_buf[8] << 8) | (_buf[7] << 16) | (_buf[6] << 24));
  uint16_t nlines = ntohs(_sBuf[5]);
  PM_INFO(_logFebv1,  id() << " Packets=" << _nProcessed << " channel=" << channel << " GTC=" << gtc << " lines=" << nlines << " index=" << _idx);
  
  uint8_t *cl = (uint8_t *)&_buf[12];
  uint8_t *cdestl = (uint8_t *)&_linesbuf[_chlines * CHBYTES];
  memcpy(cdestl, cl, nlines * CHBYTES);
  _chlines += nlines;
#undef DEBUGLINES
#ifdef DEBUGLINES
  for (int i = 0; i < nlines; i++)
    {
      for (int j = 0; j < CHBYTES; j++)
	printf("%.2x ", (cl[i * CHBYTES + j]));
      printf("\n");
    }
#endif

#ifdef DEBUGPACKET
  printf("\n==> ");
  for (int i = 0; i < _idx; i++)
    {
      printf("%.2x ", (_buf[i]));

      if (i % 16 == 15)
	{
	  printf("\n==> ");
	}
    }
  printf("\n");
#endif
  // fprintf(stderr,"packet processed \n");

  uint16_t expectedSize = 16 + nlines * CHBYTES;
  if (_idx > (expectedSize + 1))
    {
      char temp[0x10000];
      uint16_t remain = _idx - expectedSize;
      memcpy(temp, &_buf[expectedSize], remain);
      memcpy(_buf, temp, remain);
      _idx = remain;
      this->processPacket();
    }
  return true;

}
void febv1::dataHandler::processEventTdc()
{
  uint8_t temp[0x100000];
  uint32_t *itemp = (uint32_t *)temp;
  uint64_t *ltemp = (uint64_t *)temp;
  itemp[0] = _event;

  itemp[1] = _lastGTC;
  ltemp[1] = _lastABCID;
  itemp[4] = _event;
  itemp[5] = ipid();
  itemp[6] = _chlines;
  uint32_t idx = 28; // 4 x5 int + 1 int64
  uint32_t trbcid = 0;
  memcpy(&temp[idx], _linesbuf, _chlines * CHBYTES);
  idx += (_chlines * CHBYTES);
  if (_dsData != NULL)
    {
      memcpy((unsigned char *)_dsData->payload(), temp, idx);
      _dsData->publish(_lastABCID, _lastGTC, idx);
      if (_event % 100 == 0)
	PM_INFO(_logFebv1,  id() << "Publish  Event=" << _event << " GTC=" << _lastGTC << " ABCID=" << _lastABCID << " Lines=" << _chlines << " size=" << idx);
    }
}
