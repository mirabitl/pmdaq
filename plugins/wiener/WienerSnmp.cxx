#include "WienerSnmp.hh"
using namespace  std;
using namespace wiener;

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>

std::string wiener::WienerSnmp::exec(const char* cmd) {
  FILE* pipe = popen(cmd, "r");
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while (!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
    
}
  
wiener::WienerSnmp::WienerSnmp(std::string ipa) : _ip(ipa){}
std::string wiener::WienerSnmp::getSysMainSwitch()
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" sysMainSwitch.0";
  return exec(sc.str().c_str());
}
std::string wiener::WienerSnmp::setOutputVoltage(uint32_t module,uint32_t voie,float tension)
{
  std::stringstream sc;
  sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
  sc<<_ip<<" outputVoltage.u"<<100*module+voie<<" F "<<tension;
  return exec(sc.str().c_str());

}
float wiener::WienerSnmp::getOutputVoltage(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputVoltage.u"<<100*module+voie;
  std::string res=exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  std::vector<std::string> strs;
  boost::split(strs,res, boost::is_any_of(" "));
  return atof(strs[4].c_str());
}
std::string wiener::WienerSnmp::setOutputVoltageRiseRate(uint32_t module,uint32_t voie,float val)
{
  std::stringstream sc;
  sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
  sc<<_ip<<" outputVoltageRiseRate.u"<<100*module+voie<<" F "<<val;
  return exec(sc.str().c_str());

}
float wiener::WienerSnmp::getOutputVoltageRiseRate(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputVoltageRiseRate.u"<<100*module+voie;
  std::string res=exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  std::vector<std::string> strs;
  boost::split(strs,res, boost::is_any_of(" "));
  return atof(strs[4].c_str());
}
std::string wiener::WienerSnmp::setOutputCurrentLimit(uint32_t module,uint32_t voie,float cur )
{
  std::stringstream sc;
  sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
  sc<<_ip<<" outputCurrent.u"<<100*module+voie<<" F "<<cur;
  return exec(sc.str().c_str());

}
float wiener::WienerSnmp::getOutputCurrentLimit(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputCurrent.u"<<100*module+voie;
  std::string res=exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  std::vector<std::string> strs;
  boost::split(strs,res, boost::is_any_of(" "));
  return atof(strs[4].c_str());
}
float wiener::WienerSnmp::getOutputMeasurementSenseVoltage(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputMeasurementSenseVoltage.u"<<100*module+voie;
  std::string res=exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  std::vector<std::string> strs;
  boost::split(strs,res, boost::is_any_of(" "));
  return atof(strs[4].c_str());
}
float wiener::WienerSnmp::getOutputMeasurementCurrent(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  struct stat buffer;   
  bool newvers=(stat ("/usr/local/bin/snmpget", &buffer) == 0);
  if (newvers)
    sc<<"/usr/local/bin/snmpget -v 2c -O p12.9 -m +WIENER-CRATE-MIB -c public ";
  else
    sc<<"snmpget -v 2c  -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputMeasurementCurrent.u"<<100*module+voie;
  std::string res=exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  std::vector<std::string> strs;
  boost::split(strs,res, boost::is_any_of(" "));
  return atof(strs[4].c_str());
}
std::string wiener::WienerSnmp::setOutputSwitch(uint32_t module,uint32_t voie,uint32_t val )
{
  std::stringstream sc;
  sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
  sc<<_ip<<" outputSwitch.u"<<100*module+voie<<" i "<<val;
  return exec(sc.str().c_str());

}
std::string wiener::WienerSnmp::getOutputSwitch(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputSwitch.u"<<100*module+voie;
  std::string res=exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  std::vector<std::string> strs;
  boost::split(strs,res, boost::is_any_of(" "));
  return strs[3];
}
std::string wiener::WienerSnmp::getOutputStatus(uint32_t module,uint32_t voie)
{
  std::stringstream sc;
  sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
  sc<<_ip<<" outputStatus.u"<<100*module+voie;
  std::string res=exec(sc.str().c_str());
  //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
  //std::vector<std::string> strs;
  //boost::split(strs,res, boost::is_any_of(" "));
  return res;
}
