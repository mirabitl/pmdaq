#include "udp_data_source.hh"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "stdafx.hh"
#include <iomanip>
#include <mutex>


udp_data_source::udp_data_source() 
{
   _throttle_low_limit = 10000;
   _throttle_high_limit = 20000;
   _run = 0;
   _nacq = 0;
   _gtc = 0;
   _throttle_start = false;
}

void udp_data_source::initialise()
{

  // Register state

  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");

  this->addTransition("INITIALISE", "CREATED", "INITIALISED", std::bind(&udp_data_source::fsm_initialise, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "INITIALISED", "CONFIGURED", std::bind(&udp_data_source::configure, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "CONFIGURED", "CONFIGURED", std::bind(&udp_data_source::configure, this, std::placeholders::_1));

  this->addTransition("START", "CONFIGURED", "RUNNING", std::bind(&udp_data_source::start, this, std::placeholders::_1));
  this->addTransition("STOP", "RUNNING", "CONFIGURED", std::bind(&udp_data_source::stop, this, std::placeholders::_1));
  this->addTransition("DESTROY", "CONFIGURED", "CREATED", std::bind(&udp_data_source::destroy, this, std::placeholders::_1));
  this->addCommand("STATUS", std::bind(&udp_data_source::c_status, this, std::placeholders::_1));
  this->addCommand("PAUSE", std::bind(&udp_data_source::c_pause, this, std::placeholders::_1));
  this->addCommand("RESUME", std::bind(&udp_data_source::c_resume, this, std::placeholders::_1));

  this->addCommand("ADDTOKENS", std::bind(&udp_data_source::c_add_tokens, this, std::placeholders::_1));

  // std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
}
void udp_data_source::end()
{
  // Stop any running process

  if (_producer_thread != NULL)
    {
      _running = false;
      _producer_thread->join();
      delete _producer_thread;
      _producer_thread = NULL;
    }
}

json::value udp_data_source::status()
{
  json::value obj;
  obj["tokens"] = json::value::number(_tokens.load());
  if (_running) {
    obj["run"] = json::value::number(_run);
    obj["event"] = json::value::number(_gtc);
  }
  return obj;
}


void udp_data_source::c_status(http_request m)
{
  PM_INFO(_logUdpDS, "Status CMD called ");
  auto par = json::value::object();

  par["STATUS"] = this->status();
  par["DETID"] = _detId;
  par["SOURCEID"] = _sourceId;
  mqtt_publish("status",par);
  Reply(status_codes::OK, par);
}
void udp_data_source::c_pause(http_request m)
{
  PM_INFO(_logUdpDS, "Pause CMD called ");
  auto par = json::value::object();

  _pause=true;
  par["STATUS"] = this->status();
  par["DETID"] = _detId;
  par["SOURCEID"] = _sourceId;
  mqtt_publish("status",par);
  Reply(status_codes::OK, par);
}
void udp_data_source::c_resume(http_request m)
{
  PM_INFO(_logUdpDS, "Resumee CMD called ");
  auto par = json::value::object();

  _pause=false;
  par["STATUS"] = this->status();
  par["DETID"] = _detId;
  par["SOURCEID"] = _sourceId;
  mqtt_publish("status",par);
  Reply(status_codes::OK, par);
}

void udp_data_source::c_add_tokens(http_request m)
{
  PM_INFO(_logUdpDS, "Add tokens called ");
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));
  uint32_t tokens = utils::queryIntValue(m, "tokens", 10000);
  _tokens+=tokens;
  par["TOKENS"] = json::value::number(_tokens.load());
  Reply(status_codes::OK, par);
}

void udp_data_source::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PM_INFO(_logUdpDS, "****** CMD: INITIALISING");
  // Send Initialise command
  if (params().as_object().find("udp") == params().as_object().end())
  {
    PMF_ERROR(_logUdpDS, "No udp tag");
    par["status"] = json::value::string(U("Missing febv1 tag "));
    Reply(status_codes::OK, par);
    return;
  }
  
  PMF_INFO(_logUdpDS, " Init done  ");
  par["status"] = this->status();
  Reply(status_codes::OK, par);
}

void udp_data_source::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logUdpDS, " CMD: Configuring");

  _tokens = 100000;
  _throttle_low_limit = 10000;
  _throttle_high_limit = 20000;
  _run = 0;
  _nacq = 0;
  par["status"] = this->status();


  Reply(status_codes::OK, par);
}

/////////////////////////////////////////////////////////
void udp_data_source::start(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logUdpDS, " CMD: STARTING");
  // Now Send the START COMMAND
  _run = utils::queryIntValue(m, "run", 1);
  if (_running) return;
  _running = true;
 
  _producer_thread = new std::thread(&udp_data_source::acquiring_data, this);

  par["status"] = this->status();
  Reply(status_codes::OK, par);
}
void udp_data_source::stop(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logUdpDS, " CMD: STOPPING ");

  // Now Send the STOP COMMAND
  if (_producer_thread != NULL && _running)
    {
      _running = false;
      if (_producer_thread->joinable())
	_producer_thread->join();
      delete _producer_thread;
      _producer_thread = NULL;
    }
  par["status"] = this->status();

  Reply(status_codes::OK, par);
}
void udp_data_source::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logUdpDS, " CMD: DESTROYING");
  // Now Send the DESTROY COMMAND
  if (_producer_thread != NULL)
    {
      _running = false;
      _producer_thread->join();
      delete _producer_thread;
      _producer_thread = NULL;
    }

  par["answer"] = this->status();

  Reply(status_codes::OK, par);

  // To be done: _ShmDS->clear();
}
void udp_data_source::acquiring_data() {
  //const char* UDP_IP = "192.168.100.1";
  //const int UDP_PORT = 8765;
  const int PACKET_SIZE = 1472;
  web::json::value judp = params()["udp"];
  const char* UDP_IP =judp["ip"].as_string().c_str();
  const int UDP_PORT = judp["port"].as_integer();
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  sockaddr_in servaddr{};
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(UDP_PORT);
  inet_pton(AF_INET, UDP_IP, &servaddr.sin_addr);
  
  std::vector<uint8_t> packet(PACKET_SIZE, 0);
  for (int i = 20; i < 1400; ++i)
    packet[i] = i % 255;
  
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> dist(1, 247);
  std::vector<int> vs(100000);
  for (auto& v : vs) v = dist(rng);

  _nacq = 0;
  _gtc = 0;
  int offset = 0, elen = 0, rlen = 0;
  double t0 = current_time();

  while (_running) {
    if (throttle(offset)) continue;
    if (_pause) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      continue;
    }

    if (offset == 0) {
      elen = vs[_gtc % 100000] * PACKET_SIZE;
      _gtc++;
      rlen = elen;
    }

    _nacq++;
    writeInt(packet.data(), _nacq, 0);
    writeInt(packet.data(), offset, 4);
    writeInt(packet.data(), elen, 8);
    writeInt(packet.data(), _gtc, 12);
    writeInt(packet.data(), _tokens.load(), 16);

    rlen -= PACKET_SIZE;
    offset += PACKET_SIZE;
    if (rlen < 1) offset = 0;

    if (_nacq % 3000 == 1 && _nacq > 1) {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
      double t = current_time();
      double MB = _nacq * PACKET_SIZE / (1024.0 * 1024.0);
      double dt = t - t0;
      double MBS = (MB / dt) * 8;
      PMF_INFO(_logUdpDS, "Event " + std::to_string(_gtc) + " Packet " + std::to_string(_nacq) +" MB " + format(MB) + " Throughput Mbit/s " + format(MBS));
    }

    sendto(sockfd, packet.data(), PACKET_SIZE, 0,
	   (sockaddr*)&servaddr, sizeof(servaddr));
    _tokens--;
  }

  close(sockfd);
  PMF_INFO(_logUdpDS, "Acquisition thread finished, Run " + std::to_string(_run));
}

bool udp_data_source::throttle(int offset) {
  if (offset != 0) return false;
  if (_tokens < _throttle_low_limit) {
            _throttle_start = true;
            return true;}
  return _throttle_start && _tokens < _throttle_high_limit;
}

void udp_data_source::writeInt(uint8_t* buffer, int value, int offset) {
  buffer[offset] = (value >> 24) & 0xFF;
  buffer[offset + 1] = (value >> 16) & 0xFF;
  buffer[offset + 2] = (value >> 8) & 0xFF;
  buffer[offset + 3] = value & 0xFF;
}

double udp_data_source::current_time() {
  using namespace std::chrono;
  return duration_cast<duration<double>>(steady_clock::now().time_since_epoch()).count();
}

std::string udp_data_source::format(double value) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << value;
  return oss.str();
}

extern "C"
{
  // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.
  handlerPlugin *loadProcessor(void)
  {
    return (new udp_data_source);
  }
  // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed
  // to it.  This isn't a very safe function, since there's no
  // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin *obj)
  {
    delete obj;
  }
}
