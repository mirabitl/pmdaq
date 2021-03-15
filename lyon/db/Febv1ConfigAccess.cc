#include "Febv1ConfigAccess.hh"
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
//#include "WiznetMessageHandler.hh"
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>


using namespace web;




Febv1ConfigAccess::Febv1ConfigAccess()
{
  _slcBytes = 0;
  memset(_slcBuffer, 0, 0x1000 * sizeof(uint16_t));
  memset(_slcAddr, 0, 0x1000 * sizeof(uint16_t));
  _jall = web::json::value::null();
  this->clear();
}

void Febv1ConfigAccess::parseMongoDb(std::string state,uint32_t version)
{
  std::stringstream scmd;
  scmd<<"/bin/bash -c 'mgroc --download --state="<<state<<" --version="<<version<<"'";
  system(scmd.str().c_str());
  std::stringstream sname;
  sname<<"/dev/shm/mgroc/"<<state<<"_"<<version<<".json";


  this->parseJsonFile(sname.str());
  if (_jall.as_object().find("asics")==_jall.as_object().end())
    {
      std::cout << " No DIF tag found " << std::endl;
      return;
    }
  
}

void Febv1ConfigAccess::parseJsonFile(std::string jsf)
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
void Febv1ConfigAccess::parseJson()
{

  
  for (auto ita = _jall["asics"].as_array().begin(); ita != _jall["asics"].as_array().end(); ita++)
    {
      web::json::value asic = *ita;
      std::string sipadr=asic["address"].as_string();
      uint32_t ipadr = utils::convertIP(sipadr);
      uint8_t header = asic["num"].as_integer();
      PR2 prs;
      prs.setJson(asic["slc"]);
      uint64_t eid = ((uint64_t) ipadr) << 32 | header;
      _asicMap.insert(std::pair<uint64_t, PR2>(eid, prs));
    }

}

void Febv1ConfigAccess::parseJsonUrl(std::string jsf)
{
  http_response r=utils::requesturl(jsf);
  _jall=r.extract_json().get();
  if (_jall.as_object().find("asics")==_jall.as_object().end())
    {
      std::cout<<" No asics tag found "<<std::endl;
      return;
    }

  this->parseJson();
}
uint16_t *Febv1ConfigAccess::slcBuffer() { return _slcBuffer; }
uint16_t *Febv1ConfigAccess::slcAddr() { return _slcAddr; }
uint32_t Febv1ConfigAccess::slcBytes() { return _slcBytes; }
std::map<uint64_t, PR2> &Febv1ConfigAccess::asicMap() { return _asicMap; }
void Febv1ConfigAccess::prepareSlowControl(std::string ipadr)
{
std::cout<<"entering TDCConfigAccess.cc  Febv1ConfigAccess::prepareSlowControl"<<std::endl;

  // Initialise
  _slcBytes = 0;
  uint64_t eid = ((uint64_t)utils::convertIP(ipadr)) << 32;
  // Loop on 4 Asic maximum
  for (int ias = 4; ias >= 1; ias--)
  {
    uint64_t eisearch = eid | ias;
    std::map<uint64_t, PR2>::iterator im = _asicMap.find(eisearch);
    if (im == _asicMap.end())
      continue;
    printf("DIF %lx ,Asic %d Found\n", eid >> 32, ias);
    im->second.prepare4Tdc(_slcAddr, _slcBuffer, _slcBytes);
    _slcBytes += SLC_BYTES_LENGTH;
  }
  if (_slcBytes >= SLC_BYTES_LENGTH)
  {
    _slcBuffer[_slcBytes] = 0x3;
    _slcAddr[_slcBytes] = 0x201;
    _slcBytes++;
  }
}
void Febv1ConfigAccess::clear()
{
  _asicMap.clear();
}
void Febv1ConfigAccess::dumpMap()
{
  for (auto x : _asicMap)
  {
    uint32_t ip = (x.first) >> 32 & 0XFFFFFFFF;
    uint8_t as = (x.first) & 0xFF;
    std::cout << " DIF " << std::hex << ip << std::dec << " ASIC " << (int)as << std::endl;
    x.second.dumpJson();
  }
}
void Febv1ConfigAccess::dumpToShm(std::string path)
{
}
void Febv1ConfigAccess::connect()
{
}
void Febv1ConfigAccess::publish()
{
}
