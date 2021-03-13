#include "monitor.hh"
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
#include <dlfcn.h>



using namespace monitoring;

monitoring::supervisor::supervisor() : _running(false),_period(30)
{
}
void monitoring::supervisor::initialise()
{
  // Register state
  this->addState("CREATED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");

  // Register transitions
  this->addTransition("CONFIGURE", "CREATED", "CONFIGURED", std::bind(&monitoring::supervisor::configure, this,std::placeholders::_1));
  this->addTransition("START", "CONFIGURED", "RUNNING",std::bind(&monitoring::supervisor::start, this,std::placeholders::_1));
  this->addTransition("STOP", "RUNNING", "CONFIGURED",std::bind(&monitoring::supervisor::stop, this,std::placeholders::_1));
  this->addTransition("HALT", "CONFIGURED", "CREATED", std::bind(&monitoring::supervisor::halt, this,std::placeholders::_1));

  // Standalone command
  this->addCommand("STATUS",std::bind(&monitoring::supervisor::c_status, this,std::placeholders::_1));

  this->registerCommands();

}
void monitoring::supervisor::end()
{
  if (!_running) return;
  _running=false;
  _thr.join();
}

void monitoring::supervisor::configure(http_request m)
{
  PM_INFO(_logPdaq,  "Configure received");
  auto prep = json::value::object();
  // Store message content in paramters
  auto querym = uri::split_query(uri::decode(m.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    if (it2->first.compare("period")==0)
      {
	params()["period"]=json::value::string(U(it2->second)); 
      }
  // Check that needed parameters exists

  
  if (params().as_object().find("period")==params().as_object().end())
    {
      PM_ERROR(_logPdaq, "Missing period, period of monitoring readout");
      prep["error"]=json::value::string(U( "Missing period, period of monitoring readout"));
      Reply(status_codes::BadRequest,prep);
      return;
    }
  else
    _period=params()["period"].as_integer();
  // Look for zmonStore declarations

   if (params().as_object().find("stores")!=params().as_object().end())
    {
      auto parray_keys = json::value::array();int ns=0;
      for (auto it = params()["stores"].as_array().begin(); it != params()["stores"].as_array().end(); ++it)
	{
	  PM_INFO(_logPdaq, "registering " << (*it).as_string());
	  this->registerStore((*it).as_string());
	  parray_keys[ns++]=json::value::string(U((*it).as_string()));
	}

      PM_INFO(_logPdaq, " Setting parameters for stores ");
      for (auto x:_stores)
	x->loadParameters(params());
      PM_INFO(_logPdaq, " Connect stores  ");
      for (auto x:_stores)
	x->connect();
      prep["stores"] = parray_keys;
    }

  // 

  this->open();

  PM_DEBUG(_logPdaq, "end of configure");
  Reply(status_codes::OK,prep);
  return;
}
void monitoring::supervisor::registerStore(std::string name)
{
  std::stringstream s;
  s << "lib" << name << ".so";
  void *library = dlopen(s.str().c_str(), RTLD_NOW);

  //printf("%s %x \n",dlerror(),(unsigned int) library);
  PM_INFO(_logPdaq, " Error " << dlerror() << " Library open address " << std::hex << library << std::dec);
  // Get the loadFilter function, for loading objects
  monitoring::monStore *(*create)();
  create = (monitoring::monStore * (*)()) dlsym(library, "loadStore");
  PM_INFO(_logPdaq, " Error " << dlerror() << " file " << s.str() << " loads to processor address " << std::hex << create << std::dec);
  //printf("%s %x \n",dlerror(),(unsigned int) create);
  // printf("%s lods to %x \n",s.str().c_str(),(unsigned int) create);
  //void (*destroy)(Filter*);
  // destroy = (void (*)(Filter*))dlsym(library, "deleteFilter");
  // Get a new filter object
  monitoring::monStore *a = (monitoring::monStore *)create();
  _stores.push_back(a);

}

void monitoring::supervisor::start(http_request m)
{
  PM_DEBUG(_logPdaq, "Received Start ");
  auto par = json::value::object();


    _thr=std::thread(&monitoring::supervisor::monitor,this);
	
  _running = true;
  par["status"] = json::value::string(U("RUNNING"));
  Reply(status_codes::OK,par);

}
void monitoring::supervisor::stop(http_request m)
{
  PM_DEBUG(_logPdaq, "Received STOP");
  if (_running)
    {
      _running=false;
      _thr.join();
    }
  auto par = json::value::object();
  par["status"] = json::value::string(U("STOPPED"));
  Reply(status_codes::OK,par);
 
}
void monitoring::supervisor::halt(http_request m)
{
  PM_DEBUG(_logPdaq, "Received HALT");
  if (_running)
    {
      _running=false;
      _thr.join();
    }
  this->close();
  auto par = json::value::object();
  par["status"] = json::value::string(U("HALTED"));
  Reply(status_codes::OK,par);
 
}
void monitoring::supervisor::c_status(http_request m)
{
  PM_DEBUG(_logPdaq, "Received CMD STATUS");
  auto par = json::value::object();
  par["status"] = this->status();
  Reply(status_codes::OK,par);
 
}

web::json::value monitoring::supervisor::status()
{
  
 auto par = json::value::object();
 par["status"]=json::value::string(U("NOTIMPLEMENTED"));
 return par;
}
std::string  monitoring::supervisor::hardware(){return "NOTIMPLEMENTED";}
void monitoring::supervisor::registerCommands(){;}
void monitoring::supervisor::open(){;}
void monitoring::supervisor::close(){;}


void monitoring::supervisor::monitor()
{

  while (_running)
    {
    
      // Save the status in all conncted stores
	  for (auto y:_stores)
	    y->store(this->session(),this->hardware(),(uint32_t) time(0),this->status());
	
      if (!_running) break;
      ::sleep(_period);

      std::cout<<"End of monitoring task"<<std::endl;

    }
}
