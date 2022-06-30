#pragma once
#include "hr2.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include <map>
namespace lmdbtool
{
class ConfigAccess 
{
public:
  ConfigAccess();
  ~ConfigAccess(){;}
  void parseMongoDb(std::string state,uint32_t version);
  void parseJsonFile(std::string jsf);
  void parseJson();
  void parseJsonUrl(std::string jsf);
  std::map<uint64_t,lmdbtool::hr2>& asicMap();
  void clear();
  void dumpMap();
 uint32_t convertIP(std::string hname);

private:
  std::map<uint64_t,lmdbtool::hr2> _asicMap;
  Json::Value _jall;
  Json::Value _jasic;
  std::string _directory;
};
};
