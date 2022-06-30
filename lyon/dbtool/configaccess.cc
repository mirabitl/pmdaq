#include "configaccess.hh"
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
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <netdb.h>


using namespace lmdbtool;
lmdbtool::ConfigAccess::ConfigAccess()
{
 
  _jall=Json::Value::null;
  _jasic=Json::Value::null;

}
uint32_t lmdbtool::ConfigAccess::convertIP(std::string hname)
{
  struct hostent *he;
  struct in_addr **addr_list;
  int i;
  char ip[100];
  if ((he = gethostbyname(hname.c_str())) == NULL)
  {
    return 0;
  }

  addr_list = (struct in_addr **)he->h_addr_list;

  for (i = 0; addr_list[i] != NULL; i++)
  {
    //Return the first one;
    strcpy(ip, inet_ntoa(*addr_list[i]));
    break;
  }

  in_addr_t ls1 = inet_addr(ip);
  return (uint32_t)ls1;
}

void lmdbtool::ConfigAccess::parseMongoDb(std::string state,uint32_t version)
{
  std::stringstream scmd;
  scmd<<"/bin/bash -c 'mgroc --download --state="<<state<<" --version="<<version<<"'";
  system(scmd.str().c_str());
  std::stringstream sname;
  sname<<"/dev/shm/mgroc/"<<state<<"_"<<version<<".json";


  fprintf(stderr,"Parsing the file %s\n",sname.str().c_str());
  Json::Reader reader;

  std::ifstream ifs(sname.str().c_str(), std::ifstream::in);
  //      Json::Value _jall;
  fprintf(stderr,"Before Parsing the file %s\n",sname.str().c_str());
  bool parsingSuccessful = reader.parse(ifs, _jall, false);
  fprintf(stderr,"After Parsing the file %s done %d\n",sname.str().c_str(),parsingSuccessful);

  //std::cout<<"Before JALL "<<jall<<std::flush<<std::endl;
  //  _jall=jall;
  //std::cout<<"After JALL "<<_jall<<std::flush<<std::endl;
  if (!_jall.isMember("asics"))
  {
    std::cout << " No DIF tag found " << std::endl;
    return;
  }
  int status = mkdir("/dev/shm/db", S_IRWXU | S_IRWXG | S_IRWXO );
  std::stringstream ss;
  ss<<"/dev/shm/db/"<<state<<"_"<<version;
  _directory=ss.str();
  status = mkdir(_directory.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );
  this->parseJson();
  
}

void lmdbtool::ConfigAccess::parseJsonFile(std::string jsf)
{
  Json::Reader reader;
  std::ifstream ifs (jsf.c_str(), std::ifstream::in);
  //      Json::Value _jall;
  bool parsingSuccessful = reader.parse(ifs,_jall,false);
  if (!_jall.isMember("DIF"))
    {
      std::cout<<" No DIF tag found "<<std::endl;
      return;
    }
  this->parseJson();
}
void lmdbtool::ConfigAccess::parseJson()
{


  const Json::Value &asics = _jall["asics"];
  
  for (Json::ValueConstIterator ita = asics.begin(); ita != asics.end(); ++ita)
    {
      const Json::Value &asic = *ita;
      std::string ipadr = asic["address"].asString();
      uint32_t header = asic["num"].asUInt();
      uint32_t dif = asic["dif"].asUInt();

      fprintf(stderr,"Insering %d %d\n",dif,header);
      lmdbtool::hr2 prs;
      prs.setJson(asic["slc"]);

      //std::cout<<asic["slc"]<<std::flush<<std::endl;
      uint64_t eid = ((uint64_t)  convertIP(ipadr)) << 32 | header;
      std::stringstream s("");
      s<<_directory<<"/"<<dif<<"_"<<header<<"_"<<eid;
      if (prs.isEnabled())
	prs.store(s.str());

      _asicMap.insert(std::pair<uint64_t, lmdbtool::hr2>(eid, prs));
      //prs.dumpBinary();
    }

  
}


std::map<uint64_t,lmdbtool::hr2>&  lmdbtool::ConfigAccess::asicMap(){return _asicMap;}




void lmdbtool::ConfigAccess::clear()
{
  _asicMap.clear();
}
void lmdbtool::ConfigAccess::dumpMap()
{
  for (auto x:_asicMap)
    {
      uint32_t ip=(x.first)>>32&0XFFFFFFFF;
      uint8_t as=(x.first)&0xFF;
      std::cout<<" DIF "<<std::hex<<ip<<std::dec<<" ASIC "<<(int) as<<std::endl;
      x.second.dumpJson();
    }
}

int main(int argc, char** argv)
{
  if (argc!=3)
    exit(0);
  std::string state(argv[1]);
  uint32_t version=stol(argv[2]);
  lmdbtool::ConfigAccess hca;
  hca.parseMongoDb(state,version);
}
