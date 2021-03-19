#include "Gricv0Interface.hh"
using namespace lydaq;

/// Board

lydaq::gricv0::board::board(std::string ip) : _ip(ip)
{
  fprintf(stderr,"Creating registeraccess at address %s  \n",ip.c_str());

  _regh=new gricv0::registerHandler(ip);
    fprintf(stderr,"Creating slcaccess at address %s  \n",ip.c_str());
  _sensorh=new gricv0::sensorHandler(ip);
    fprintf(stderr,"Creating dataaccess at address %s  \n",ip.c_str());
  _datah=new gricv0::dataHandler(ip);
}


				   
