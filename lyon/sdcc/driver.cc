/************************************************************************/
/* ILC test beam daq							*/	
/* C. Combaret								*/
/* V 1.0								*/
/* first release : 30-01-2008						*/
/* revs : 								*/
/************************************************************************/
#include "driver.hh"
#include <iostream>
#include <sstream>
#include <sys/timeb.h>


#define MY_DEBUG
using namespace mdcc;
using namespace sdcc;

sdcc::driver::driver(char * deviceIdentifier )      : mdcc::FtdiUsbDriver(deviceIdentifier) 
{


}

sdcc::driver::~driver()     //throw (LocalHardwareException)

{
  std::cout<<"destructor called"<<std::endl;
  
 
}	



int32_t sdcc::driver:: CCCCommandDIFReset(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x00;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

int32_t sdcc::driver::CCCCommandBCIDReset(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x01;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

int32_t sdcc::driver::CCCCommandStartAcquisitionAuto(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x02;
  printf ("sdcc send start acq\n");
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

int32_t sdcc::driver::CCCCommandRamfullExt(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x03;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

int32_t sdcc::driver::CCCCommandTriggerExt(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x04;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	


int32_t sdcc::driver::CCCCommandStopAcquisition(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x05;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

int32_t sdcc::driver::CCCCommandDigitalReadout(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x06;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

int32_t sdcc::driver::CCCCommandTrigger(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x07;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

int32_t sdcc::driver::CCCCommandSpillOn(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x0C;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

int32_t sdcc::driver::CCCCommandSpillOff(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x0D;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

int32_t sdcc::driver::CCCCommandClearMemory(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x08;
  printf (" Not implemented anymore\n");
  return -2;
}	

int32_t sdcc::driver::CCCCommandStartSingleAcquisition(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x09;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

int32_t sdcc::driver::CCCCommandPulseLemo(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x0A;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

int32_t sdcc::driver::CCCCommandRazChannel(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x0B;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	
/*
  int32_t sdcc::driver::CCCCommandNoCommand(void)    //throw (LocalHardwareException)
  {
  uint32_t taddress=0x0E;
	
  CCCCommandWrite	(taddress);	
  if (!isOk())
  {
  PM_ERROR(_logSdcc,"error found");
  return -2;
  }
  return 0;
  }	
*/
int32_t sdcc::driver::CCCCommandResetCCC(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x0F;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logSdcc,"error found");
	return -2;
    }
  return 0;
}	

