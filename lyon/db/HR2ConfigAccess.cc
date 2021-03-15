#include "HR2ConfigAccess.hh"
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
HR2ConfigAccess::HR2ConfigAccess()
{
 
  _jall=web::json::value::null();
  _jasic=web::json::value::null();
}
void HR2ConfigAccess::parseMongoDb(std::string state,uint32_t version)
{
  std::stringstream scmd;
  scmd<<"/bin/bash -c 'mgroc --download --state="<<state<<" --version="<<version<<"'";
  system(scmd.str().c_str());
  std::stringstream sname;
  sname<<"/dev/shm/mgroc/"<<state<<"_"<<version<<".json";


  fprintf(stderr,"Parsing the file %s\n",sname.str().c_str());
  this->parseJsonFile(sname.str());
  
}

void HR2ConfigAccess::parseJsonFile(std::string jsf)
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
void HR2ConfigAccess::parseJson()
{
  web::json::value asics = _jall["asics"];
  
  for (auto ita = asics.as_array().begin(); ita != asics.as_array().end(); ita++)
	{
	  web::json::value asic = (*ita);
	  std::string ipadr = asic["address"].as_string();
	  uint8_t header=asic["num"].as_integer();
	  HR2Slow prs;prs.setJson(asic["slc"]);
	  uint64_t eid=utils::asicTag(ipadr,header);
	  //((uint64_t) TdcConfigAccess::convertIP(ipadr))<<32|header;
	  _asicMap.insert(std::pair<uint64_t,HR2Slow>(eid,prs));

	}

}

void HR2ConfigAccess::parseJsonUrl(std::string jsf)
{
  http_response r=utils::requesturl(jsf);
  _jall=r.extract_json().get();
  this->parseJson();
}
uint8_t* HR2ConfigAccess::slcBuffer(){return _slcBuffer;}
uint32_t  HR2ConfigAccess::slcBytes(){return _slcBytes;}
std::map<uint64_t,HR2Slow>&  HR2ConfigAccess::asicMap(){return _asicMap;}

void  HR2ConfigAccess::prepareSlowControl(std::string ipadr,bool inverted)
{
  // Initialise
  _slcBytes=0;
  uint64_t eid=((uint64_t) utils::convertIP(ipadr))<<32;
  // Loop on 48 Asic maximum
  for (int ias=48;ias>=1;ias--)
    {
      uint64_t eisearch= eid|ias;
      std::map<uint64_t,HR2Slow>::iterator im=_asicMap.find(eisearch);
      if (im==_asicMap.end()) continue;
      if (!im->second.isEnabled())
	{
	  printf("\t ===> DIF %lx ,Asic %d disabled\n",eid>>32,ias);
	  continue;
	}
      printf("DIF %lx ,Asic %d Found\n",eid,ias);
      if (!inverted)
	memcpy(&_slcBuffer[_slcBytes],im->second.ucPtr(),109);
      else
	{
	  uint8_t* dest=&_slcBuffer[_slcBytes];
	  uint8_t* sour=im->second.ucPtr();
	for(uint32_t ii=0;ii<109;ii++)
	  dest[108-ii]=sour[ii];
	}
      //memcpy(&_slcBuffer[_slcBytes],im->second.ucInvertedPtr(),109);
      _slcBytes+=109;
    }
}


void HR2ConfigAccess::clear()
{
  _asicMap.clear();
}
void HR2ConfigAccess::dumpMap()
{
  for (auto x:_asicMap)
    {
      uint32_t ip=(x.first)>>32&0XFFFFFFFF;
      uint8_t as=(x.first)&0xFF;
      std::cout<<" DIF "<<std::hex<<ip<<std::dec<<" ASIC "<<(int) as<<std::endl;
      x.second.dumpJson();
    }
}
