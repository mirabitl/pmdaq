#include "LIROCConfigAccess.hh"
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
#include <arpa/inet.h>
#include <boost/format.hpp>
#include "utils.hh"

using namespace web;
LIROCConfigAccess::LIROCConfigAccess()
{
 
  _jall=web::json::value::null();
  _jasic=web::json::value::null();
}
void LIROCConfigAccess::parseMongoDb(std::string state,uint32_t version)
{
  std::stringstream scmd;
  scmd<<"/bin/bash -c 'mgroc --download --state="<<state<<" --version="<<version<<"'";
  system(scmd.str().c_str());
  std::stringstream sname;
  sname<<"/dev/shm/mgroc/"<<state<<"_"<<version<<".json";


  fprintf(stderr,"Parsing the file %s\n",sname.str().c_str());
  this->parseJsonFile(sname.str());
  
}

void LIROCConfigAccess::parseJsonFile(std::string jsf)
{

  web::json::value output;  // JSON read from input file
  
  try
    {
      // Open the file stream
      std::ifstream f(jsf);
      // String stream for holding the JSON file
      std::stringstream strStream;
      
      // Stream file stream into string stream
      strStream << f.rdbuf();
      f.close();  // Close the filestream
      
      // Parse the string stream into a JSON object
      output = web::json::value::parse(strStream);
    }
  catch (web::json::json_exception excep)
    {
      _jall=web::json::value::null();
	  //throw web::json::json_exception("Error Parsing JSON file " + jsonFileName);
    }
  _jall=output;
  if (_jall.as_object().find("asics")==_jall.as_object().end())
    {
      std::cout<<" No asics tag found "<<std::endl;
      return;
    }
  

  this->parseJson();
}
void LIROCConfigAccess::parseJson()
{
  web::json::value asics = _jall["asics"];
  
  for (auto ita = asics.as_array().begin(); ita != asics.as_array().end(); ita++)
	{
	  web::json::value asic = (*ita);
	  std::string ipadr = asic["address"].as_string();
	  uint8_t header=asic["num"].as_integer();
	  LRSlow prs;prs.setJson(asic["slc"]);
	  uint64_t eid=utils::asicTag(ipadr,header);
	  //((uint64_t) TdcConfigAccess::convertIP(ipadr))<<32|header;
	  _asicMap.insert(std::pair<uint64_t,LRSlow>(eid,prs));

	}

}

void LIROCConfigAccess::parseJsonUrl(std::string jsf)
{
  http_response r=utils::requesturl(jsf);
  _jall=r.extract_json().get();
  this->parseJson();
}
uint32_t* LIROCConfigAccess::slcBuffer(){return _slcBuffer;}
uint32_t  LIROCConfigAccess::slcWords(){return _slcWords;}
std::map<uint64_t,LRSlow>&  LIROCConfigAccess::asicMap(){return _asicMap;}

void  LIROCConfigAccess::prepareSlowControl(std::string ipadr)
{
  // Initialise
  _slcWords=0;
  uint64_t eid=((uint64_t) utils::convertIP(ipadr))<<32;
  // Loop on 1 Asic maximum
  for (int ias=1;ias>=1;ias--)
    {
      uint64_t eisearch= eid|ias;
      std::map<uint64_t,LRSlow>::iterator im=_asicMap.find(eisearch);
      if (im==_asicMap.end()) continue;
      memcpy(&_slcBuffer[_slcWords],im->second.board_ptr(),139*sizeof(uint32_t));
      _slcWords+=139;
    }
}


void LIROCConfigAccess::clear()
{
  _asicMap.clear();
}
void LIROCConfigAccess::dumpMap()
{
  for (auto x:_asicMap)
    {
      uint32_t ip=(x.first)>>32&0XFFFFFFFF;
      uint8_t as=(x.first)&0xFF;
      std::cout<<" DIF "<<std::hex<<ip<<std::dec<<" ASIC "<<(int) as<<std::endl;
      x.second.dumpJson();
    }
}
