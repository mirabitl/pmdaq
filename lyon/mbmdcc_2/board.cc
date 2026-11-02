#include "MBMDCCInterface.hh"

mbmdcc_2::board::board(const std::string& ip)
    : _ip(ip), _regh(new mbmdcc_2::registerHandler(ip))
{
}

mbmdcc_2::board::~board()
{
  delete _regh;
}
