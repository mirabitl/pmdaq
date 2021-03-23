#include "pns.hh"
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
#include "utils.hh"
// std::vector<std::string> split(const std::string &s, char delim)
// {
//   std::stringstream ss(s);
//   std::string item;
//   std::vector<std::string> elems;
//   while (std::getline(ss, item, delim))
//   {
//     elems.push_back(item);
//     // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
//   }
//   return elems;
// }

pns::pns() : _host(""), _port(0)
{
  _services.clear();
  _sessions.clear();
}

std::vector<std::string> pns::getPaths(std::string query)
{
  if (_port == 0)
  {
    ucout << url() << std::endl;
    // remove http:// and trailing / so
    auto v = utils::split(url().substr(7, url().length() - 8), ':');
    _host.assign(v[0]);
    _port = std::stoi(v[1]);

    ucout << " HOST found " << _host << " Port:" << _port << std::endl;
  }
  // Construct list of commands
  std::vector<std::string> v;
  v.push_back("/PNS/UPDATE");
  v.push_back("/PNS/LIST");
  v.push_back("/PNS/REMOVE");
  v.push_back("/PNS/PURGE");
  v.push_back("/PNS/SESSION/UPDATE");
  v.push_back("/PNS/SESSION/LIST");
  v.push_back("/PNS/SESSION/PURGE");
  return v;
}

void pns::processRequest(http_request &message)
{
  std::string cmd = uri::decode(message.relative_uri().path());
  if (cmd.compare("/PNS/UPDATE") == 0)
    this->update(message);
  else if (cmd.compare("/PNS/LIST") == 0)
    this->list(message);
  else if (cmd.compare("/PNS/REMOVE") == 0)
    this->remove(message);
  else if (cmd.compare("/PNS/PURGE") == 0)
    this->purge(message);
  else if (cmd.compare("/PNS/SESSION/LIST") == 0)
    this->session_list(message);
  else if (cmd.compare("/PNS/SESSION_UPDATE") == 0)
    this->session_update(message);
  else if (cmd.compare("/PNS/SESSION_PURGE") == 0)
    this->session_purge(message);
  else
  {
    json::value jrep;
    jrep["status"] = json::value::string(U("FAILED"));
    std::stringstream s0;
    s0.str(std::string());
    s0 << uri::decode(message.relative_uri().path()) << "not a PNS command ";
    jrep["comment"] = json::value::string(U(s0.str()));
    message.reply(status_codes::BadRequest, jrep);
  }
}
void pns::terminate()
{
}
json::value pns::registered(std::string r_session)
{
  auto par = json::value();
  int np = 0;
  for (auto it = _services.begin(); it != _services.end(); it++)
  {
    auto v = utils::split(it->first, ':');
    auto vp= utils::split(v[2],'/');
    if (r_session.compare("NONE")!=0)
      if (vp[3].compare(r_session)!=0) continue;

    utility::ostringstream_t buf;
    buf << U(it->first) << U("?") << U(it->second);
    par[np] = json::value::string(buf.str());
    np++;
  }
  return par;
}
void pns::list(http_request message)
{
  auto rep = json::value();
  auto r_session=utils::queryStringValue(message,"session","NONE");
  rep["REGISTERED"] = registered(r_session);
  message.reply(status_codes::OK, rep);
}
void pns::update(http_request message)
{

  //Decode and build path
  auto querym = uri::split_query(uri::decode(message.relative_uri().query()));
  std::string host, port, path, state;
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
  {
    if (it2->first.compare("host") == 0)
      host.assign(it2->second);
    if (it2->first.compare("port") == 0)
      port.assign(it2->second);
    if (it2->first.compare("path") == 0)
      path.assign(it2->second);
    if (it2->first.compare("state") == 0)
      state.assign(it2->second);
  }
  std::stringstream bpath;
  bpath << host << ":" << port << ":" << path;
  std::string cmd = bpath.str();
  auto itf = _services.find(cmd);
  if (itf == _services.end())
  {
    std::pair<std::string, std::string> p(cmd, state);
    _services.insert(p);
  }
  else
    itf->second.assign(state);
  // Return the registered list
  auto rep = json::value();
  rep["REGISTERED"] = registered();
  message.reply(status_codes::OK, rep);
}

void pns::remove(http_request message)
{

  //Decode and build path
  auto querym = uri::split_query(uri::decode(message.relative_uri().query()));
  std::string host, port, path, state;
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
  {
    if (it2->first.compare("host") == 0)
      host.assign(it2->second);
    if (it2->first.compare("port") == 0)
      port.assign(it2->second);
    if (it2->first.compare("path") == 0)
      path.assign(it2->second);
  }
  std::stringstream bpath;
  bpath << host << ":" << port << ":" << path;
  std::string cmd = bpath.str();
  if (path.compare("") != 0)
    for (auto it2 = _services.cbegin(); it2 != _services.cend() /* not hoisted */; /* no increment */)
    {
      auto v = utils::split(it2->first, ':');

      if (v[2].compare(path) == 0)
      {
        ucout << "removing " << it2->first << std::endl;
        _services.erase(it2++); // or "it = m.erase(it)" since C++11
      }
      else
        ++it2;
    }
  else
    PM_ERROR(_logPdaq, " Missing path parameter");

  // Return the registered list
  auto rep = json::value();
  rep["REGISTERED"] = registered();
  message.reply(status_codes::OK, rep);
}

void pns::purge(http_request message)
{

  //Decode and build path
  for (auto it2 = _services.cbegin(); it2 != _services.cend() /* not hoisted */; /* no increment */)
  {
    //auto v=split(it2->second,'?');
    ucout << "path " << it2->first << " state " << it2->second << std::endl;
    if (it2->second.compare("DEAD") == 0)
    {
      ucout << "removing " << it2->first << std::endl;
      _services.erase(it2++); // or "it = m.erase(it)" since C++11
    }
    else
      ++it2;
  }
  // Return the registered list
  auto rep = json::value();
  rep["REGISTERED"] = registered();
  message.reply(status_codes::OK, rep);
}
web::json::value pns::session_registered(std::string r_session)
{
  auto par = json::value();
  int np = 0;
  for (auto it = _sessions.begin(); it != _sessions.end(); it++)
    {
    if (r_session.compare("NONE")!=0)
      if (it->first.compare(r_session)!=0) continue;
    utility::ostringstream_t buf;
    buf << U(it->first) << U(":") << U(it->second);
    par[np] = json::value::string(buf.str());
    np++;
  }
  return par;

}
void pns::session_list(http_request message)
{
  auto rep = json::value();
  auto r_session=utils::queryStringValue(message,"session","NONE");
 
  rep["REGISTERED"] = session_registered(r_session);
  message.reply(status_codes::OK, rep);
}
void pns::session_update(http_request message)
{
  auto rep = json::value();
  //Decode and build path
  auto r_session=utils::queryStringValue(message,"session","NONE");
  if (r_session.compare("NONE")==0)
    {
      rep["status"]=json::value::string(U("Missing session"));
      message.reply(status_codes::OK, rep);
    }
  auto r_state=utils::queryStringValue(message,"state","NONE");
  if (r_state.compare("NONE")==0)
    {
      rep["status"]=json::value::string(U("Missing state"));
      message.reply(status_codes::OK, rep);
    }

  auto itf = _sessions.find(r_session);
  if (itf == _sessions.end())
  {
    std::pair<std::string, std::string> p(r_session,r_state);
    _sessions.insert(p);
  }
  else
    itf->second.assign(r_state);
  // Return the registered list
  rep["REGISTERED"] =session_registered(r_session);
  message.reply(status_codes::OK, rep);
}
void pns::session_purge(http_request message)
{

  //Decode and build path
  for (auto it2 = _sessions.cbegin(); it2 != _sessions.cend() /* not hoisted */; /* no increment */)
  {
    //auto v=split(it2->second,'?');
    ucout << "Session " << it2->first << " state " << it2->second << std::endl;
    if (it2->second.compare("DEAD") == 0)
    {
      ucout << "removing " << it2->first << std::endl;
      _sessions.erase(it2++); // or "it = m.erase(it)" since C++11
    }
    else
      ++it2;
  }
  // Return the registered list
  auto rep = json::value();
  rep["REGISTERED"] = session_registered();
  message.reply(status_codes::OK, rep);
}

extern "C"
{
  // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.
  handlerPlugin *loadProcessor(void)
  {
    return (new pns);
  }
  // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed
  // to it.  This isn't a very safe function, since there's no
  // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin *obj)
  {
    delete obj;
  }
}
