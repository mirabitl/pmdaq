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
#include <sys/time.h>
#include <sys/resource.h>

int file_select_3(const struct direct *entry)  
{  
  if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))  
    return (0);  
  else  
    return (1);  
}  

using namespace web;
HR2ConfigAccess::HR2ConfigAccess()
{
 
  _jall=web::json::value::object();
  //_jasic=web::json::value::null();
}

void HR2ConfigAccess::parseMongoDb(std::string state,uint32_t version)
{
  struct rusage r_usage;
  getrusage(RUSAGE_SELF,&r_usage);
  // Print the maximum resident set size used (in kilobytes).
  fprintf(stderr,"%s %d Memory usage: %ld Mb\n",__FUNCTION__,__LINE__,r_usage.ru_maxrss/1024);
  
  std::stringstream scmd;
  scmd<<"/bin/bash -c 'mgroc --download --state="<<state<<" --version="<<version<<"'";
  system(scmd.str().c_str());
  std::stringstream sname;
  sname<<"/dev/shm/mgroc/"<<state<<"_"<<version<<".json";

  getrusage(RUSAGE_SELF,&r_usage);
  // Print the maximum resident set size used (in kilobytes).
  fprintf(stderr,"%s %d Memory usage: %ld Mb\n",__FUNCTION__,__LINE__,r_usage.ru_maxrss/1024);

  fprintf(stderr,"Parsing the file %s\n",sname.str().c_str());
  this->parseJsonFile(sname.str());
  
}

void HR2ConfigAccess::ls(std::string state,uint32_t version,std::vector<std::string>& res)
{
 
  res.clear();
  int count,i;  
  struct direct **files;  
  std::stringstream sc;
  sc.str(std::string());
  sc<<"/dev/shm/db/"<<state<<"_"<<version<<"/";
  
  count = scandir(sc.str().c_str(), &files, file_select_3, alphasort);          
  /* If no files found, make a non-selectable menu item */  
  if(count <= 0)    {return ;}
       
  std::stringstream sd;         
  //printf("Number of files = %d\n",count);  
  for (i=1; i<count+1; ++i)  
    {
      // file name
      std::string fName;
      fName.assign(files[i-1]->d_name);
      res.push_back(fName);
       free(files[i-1]);
    }
  free(files);
  return;
}

void HR2ConfigAccess::parseMongoDb2(std::string state,uint32_t version)
{
  struct rusage r_usage;
  getrusage(RUSAGE_SELF,&r_usage);
  // Print the maximum resident set size used (in kilobytes).
  fprintf(stderr,"%s %d Memory usage: %ld Mb\n",__FUNCTION__,__LINE__,r_usage.ru_maxrss/1024);
  
  std::stringstream scmd;
  scmd<<"/bin/bash -c 'dbt "<<state<<" "<<version<<"'";
  system(scmd.str().c_str());

  getrusage(RUSAGE_SELF,&r_usage);
  // Print the maximum resident set size used (in kilobytes).
  fprintf(stderr,"%s %d Memory usage: %ld Mb\n",__FUNCTION__,__LINE__,r_usage.ru_maxrss/1024);

  std::vector<std::string> vnames;
  this->ls(state,version,vnames);
  for (auto x :vnames)
    {
      uint32_t dif,asic;
      uint64_t eid;
      sscanf(x.c_str(),"%d_%d_%lu64",&dif,&asic,&eid);

      //std::cout<<x<<" "<<dif<<" "<<asic<<" "<<std::dec<<eid<<std::dec<<std::endl;

      std::stringstream fn;
      fn<<"/dev/shm/db/"<<state<<"_"<<version<<"/"<<x;
      HR2Slow prs;prs.load(fn.str());
      //((uint64_t) TdcConfigAccess::convertIP(ipadr))<<32|header;
      _asicMap.insert(std::pair<uint64_t,HR2Slow>(eid,prs));
      //(*ita).erase("slc");
    }
    

  
}
void HR2ConfigAccess::parseJsonFile(std::string jsf)
{

  web::json::value output;  // JSON read from input file
  struct rusage r_usage;
  getrusage(RUSAGE_SELF,&r_usage);
  // Print the maximum resident set size used (in kilobytes).
  fprintf(stderr,"%s %d Memory usage: %ld Mb\n",__FUNCTION__,__LINE__,r_usage.ru_maxrss/1024);


  try
    {
      fprintf(stderr,"reading the file %s\n",jsf.c_str());
      // Open the file stream
      std::ifstream f(jsf);
      // String stream for holding the JSON file
      std::stringstream strStream;
      
      // Stream file stream into string stream
      strStream << f.rdbuf();
      f.close();  // Close the filestream


      //
      fprintf(stderr,"Parsing the string\n");
      // Parse the string stream into a JSON object
      _jall = web::json::value::parse(strStream);
      fprintf(stderr,"Parsing done\n");

      getrusage(RUSAGE_SELF,&r_usage);
      // Print the maximum resident set size used (in kilobytes).
      fprintf(stderr,"%s %d Memory usage: %ld Mb\n",__FUNCTION__,__LINE__,r_usage.ru_maxrss/1024);

    }
  catch (web::json::json_exception excep)
    {
      _jall=web::json::value::null();
	  //throw web::json::json_exception("Error Parsing JSON file " + jsonFileName);
    }
  //_jall=output;
  
  if (_jall.as_object().find("asics")==_jall.as_object().end())
    {
      std::cout<<" No asics tag found "<<std::endl;
      return;
    }
  getrusage(RUSAGE_SELF,&r_usage);
  // Print the maximum resident set size used (in kilobytes).
  fprintf(stderr,"%s %d Memory usage: %ld Mb\n",__FUNCTION__,__LINE__,r_usage.ru_maxrss/1024);

  fprintf(stderr,"Call parseJson\n");
  this->parseJson();
  getrusage(RUSAGE_SELF,&r_usage);
  // Print the maximum resident set size used (in kilobytes).
  fprintf(stderr,"%s %d Memory usage: %ld Mb\n",__FUNCTION__,__LINE__,r_usage.ru_maxrss/1024);


}
void HR2ConfigAccess::parseJson()
{
  //web::json::value asics = _jall["asics"];
  
  for (auto ita =_jall["asics"].as_array().begin(); ita != _jall["asics"].as_array().end(); ita++)
	{
	  web::json::value asic = (*ita);
	  std::string ipadr = asic["address"].as_string();
	  uint8_t header=asic["num"].as_integer();
	  HR2Slow prs;prs.setJson(asic["slc"]);
	  uint64_t eid=utils::asicTag(ipadr,header);
	  //((uint64_t) TdcConfigAccess::convertIP(ipadr))<<32|header;
	  _asicMap.insert(std::pair<uint64_t,HR2Slow>(eid,prs));
	  //(*ita).erase("slc");
	}
  // _jall.erase("asics");
  // _jall=web::json::value::null();

}

void HR2ConfigAccess::parseJsonUrl(std::string jsf)
{
  http_response r=utils::requesturl(jsf);
  auto jall=r.extract_json().get();
  _jall=jall;
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
  bool debug=false;
  // Loop on 48 Asic maximum
  for (int ias=48;ias>=1;ias--)
    {
      uint64_t eisearch= eid|ias;
      //std::cout<<eisearch<<std::endl;
      std::map<uint64_t,HR2Slow>::iterator im=_asicMap.find(eisearch);
      if (im==_asicMap.end()) continue;
      //std::cout<<ipadr<<" "<<ias<<" "<<eisearch<<"  found"<<std::endl;

      if (!im->second.isEnabled())
	{
	  printf("\t ===> DIF %s ,Asic %d disabled\n",ipadr.c_str(),ias);
	  continue;
	}
      if (debug)
	printf("DIF %s ,Asic %d Found\n",ipadr.c_str(),ias);
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
