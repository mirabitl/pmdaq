#pragma once
#include <stdio.h>
#include <cpprest/uri.h>
#include <cpprest/http_listener.h>
#include <cpprest/asyncrt_utils.h>
#include "handlerPlugin.hh" 
#include "pluginInfo.hh"
#pragma comment(lib, "cpprest_2_7.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "httpapi.lib")
 
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;
 #undef OLDPLUGIN
class baseServer
{
public:
  baseServer() {}
  baseServer(utility::string_t url);
  pplx::task<void> open() { return m_listener.open(); }
  pplx::task<void> close() { return m_listener.close(); }
  void registerPlugin(std::string name,std::string query);
  std::string url(){return _url;}
private:
 void handle_get_or_post(http_request message);
 http_listener m_listener;
  uint32_t req;
  #ifdef OLDPLUGIN
  std::map<std::string,handlerPlugin*> _plugins;
  #else
  std::map<std::string,pluginInfo<handlerPlugin>*> _plugins;
  #endif
  std::string _url;
};
