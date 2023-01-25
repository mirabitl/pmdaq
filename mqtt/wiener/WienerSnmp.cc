#include "WienerSnmp.hh"
using namespace  std;
using namespace wiener;

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>


std::vector<std::string> split(const std::string &s, char delim)
{
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim))
    {
      elems.push_back(item);
      // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
    }
  return elems;
}

std::string wiener::WienerDevice::exec(const char* cmd) {
  //fprintf(stderr,"Command : %s",cmd);
  //PM_INFO(_logWiener,"Command called "<<cmd);
  FILE* pipe = popen(cmd, "r");
  if (!pipe) return "ERROR";
  //char buffer[128];
  std::string result = "";
  size_t nc=0;
  char* line;
  while (!feof(pipe)) {
    //if (fgets(buffer, 128, pipe) != NULL)
    if (getline(&line, &nc, pipe) !=-1)
      {
	fprintf(stderr,"%ld chae read %s",nc,line);
	result += line;
      }
    else
      break;
  }
  pclose(pipe);
  //std::cout<<"Command done "<<result<<std::endl;
  return result;
    
}
  
wiener::WienerDevice::WienerDevice(std::string ipa) : _ip(ipa){}
std::string wiener::WienerDevice::getSysMainSwitch()
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" sysMainSwitch.0";
  return this->exec(sc.str().c_str());
}
std::string wiener::WienerDevice::setOutputVoltage(uint32_t module,uint32_t voie,float tension)
{
  std::stringstream sc;
  sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
  sc<<_ip<<" outputVoltage.u"<<100*module+voie<<" F "<<tension;
  return this->exec(sc.str().c_str());

}
float wiener::WienerDevice::getOutputVoltage(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputVoltage.u"<<100*module+voie;
  std::string res=this->exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  //  std::vector<std::string> strs;
  //boost::split(strs,res, boost::is_any_of(" "));
  auto strs=split(res,' ');
  float f=0;
  try{
     f = std::stof(strs[4]);
  } catch( ... )
    {
      f=-1;
    }
  return f;
}
std::string wiener::WienerDevice::setOutputVoltageRiseRate(uint32_t module,uint32_t voie,float val)
{
  std::stringstream sc;
  sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
  sc<<_ip<<" outputVoltageRiseRate.u"<<100*module+voie<<" F "<<val;
  return this->exec(sc.str().c_str());

}
float wiener::WienerDevice::getOutputVoltageRiseRate(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputVoltageRiseRate.u"<<100*module+voie;
  std::string res=this->exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  //std::vector<std::string> strs;
  //boost::split(strs,res, boost::is_any_of(" "));
  //return atof(strs[4].c_str());
  auto strs=split(res,' ');

  float f=0;
  try{
     f = std::stof(strs[4]);
  } catch( ... )
    {
      f=-1;
    }
  return f;



}
std::string wiener::WienerDevice::setOutputCurrentLimit(uint32_t module,uint32_t voie,float cur )
{
  std::stringstream sc;
  sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
  sc<<_ip<<" outputCurrent.u"<<100*module+voie<<" F "<<cur;
  return this->exec(sc.str().c_str());

}
float wiener::WienerDevice::getOutputCurrentLimit(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputCurrent.u"<<100*module+voie;
  std::string res=this->exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  //std::vector<std::string> strs;
  //boost::split(strs,res, boost::is_any_of(" "));
  //return atof(strs[4].c_str());
  auto strs=split(res,' ');
  float f=0;
  try{
     f = std::stof(strs[4]);
  } catch( ... )
    {
      f=-1;
    }
  return f;
  //return std::stof(strs[4]);

}
float wiener::WienerDevice::getOutputMeasurementSenseVoltage(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputMeasurementSenseVoltage.u"<<100*module+voie;
  std::string res=this->exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  //std::vector<std::string> strs;
  //boost::split(strs,res, boost::is_any_of(" "));
  //return atof(strs[4].c_str());
  auto strs=split(res,' ');
  //std::cout<<" Sense voltage "<<strs[4]<<std::endl;
  float f=0;
  try{
    f = std::stof(strs[4]);
  } catch( ... )
    {
      f=-1;
    }
  //std::cout<<" Sense voltage stof "<<f<<std::endl;
  return f;
  //return std::stof(strs[4]);
  
}
float wiener::WienerDevice::getOutputMeasurementCurrent(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  struct stat buffer;   
  bool newvers=(stat ("/usr/local/bin/snmpget", &buffer) == 0);
  if (newvers)
    sc<<"/usr/local/bin/snmpget -v 2c -O p12.9 -m +WIENER-CRATE-MIB -c public ";
  else
    sc<<"snmpget -v 2c  -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputMeasurementCurrent.u"<<100*module+voie;
  std::string res=this->exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  // std::vector<std::string> strs;
  //boost::split(strs,res, boost::is_any_of(" "));
  //return atof(strs[4].c_str());
  auto strs=split(res,' ');
  //PM_INFO(_logWiener,"Len of strs "<<strs.size());
  //PM_INFO(_logWiener,"strs[4] "<<strs[4]);
  if (strs.size()==6)
    return std::stof(strs[4]);
  if (strs.size()==7)
    return std::stof(strs[5]);
  else
    return 0;
   

}
std::string wiener::WienerDevice::setOutputSwitch(uint32_t module,uint32_t voie,uint32_t val )
{
  std::stringstream sc;
  sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
  sc<<_ip<<" outputSwitch.u"<<100*module+voie<<" i "<<val;
  return this->exec(sc.str().c_str());

}
std::string wiener::WienerDevice::getOutputSwitch(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputSwitch.u"<<100*module+voie;
  std::string res=this->exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  //std::vector<std::string> strs;
  //boost::split(strs,res, boost::is_any_of(" "));
  //return strs[3];
  auto strs=split(res,' ');
  return strs[3];

}
std::string wiener::WienerDevice::getOutputStatus(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputStatus.u"<<100*module+voie;
  std::string res=this->exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  //std::vector<std::string> strs;
  //boost::split(strs,res, boost::is_any_of(" "));
  return res;
}
