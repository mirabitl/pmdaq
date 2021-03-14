#pragma once


#include "stdafx.hh"


class utils
{
    public:
  static http_response request(std::string host,uint32_t port,std::string path,
			web::json::value par);
  
  static std::vector<std::string> split(const std::string &s, char delim);
  static std::string pns_name();
  static bool checkpns(int port=8888);
  static int graphite_init( const char *host, const int port );
  static void graphite_finalize();
  static void graphite_send(const char *message);
  static void graphite_send_plain( const char* path, float value, unsigned long timestamp );

};

