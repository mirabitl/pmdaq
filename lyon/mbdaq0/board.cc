#include "Mbdaq0Interface.hh"


/// Board

mbdaq0::board::board(std::string ip) : _ip(ip)
{
  fprintf(stderr,"Creating registeraccess at address %s  \n",ip.c_str());

  _regh=new mbdaq0::registerHandler(ip);
#ifdef FULLDAQ
    fprintf(stderr,"Creating slcaccess at address %s  \n",ip.c_str());
  _slch=new mbdaq0::slcHandler(ip);
    fprintf(stderr,"Creating dataaccess at address %s  \n",ip.c_str());
  _datah=new mbdaq0::dataHandler(ip);
#endif
}


				   
