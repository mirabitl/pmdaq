#pragma once
#include <stdio.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "stdafx.hh"
static LoggerPtr _logWiener(Logger::getLogger("PMDAQ_WIENER"));

using namespace  std;

namespace wiener
{
  class WienerSnmp
  {
  public:
    std::string exec(const char* cmd);
    WienerSnmp(std::string ipa);
    std::string getSysMainSwitch();
    std::string setOutputVoltage(uint32_t module,uint32_t voie,float tension);
    float getOutputVoltage(uint32_t module,uint32_t voie);
    std::string setOutputVoltageRiseRate(uint32_t module,uint32_t voie,float val);
    float getOutputVoltageRiseRate(uint32_t module,uint32_t voie);
    std::string setOutputCurrentLimit(uint32_t module,uint32_t voie,float cur );
    float getOutputCurrentLimit(uint32_t module,uint32_t voie);
    float getOutputMeasurementSenseVoltage(uint32_t module,uint32_t voie);
    float getOutputMeasurementCurrent(uint32_t module,uint32_t voie);
    std::string setOutputSwitch(uint32_t module,uint32_t voie,uint32_t val );
    std::string getOutputSwitch(uint32_t module,uint32_t voie);
    std::string getOutputStatus(uint32_t module,uint32_t voie);
  private:
    std::string _ip;
  };
};
