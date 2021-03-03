#include "dummy.hh"
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

dummy::dummy() : _p_session(""),_p_name(""),_p_instance(0) {}

std::vector<std::string> dummy::getPaths(std::string query)
{
  auto querym = uri::split_query(query);
 for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
   {
     ucout << U("Query") << U(" ")
           << it2->first << U(" ") << it2->second << std::endl;
     if (it2->first.compare("session")==0)
       _p_session.assign(it2->second);
     if (it2->first.compare("name")==0)
       _p_name.assign(it2->second);
     if (it2->first.compare("instance")==0)
       _p_instance=std::stoi(it2->second);
   }
 std::stringstream sb;
 sb<<"/"<<_p_session<<"/"<<_p_name<<"/"<<_p_instance<<"/";
 _basePath=sb.str();
  this->initialise();
  this->addCommand("LIST",std::bind(&dummy::list,this,std::placeholders::_1));
  // Construct list of commands
  std::vector<std::string> v;
  for (auto it=_commands.begin();it!=_commands.end();it++)
    {v.push_back(it->first);}
  return v;
}

void dummy::processRequest(http_request& message)
{
  auto itf=_commands.find(uri::decode(message.relative_uri().path()));
  if (itf==_commands.end())
    message.reply(status_codes::BadRequest, "{\"error\":\"Invalid path no handler found\"}");
  else
    itf->second(message);
}
void dummy::addCommand(std::string s,CMDFunctor f)
{
  std::string cmd=_basePath+s;
  auto itf=_commands.find(cmd);
  if (itf==_commands.end())
    {
      std::pair<std::string,CMDFunctor> p(cmd,f);
      _commands.insert(p);
    }

}
void dummy::initialise()
{
  this->addCommand("CONFIGURE",std::bind(&dummy::configure,this,std::placeholders::_1));
  this->addCommand("START",std::bind(&dummy::start,this,std::placeholders::_1));
  this->addCommand("STOP",std::bind(&dummy::stop,this,std::placeholders::_1));
  this->addCommand("STATUS",std::bind(&dummy::status,this,std::placeholders::_1));

  _event=0;
  _started=false;
}
void dummy::configure(http_request message)
{
  auto par = json::value::object();
  par["status"] = json::value::string(U("GOOD_STATUS"));
  par["rc"] = json::value::number(1000);
  message.reply(status_codes::OK,par);
}
void dummy::status(http_request message)
{
  auto par = json::value::object();
  par["event"] = json::value::number(_event);
  par["rc"] = json::value::number(1000);
  message.reply(status_codes::OK,par);
}
void dummy::readEvent()
{
  while (_started)
    {
      _event++;
      if (_event%100==0)
	ucout<<_p_session<<" evt "<<_event<<std::endl;
      ::usleep(5000);
    }
  ucout<<"Run finished"<<_event<<std::endl;
}
void dummy::start(http_request message)
{
  auto par = json::value::object();
  _started=true;
  _thr=std::thread(&dummy::readEvent,this);


  
  par["status"] = json::value::string(U("RUNNING"));
  par["rc"] = json::value::number(2000);
  message.reply(status_codes::OK,par);
  
}
void dummy::stop(http_request message)
{
  _started=false;
  _thr.join();
  auto par = json::value::object();
  par["status"] = json::value::string(U("STOPPED"));
  par["rc"] = json::value::number(3000);
  message.reply(status_codes::OK,par);
}
void dummy::terminate()
{
  if (!_started) return;
  _started=false;
  _thr.join();
}
void dummy::list(http_request message)
{
  auto par = json::value();
  int np=0;
  for (auto it=_commands.begin();it!=_commands.end();it++)
    {par[np]=json::value::string(U(it->first));np++;}
  message.reply(status_codes::OK,par);
}

extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new dummy);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
