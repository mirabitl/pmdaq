#include "reader.hh"

//#include "toolbox/string.h"

using namespace sdcc;
sdcc::reader::reader (std::string name) :_name(name)
{
  _driver=NULL;
  //std::cout<<" On sort"<<std::hex<< this<<std::dec<<std::endl;

}

int sdcc::reader::open() //throw( LocalHardwareException ) 
{
  if (_driver==NULL)
    {
      std::cout<<"Opening "<<_name<<std::endl;
      std::cout<<"Opening 1yy "<<_name<<std::endl;



      _driver=new USBDRIVER((char*) _name.c_str());
      _driver->checkReadWrite(0x1234,100);
      _driver->checkReadWrite(0x1234,100);




      //_driver=new UsbCCCDriver((char*) _name.c_str());
      std::cout<<"Opening 2yy "<<_name<<std::endl;
      printf ("%p\n", _driver);


      if (!_driver->isOk())
	{
	  std::cout<<"fail in Opening "<<_name<<std::endl;
	  PM_FATAL(_logSdcc,"fail openning "<<_name);
	  return -1;
	}
    }
  // std::cout<<" On sort"<<std::hex<<(int) this<<std::dec<<std::endl;
  return 0;
}

int sdcc::reader::close() //throw( LocalHardwareException )
{
  delete _driver;
  _driver=NULL;
  return 0;
}
void sdcc::reader::DoSendPauseTrigger()
{
  _driver->UsbCommandWrite(0x10);	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send Pause");	
  return;
}
void sdcc::reader::DoSendResumeTrigger()
{
  _driver->UsbCommandWrite(0x11);	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send Resume");	
  return;
}


void sdcc::reader::DoSendDIFReset()
{
  _driver->CCCCommandDIFReset();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send dif reset");	
  return;
}

void sdcc::reader::DoSendBCIDReset()
{
  _driver->CCCCommandBCIDReset();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send bcid reset");	
  return;
}

void sdcc::reader::DoSendStartAcquisitionAuto()
{
  _driver->CCCCommandStartAcquisitionAuto();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send start acquisition auto");	
  return;
}

void sdcc::reader::DoSendRamfullExt()
{
  _driver->CCCCommandRamfullExt();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send ramfull ext");	
  return;
}
void sdcc::reader::DoSendTrigExt()
{
  _driver->CCCCommandTriggerExt();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send trig ext");	
  return;
}
void sdcc::reader::DoSendStopAcquisition()
{
  _driver->CCCCommandStopAcquisition();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send stop acquisition");	
  return;
}
void sdcc::reader::DoSendDigitalReadout()
{
  _driver->CCCCommandDigitalReadout();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send digital readout");	
  return;
}
void sdcc::reader::DoSendTrigger()
{
  _driver->CCCCommandTrigger();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send analog readout");	
  return;
}
void sdcc::reader::DoSendClearMemory()
{
  _driver->CCCCommandClearMemory();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send clear memory");	
  return;
}
void sdcc::reader::DoSendStartSingleAcquisition()
{
  _driver->CCCCommandStartSingleAcquisition();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send start single acquisition");	
  return;
}
void sdcc::reader::DoSendPulseLemo()
{
  _driver->CCCCommandPulseLemo();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send lemo pulse");	
  return;
}
void sdcc::reader::DoSendRazChannel()
{
  _driver->CCCCommandRazChannel();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send raz channel");	
  return;
}
void sdcc::reader::DoSendCCCReset()
{
  _driver->CCCCommandResetCCC();	
if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send ccc reset");	
  
return;
}

void sdcc::reader::DoSendSpillOn()
{
  _driver->CCCCommandSpillOn();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send ccc spill on");	
  return;
}
void sdcc::reader::DoSendSpillOff()
{
  _driver->CCCCommandSpillOn();	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send ccc spill on");	
  return;
}

void sdcc::reader::DoWriteRegister(uint32_t addr,uint32_t data)
{
  _driver->UsbRegisterWrite(addr,data);	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send ccc spill on");	
  return;

}
uint32_t sdcc::reader::DoReadRegister(uint32_t addr)
{
  uint32_t data=0;
  _driver->UsbRegisterRead(addr,&data);	
  if (!_driver->isOk())  PM_ERROR(_logSdcc, "CCCReadout : Unable to send ccc spill on");	
  return data;

}
