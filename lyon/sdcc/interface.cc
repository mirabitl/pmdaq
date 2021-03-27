
#include "interface.hh"
#include <typeinfo>
#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <istream>
#include <ostream>
//#include <iterator>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>
using namespace sdcc;

sdcc::interface::interface(std::string CCCName,std::string CCCType) 
{
  theCCCType_=CCCType;
  theCCCName_=CCCName;
}

std::string sdcc::interface::discover()
{
  std::string line;
  std::string busline,serialline;
  std::ifstream myfile ("/proc/bus/usb/devices");
  std::stringstream ccclist;
  std::vector<uint32_t> v;
  v.clear();
  uint32_t ibus,idev;
  if (myfile.is_open())
    {
      while ( myfile.good() )
	{
	  getline (myfile,line);
	  if (line.substr(0,2).compare("T:")==0)
	    {
	      busline=line.substr(2,line.size()-2);
	      size_t idbus,idlevel,iddevice,idspeed;
	      for (uint8_t ic=0;ic<busline.size();ic++)
		{
	    	  if (busline.substr(ic,4).compare("Bus=")==0) idbus=ic;
		  if (busline.substr(ic,4).compare("Lev=")==0) idlevel=ic;
		  if (busline.substr(ic,5).compare("Dev#=")==0) iddevice=ic;
		  if (busline.substr(ic,4).compare("Spd=")==0) idspeed=ic;
		}
	      ibus=atoi(busline.substr(idbus+4,idlevel-idbus-4).c_str());
	      idev=atoi(busline.substr(iddevice+5,idspeed-iddevice-5).c_str());
	    }
	  if (line.substr(0,2).compare("S:")==0)
	    {
	      serialline=line.substr(2,line.size()-2);
	      size_t idserial=0,idftdi=0;
	      for (uint8_t ic=0;ic<serialline.size();ic++)
		{
	    	  if (serialline.substr(ic,13).compare("SerialNumber=")==0) idserial=ic;
		  if (serialline.substr(ic,6).compare("DCCCCC")==0) idftdi=ic;
		}
	      if (idftdi==0 || idserial==0 ) continue;
	      uint32_t cccid=atoi(serialline.substr(idftdi+5,3).c_str());
	      v.push_back(cccid);
	      std::cout << " CCC found : "<<cccid << std::endl;
	      char cmd[256];
	      sprintf(cmd,"sudo /bin/chmod 666 /dev/bus/usb/%.3d/%.3d",ibus,idev);
	      std::cout<<"Executing ... "<<cmd<<std::endl;
	      system(cmd);
	    }
	}
      myfile.close();
    }
  else 
    {std::cout << "Unable to open file"<<std::endl; 
      PM_FATAL(_logSdcc,"Unable to eopn FTDI devices list file ");
    }
  if (v.size()>0)
    {
      for (uint8_t i=0;i<v.size()-1;i++)
	ccclist<<v[i]<<",";
      ccclist<<v[v.size()-1];
    }
  std::cout<<"List of CCCs is: "<<ccclist.str()<<std::endl;
  return ccclist.str();
}

void sdcc::interface::destroy()
{
  std::cout<<"a voir.... "<<std::endl;
}

void sdcc::interface::initialise()
{
  _sdcc=NULL;
  // Open
  if (theCCCType_.compare("DCC_CCC")==0)
    {
      PM_INFO(_logSdcc,"Initialise");
      _sdcc= new sdcc::reader(theCCCName_);
      printf ("theCCC=%p\n",_sdcc);
      _sdcc->open();
    }
  else
    {
      PM_FATAL(_logSdcc,"no more RS232 CCC "<<theCCCType_);
    }
  _sdcc->DoSendDIFReset();

  usleep((unsigned)1);

}

void sdcc::interface::configure()
{
  printf ("theCCC=%p\n",_sdcc);
  PM_INFO(_logSdcc,"Configure");
  _sdcc->DoSendDIFReset();
  usleep((unsigned)1);

}

void sdcc::interface::start( )
{
  printf ("theCCC=%p\n",_sdcc);
  PM_INFO(_logSdcc,"Start");
  _sdcc->DoSendBCIDReset();
  _sdcc->DoSendStartAcquisitionAuto();
  usleep((unsigned)1);
}

void sdcc::interface::stop()
{
  PM_INFO(_logSdcc,"Stop");
  _sdcc->DoSendStopAcquisition();

}

void sdcc::interface::test()
{
  _sdcc->DoSendDIFReset();

}
