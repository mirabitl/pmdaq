#include "MdccHandler.hh"

using namespace mdcc;
mdcc::MdccHandler::MdccHandler(std::string name,uint32_t productid) : _name(name),_productid(productid),_driver(NULL)
{
}
mdcc::MdccHandler::~MdccHandler()
{
  if (_driver!=NULL)
    this->close();
}

void mdcc::MdccHandler::open()
{
  std::cout<<_name<<" "<<_productid<<std::endl;
  _driver= new mdcc::FtdiUsbDriver((char*) _name.c_str(),_productid);
  if (!_driver->isOk())
    {      PM_FATAL(_logMdcc," Cannot open "<<_name<<" err="<<_driver->rcMessage());
      return;
    }
  _driver->UsbRegisterRead(0x1,&_version);
  _driver->UsbRegisterRead(0x100,&_id);
  if (!_driver->isOk())
    {
      PM_FATAL(_logMdcc," Cannot read version and ID ");
      return;
    }
  PM_INFO(_logMdcc," Mdcc "<<_name<<" ID="<<_id<<" version="<<_version);

}
void mdcc::MdccHandler::close()
{
  if (_driver!=NULL)
    delete _driver;
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
uint32_t mdcc::MdccHandler::version(){return this->readRegister(0x100);}
uint32_t mdcc::MdccHandler::id(){return this->readRegister(0x1);}
uint32_t mdcc::MdccHandler::mask(){return this->readRegister(0x2);}
void mdcc::MdccHandler::maskTrigger(){this->writeRegister(0x2,0x1);}
void mdcc::MdccHandler::unmaskTrigger(){this->writeRegister(0x2,0x0);}
uint32_t mdcc::MdccHandler::spillCount(){return this->readRegister(0x3);}
void mdcc::MdccHandler::resetCounter(){this->writeRegister(0x4,0x1);this->writeRegister(0x4,0x0);}
uint32_t mdcc::MdccHandler::spillOn(){return this->readRegister(0x5);}
uint32_t mdcc::MdccHandler::spillOff(){return this->readRegister(0x6);}
void mdcc::MdccHandler::setSpillOn(uint32_t nc){this->writeRegister(0x5,nc);}
void mdcc::MdccHandler::setSpillOff(uint32_t nc){this->writeRegister(0x6,nc);}
uint32_t mdcc::MdccHandler::beam(){return this->readRegister(0x7);}
void mdcc::MdccHandler::setBeam(uint32_t nc){this->writeRegister(0x7,nc);}
void mdcc::MdccHandler::calibOn(){this->writeRegister(0x8,0x2);}
void mdcc::MdccHandler::calibOff(){this->writeRegister(0x8,0x0);}
uint32_t mdcc::MdccHandler::calibCount(){return this->readRegister(0xa);}
void mdcc::MdccHandler::setCalibCount(uint32_t nc){this->writeRegister(0xa,nc);}

void mdcc::MdccHandler::setCalibRegister(uint32_t nc){this->writeRegister(0x8,nc);}

uint32_t mdcc::MdccHandler::hardReset(){return this->readRegister(0xc);}
void mdcc::MdccHandler::setHardReset(uint32_t nc){this->writeRegister(0xc,nc);}

void mdcc::MdccHandler::setSpillRegister(uint32_t nc){this->writeRegister(0xD,nc);}
uint32_t mdcc::MdccHandler::spillRegister(){return this->readRegister(0xD);}
void mdcc::MdccHandler::useSPSSpill(bool t)
{
  uint32_t reg=this->spillRegister();
  if (t)
    this->setSpillRegister(reg|1);
  else
    this->setSpillRegister(reg&~1);
}
void mdcc::MdccHandler::useTrigExt(bool t)
{
  uint32_t reg=this->spillRegister();
  if (t)
    this->setSpillRegister(reg|2);
  else
    this->setSpillRegister(reg&~2);
}

void mdcc::MdccHandler::setTriggerDelay(uint32_t nc){this->writeRegister(0xE,nc);}
uint32_t mdcc::MdccHandler::triggerDelay(){return this->readRegister(0xE);}
void mdcc::MdccHandler::setTriggerBusy(uint32_t nc){this->writeRegister(0xF,nc);}
uint32_t mdcc::MdccHandler::triggerBusy(){return this->readRegister(0xF);}

void mdcc::MdccHandler::setExternalTrigger(uint32_t nc){this->writeRegister(0x1A,nc);}
uint32_t mdcc::MdccHandler::externalTrigger(){return this->readRegister(0x1A);}

void mdcc::MdccHandler::reloadCalibCount(){

  this->maskTrigger();
  this->writeRegister(0xD,0x8);
  this->writeRegister(0x8,0x4);
  // sleep(1);
  // this->writeRegister(0x8,0x0);
  // sleep(1);
  this->unmaskTrigger();
  this->calibOn();


}




uint32_t mdcc::MdccHandler::ecalmask(){return this->readRegister(0xB);}
void mdcc::MdccHandler::maskEcal(){this->writeRegister(0xB,0x1);}
void mdcc::MdccHandler::unmaskEcal(){this->writeRegister(0xB,0x0);}
void mdcc::MdccHandler::resetTDC(uint8_t b){this->writeRegister(0xC,b);}
uint32_t mdcc::MdccHandler::busyCount(uint8_t b){return this->readRegister(0x10+(b&0xF));}



uint32_t mdcc::MdccHandler::readRegister(uint32_t adr)
{
  if (_driver==NULL)
    {
      PM_ERROR(_logMdcc,"Cannot read no driver created ");
      return 0xbad;
    }
  uint32_t rc;
  _driver->UsbRegisterRead(adr,&rc);
  if (!_driver->isOk())
    {
      PM_ERROR(_logMdcc," Cannot read at adr "<<adr);
      return 0xbad;
    }
  return rc;
}

void mdcc::MdccHandler::writeRegister(uint32_t adr,uint32_t val)
{
  if (_driver==NULL)
    {
      PM_ERROR(_logMdcc,"Cannot write no driver created ");
      return;
    }
  _driver->UsbRegisterWrite(adr,val);

  if (!_driver->isOk())
    {
      PM_ERROR(_logMdcc," Cannot write at adr "<<adr);
      return;
    }
}
