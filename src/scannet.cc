#include "stdafx.hh"
#include "utils.hh"

int main(int argc, char *argv[])
{
  if (argc<2)
    exit(0);
  std::string base(argv[1]);
  auto m=utils::scanNetwork(base);
 
}
