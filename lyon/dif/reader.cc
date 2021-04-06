#include "reader.hh"
#include <unistd.h>

using namespace dif;
/* Main program de readout */
dif::reader::reader(std::string name, uint32_t productid) : dif::driver((char *)name.c_str(), productid), theName_(name), theAsicType_(2), theNumberOfAsics_(48), theControlRegister_(0x80181B00), theCurrentSLCStatus_(0),
                                                                thePwrToPwrARegister_(0x3E8), thePwrAToPwrDRegister_(0x3E6), thePwrDToDAQRegister_(0x4E), theDAQToPwrDRegister_(0x4E), thePwrDToPwrARegister_(0x4E)
{
  sscanf(name.c_str(), "FT101%d", &theDIFId_);
}
dif::reader::~reader()
{
  printf("%s %d \n", __PRETTY_FUNCTION__, __LINE__);
}
void dif::reader::setPowerManagment(uint32_t P2PAReg, uint32_t PA2PDReg, uint32_t PD2DAQReg, uint32_t DAQ2DReg, uint32_t D2AReg)
{

  thePwrToPwrARegister_ = P2PAReg;
  thePwrAToPwrDRegister_ = PA2PDReg;
  thePwrDToDAQRegister_ = PD2DAQReg;
  theDAQToPwrDRegister_ = DAQ2DReg;
  thePwrDToPwrARegister_ = D2AReg;
}
void dif::reader::setAsicType(uint32_t asicType)
{
  theAsicType_ = asicType;
}
void dif::reader::setNumberOfAsics(uint32_t NumberOfAsics)
{
  theNumberOfAsics_ = NumberOfAsics;
}

void dif::reader::setControlRegister(uint32_t ControlRegister)
{
  theControlRegister_ = ControlRegister;
}

void dif::reader::initialise(uint32_t difid, uint32_t asicType, uint32_t NumberOfAsics, uint32_t ctrlreg, uint32_t P2PAReg, uint32_t PA2PDReg, uint32_t PD2DAQReg, uint32_t DAQ2DReg, uint32_t D2AReg)
{
  LOG4CXX_INFO(_logDif, "Initialise");
  theDIFId_ = difid;
  theAsicType_ = asicType;
  theNumberOfAsics_ = NumberOfAsics;
  theControlRegister_ = ctrlreg;
  thePwrToPwrARegister_ = P2PAReg;
  thePwrAToPwrDRegister_ = PA2PDReg;
  thePwrDToDAQRegister_ = PD2DAQReg;
  theDAQToPwrDRegister_ = DAQ2DReg;
  thePwrDToPwrARegister_ = D2AReg;
  HardrocFlushDigitalFIFO();
  theCurrentSLCStatus_ = 0;
}

void dif::reader::start()
{
  // Nothing to do yet
  LOG4CXX_INFO(_logDif, "Start");
  HardrocFlushDigitalFIFO();
}
void dif::reader::stop()
{
  LOG4CXX_INFO(_logDif, "Stop");
  HardrocFlushDigitalFIFO();
  HardrocStopDigitalAcquisitionCommand();
  HardrocFlushDigitalFIFO();
}
void dif::reader::DoRefreshNbOfASICs()
{
  uint8_t tnbasicsl1 = theNumberOfAsics_ & 0xFF;
  uint8_t tnbasicsl2 = (theNumberOfAsics_ >> 8) & 0xFF;
  uint8_t tnbasicsl3 = (theNumberOfAsics_ >> 16) & 0xFF;
  uint8_t tnbasicsl4 = (theNumberOfAsics_ >> 24) & 0xFF;
  uint8_t tnbasics = tnbasicsl1 + tnbasicsl2 + tnbasicsl3 + tnbasicsl4;
  //   printf ("thetheNumberOfAsics__= %d\n",theNumberOfAsics_);
  //   printf ("tnbasics= %d\n",tnbasics);
  //   printf ("tnbasicsl1= %d\n",tnbasicsl1);
  //   printf ("tnbasicsl2= %d\n",tnbasicsl2);
  //   printf ("tnbasicsl3= %d\n",tnbasicsl3);
  //   printf ("tnbasicsl4= %d\n",tnbasicsl4);
  if (theAsicType_ == 11)
  {

    //printf ("MicroRoc assuming all asics present, tu be completed!!\n");
    NbAsicsWrite(tnbasics, tnbasicsl1, tnbasicsl2, tnbasicsl3, tnbasicsl4);
    if (!isOk())
    {
      PM_ERROR(_logDif, "Unable to set the correct number of hardrocs" << theName_);
    }
    return;
  }
  else
  {
    //printf ("HR2 theNumberOfAsics_= %d\n",theNumberOfAsics_);
    NbAsicsWrite(tnbasics, tnbasics, 0, 0, 0);
    if (!isOk())
    {
      PM_ERROR(_logDif, "Unable to set the correct number of hardrocs" << theName_);
    }
    return;
  }
}

int32_t dif::reader::DoReadSLCStatus()
{
  uint32_t CurrentSLCStatus = 0;
  int32_t tretry = 0;
  while ((CurrentSLCStatus == 0) && (tretry < 5))
  {

    uint32_t rx, tx, ev;
    HardrocSLCStatusRead(&CurrentSLCStatus);
    if (!isOk())
      PM_ERROR(_logDif, "Unable to send command to DIF : " << theName_);
    tretry++;
  }
  std::cout << theName_ << " ";
  theCurrentSLCStatus_ = CurrentSLCStatus;
  if (theAsicType_ == 2)
  {
    if ((theCurrentSLCStatus_ & 0x0003) == 0x01)
      std::cout << "SLC CRC OK       - ";
    else if ((theCurrentSLCStatus_ & 0x0003) == 0x02)
      std::cout << "SLC CRC Failed   - ";
    else
      std::cout << "SLC CRC forb  - ";
    if ((theCurrentSLCStatus_ & 0x000C) == 0x04)
      std::cout << "All OK      - ";
    else if ((theCurrentSLCStatus_ & 0x000C) == 0x08)
      std::cout << "All Failed  - ";
    else
      std::cout << "All forb - ";
    if ((theCurrentSLCStatus_ & 0x0030) == 0x10)
      std::cout << "L1 OK     - " << std::endl;
    else if ((theCurrentSLCStatus_ & 0x0030) == 0x20)
      std::cout << "L1 Failed - " << std::endl;
    else
      std::cout << "L1 forb   - " << std::endl;
  }
  else if (theAsicType_ == 11)
  {
    if ((theCurrentSLCStatus_ & 0x0003) == 0x01)
      std::cout << "SLC CRC OK       - ";
    else if ((theCurrentSLCStatus_ & 0x0003) == 0x02)
      std::cout << "SLC CRC Failed   - ";
    else
      std::cout << "SLC CRC forb  - ";
    if ((theCurrentSLCStatus_ & 0x000C) == 0x04)
      std::cout << "All OK      - ";
    else if ((theCurrentSLCStatus_ & 0x000C) == 0x08)
      std::cout << "All Failed  - ";
    else
      std::cout << "All forb - ";
    if ((theCurrentSLCStatus_ & 0x0030) == 0x10)
      std::cout << "L1 OK     - ";
    else if ((theCurrentSLCStatus_ & 0x0030) == 0x20)
      std::cout << "L1 Failed - ";
    else
      std::cout << "L1 forb   - ";
    if ((theCurrentSLCStatus_ & 0x00C0) == 0x40)
      std::cout << "L2 OK     - ";
    else if ((theCurrentSLCStatus_ & 0x00C0) == 0x80)
      std::cout << "L2 Failed - ";
    else
      std::cout << "L2 forb   - ";
    if ((theCurrentSLCStatus_ & 0x0300) == 0x0100)
      std::cout << "L3 OK     - ";
    else if ((theCurrentSLCStatus_ & 0x0300) == 0x0200)
      std::cout << "L3 Failed - ";
    else
      std::cout << "L3 forb   - ";
    if ((theCurrentSLCStatus_ & 0x0C00) == 0x0400)
      std::cout << "L4 OK" << std::endl;
    else if ((theCurrentSLCStatus_ & 0x0C00) == 0x0800)
      std::cout << "L4 Failed" << std::endl;
    else
      std::cout << "L4 forb -" << std::endl;
  }

  return CurrentSLCStatus;
}

void dif::reader::configureRegisters()
{

  UsbSetDIFID(theDIFId_);
  uint32_t toto;
  GetControlRegister(&toto);
  //	printf ("avant control reg default value = %08lx\n",toto);
  //std::cout <<__PRETTY_FUNCTION__<<"DIFID set "<<std::endl;
  DoRefreshNbOfASICs();

  SetEventsBetweenTemperatureReadout(5);
  SetAnalogConfigureRegister(0xC0054000);
  HardrocFlushDigitalFIFO();

  //printf("Power %d %d %d %d \n",ds->PowerAnalog,ds->PowerDAC,ds->PowerDigital,ds->PowerADC);
  uint32_t CurrentDIFFwVersion;
  UsbGetFirmwareRevision(&CurrentDIFFwVersion);
  PM_INFO(_logDif, " DIF" << theName_ << " Version " << std::hex << CurrentDIFFwVersion << std::dec);
  DIFCptReset();
  //	printf ("control reg settings to be modified!!!\n");

  uint32_t ttype;
  GetControlRegister(&toto);
  //	printf ("control reg default value = %08lx\n",toto);
  // 	toto = 0x80181B40;// BT
  //	toto=0x81181B40; // DCC CCC et BT
  //	toto = 0x80181B40;// BT CCC
  //	toto = 0x80181B00;// ILC CCC
  //	  toto = 0x81181B00; // ilc et DCC-CCC
  toto = theControlRegister_;
  //	printf ("control reg new  value = %08lx\n",toto);
  if (theAsicType_ == 2)
    SetChipTypeRegister(0x100);
  else if (theAsicType_ == 11)
    SetChipTypeRegister(0x1000);

  if (!isOk())
    PM_ERROR(_logDif, " Unable to set asic type in dif" << theName_);

  GetChipTypeRegister(&ttype);
  if (!isOk())
    PM_ERROR(_logDif, " Unable to get asic type in dif" << theName_);

  SetControlRegister(toto);

  if (!isOk())
    PM_ERROR(_logDif, " Unable to send control reg value" << theName_);
  GetControlRegister(&toto);
  PM_INFO(_logDif, "  CtrlReg =" << std::hex << toto << std::dec);

  SetPwrToPwrARegister(thePwrToPwrARegister_);

  if (!isOk())
    PM_ERROR(_logDif, "Unable to send pwr to A reg value" << theName_);

  SetPwrAToPwrDRegister(thePwrAToPwrDRegister_);
  if (!isOk())
    PM_ERROR(_logDif, ": Unable to send A to D reg value" << theName_);

  SetPwrDToDAQRegister(thePwrDToDAQRegister_);
  if (!isOk())
    PM_ERROR(_logDif, ": Unable to send D to Daq reg value" << theName_);

  SetDAQToPwrDRegister(theDAQToPwrDRegister_);
  if (!isOk())
    PM_ERROR(_logDif, ": Unable to send daq to D reg value" << theName_);

  SetPwrDToPwrARegister(thePwrDToPwrARegister_);

  if (!isOk())
    PM_ERROR(_logDif, ": Unable to send D to A reg value" << theName_);
}

void dif::reader::DoSendDIFTemperatureCommand()
{
  //std::cout<<"Usb do send dif temperature command"<<std::endl;
  HardrocCommandAskDifTemperature();
  if (!isOk())
    PM_ERROR(_logDif, " \t Unable to send dif temperature command" << theName_);
  return;
}

void dif::reader::DoSendASUTemperatureCommand()
{
  //std::cout<<"Usb do send asu temperature command"<<std::endl;
  HardrocCommandAskAsuTemperature();
  if (!isOk())
    PM_ERROR(_logDif, " \t Unable to send asu temperature command");
  return;
}

void dif::reader::DoGetASUTemperatureCommand(uint32_t *ttemp1, uint32_t *ttemp2)
{
  GetASUTemperature(ttemp1, ttemp2);
  if (!isOk())
    PM_ERROR(_logDif, " \t Unable to send asu temperature command");
  //	printf ("DIFReadout ASUTemp = %d %d \n",*ttemp1,*ttemp2);
  theTemperatureBuffer_[0] = (*ttemp1) * 1.0;
  theTemperatureBuffer_[1] = (*ttemp2) * 1.0;
  return;
}

void dif::reader::DoGetDIFTemperatureCommand(uint32_t *ttemp)
{
  GetDIFTemperature(ttemp);
  if (!isOk())
    PM_ERROR(_logDif, " \t Unable to send asu temperature command");
  //	printf ("DIFReadout DIFTemp = %d \n",*ttemp);
  theTemperatureBuffer_[0] = (*ttemp) * 1.0;
  return;
}

void dif::reader::DoSetTemperatureReadoutToAuto(uint32_t tvalue)
{
  uint32_t ttemp;
  SetTemperatureReadoutToAuto(tvalue);
  if (!isOk())
    PM_ERROR(_logDif, " \t  Unable to send asu temperature mode");
  return;
}

void dif::reader::DoGetTemperatureReadoutAutoStatus(uint32_t *tvalue)
{
  uint32_t ttemp;

  GetTemperatureReadoutAutoStatus(tvalue);
  if (!isOk())
    PM_ERROR(_logDif, " \t Unable to send asu temperature mode");
  //	printf("DIFReadout Temperature status =/n",*tvalue);
  return;
}

void dif::reader::DoSetEventsBetweenTemperatureReadout(uint32_t tvalue)
{
  uint32_t ttemp;

  SetEventsBetweenTemperatureReadout(tvalue);
  if (!isOk())
    PM_ERROR(_logDif, " Unable to set temperature readoutfrequency");
  return;
}

void dif::reader::DoSetAnalogConfigureRegister(uint32_t tdata)
{
  uint32_t ttemp;
  SetAnalogConfigureRegister(tdata);
  if (!isOk())
    PM_ERROR(_logDif, "Unable to set analog  conf register");
  return;
}

static unsigned char vframe[HARDROCV2_SLC_FRAME_SIZE];
int32_t dif::reader::configureChips(SingleHardrocV2ConfigurationFrame *slow) //throw( LocalHardwareException )
{
  // send configure command
  unsigned short tCRC;
  unsigned char CurrentCRC[2];
  uint32_t framesize = 0;
  // set default register values
  uint32_t regctrl;
  //   UsbRegisterRead(2,&regctrl);
  //   printf("Apres test  %x \n",regctrl);
  //   getchar();
  //   UsbRegisterRead(0,&regctrl);
  //   printf("Apres Reg0  %x \n",regctrl);
  //   getchar();
  if (theAsicType_ == 2)
    framesize = HARDROCV2_SLC_FRAME_SIZE;
  else if (theAsicType_ == 11)
    framesize = MICROROC_SLC_FRAME_SIZE;
  else
    std::cout << "Bad Asic type" << std::endl;

  //UsbRegisterWrite2(0,0x100);
  tCRC = 0xFFFF; // initial value of the CRC
                 //printf ("avant HardrocCommandSLCWrite\n");
                 //getchar();

  HardrocCommandSLCWrite();

  if (!isOk())
  {
    PM_ERROR(_logDif, theName_ << ": Unable to send start SLC command to DIF");
    return 0;
  }
  // printf("avant Frame \n");
  //getchar();

  for (int tAsic = theNumberOfAsics_; tAsic > 0; tAsic--)
  {
    //      std::cout<<"Configuring "<<tAsic<<std::endl;
    //unsigned char vframe[framesize];
    for (int tbyte = 0; tbyte < framesize; tbyte++)
    {

      //slow[tAsic-1][tbyte]=0x40;//tbyte + (((tAsic-1)&0x01)<<7);

      printf("%02x", slow[tAsic - 1][tbyte]);
      vframe[tbyte] = slow[tAsic - 1][tbyte];
    }
    printf("\n");

    for (int tbyte = 0; tbyte < framesize; tbyte++)
    {
      tCRC = ((tCRC & 0xFF) << 8) ^ (FtdiUsbDriver::CrcTable[(tCRC >> 8) ^ slow[tAsic - 1][tbyte]]);
    }
    //HardrocCommandSLCWriteSingleSLCFrame(&(theSlowBuffer_[tAsic-1][0]));
    if (theAsicType_ == 2)
      CommandSLCWriteSingleSLCFrame(vframe, framesize);
    if (theAsicType_ == 11)
      CommandSLCWriteSingleSLCFrame(vframe, framesize);
    //printf("apres Frame %d \n",framesize);
    // getchar();

    if (!isOk())
    {
      PM_ERROR(_logDif, ":Unable to send SLC frame to DIF");
      return 0;
    }

  } //for (int tAsic=NbOfASICs;tAsic>0;tAsic--)
  //  printf("avant CRC\n");
  // getchar();

  CurrentCRC[0] = tCRC >> 8; // MSB first
  CurrentCRC[1] = tCRC & 0xFF;

  HardrocCommandSLCWriteCRC(&CurrentCRC[0]);
  if (!isOk())
  {
    PM_ERROR(_logDif, ":Unable to send CRC");
    return 0;
  }

  //  checkReadWrite(0x100,10);
  //printf("avant Test\n");
  //getchar();
  //UsbRegisterRead(2,&regctrl);
  //printf("Avant SLCStatus  %x \n",regctrl);
  //getchar();

  usleep(400000); // was 500 ms
  //  int tretry=0;
  //  printf ("before slc status\n");
  theCurrentSLCStatus_ = this->DoReadSLCStatus();

  return theCurrentSLCStatus_;
}

uint32_t dif::reader::DoHardrocV2ReadoutDigitalData(unsigned char *CurrentDIFDigitalData)
{

  unsigned int ComputedCRC = 0xFFFF;
  unsigned int ReceivedCRC = 0xFFFF;
  unsigned char tdata;
  unsigned char tdata32[42];
  unsigned char tdataana[MAX_ANALOG_DATA_SIZE];
  int tHardrocIndex = 0;
  int tMemoryIndex = 0;
  int tBunchIndex = 0;
  int tindex = 0;
  double tefficacity = 0;
  int tDIFIDIndex = 0;
  char tdisplay[600];
  unsigned tCurrentAbsoluteBCIDMSB;
  uint32_t tCurrentAbsoluteBCIDLSB;

  memset(CurrentDIFDigitalData, 0, MAX_EVENT_SIZE);
  uint16_t theadersize;
  // on calcule le CRC a la volee

  UsbReadByte(&tdata);
  // Global header
  if (!isOk())
  {
    PM_DEBUG(_logDif, " "
                          << "==>"
                          << "DIF : no data, exiting ");
    return 0;
  }
  uint32_t CurrentNbOfEventsPerTrigger = 0;

  //  printf ("%02x",tdata);

  if ((tdata != 0xB0) && (tdata != 0xBB)) // global header
  {
    PM_ERROR(_logDif, " "
                          << "==>"
                          << "DIF : Bad global header(" << std::hex << (int)tdata << std::dec << " instead of 0xb0 or 0xbb), exiting");
    HardrocFlushDigitalFIFO();
    return 0;
  }
  tindex = 0;

  CurrentDIFDigitalData[tindex++] = tdata;                                                                             // global header
  ComputedCRC = ((ComputedCRC & 0xFF) << 8) ^ (FtdiUsbDriver::CrcTable[((ComputedCRC >> 8) ^ (tdata & 0xFF)) & 0xFF]); // global header

  tDIFIDIndex = tindex;
  // DIF ID
  //DIF trigger counter  (32 bits)
  //acq trigger counter  (32 bits)
  //global trigger counter  (32 bits)
  //absolute bcid counter (48bits, natural binary, MSB first)
  //timedif counter (24bits, natural binary, MSB first)
  if (tdata == 0xb0)
    theadersize = 23;
  else
    theadersize = 32;

  UsbReadnBytes(tdata32, theadersize);

  if (!isOk())
  {
    PM_ERROR(_logDif, " "
                          << "==> no DIF Header");
    HardrocFlushDigitalFIFO();
    return 0;
  }

  for (int ti = 0; ti < theadersize; ti++)
  {
    //printf ("%02x",tdata32[ti]);
    CurrentDIFDigitalData[tindex++] = tdata32[ti];
    ComputedCRC = ((ComputedCRC & 0xFF) << 8) ^ (FtdiUsbDriver::CrcTable[((ComputedCRC >> 8) ^ (tdata32[ti] & 0xFF)) & 0xFF]); // dif_id
  }
  //printf ("\n");
  uint32_t CurrentDifId = tdata32[0];
  // printf ("DIF = %d\n",CurrentDifId);
  if (CurrentDifId != theDIFId_)
  {
    PM_ERROR(_logDif, " "
                          << "==>"
                          << "DIF " << CurrentDifId << " :  Invalid  DIF ID " << theDIFId_);
    HardrocFlushDigitalFIFO();
    return 0;
  }
  CurrentDIFDigitalData[tDIFIDIndex] = CurrentDifId;

  uint32_t CurrentDIFTriggerCounter = tdata32[1] << 24;
  CurrentDIFTriggerCounter += tdata32[2] << 16;
  CurrentDIFTriggerCounter += tdata32[3] << 8;
  CurrentDIFTriggerCounter += tdata32[4];
  //  std::cout<<CurrentDIFTriggerCounter<<" is read"<<std::endl;
  uint32_t CurrentAcqTriggerCounter = tdata32[5] << 24;
  CurrentAcqTriggerCounter += tdata32[6] << 16;
  CurrentAcqTriggerCounter += tdata32[7] << 8;
  CurrentAcqTriggerCounter += tdata32[8];

  uint32_t CurrentGlobalTriggerCounter = tdata32[9] << 24;
  CurrentGlobalTriggerCounter += tdata32[10] << 16;
  CurrentGlobalTriggerCounter += tdata32[11] << 8;
  CurrentGlobalTriggerCounter += tdata32[12];

  tCurrentAbsoluteBCIDMSB = tdata32[13] << 8;
  tCurrentAbsoluteBCIDMSB += tdata32[14];

  tCurrentAbsoluteBCIDLSB = tdata32[15] << 24;
  tCurrentAbsoluteBCIDLSB += tdata32[16] << 16;
  tCurrentAbsoluteBCIDLSB += tdata32[17] << 8;
  tCurrentAbsoluteBCIDLSB += tdata32[18];

  uint32_t CurrentTimeDif = tdata32[19] << 16;
  CurrentTimeDif += tdata32[20] << 8;
  CurrentTimeDif += tdata32[21];

  uint32_t CurrentNbLines = tdata32[22] >> 4;
  //std::cout<<"CurrentNbLines="<<CurrentNbLines<<std::endl;
  uint32_t CurrentTemperatureASU1;
  uint32_t CurrentTemperatureASU2;
  uint32_t CurrentTemperatureDIF;
  if (tdata == 0xbb)
  {
    CurrentTemperatureASU1 = tdata32[23] << 24;
    CurrentTemperatureASU1 += tdata32[24] << 16;
    CurrentTemperatureASU1 += tdata32[25] << 8;
    CurrentTemperatureASU1 += tdata32[26];

    CurrentTemperatureASU2 = tdata32[27] << 24;
    CurrentTemperatureASU2 += tdata32[28] << 16;
    CurrentTemperatureASU2 += tdata32[29] << 8;
    CurrentTemperatureASU2 += tdata32[30];

    CurrentTemperatureDIF = tdata32[31];

    //		printf ("Temps = %d %d %d \n",CurrentTemperatureDIF,CurrentTemperatureASU1,CurrentTemperatureASU2);
  }

  uint32_t CurrentHardrocHeader = 0;
  int tcontinue = 1;
  while (tcontinue)
  {

    UsbReadByte(&tdata);
    //frame hearder or Global trailer
    if (!isOk())
    {
      PM_ERROR(_logDif, " "
                            << "==>"
                            << "DIF" << theDIFId_ << ":  There should be a frame header/global trailer");
      HardrocFlushDigitalFIFO();
      return 0;
    }
    CurrentDIFDigitalData[tindex++] = tdata;
    uint32_t theader = tdata;

    if (theader == 0xC4) // analog frame header
    {
      for (uint32_t tl = 0; tl < CurrentNbLines; tl++)
      {
        UsbReadByte(&tdata);
        //nb of chips on line tl
        if (!isOk())
        {
          PM_ERROR(_logDif, "DIF %s :  There should be number of chip on line %d" << theDIFId_ << " " << tl);
          HardrocFlushDigitalFIFO();
          return 0;
        }

        CurrentDIFDigitalData[tindex++] = tdata;
        uint32_t tanasize = tdata * 64 * 2;
        if (tdata > 12)
          printf("erreur taille data analogiques = %d\n", tdata);

        UsbReadnBytes(tdataana, tanasize);
        if (!isOk())
        {
          PM_ERROR(_logDif, "DIF " << theDIFId_ << "There should analog data");
          HardrocFlushDigitalFIFO();
          return 0;
        }
        for (uint32_t ti = 0; ti < tanasize; ti++)
        {
          CurrentDIFDigitalData[tindex++] = tdataana[ti];
        }
      } //for (uint32_t tl=0;tl<	CurrentNbLines;tl++)

      UsbReadByte(&tdata);
      //0xD4
      if (!isOk())
      {
        PM_ERROR(_logDif, "DIF :  There should be a 0xD4" << theName_);
        HardrocFlushDigitalFIFO();
        return 0;
      }
      //			printf ("%02x\n",tdata);
      CurrentDIFDigitalData[tindex++] = tdata; //0xD4
    }
    else if (theader == 0xB4) // frame header
    {
      ComputedCRC = ((ComputedCRC & 0xFF) << 8) ^ (FtdiUsbDriver::CrcTable[((ComputedCRC >> 8) ^ (theader & 0xFF)) & 0xFF]); // B4 in crc but not C4
      while (1)
      {
        UsbReadByte(&tdata);

        if (!isOk())
        {

          PM_ERROR(_logDif, " There should be a valid frame trailer/hardroc header" << theName_);
          HardrocFlushDigitalFIFO();
          return 0;
        }
        if ((tdata != 0xA3) && (tdata != 0xC3)) //not a frame trailer, so a hardroc header
        {
          if (tMemoryIndex > ASIC_MEM_DEPTH)
          {
            PM_ERROR(_logDif, " unable to read more than 128 Memory indexes" << theName_);
            HardrocFlushDigitalFIFO();
            return 0;
          }
          ComputedCRC = ((ComputedCRC & 0xFF) << 8) ^ (FtdiUsbDriver::CrcTable[((ComputedCRC >> 8) ^ (tdata & 0xFF)) & 0xFF]); // hardroc header
          CurrentDIFDigitalData[tindex++] = tdata;
          CurrentHardrocHeader = tdata;
          CurrentNbOfEventsPerTrigger++;

          UsbReadnBytes(tdata32, 19);
          // BCID
          if (!isOk())
          {
            PM_ERROR(_logDif, " There should be a hardroc frame" << theName_);
            HardrocFlushDigitalFIFO();
            return 0;
          }
          // bcid (3 bytes)
          // data (16bytes)
          //printf("ASIC %d :",tdata);
          for (int ti = 0; ti < 19; ti++)
          {
            // printf("%02x",tdata32[ti]);
            CurrentDIFDigitalData[tindex++] = tdata32[ti];
            ComputedCRC = ((ComputedCRC & 0xFF) << 8) ^ (FtdiUsbDriver::CrcTable[((ComputedCRC >> 8) ^ (tdata32[ti] & 0xFF)) & 0xFF]);
          }
          //printf("\n");
          tMemoryIndex++;
          if (tMemoryIndex > ASIC_MEM_DEPTH)
          {
            PM_ERROR(_logDif, "tMemoryIndex > ASIC_MEM_DEPTH" << theName_);
            return 0;
          }
        }
        else //if ((tdata != 0xA3)&&(tdata != 0xC3))
        {    //frame trailer
          if (tdata == 0xC3)
          {
            PM_ERROR(_logDif, " Incomplete frame received (0xC3)" << theName_);
            HardrocFlushDigitalFIFO();
            return 0;
          }
          ComputedCRC = ((ComputedCRC & 0xFF) << 8) ^ (FtdiUsbDriver::CrcTable[((ComputedCRC >> 8) ^ (tdata & 0xFF)) & 0xFF]); // frame trailer
          CurrentDIFDigitalData[tindex++] = tdata;
          tMemoryIndex = 0; // next hardroc, so mem index should be resetedSBad glo
          tHardrocIndex++;
          if (tHardrocIndex > MAX_NB_OF_ASICS + 1)
          {
            PM_ERROR(_logDif, " tHardrocIndex= " << tHardrocIndex << " > MAX_NB_OF_ASICS" << std::hex << (int)tdata << std::dec);

            for (int ic = 0; ic < tindex; ic++)
              printf("%02x", CurrentDIFDigitalData[ic]);
            printf("\n");

            //PM_ERROR(_logDif," "<<"==>"<<toolbox::toString("tHardrocIndex > MAX_NB_OF_ASICS"));

            HardrocFlushDigitalFIFO();
            return 0;
          }
          break;
        }                   //if ((tdata != 0xA3)&&(tdata != 0xC3))
      }                     //while (1)
    }                       //if (tdata==0xB4)
    else if (tdata == 0xA0) // global trailer
    {

      UsbRead2Bytes(&ReceivedCRC);
      // CRC
      if (!isOk())
      {
        PM_ERROR(_logDif, " There should be a valid CRC" << theName_);
        HardrocFlushDigitalFIFO();
        return 0;
      }
      CurrentDIFDigitalData[tindex++] = (ReceivedCRC >> 8) & 0xFF;
      CurrentDIFDigitalData[tindex++] = (ReceivedCRC >> 0) & 0xFF;

      if (ComputedCRC == ReceivedCRC)
      {
        tcontinue = 0;
      }
      else
      {
#ifdef DISPLAY_CRC
        PM_ERROR(_logDif, " CRC mismatch ( received" << ReceivedCRC << " instead " << ComputedCRC << " " << theName_);
#endif
        tcontinue = 0;
      }
    } //else if (tdata == 0xA0)
  }   //while (tcontinue)

  /*
	  printf ("\n");
	  for (int i=0;i<tindex;i++)
	  printf ("%02x", CurrentDIFDigitalData[i]);
	  printf ("\n");
	*/
  //this->append(CurrentDIFDigitalData,tindex);
  return tindex;
}
