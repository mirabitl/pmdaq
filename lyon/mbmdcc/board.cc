#include "MBMDCCInterface.hh"


/// Board

mbmdcc::board::board(std::string ip) : _ip(ip)
{
  fprintf(stderr,"Creating registeraccess at address %s  \n",ip.c_str());

  _regh=new mbmdcc::registerHandler(ip);
}


				   
