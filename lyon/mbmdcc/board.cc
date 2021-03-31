#include "MBMDCCInterface.hh"


/// Board

mbmdcc::board::board(std::string ip) : _ip(ip)
{
  PM_INFO(_logMbmdcc,"Creating registeraccess at address "<<ip.c_str());

  _regh=new mbmdcc::registerHandler(ip);
  
}


				   
