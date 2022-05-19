#include "IpdcHandler.hh"

using namespace ipdc;
ipdc::IpdcHandler::IpdcHandler(std::string name,uint32_t productid) : _name(name),_productid(productid),_driver(NULL)
{
}
ipdc::IpdcHandler::~IpdcHandler()
{
  if (_driver!=NULL)
    this->close();
}

void ipdc::IpdcHandler::open()
{
  std::cout<<_name<<" "<<_productid<<std::endl;
  _driver= new mdcc::FtdiUsbDriver((char*) _name.c_str(),_productid);
  if (!_driver->isOk())
    {      PM_FATAL(_logIpdc," Cannot open "<<_name<<" err="<<_driver->rcMessage());
      return;
    }
  _driver->UsbRegisterRead(0x1,&_version);
  _driver->UsbRegisterRead(0x100,&_id);
  if (!_driver->isOk())
    {
      PM_FATAL(_logIpdc," Cannot read version and ID ");
      return;
    }
  PM_INFO(_logIpdc," Ipdc "<<_name<<" ID="<<_id<<" version="<<_version);

}
void ipdc::IpdcHandler::close()
{
  if (_driver!=NULL)
    {
     this->maskTrigger();
    delete _driver;
    _driver=NULL;
    }
}

/*
  when x"0001" => USB_data_out <= ID_register;
  when x"0002" => USB_data_out <= software_veto_register;
  when x"0003" => USB_data_out <= spillNb_register;
  when x"0004" => USB_data_out <= Control_register;
  when x"0005" => USB_data_out <= spillon_register;
  when x"0006" => USB_data_out <= spilloff_register;
  when x"0007" => USB_data_out <= beam_register;
  when x"0008" => USB_data_out <= Calib_register;
  when x"0009" => USB_data_out <= Calib_Counter_register;
  when x"000A" => USB_data_out <= nb_windows_register;
  when x"000B" => USB_data_out <= software_ECALveto_register;
  when x"000C" => USB_data_out <= Rstdet_register;
  0xD   bit 0 => start/end of spill used
  bit 1 => trigext used
  default 0

  0XE delay trigext
  OXF length busy trigext 

  when x"0010" => USB_data_out <= busy0Nb_register;
  when x"0011" => USB_data_out <= busy1Nb_register;
  when x"0012" => USB_data_out <= busy2Nb_register;
  when x"0013" => USB_data_out <= busy3Nb_register;
  when x"0014" => USB_data_out <= busy4Nb_register;
  when x"0015" => USB_data_out <= busy5Nb_register;
  when x"0016" => USB_data_out <= busy6Nb_register;
  when x"0017" => USB_data_out <= busy7Nb_register;
  when x"0018" => USB_data_out <= busy8Nb_register;
  when x"0019" => USB_data_out <= busy9Nb_register;
  when x"001A" => USB_data_out <= busy10Nb_register;
  when x"001B" => USB_data_out <= busy11Nb_register;
  when x"001C" => USB_data_out <= busy12Nb_register;
  when x"001D" => USB_data_out <= busy13Nb_register;
  when x"001E" => USB_data_out <= busy14Nb_register;
  when x"001F" => USB_data_out <= busy15Nb_register;

  when x"0020" => USB_data_out <= spare0Nb_register;
  when x"0021" => USB_data_out <= spare1Nb_register;
 
  when x"0100" => USB_data_out <= version;
*/


#define IPDC_BUSY_MIN 0x20
#define IPDC_BUSY_ENABLE 0x21
#define IPDC_SPILL_ON 0x5
#define IPDC_SPILL_OFF 0x6

/* 
1 (Start/Stop) 2 (start + Spill_on)  4 (spill_on/Spill_off) 8 (calibration) 
*/
#define  IPDC_WINDOW_CONFIG 0xD
#define  IPDC_TRIGEXT_DELAY 0xE
#define  IPDC_TRIGEXT_LENGTH 0xF

/* 

Bit 0 Mask triggext pendant le busy
Bit 1 Triggext latch reset when busy 

*/
#define IPDC_TRIGEXT_CONFIG  0x10
#define IPDC_MASK 0x2

/*
Bit 1 Enable calib
Bit 2 reload calib count
 */
#define IPDC_CALIB_CTRL 0x8
#define IPDC_CALIB_COUNT 0xa
#define IPDC_HARD_RESET 0xC


#define USBDC_TEST 0x0
#define USBDC_ID 0x1
#define USBDC_NSPILL 0x3
#define USBDC_VERSION 0x100
#define USBDC_BUSY 0x11
#define USBDC_RESET_COUNTER 0x4

uint32_t ipdc::IpdcHandler::version(){return this->readRegister(USBDC_VERSION);}
uint32_t ipdc::IpdcHandler::id(){return this->readRegister(USBDC_ID);}
uint32_t ipdc::IpdcHandler::mask(){return this->readRegister(IPDC_MASK);}
void ipdc::IpdcHandler::maskTrigger(){this->writeRegister(IPDC_MASK,0x1);}
void ipdc::IpdcHandler::unmaskTrigger(){this->writeRegister(IPDC_MASK,0x0);}
uint32_t ipdc::IpdcHandler::spillCount(){return this->readRegister(USBDC_NSPILL);}
void ipdc::IpdcHandler::resetCounter(){this->writeRegister(USBDC_RESET_COUNTER,0x1);this->writeRegister(USBDC_RESET_COUNTER,0x0);}
uint32_t ipdc::IpdcHandler::spillOn(){return this->readRegister(IPDC_SPILL_ON);}
uint32_t ipdc::IpdcHandler::spillOff(){return this->readRegister(IPDC_SPILL_OFF);}
uint32_t ipdc::IpdcHandler::busyEnable(){return this->readRegister(IPDC_BUSY_ENABLE);}
void ipdc::IpdcHandler::setSpillOn(uint32_t nc){this->writeRegister(IPDC_SPILL_ON,nc);}
void ipdc::IpdcHandler::setSpillOff(uint32_t nc){this->writeRegister(IPDC_SPILL_OFF,nc);}

void ipdc::IpdcHandler::setBusyEnable(uint32_t nc){this->writeRegister(IPDC_BUSY_ENABLE,nc);}



void ipdc::IpdcHandler::calibOn(){this->writeRegister(IPDC_CALIB_CTRL,0x2);}
void ipdc::IpdcHandler::calibOff(){this->writeRegister(IPDC_CALIB_CTRL,0x0);}
uint32_t ipdc::IpdcHandler::calibCount(){return this->readRegister(IPDC_CALIB_COUNT);}
void ipdc::IpdcHandler::setCalibCount(uint32_t nc){this->writeRegister(IPDC_CALIB_COUNT,nc);}

void ipdc::IpdcHandler::setCalibRegister(uint32_t nc){this->writeRegister(IPDC_CALIB_CTRL,nc);}

uint32_t ipdc::IpdcHandler::hardReset(){return this->readRegister(IPDC_HARD_RESET);}
void ipdc::IpdcHandler::setHardReset(uint32_t nc){this->writeRegister(IPDC_HARD_RESET,nc);}

void ipdc::IpdcHandler::setSpillRegister(uint32_t nc){this->writeRegister(IPDC_WINDOW_CONFIG,nc);}
uint32_t ipdc::IpdcHandler::spillRegister(){return this->readRegister(IPDC_WINDOW_CONFIG);}


void ipdc::IpdcHandler::setTriggerDelay(uint32_t nc){this->writeRegister(IPDC_TRIGEXT_DELAY,nc);}
uint32_t ipdc::IpdcHandler::triggerDelay(){return this->readRegister(IPDC_TRIGEXT_DELAY);}
void ipdc::IpdcHandler::setTriggerBusy(uint32_t nc){this->writeRegister(IPDC_TRIGEXT_LENGTH,nc);}
uint32_t ipdc::IpdcHandler::triggerBusy(){return this->readRegister(IPDC_TRIGEXT_LENGTH);}

void ipdc::IpdcHandler::setExternalTrigger(uint32_t nc){this->writeRegister(IPDC_TRIGEXT_CONFIG,nc);}
uint32_t ipdc::IpdcHandler::externalTrigger(){return this->readRegister(IPDC_TRIGEXT_CONFIG);}

void ipdc::IpdcHandler::reloadCalibCount(){

  this->maskTrigger();
  this->writeRegister(IPDC_WINDOW_CONFIG,0x8);
  this->writeRegister(IPDC_CALIB_CTRL,0x4);
  // sleep(1);
  // this->writeRegister(0x8,0x0);
  // sleep(1);
  this->unmaskTrigger();
  this->calibOn();


}




void ipdc::IpdcHandler::resetTDC(uint8_t b){this->writeRegister(IPDC_HARD_RESET,b);}
uint32_t ipdc::IpdcHandler::busyCount(uint8_t b)
{
  if (b==0)
    return this->readRegister(USBDC_BUSY+(b&0xF));
  else
    return 0xbad;}



uint32_t ipdc::IpdcHandler::readRegister(uint32_t adr)
{
  if (_driver==NULL)
    {
      PM_ERROR(_logIpdc,"Cannot read no driver created ");
      return 0xbad;
    }
  uint32_t rc;
  _driver->UsbRegisterRead(adr,&rc);
  if (!_driver->isOk())
    {
      PM_ERROR(_logIpdc," Cannot read at adr "<<adr);
      return 0xbad;
    }
  return rc;
}

void ipdc::IpdcHandler::writeRegister(uint32_t adr,uint32_t val)
{
  if (_driver==NULL)
    {
      PM_ERROR(_logIpdc,"Cannot write no driver created ");
      return;
    }
  _driver->UsbRegisterWrite(adr,val);

  if (!_driver->isOk())
    {
      PM_ERROR(_logIpdc," Cannot write at adr "<<adr);
      return;
    }
}
