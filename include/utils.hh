#pragma once


#include "stdafx.hh"
#include <map>

class utils
{
    public:

  static uint32_t convertIP(std::string hname);
  static uint64_t asicTag(std::string hname,uint32_t header);
  static uint64_t asicTag(uint32_t ipa,uint32_t header);

  static http_response requesturl(std::string address);
  static http_response request(std::string host,uint32_t port,std::string path,
			web::json::value par);
  
  static std::vector<std::string> split(const std::string &s, char delim);
  static std::string pns_name();
  static bool checkpns(int port=8888);
  static int graphite_init( const char *host, const int port );
  static void graphite_finalize();
  static void graphite_send(const char *message);
  static void graphite_send_plain( const char* path, float value, unsigned long timestamp );
  static std::map<uint32_t,std::string> scanNetwork(std::string base);
  static std::string lmexec(const char* cmd);
  static std::string  findUrl(std::string session, std::string appname,uint32_t appinstance);
  static http_response sendCommand(std::string url, std::string command,web::json::value par);

  static uint32_t queryIntValue(http_request m,std::string n,uint32_t def_val);
  static std::string queryStringValue(http_request m,std::string n,std::string def_val);
  static bool isMember(web::json::value p,std::string key);

};

