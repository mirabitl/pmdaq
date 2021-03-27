#pragma once
#include <stdint.h>
#include <iostream>
#include "reader.hh"

namespace sdcc
{
  class interface 
  {
  public:
    interface(std::string CCCName = "DCCCCC01",std::string CCCType="DCC_CCC");
    static std::string discover();
    void initialise();
    void destroy();
    void configure();
    void start();
    void stop();
    void test();
  
  
    inline sdcc::reader* getCCCReadout(){return _sdcc;}
  private:
    sdcc::reader* _sdcc;

    std::string theCCCType_;
    std::string theCCCName_;
  };
};

