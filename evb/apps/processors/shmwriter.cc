#include "shmwriter.hh"
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
#include "utils.hh"
//#include "DIFReadoutConstant.h"
#include <iostream>
#include <sstream>
using namespace pm;

shmwriter::shmwriter(std::string dire) : _filepath(dire), _started(false)
{
}

void shmwriter::start(uint32_t run) //,std::string dir,std::string setup)
{

  _run = run;
  _started = true;
}
void shmwriter::processRunHeader(std::vector<uint32_t> header)
{
  if (!_started)
    return;

  uint32_t ibuf[256];
  for (int i = 0; i < header.size(); i++)
    ibuf[i] = header[i];
  // Construct one zdaq buffer with header content
  pm::buffer b(128 + header.size());
  b.setDetectorId(255);
  b.setDataSourceId(1);
  b.setEventId(_run);
  b.setBxId(0);
  b.setPayload(ibuf, header.size() * sizeof(uint32_t));
  unsigned char *cdata = (unsigned char *)b.ptr();
  int32_t *idata = (int32_t *)cdata;
  int difsize = b.size();
  utils::store(b.detectorId(), b.dataSourceId(),
                b.eventId(),b.bxId(), b.ptr(), b.size(), _filepath);
}

void shmwriter::loadParameters(json::value params)
{
  if (params.as_object().find("filepath") != params.as_object().end())
    _filepath = params["filepath"].as_string();
}
void shmwriter::processEvent(uint32_t gtc, std::vector<pm::buffer *> vbuf) // writeEvent(uint32_t gtc,std::vector<unsigned char*> vbuf)
{
  if (!_started)
    return;
  uint32_t theNumberOfDIF = vbuf.size();

  std::vector<std::string> vnames;

  // list files in shm directory
  utils::ls(_filepath, vnames);
  // if (vnames.size()>20*theNumberOfDIF) return;

  if (gtc % 100 == 0)
    std::cout << "Standard completion GTC " << gtc << std::endl;
  for (std::vector<pm::buffer *>::iterator iv = vbuf.begin(); iv != vbuf.end(); iv++)
  {
    (*iv)->uncompress();
    utils::store((*iv)->detectorId(), (*iv)->dataSourceId(),
		     (*iv)->eventId(), (*iv)->bxId(), (*iv)->ptr(), (*iv)->size(), _filepath);
  }
}

void shmwriter::stop()
{
}



extern "C"
{
  // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.
  pm::evbprocessor *loadProcessor(void)
  {
    return (new pm::shmwriter);
  }
  // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed
  // to it.  This isn't a very safe function, since there's no
  // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(pm::evbprocessor *obj)
  {
    delete obj;
  }
}
