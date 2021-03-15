#include "Febv1Interface.hh"


/// Board

febv1::board::board(std::string ip) : _ip(ip)
{
  fprintf(stderr,"Creating registeraccess at address %s  \n",ip.c_str());
  _regh=new febv1::registerHandler(ip);
  fprintf(stderr,"Creating dataaccess at address %s  \n",ip.c_str());
  _datah=new febv1::dataHandler(ip);
}


				   
