#pragma once
#include "pmSender.hh"
#include <thread>

#include "fsmw.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "stdafx.hh"
#include "utils.hh"
static LoggerPtr _logFebv2(Logger::getLogger("PMDAQ_FEBV2"));

class Febv2Manager : public fsmw
{
public:
  Febv2Manager();

  virtual void initialise();
  virtual void end();

  /// Create the feb  handler
  void createfeb(http_request m);
  /// INITIALISE  handler
  void fsm_initialise(http_request m);
  /// CONFIGURE  handler
  void configure(http_request m);
  /// START  handler
  void start(http_request m);
  /// STOP  handler
  void stop(http_request m);
  /// DESTROY  handler
  void destroy(http_request m);

  /// STATUS Command handler
  void c_status(http_request m);

  /// DOWNLOADDB Command handler
  void c_downloadDB(http_request m);
  /// DOWNLOADDB Command handler
  void c_vthshift(http_request m);
  void c_paccomp(http_request m);
  void c_delay_reset(http_request m);
  /// SHM managment
  void clearShm();
  void spy_shm();

  web::json::value post(std::string command, web::json::value v = web::json::value::null());
  web::json::value decode_spyne_answer(web::json::value v, std::string c);

private:
  uint32_t _run, _type;
  uint8_t _delay;
  uint8_t _duration;
  std::string _feb_host;
  uint32_t _feb_port, _detId, _sourceId;

  pm::pmSender *_dsData;
  zmq::context_t *_context;
  bool _running;
  std::string _shmPath;
  std::thread *g_mon;
};
