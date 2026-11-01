#include "Gricv1Interface.hh"


/// Board

gricv1_2::board::board(std::string ip,mpi2::MpiMessageHandler* transport) : _ip(ip)
{
  fprintf(stderr,"Creating registeraccess at address %s  \n",ip.c_str());

  _regh=new gricv1_2::registerHandler(ip,transport);
    fprintf(stderr,"Creating slcaccess at address %s  \n",ip.c_str());
  _slch=new gricv1_2::slcHandler(ip,transport);
    fprintf(stderr,"Creating dataaccess at address %s  \n",ip.c_str());
  _datah=new gricv1_2::dataHandler(ip,transport);
}

gricv1_2::board::~board()
{
  delete _regh;
  delete _slch;
  delete _datah;
}
