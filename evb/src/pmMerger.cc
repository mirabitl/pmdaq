#include <dlfcn.h>

#include "pmMerger.hh"
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

using namespace pm;
pmMerger::pmMerger(zmq::context_t *c) : pmPuller(c), _running(false), _nDifs(0), _purge(true), _writeHeader(false), _nextEventHeader(-1)
{
  _eventMap.clear();
  _processors.clear();

  _mReceived.clear();

  /*
  _statusPublisher = new  pm::mon::zPublisher("builder","example",4444,c);

  PM_INFO(_logPdaq," Status Publisher created on port 4444");
  */
}
pmMerger::~pmMerger()
{
   PM_INFO(_logPdaq, "delete PMmerger"<<_running);
  if (_running)
  {
    this->stop();
    _running=false;
  }
  this->clear();
   PM_INFO(_logPdaq," end of destructor");

}
void pmMerger::clear()
{
   PM_INFO(_logPdaq," Deleting existing buffer");
  for (std::map<uint64_t, std::vector<pm::buffer *>>::iterator it = _eventMap.begin(); it != _eventMap.end(); it++)
  {
    for (std::vector<pm::buffer *>::iterator jt = it->second.begin(); jt != it->second.end(); jt++)
      delete (*jt);
  }
     PM_INFO(_logPdaq," clearing map");

  _eventMap.clear();
  _processors.clear();

  _mReceived.clear();
}

void pmMerger::registerProcessor(std::string name)
{
  std::stringstream s;
  s << "lib" << name << ".so";
  void *library = dlopen(s.str().c_str(), RTLD_NOW);

  //printf("%s %x \n",dlerror(),(unsigned int) library);
  PM_INFO(_logPdaq, " Error " << dlerror() << " Library open address " << std::hex << library << std::dec);
  // Get the loadFilter function, for loading objects
  pm::evbprocessor *(*create)();
  create = (pm::evbprocessor * (*)()) dlsym(library, "loadProcessor");
  PM_INFO(_logPdaq, " Error " << dlerror() << " file " << s.str() << " loads to processor address " << std::hex << create << std::dec);
  //printf("%s %x \n",dlerror(),(unsigned int) create);
  // printf("%s lods to %x \n",s.str().c_str(),(unsigned int) create);
  //void (*destroy)(Filter*);
  // destroy = (void (*)(Filter*))dlsym(library, "deleteFilter");
  // Get a new filter object
  pm::evbprocessor *a = (pm::evbprocessor *)create();
  _processors.push_back(a);
}

void pmMerger::unregisterProcessor(pm::evbprocessor *p)
{
  std::vector<pm::evbprocessor *>::iterator it = std::find(_processors.begin(), _processors.end(), p);
  if (it != _processors.end())
    _processors.erase(it);
}
void pmMerger::registerDataSource(std::string url)
{

  PM_INFO(_logPdaq, "Adding input Stream " << url);

  this->addInputStream(url, true);
  PM_INFO(_logPdaq, url<<" added");

}

uint32_t pmMerger::numberOfDataPacket(uint32_t k)
{
  std::map<uint64_t, std::vector<pm::buffer *>>::iterator it = _eventMap.find(k);
  if (it != _eventMap.end())
    return it->second.size();
  else
    return 0;
}

void pmMerger::processEvent(uint32_t idx)
{
  std::map<uint64_t, std::vector<pm::buffer *>>::iterator it = _eventMap.find(idx);
  //printf("Processing %d Size %d for %d Map %d \n",it->first,it->second.size(),numberOfDataSource(),_eventMap.size());
  if (it->second.size() != numberOfDataSource())
    return;
  if (it->first == 0)
    return; // do not process event 0
  _evt = it->first;
  _build++;
  //std::cout<<"full  event find " <<it->first<<std::endl;
  for (std::vector<pm::evbprocessor *>::iterator itp = _processors.begin(); itp != _processors.end(); itp++)
  {

    if (_writeHeader)
    {
      PM_INFO(_logPdaq, "Processing Header " << _evt << " " << _nextEventHeader << " " << idx);
      if (_nextEventHeader > 0 && _nextEventHeader == idx)
      {
        (*itp)->processRunHeader(_runHeader);
        _writeHeader = false;
        _nextEventHeader = -1;
      }
    }
    (*itp)->processEvent(it->first, it->second);
  }

  // remove completed events
  // for (std::vector<pm::buffer*>::iterator iv=it->second.begin();iv!=it->second.end();iv++) delete (*iv);
  // it->second.clear();
  // _eventMap.erase(it);
  //printf("End of processing %d Map size %d \n",_evt,_eventMap.size());
  if (_build % 100 == 0)
    PM_DEBUG(_logPdaq, "End of processing of event " << _evt << " remaining map size " << _eventMap.size() << "  built" << _build);
  // Clearing uncompleted event with GTC< 100 current GTC

  /*
  if (_build%1000==0)
    {
      PM_DEBUG(_logPdaq,"Publishing status "<<this->status());
      _statusPublisher->post(this->status());
    }
  */
}
void pmMerger::processRunHeader()
{
  _writeHeader = true;
  /*
  for (std::vector<pm::evbprocessor*>::iterator itp=_processors.begin();itp!=_processors.end();itp++)
    {
      //std::cout<<"On enevoie"<<std::endl;
      (*itp)->processRunHeader(_runHeader);
      //std::cout<<"Apres enevoie"<<std::endl;
    }
  */
}
void pmMerger::loadParameters(web::json::value params)
{
  for (std::vector<pm::evbprocessor *>::iterator itp = _processors.begin(); itp != _processors.end(); itp++)
  {
    (*itp)->loadParameters(params);
  }
}

void pmMerger::start(uint32_t nr)
{
  // Do the start of the the processors
  _run = nr;
  _evt = 0;
  _build = 0;
  _totalSize = 0;
  _compressedSize = 0;
  // clear event Map
  for (std::map<uint64_t, std::vector<pm::buffer *>>::iterator it = _eventMap.begin(); it != _eventMap.end(); it++)
  {
    for (std::vector<pm::buffer *>::iterator jt = it->second.begin(); jt != it->second.end(); jt++)
      delete (*jt);
  }
  _eventMap.clear();

  PM_INFO(_logPdaq, "run : " << _run << " ZMMERGER START for " << numberOfDataSource() << " sources");
  for (std::vector<pm::evbprocessor *>::iterator itp = _processors.begin(); itp != _processors.end(); itp++)
  {
    (*itp)->start(nr);
  }

  _running = true;
  _gThread=std::thread(&pm::pmMerger::scanMemory, this);
  //_gThread.create_thread(boost::bind(&pm::pmMerger::processEvents, this));
}
void pmMerger::scanMemory()
{
  this->enablePolling();
  this->poll();
}
void pmMerger::stop()
{
  _running = false;
  this->disablePolling();
  PM_INFO(_logPdaq, "Stopping the threads");
  //  printf("ZmMeger =>Stopping the threads \n");
  _gThread.join();

  // Do the stop of the the processors
  PM_INFO(_logPdaq, "Stopping theprocessors");
  //printf("ZmMeger =>Stopping the processors \n");
  for (std::vector<pm::evbprocessor *>::iterator itp = _processors.begin(); itp != _processors.end(); itp++)
  {
    (*itp)->stop();
  }
  PM_INFO(_logPdaq, "Leaving Stop method");
}
void pmMerger::processData(std::string idd, zmq::message_t *message)
{
  //printf("Processing %s  Map size %d \n",idd.c_str(),_eventMap.size());

  if (this->registered())
    _nDifs = this->registered();
  uint32_t detid, sid, gtc;
  uint64_t bx;
  sscanf(idd.c_str(), "DS-%d-%d %d %ld", &detid, &sid, &gtc, &bx);
  //fprintf(stderr,"Message %s DS-%d-%d %d %ld\n",idd.c_str(),detid,sid,gtc,bx);
  std::map<uint64_t, std::vector<pm::buffer *>>::iterator it_gtc = _eventMap.find(gtc);
  if (gtc % 20 == 0)
    PM_INFO(_logPdaq, "Event Map size " << _eventMap.size());
  pm::buffer *b = new pm::buffer(512 * 1024);
  // uint32_t* iptr=(uint32_t*) message->data();
  //   uint8_t* cptr=(uint8_t*) message->data();
  //   uint64_t* iptr64=(uint64_t*) &cptr[12];
  // printf("Message 0) %x %d %d %ld \n",iptr[0],iptr[1],iptr[2],iptr64[0]);

  memcpy(b->ptr(), message->data(), message->size());
  b->setSize(message->size());
  _compressedSize += b->payloadSize();
  // printf("Message 1) %d %d %d \n",b->detectorId(),b->dataSourceId(),b->eventId());
  b->uncompress();

  _totalSize += b->payloadSize();
  // printf("Message 2) %d %d %d \n",b->detectorId(),b->dataSourceId(),b->eventId());
  if (it_gtc != _eventMap.end())
    it_gtc->second.push_back(b);
  else
  {
    std::vector<pm::buffer *> v;
    v.clear();
    v.push_back(b);

    std::pair<uint64_t, std::vector<pm::buffer *>> p(gtc, v);
    _eventMap.insert(p);
    it_gtc = _eventMap.find(gtc);
  }
  int32_t lastgtc = 0;
  for (std::map<uint64_t, std::vector<pm::buffer *>>::iterator itm = _eventMap.begin(); itm != _eventMap.end(); itm++)
  {
    if (itm->second.size() == this->numberOfDataSource())
    {
      //if (it_gtc->first%100==0)
      //printf("GTC %lu %lu  %d\n",itm->first,itm->second.size(),this->numberOfDataSource());
      this->processEvent(itm->first);
      lastgtc = itm->first;
    }
  }
  // Clear writen event
  for (std::map<uint64_t, std::vector<pm::buffer *>>::iterator it = _eventMap.begin(); it != _eventMap.end();)
  {

    if (it->second.size() == this->numberOfDataSource())
    {
      //std::cout<<"Deleting Event "<<it->first<<" Last gtc "<<lastgtc<<std::endl;
      for (std::vector<pm::buffer *>::iterator iv = it->second.begin(); iv != it->second.end(); iv++)
        delete (*iv);
      it->second.clear();
      _eventMap.erase(it++);
    }
    else
      it++;
  }

  // Clear old uncompleted event
  if (_purge)
  {
    if (gtc % 20 == 0)
      PM_INFO(_logPdaq, "PURGING size " << _eventMap.size());
    for (std::map<uint64_t, std::vector<pm::buffer *>>::iterator it = _eventMap.begin(); it != _eventMap.end();)
    {

      if (it->first + 1000 < lastgtc)
      {
        //std::cout<<"Deleting Event "<<it->first<<" Last gtc "<<lastgtc<<std::endl;
        for (std::vector<pm::buffer *>::iterator iv = it->second.begin(); iv != it->second.end(); iv++)
          delete (*iv);
        it->second.clear();
        _eventMap.erase(it++);
      }
      else
        it++;
    }
    // Force purge if size>200
    if (_eventMap.size() > 200)
    {
      PM_INFO(_logPdaq, "REAL PURGING size " << _eventMap.size());
      for (std::map<uint64_t, std::vector<pm::buffer *>>::iterator it = _eventMap.begin(); it != _eventMap.end();)
      {

        if (it->first > 1)
        {
          std::cout << "Deleting Event " << it->first << " Last gtc " << lastgtc << " size " << it->second.size() << std::endl;
          for (std::vector<pm::buffer *>::iterator iv = it->second.begin(); iv != it->second.end(); iv++)
            delete (*iv);
          it->second.clear();
          _eventMap.erase(it++);
        }
        else
          it++;
      }
      PM_INFO(_logPdaq, "END PURGING size " << _eventMap.size());
    }
  }
  // Fill summary
  std::stringstream ss;
  ss << "DS-" << detid << "-" << sid;

  std::map<std::string, uint64_t>::iterator itsum = _mReceived.find(ss.str());
  if (itsum == _mReceived.end())
  {
    std::pair<std::string, uint64_t> p(ss.str(), 1);
    _mReceived.insert(p);
  }
  else
    itsum->second++;
}
void pmMerger::summary()
{
  for (auto x : _mReceived)
  {
    printf("%s => %lu \n", x.first.c_str(), x.second);
  }
}

web::json::value pmMerger::status()
{
  web::json::value jrep, jsta;
  jsta["run"] = json::value::number(_run);
  jsta["event"] = json::value::number(_evt);
  jsta["build"] = json::value::number(_build);
  jsta["compressed"] = json::value::number(_compressedSize);
  jsta["total"] = json::value::number(_totalSize);

  jsta["running"] = json::value::number(_running);
  jsta["purge"] =json::value::number( _purge);
  jsta["size"] = json::value::number((uint32_t)_eventMap.size());
  jsta["registered"] = json::value::number(this->registered());
  uint32_t nr=0;
  for (auto x : _mReceived)
  {
    json::value jds;
    jds["id"] = json::value::string(U(x.first));
    jds["received"] = json::value::number(x.second);
    jrep[nr++]=jds;
  }
  jsta["difs"] = jrep;
  return jsta;
}
