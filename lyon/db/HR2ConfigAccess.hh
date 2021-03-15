#pragma once
#include "HR2Slow.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include <map>
#include "stdafx.hh"

class HR2ConfigAccess 
{
public:
  HR2ConfigAccess();
  ~HR2ConfigAccess(){;}
  void parseMongoDb(std::string state,uint32_t version);
  void parseJsonFile(std::string jsf);
  void parseJson();
  void parseJsonUrl(std::string jsf);
  std::map<uint64_t,HR2Slow>& asicMap();
  void clear();
  void dumpMap();
  void  prepareSlowControl(std::string ipadr,bool inverted=false);
  uint8_t* slcBuffer();
  uint32_t slcBytes();
private:
  std::map<uint64_t,HR2Slow> _asicMap;
  json::value _jall;
  json::value _jasic;
  uint8_t _slcBuffer[65536];
  uint32_t _slcBytes;
};
