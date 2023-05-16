#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sstream>
#include <iostream>
#include "pmSender.hh"
#include "stdafx.hh"
#include "utils.hh"
#include "pmLogger.hh"
#include "zhelpers.hpp"

#include <unistd.h>
using namespace pm;
pmSender::pmSender(zmq::context_t *c, uint32_t det, uint32_t dif) : _detId(det), _sourceId(dif), _context(c), _compress(true)
{
  std::stringstream sheader;
  sheader << "DS-" << det << "-" << dif;
  _header = sheader.str();
  _buffer = new pm::buffer(1024 * 1024);

  _buffer->setDetectorId(det);
  _buffer->setDataSourceId(dif);
}
void pmSender::connect(std::string dest)
{
  zmq::socket_t *sender = new zmq::socket_t((*_context), ZMQ_PUSH);
  sender->setsockopt(ZMQ_SNDHWM, 128);
  sender->connect(dest);
  _vSender.push_back(sender);
}

char *pmSender::payload() { return _buffer->payload(); }
void pmSender::publish(uint64_t bx, uint32_t gtc, uint32_t len)
{
  std::stringstream ss;
  //printf("SENDER detid %x \n",_buffer->detectorId());
  ss << _header << " " << gtc << " " << bx;
  try
  {
    //s_sendmore((*_sender),ss.str());
    //std::cout<<" bx"<<(uint64_t) bx<<" "<<ss.str()<<"\n";
    zmq::message_t message1(ss.str().size());
    memcpy(message1.data(), ss.str().data(), ss.str().size());

    bool rc = _vSender[gtc % _vSender.size()]->send(message1, ZMQ_SNDMORE);

    _buffer->setBxId(bx);
    _buffer->setEventId(gtc);
    _buffer->setPayloadSize(len);

    if (_compress)
    {
      uint32_t bb = _buffer->size();

      _buffer->compress();
      PM_DEBUG(_logPdaq, " Compressing data" << _buffer->size());
    }
    zmq::message_t message(_buffer->size());
    memcpy(message.data(), _buffer->ptr(), _buffer->size());
    _vSender[gtc % _vSender.size()]->send(message);
  }
  catch (zmq::error_t e)
  {
    PM_ERROR(_logPdaq, e.num() << " error number");
    return;
  }
}
void pmSender::collectorRegister()
{
  std::stringstream ss;
  //printf("SENDER detid %x \n",_buffer->detectorId());
  ss << "ID-" << _detId << "-" << _sourceId;
  for (auto snd = _vSender.begin(); snd != _vSender.end(); snd++)
  {
    try
    {
      //s_sendmore((*snd),ss.str());
      //std::cout<<" bx"<<(uint64_t) bx<<" "<<ss.str()<<"\n";
      zmq::message_t message1(ss.str().size());
      memcpy(message1.data(), ss.str().data(), ss.str().size());

      bool rc = (*snd)->send(message1, ZMQ_SNDMORE);

      _buffer->setBxId(0);
      _buffer->setEventId(0);
      _buffer->setPayloadSize(64);

      zmq::message_t message(_buffer->size());
      memcpy(message.data(), _buffer->ptr(), _buffer->size());
      (*snd)->send(message);
    }
    catch (zmq::error_t e)
    {
      PM_ERROR(_logPdaq, e.num() << " error number");

      return;
    }
  }
}
void pmSender::autoDiscover(std::string session, std::string appname, std::string portname)
{

  std::map<uint32_t, std::string> mStream;
  mStream.clear();
  // Request PNS session process with process name = appname
  if (!utils::checkpns())
  {
    PM_ERROR(_logPdaq, "No Process Name Server found");
    return;
  }

  http_response rep = utils::request(utils::pns_name(), 8888, "/PNS/LIST", json::value::null());

  auto reg_list = rep.extract_json();
  auto serv_list = reg_list.get().as_object()["REGISTERED"];
  if (serv_list.is_null())
  {
    PM_ERROR(_logPdaq, "No Process registered in PNS");
    return;
  }
  for (auto it = serv_list.as_array().begin(); it != serv_list.as_array().end(); ++it)
  {
    std::string rec = (*it).as_string();
    PM_DEBUG(_logPdaq, "PNS Service: " << rec);
    auto v = utils::split(rec, ':');
    std::string phost = v[0];
    uint32_t pport = std::stoi(v[1]);
    std::string ppath = v[2];
    PM_DEBUG(_logPdaq, "PNS Service: " << phost << " " << pport << " " << ppath);
    auto vp0 = utils::split(ppath, '?');
    auto vp = utils::split(vp0[0], '/');
    PM_DEBUG(_logPdaq, "VP size: " << vp.size());
    PM_DEBUG(_logPdaq, "VP :  [1]" << vp[1] << " " << session << " [2]" << vp[2] << " " << appname);
    if (vp[1].compare(session) != 0)
      continue;
    if (vp[2].compare(appname) != 0)
      continue;
    uint32_t instance = std::stoi(vp[3]);
    http_response repb = utils::request(phost, pport, vp0[0] + "PARAMS", json::value::null());
    PM_ERROR(_logPdaq, repb.to_string());
    auto par_list = repb.extract_json().get();

    for (auto iter = par_list.as_object().begin(); iter != par_list.as_object().end(); ++iter)
      if (iter->first.compare(portname) == 0)
      {

        std::stringstream ss;
        ss << "tcp://" << phost << ":" << iter->second.as_integer();
        std::pair<uint32_t, std::string> p(instance, ss.str());
        mStream.insert(p);
        PM_INFO(_logPdaq, " Builder paramaters " << phost << " collecting on " << ss.str());
        break;
      }
  }
  // Connect to the specified streams
  for (uint32_t i = 0; i < mStream.size(); i++)
    this->connect(mStream[i]);
}
