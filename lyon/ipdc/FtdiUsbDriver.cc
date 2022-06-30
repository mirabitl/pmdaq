/************************************************************************/
/* ILC test beam daq							*/	
/* C. Combaret								*/
/* V 1.0								*/
/* first release : 30-01-2008						*/
/* revs : 								*/
/************************************************************************/
#include "FtdiUsbDriver.hh"
#include <iostream>
#include <sstream>
#include <sys/timeb.h>

using namespace mdcc;

uint16_t mdcc::FtdiUsbDriver::CrcTable[256] = 
  {     
   0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
   0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
   0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
   0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
   0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
   0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
   0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
   0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
   0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
   0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
   0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
   0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
   0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
   0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
   0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
   0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
   0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
   0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
   0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
   0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
   0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
   0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
   0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
   0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
   0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
   0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
   0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
   0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
   0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
   0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
   0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
   0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
  };



#define MY_DEBUG

mdcc::FtdiUsbDriver::FtdiUsbDriver(char * deviceIdentifier, uint32_t productid )    
{
  _rc=mdcc::FtdiUsbDriver::RC::OK;
  int ret;
  int ntry=0;
  theProduct_=productid;
  printf("%s \n",deviceIdentifier);
 start:
  if (ftdi_init(&theFtdi) < 0)
    {
      _rc=RC::OPENFAILED;
      fprintf(stderr, "ftdi_init failed\n");
      PM_FATAL(_logFTDI,"ftdi_init failed "<<deviceIdentifier);
      return;
      //_exit(1);
    }
  memset(theName_,0,12);
  memcpy(theName_,deviceIdentifier,8);


  std::stringstream sn("");
  sn<<deviceIdentifier;
  //memcpy(theName_,deviceIdentifier,8);

  if ((ret = ftdi_usb_open_desc(&theFtdi, 0x0403,theProduct_,NULL,sn.str().c_str())) < 0)
    {
      _rc=RC::OPENFAILED;
      fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(&theFtdi));
      PM_FATAL(_logFTDI,"unable to open device "<<deviceIdentifier<<" Name:"<<sn.str().c_str()<<"| Product:"<<std::hex<<theProduct_<<std::dec);
      return;
      //_exit(1);
    }

  memset(theName_,0,12);
  memcpy(theName_,deviceIdentifier,8);
  uint32_t regctrl=0;
	
  //ret=UsbRegisterWrite(2,0x1234567);

  //::usleep(100000);
  // printf("%x write rte \n",ret);
  //getchar();
  //ret=UsbRegisterRead(2,&regctrl);
  ret=ftdi_usb_reset(&theFtdi); 
  printf("Reset %d %s %x \n",ret,deviceIdentifier,productid);
  ftdi_disable_bitbang(&theFtdi); 	
  ftdi_setflowctrl(&theFtdi,SIO_DISABLE_FLOW_CTRL);
  ftdi_set_latency_timer(&theFtdi,2); // ca marchait avec0x200 on remet 2

  ftdi_write_data_set_chunksize (&theFtdi,65535);
  ftdi_read_data_set_chunksize (&theFtdi,65535);
  if (theProduct_==0x6014) ftdi_set_bitmode(&theFtdi,0,0);
  ftdi_usb_purge_buffers(&theFtdi);
  timeOut=10;
  // Register tests

  //  for (uint32_t i=0;i<1;i++)
  // 	{
  // 	ret=UsbRegisterWrite(2,0x1234567);
  //         printf("ret=%d write 0x1234567 \n",ret);
  //         }
  //         usleep(100);
  for (int i=0;i<10;i++)
    {
      ::usleep(100000);
    ret=UsbRegisterRead(1,&regctrl);
    printf(" \t ret=%d read %x \n",ret,regctrl);
    }
  // 	UsbRegisterWrite(2,0xfedcdead);
  //         printf("ret=%d write 0xfedcdead \n",ret);
  // 	//getchar();
  //         usleep(100);

  // 	ret=UsbRegisterRead(2,&regctrl);
  // 	printf(" ret=%d  read %x \n",ret,regctrl);
  // 	ret=UsbRegisterRead(2,&regctrl);
  // 	printf(" ret=%d  read %x \n",ret,regctrl);
	
  //checkReadWrite(0x100,128);
}

void mdcc::FtdiUsbDriver::checkReadWrite(uint32_t start,uint32_t count)  
{
  _rc=mdcc::FtdiUsbDriver::RC::OK;
  int32_t reg=0x1024,ret=0;uint32_t regctrl=0;
  //     printf("single Writing %x ",reg);
  // 			getchar();
  //       ret=UsbRegisterWrite2(2,reg);
  //       printf(" rc= %d ===>",ret);
			
  // 			getchar();
  //       ret=UsbRegisterRead(2,&regctrl);
  //       printf("single  Reading %x rc= %d \n",regctrl,ret);
  // 			getchar();
  bool write=true;
  for (uint32_t ireg=start;ireg<start+count;ireg++)
    {
      if (write)
	{
	  printf("Writing %x ",ireg);
	  ret=UsbRegisterWrite2(2,ireg);
	  printf(" rc= %d ===>",ret);
	  usleep(100);
	}
      if (ireg==start+count-1)
	{
	  ret=UsbRegisterRead(2,&regctrl);
	  printf(" Reading %x %x rc= %d \n",regctrl,ireg,ret);
	  usleep(100);
	  if (regctrl!=ireg)
	    {
	      printf(" Error Reading  \n");
	      std::string errorMessage( "Cannot Read test register from FT245" );
	      PM_FATAL(_logFTDI,errorMessage<<theName_);
	      _rc=mdcc::FtdiUsbDriver::RC::TESTREG;
	    }
	}
      //getchar();
    }
} 
mdcc::FtdiUsbDriver::~FtdiUsbDriver()    

{
  int ret=0;_rc=mdcc::FtdiUsbDriver::RC::OK;
  
  if ((ret = ftdi_usb_close(&theFtdi)) < 0)
    {
      fprintf(stderr, "unable to close ftdi device: %d (%s)\n", ret, ftdi_get_error_string(&theFtdi));
      PM_ERROR(_logFTDI,"unable to close ftdi device: "<<theName_);
      ftdi_deinit(&theFtdi);

      std::stringstream errorMessage;
      errorMessage <<  ftdi_get_error_string(&theFtdi)<< "unable to close ftdi device (ret: "<< ret << ")"<< std::ends;
      _rc=mdcc::FtdiUsbDriver::RC::CLOSE;
      return;

    }

  ftdi_deinit(&theFtdi);



}	

void mdcc::FtdiUsbDriver::FT245Purge( void )
{

  int32_t status;
  if ( ( status =   ftdi_usb_purge_buffers(&theFtdi))!=0) 	
    {
      std::stringstream errorMessage;
      errorMessage << "Could not purge  FT245 FIFOs (status: "<< status << ")"<< std::ends;
      PM_ERROR(_logFTDI,errorMessage.str());
      _rc=mdcc::FtdiUsbDriver::RC::PURGE;;
    }
}	

int32_t mdcc::FtdiUsbDriver::read( unsigned char  *resultPtr )
{
  _rc=mdcc::FtdiUsbDriver::RC::OK;
  uint32_t tbytesread=0;	
  uint32_t tbytestoread=1;

  //if( FT_Read( handleFT245, (void*)resultPtr, tbytestoread, &tbytesread ) != FT_OK ) 
  int32_t ret= 0;
  int32_t ntry=0;
  while (ret==0)
    {
      ret=ftdi_read_data(&theFtdi,resultPtr,tbytestoread); 
      if (ret==0) 
	{
	  ntry++;
	  usleep(1000);
	}
      if (ntry>timeOut) break;
    }
  if( ret==-666)
    {
      char errorMessage [100];
      sprintf (errorMessage,"%s USB device unavailable",__PRETTY_FUNCTION__);
      resultPtr=0;
      _rc=mdcc::FtdiUsbDriver::RC::UNAVAILABLE;
      return ret;
    }
  if( ret<0)
    {
      char errorMessage [100];
      sprintf (errorMessage,"%s usb_bulk_read error = %d",theName_,ret);
      resultPtr=0;
      _rc=mdcc::FtdiUsbDriver::RC::BULKERROR;
      return ret;
    }
  if (ret==0)
    {
      char errorMessage [100];
      sprintf (errorMessage,"Time out after %d trials No data available",ntry);
      resultPtr=0;
      _rc=mdcc::FtdiUsbDriver::RC::TOOMANYTRIALS;
      return ret;

    }
  if (ret != 1) 
    {
      std::string errorMessage( "Timeout occured while reading from FT245" );
#ifdef DEBUG_LOWLEVEL
      std::cout<<errorMessage<<std::endl;
#endif
      resultPtr=0;
      _rc=mdcc::FtdiUsbDriver::RC::TIMEOUT;
      return ret;
    }
  return ret;
}
void mdcc::FtdiUsbDriver::readNb( unsigned char  *resultPtr,int32_t nbbytes )
//throw( LocalHardwareException ) 
{
  _rc=mdcc::FtdiUsbDriver::RC::OK;
  uint32_t tbytesread=0;	

  int32_t ret= 0;
  int32_t ntry=0;
  while (tbytesread!=nbbytes)
    {
      ret=ftdi_read_data(&theFtdi,resultPtr,nbbytes-tbytesread); 
      if (ret==0) 
	{
	  ntry++;
	  usleep(100);
	}
      if (ret>0) {tbytesread+=ret;ntry=1;}
      if (ntry>timeOut) break;
    }
  if( ret==-666)
    {
      char errorMessage [100];
      sprintf (errorMessage,"%s USB device unavailable",__PRETTY_FUNCTION__);
      memset(resultPtr,0,nbbytes);
      _rc=mdcc::FtdiUsbDriver::RC::UNAVAILABLE;
      return;
    }
  if( ret<0)
    {
      char errorMessage [100];
      sprintf (errorMessage," usb_bulk_read error = %d",ret);
      memset(resultPtr,0,nbbytes);
      _rc=mdcc::FtdiUsbDriver::RC::BULKERROR;
      return;
    }
  if (ret==0)
    {
      char errorMessage [100];
      sprintf (errorMessage,"Time out after %d trials No data available",ntry);
      memset(resultPtr,0,nbbytes);
      _rc=mdcc::FtdiUsbDriver::RC::TOOMANYTRIALS;
      return;

    }

  if (tbytesread != (int32_t) nbbytes) 
    {
      memset(resultPtr,0,nbbytes);

      printf ("Error in nbbytes : only %d bytes read for, %d requested\n ",ret,nbbytes);
      std::string errorMessage( "Timeout occured while reading from FT245" );
#ifdef DEBUG_LOWLEVEL
      std::cout<<errorMessage<<std::endl;
#endif
      _rc=mdcc::FtdiUsbDriver::RC::TIMEOUT;
      return;
    }
  return;
}

void mdcc::FtdiUsbDriver::write( unsigned char  data)
//throw( LocalHardwareException ) 
{
  uint32_t tbyteswritten=0;	
  uint32_t tbytestowrite=1;


  int32_t ret=-666;

  ret=ftdi_write_data(&theFtdi,&data,1);
  if( ret==-666)
    {
      char errorMessage [100];
      sprintf (errorMessage,"%s USB device unavailable",__PRETTY_FUNCTION__);
      _rc=mdcc::FtdiUsbDriver::RC::UNAVAILABLE;
    }
  if( ret<0)
    {
      char errorMessage [100];
      sprintf (errorMessage," usb_bulk_read error = %d",ret);
      _rc=mdcc::FtdiUsbDriver::RC::BULKERROR;
    }
  if (ret != 1) 
    {
      std::string errorMessage( "Timeout occured while reading from FT245" );
#ifdef DEBUG_LOWLEVEL
      std::cout<<errorMessage<<std::endl;
#endif
      _rc=mdcc::FtdiUsbDriver::RC::TIMEOUT;
    }
  return;
}

void mdcc::FtdiUsbDriver::writeNb( unsigned char  *cdata, uint32_t nb)
//throw( LocalHardwareException ) 
{
  uint32_t tbyteswritten=0;	
  int32_t tbytestowrite=nb;
  int32_t ret=ftdi_write_data(&theFtdi,cdata,tbytestowrite);
  if( ret==-666)
    {
      char errorMessage [100];
      sprintf (errorMessage,"%s USB device unavailable",__PRETTY_FUNCTION__);
      _rc=mdcc::FtdiUsbDriver::RC::UNAVAILABLE;
    }
  if( ret<0)
    {
      char errorMessage [100];
      sprintf (errorMessage," usb_bulk_read error = %d",ret);
      _rc=mdcc::FtdiUsbDriver::RC::BULKERROR;
    }

  if (ret != nb) 
    {
      std::string errorMessage( "Timeout occured while writing from FT245" );
#ifdef DEBUG_LOWLEVEL
      std::cout<<errorMessage<<std::endl;
#endif
      _rc=mdcc::FtdiUsbDriver::RC::TIMEOUT;
    }
  return;
}

void mdcc::FtdiUsbDriver::readEEPROM( uint32_t address, 	uint32_t *resultPtr )
//throw( LocalHardwareException ) 
{
  _rc=mdcc::FtdiUsbDriver::RC::OK;
	
  uint16_t data;
  int32_t  ret=ftdi_read_eeprom_location (&theFtdi, address, &data);
  *resultPtr=data;

  if( ret!=0)
    {
      std::string errorMessage( "Could not read EEPROM from FT245 BusAdapter" );
      _rc=mdcc::FtdiUsbDriver::RC::EEPROMREAD;
    }
}

void mdcc::FtdiUsbDriver::writeEEPROM( 	uint32_t address,	uint32_t data)
//throw( LocalHardwareException ) 
{
  _rc=mdcc::FtdiUsbDriver::RC::OK;
  int32_t ret= 	ftdi_write_eeprom_location (&theFtdi, address, data);

  if( ret!=0 ) 
    {
      std::string errorMessage( "Could not write EEPROM from FT245 BusAdapter" );
      _rc=mdcc::FtdiUsbDriver::RC::EEPROMWRITE;
    }
}

void mdcc::FtdiUsbDriver::resetEEPROM( 	void)
//throw( LocalHardwareException ) 
{
  _rc=mdcc::FtdiUsbDriver::RC::OK;
  int32_t ret=ftdi_erase_eeprom (&theFtdi);
  if( ret!=0)
    {
      std::string errorMessage( "Could not reset EEPROM from FT245 BusAdapter" );
      _rc=mdcc::FtdiUsbDriver::RC::EEPROMERASE;

    }
}

void mdcc::FtdiUsbDriver::resetBus( ) 
//throw(LocalHardwareException) 
{
  _rc=mdcc::FtdiUsbDriver::RC::OK;
  int32_t ret=ftdi_usb_reset(&theFtdi);
  if (ret!=0)
    {
      _rc=mdcc::FtdiUsbDriver::RC::RESETBUS;
    }
}

void mdcc::FtdiUsbDriver::readStatus( uint32_t *RXQueue, uint32_t *TXQueue, uint32_t *Event) //Dummy 
//throw( LocalHardwareException ) 
{
  //   if( FT_GetStatus( 	handleFT245, RXQueue,TXQueue,Event) != FT_OK ) 
  //     {
  //       std::string errorMessage( "Could not read status from FT245 BusAdapter" );
  //       throw (LocalHardwareException( "FT245" ,errorMessage, __FILE__, __LINE__, __FUNCTION__ ) );    
  //     }
  *RXQueue=1;
}

int32_t FtdiUsbDriver :: UsbReadByte(unsigned char  *tbyte)
//throw (LocalHardwareException)
{

    this->read(tbyte);
    if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
      {
	(*tbyte)=0;
	PM_ERROR(_logFTDI,this->rcMessage());
	return 0;
      }
    return 1;
}	

int32_t FtdiUsbDriver :: UsbRead4Bytes(uint32_t *data)
//throw (LocalHardwareException)
{
  unsigned char  ttampon[5];

  this->readNb(ttampon,4);			

  if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      (*data)=0;
      PM_ERROR(_logFTDI,this->rcMessage());
      return 0;
    }

  (*data)=ttampon[0]<<24;
  (*data)|=ttampon[1]<<16;
  (*data)|=ttampon[2]<<8;
  (*data)|=ttampon[3];

  return 4;
}	

int32_t FtdiUsbDriver :: UsbRead16Bytes(unsigned char  *data)
//throw (LocalHardwareException)
{
  this->readNb(data,16);
  if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      for (int32_t ti=0;ti<16;ti++)
	data[ti]=0;

      PM_ERROR(_logFTDI,this->rcMessage());
      return 0;
    }

  return 16;
}	

int32_t FtdiUsbDriver :: UsbRead22Bytes(unsigned char  *data)

{
  this->readNb(data,22);
  if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      memset(data,0,22);

      PM_ERROR(_logFTDI,this->rcMessage());
      return 0;
    }

  return 22;

}	

int32_t FtdiUsbDriver::UsbReadnBytes(unsigned char  *data, int32_t nbbytes)
{

  this->readNb(data,nbbytes);
  if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      memset(data,0,nbbytes);

      PM_ERROR(_logFTDI,this->rcMessage());
      return 0;
    }

  return nbbytes;

}	


int32_t FtdiUsbDriver :: UsbRead3Bytes(uint32_t *data)
//throw (LocalHardwareException)
{
  unsigned char  ttampon[5];
  this->readNb(ttampon,3);
  if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      memset(data,0,sizeof(uint32_t));
      PM_ERROR(_logFTDI,this->rcMessage());
      return 0;
    }

  (*data)=ttampon[0]<<16;
  (*data)|=ttampon[1]<<8;
  (*data)|=ttampon[2];

  return 3;
}	

int32_t FtdiUsbDriver :: UsbRead2Bytes(uint32_t *data)
//throw (LocalHardwareException)
{
  unsigned char  ttampon[5];
  this->readNb(ttampon,2);
  if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      memset(data,0,sizeof(uint32_t));
      PM_ERROR(_logFTDI,this->rcMessage());
      return 0;
    }

  (*data)=ttampon[0]<<8;
  (*data)|=ttampon[1];
  return 2;

}	

int32_t FtdiUsbDriver :: UsbRegisterRead(uint32_t address, uint32_t *data)
//throw (LocalHardwareException)
{
  uint32_t taddress;
  unsigned char  ttampon[5];
  //	printf ("add avant = %x\n",address);
	
  taddress=address|0x4000;						// read, so bit 14=1
  taddress=taddress&0x7FFF;						// register mode, so bit 15=0, data are 14 LSB
  ttampon[0] = (taddress>>8)&0xFF;
  ttampon[1] = taddress&0xFF;
  //	printf ("ttampon = %02x\n",ttampon[1],ttampon[0]);
  this->writeNb(ttampon,2);		// write address
  

  if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      memset(data,0,sizeof(uint32_t));
      PM_ERROR(_logFTDI,this->rcMessage());
      return -2;
    }

 	
  this->readNb(ttampon,4);
    if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      memset(data,0,sizeof(uint32_t));
      PM_ERROR(_logFTDI,this->rcMessage());
      return -1;
    }
    
  (*data)=ttampon[0]<<24;
  (*data)|=ttampon[1]<<16;
  (*data)|=ttampon[2]<<8;
  (*data)|=ttampon[3];
  return 0;
}	

 
int32_t FtdiUsbDriver :: UsbRegisterWrite(uint32_t address, uint32_t data)
//throw (LocalHardwareException)
{
#undef VRAIMENT
#ifdef VRAIMENT
  uint32_t taddress;
	
  taddress=address&0x3FFF;						// keep only 14 LSB, write, so bit 14=0,register mode, so bit 15=0
  try	{		write((taddress>>8)&0xFF);	}	// write address MSB
  catch (LocalHardwareException& e)
    {
      std::cout<<e.message()<<std::endl;throw (e);
      return -2;
    }
  try	{		write(taddress&0xFF);	}				// write address MSB
  catch (LocalHardwareException& e)
    {
      std::cout<<e.message()<<std::endl;throw (e);
      return -2;
    }
  try	{		write ((data>>24)&0xFF);}			  // write data MSB (31 to 24)
  catch (LocalHardwareException& e)
    {
      std::cout<<e.message()<<std::endl;throw (e);
      return -2;
    }
  try	{		write ((data>>16)&0xFF);}								// write data  bits (23 to 16)
  catch (LocalHardwareException& e)
    {
      std::cout<<e.message()<<std::endl;throw (e);
      return -2;
    }
  try	{		write ((data>>8)&0xFF);}								// write data bits (15 to 8)
  catch (LocalHardwareException& e)
    {
      std::cout<<e.message()<<std::endl;throw (e);
      return -2;
    }
  try	{		write (data&0xFF);		}						// write data LSB (7 to 0)
  catch (LocalHardwareException& e)
    {
      std::cout<<e.message()<<std::endl;throw (e);
      return -2;
    }
  return 0;
#else
  return this->UsbRegisterWrite2(address,data);
#endif  
}	

int32_t FtdiUsbDriver::UsbRegisterWrite2(uint32_t address, uint32_t data)
{
  uint32_t taddress;
  unsigned char  ttampon[7];

  taddress=address&0x3FFF;						// keep only 14 LSB, write, so bit 14=0,register mode, so bit 15=0
  ttampon[0] = (taddress>>8)&0xFF;
  ttampon[1] = taddress&0xFF;
  ttampon[2] = (data>>24)&0xFF;
  ttampon[3] = (data>>16)&0xFF;
  ttampon[4] = (data>>8)&0xFF;
  ttampon[5] = data&0xFF;
  this->writeNb(ttampon,6);		// write address and data all in one
  if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      PM_ERROR(_logFTDI,this->rcMessage());
      return -2;
    }
	
  return 0;
}	

int32_t mdcc::FtdiUsbDriver::UsbGetFirmwareRevision(uint32_t *version) 
{
  uint32_t taddress=0x100;
  uint32_t tdata;
  this->UsbRegisterRead(taddress,&tdata);
  if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      PM_ERROR(_logFTDI,this->rcMessage());
      *version=0;
      return -1;
    }
  *version =tdata;
  return 0;
}

int32_t FtdiUsbDriver :: UsbCommandWrite(uint32_t command)
//throw (LocalHardwareException)
{

  uint32_t taddress;
  unsigned char  ttampon[7];

  taddress=command | 0x8000;						// keep only 14 LSB, write, so bit 14=0,register mode, so bit 15=0
  ttampon[0] = (taddress>>8)&0xFF;
  ttampon[1] = taddress&0xFF;
	
  this->writeNb(ttampon,2);
  if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      PM_ERROR(_logFTDI,this->rcMessage());
      return -2;
    }
  
  return 0;

}

int32_t  FtdiUsbDriver ::FT245GetStatus(int32_t *RXQueue,int32_t *TXQueue ,int32_t *Event)   
{
  *RXQueue=1;
  return 0;
}



int32_t FtdiUsbDriver :: FT245Reset(void)
//throw (LocalHardwareException)
{

  this->resetBus();

  if (_rc!=mdcc::FtdiUsbDriver::RC::OK)
    {
      PM_ERROR(_logFTDI,this->rcMessage());
      return -2;
    }
  return 0;
}	


int32_t mdcc::FtdiUsbDriver::SetTestRegister(int32_t tvalue)   
{
  uint32_t taddress=0x02;
	
 this->UsbRegisterWrite(taddress,tvalue);
 if (_rc!=mdcc::FtdiUsbDriver::RC::OK) return -2;
  return 0;
}	

int32_t mdcc::FtdiUsbDriver::GetTestRegister(uint32_t *tvalue)   
{
  uint32_t taddress=0x02;
	
  this->UsbRegisterRead(taddress,tvalue);
  if (_rc!=mdcc::FtdiUsbDriver::RC::OK) return -2;
  return 0;
}	

std::string mdcc::FtdiUsbDriver::rcMessage()
{
  std::string msg;
  switch (_rc)
    {
    case mdcc::FtdiUsbDriver::RC::OK:
      msg.assign("OK");
      break;
    case mdcc::FtdiUsbDriver::RC::TESTREG:
      msg.assign("Cannot read test register");
      break;
    case mdcc::FtdiUsbDriver::RC::CLOSE:
      msg.assign("Cannot close device");
      break;
    case mdcc::FtdiUsbDriver::RC::PURGE:
      msg.assign("Cannot purge device");
      break;
    case mdcc::FtdiUsbDriver::RC::UNAVAILABLE:
      msg.assign("Device Unavailable");
      break;
    case mdcc::FtdiUsbDriver::RC::BULKERROR:
      msg.assign("USB bulk error");
      break;
    case mdcc::FtdiUsbDriver::RC::TOOMANYTRIALS:
      msg.assign("Too may read trials");
      break;
    case mdcc::FtdiUsbDriver::RC::TIMEOUT:
      msg.assign("Device timeout");
      break;
    case mdcc::FtdiUsbDriver::RC::EEPROMREAD:
       msg.assign("Cannot read EEPROM");
      break;
    case mdcc::FtdiUsbDriver::RC::EEPROMWRITE:
       msg.assign("Cannot write EEPROM");
      break;
    case mdcc::FtdiUsbDriver::RC::EEPROMERASE:
       msg.assign("Cannot erase EEPROM");
      break;
    case mdcc::FtdiUsbDriver::RC::RESETBUS:
       msg.assign("Cannot reset Bus");
      break;
    default:
      msg.assign("Unknown erro code");
    }
  std::stringstream ss;
  ss<<"device: "<<theName_<<" rc="<<msg;
  return ss.str();
}

