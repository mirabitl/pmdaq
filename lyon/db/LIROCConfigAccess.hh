#pragma once
#include "LRSlow.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include <map>
#include "stdafx.hh"

class LIROCConfigAccess 
{
public:
  LIROCConfigAccess();
  ~LIROCConfigAccess(){;}
  void parseMongoDb(std::string state,uint32_t version);
  void parseJsonFile(std::string jsf);
  void parseJson();
  void parseJsonUrl(std::string jsf);
  std::map<uint64_t,LRSlow>& asicMap();
  void clear();
  void dumpMap();
  void  prepareSlowControl(std::string ipadr);
  uint32_t* slcBuffer();
  uint32_t slcWords();
private:
  std::map<uint64_t,LRSlow> _asicMap;
  json::value _jall;
  json::value _jasic;
  uint32_t _slcBuffer[48*139];
  uint32_t _slcWords;
};
