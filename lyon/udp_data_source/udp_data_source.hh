#pragma once

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
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <cstring>
#include <random>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <string>
#include <map>
static LoggerPtr _logUdpDS(Logger::getLogger("PMDAQ_UDP_DS"));

class udp_data_source : public fsmw
{
public:
  udp_data_source();

  virtual void initialise();
  virtual void end();

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

  web::json::value status();
  /// STATUS Command handler
  void c_status(http_request m);
  /// STATUS Command handler
  void c_pause(http_request m);
  /// STATUS Command handler
  void c_resume(http_request m);

  /// DOWNLOADDB Command handler
  void c_add_tokens(http_request m);
  /// UDP managment
  void acquiring_data();
  bool throttle(int offset);
  static void writeInt(uint8_t* buffer, int value, int offset);
  static double current_time();
  static std::string format(double value);
private:
 uint32_t _detId, _sourceId;
 uint32_t _run, _gtc,_nacq;
  std::atomic<bool> _running;
  std::atomic<int> _tokens;
  int _throttle_low_limit, _throttle_high_limit;
  bool _throttle_start, _pause;
  std::thread* _producer_thread;



};
