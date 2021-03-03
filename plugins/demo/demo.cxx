#include "demo.hh"
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
#include <sys/syscall.h>
demo::demo() {}


void demo::initialise()
{
  this->addState("CONFIGURED");
  this->addState("RUNNING");
  this->addTransition("CONFIGURE","CREATED","CONFIGURED",std::bind(&demo::configure,this,std::placeholders::_1));
  this->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",std::bind(&demo::configure,this,std::placeholders::_1));
  this->addTransition("START","CONFIGURED","RUNNING",std::bind(&demo::start,this,std::placeholders::_1));
  this->addTransition("STOP","RUNNING","CONFIGURED",std::bind(&demo::stop,this,std::placeholders::_1));
  this->addCommand("STATUS",std::bind(&demo::status,this,std::placeholders::_1));

  _event=0;
  _started=false;
}
void demo::terminate()
{
  if (!_started) return;
  _started=false;
  _thr.join();
}

void demo::configure(http_request message)
{
  auto par = json::value::object();
  par["status"] = json::value::string(U("GOOD_STATUS"));
  par["rc"] = json::value::number(1000);
  message.reply(status_codes::OK,par);
}
void demo::status(http_request message)
{
  auto par = json::value::object();
  par["event"] = json::value::number(_event);
  par["rc"] = json::value::number(1000);
  message.reply(status_codes::OK,par);
}
void demo::readEvent()
{
  pid_t  tid=syscall(__NR_gettid);
  while (_started)
    {
      _event++;
      if (_event%100==0)
	ucout<<_p_session<<"/"<<_p_name<<"/"<<_p_instance<<" evt "<<_event<<" pid "<<tid<<std::endl;
      ::usleep(5000);
    }
  ucout<<"Run finished"<<_event<<std::endl;
}
void demo::start(http_request message)
{
  auto par = json::value::object();
  _started=true;
  _event=0;
  _thr=std::thread(&demo::readEvent,this);


  
  par["status"] = json::value::string(U("RUNNING"));
  par["rc"] = json::value::number(2000);
  message.reply(status_codes::OK,par);
  
}
void demo::stop(http_request message)
{
  _started=false;
  _thr.join();
  auto par = json::value::object();
  par["status"] = json::value::string(U("STOPPED"));
  par["rc"] = json::value::number(3000);
  par["event"] = json::value::number(_event);
  message.reply(status_codes::OK,par);
}

extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new demo);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
