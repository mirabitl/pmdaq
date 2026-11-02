#include "Gricv0_3Interface.hh"


/// Board

gricv0_3::board::board(std::string ip) : _ip(ip)
{
  fprintf(stderr,"Creating registeraccess at address %s  \n",ip.c_str());

  _regh=new gricv0_3::registerHandler(ip);
    fprintf(stderr,"Creating slcaccess at address %s  \n",ip.c_str());
  _sensorh=new gricv0_3::sensorHandler(ip);
    fprintf(stderr,"Creating dataaccess at address %s  \n",ip.c_str());
  _datah=new gricv0_3::dataHandler(ip);
}

gricv0_3::board::~board()
{
  delete _regh;
  delete _sensorh;
  delete _datah;
}

				   
