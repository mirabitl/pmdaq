#include "Gricv1Interface.hh"


/// Board

gricv1_3::board::board(std::string ip,mpi2::MpiMessageHandler* transport) : _ip(ip)
{
  fprintf(stderr,"Creating registeraccess at address %s  \n",ip.c_str());

  _regh=new gricv1_3::registerHandler(ip,transport);
    fprintf(stderr,"Creating slcaccess at address %s  \n",ip.c_str());
  _slch=new gricv1_3::slcHandler(ip,transport);
    fprintf(stderr,"Creating dataaccess at address %s  \n",ip.c_str());
  _datah=new gricv1_3::dataHandler(ip,transport);
}

gricv1_3::board::~board()
{
  delete _regh;
  delete _slch;
  delete _datah;
}
