#pragma once
#include <string>
//#include "stdafx.hh"
#include <mutex>
#include <thread>

//static LoggerPtr _logSyx27(Logger::getLogger("PMDAQ_SYX27"));
#include "cpprest/json.h"

namespace caen
{
  class HVCaenInterface
  {
  public:
    HVCaenInterface(std::string host,std::string user,std::string pwd);
    virtual ~HVCaenInterface();
    virtual void Connect();
    virtual void Disconnect();
    virtual void SetOn(uint32_t channel);
    virtual void SetOff(uint32_t channel);
    virtual void SetCurrent(uint32_t channel,float imax);
    virtual void SetVoltage(uint32_t channel,float v0);
    virtual void SetVoltageRampUp(uint32_t channel,float v0);
    virtual float GetCurrentSet(uint32_t channel);
    virtual float GetVoltageSet(uint32_t channel);
    virtual float GetCurrentRead(uint32_t channel);
    virtual float GetVoltageRead(uint32_t channel);
    virtual float GetVoltageRampUp(uint32_t channel);
    virtual std::string GetName(uint32_t channel);
    virtual uint32_t GetStatus(uint32_t channel);
    bool isConnected(){ return (connected_);}
    inline int32_t BoardSlot(uint32_t ch){return (ch/6);}
    inline int32_t BoardChannel(uint32_t ch){return ch%6;}
    float GetFloatValue(std::string name, uint32_t slot,uint32_t channel);
    int32_t GetIntValue(std::string name, uint32_t slot,uint32_t channel);
    std::string GetStringValue(std::string name, uint32_t slot,uint32_t channel);
    void SetFloatValue(std::string name, uint32_t slot,uint32_t channel,float v);
    void SetIntValue(std::string name, uint32_t slot,uint32_t channel,int32_t v);
    web::json::value ChannelInfo(uint32_t slot,uint32_t channel);
    void ping();
    void GetSysProp();
  private:
    std::string theHost_,theUser_,thePassword_,theIp_;
    int32_t theID_;
    int32_t theHandle_;
    bool connected_;
    std::thread g_ping;
    std::mutex _sem;
    bool _ping;
    
  };
};

