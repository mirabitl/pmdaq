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
//#define DEBUG_LOWLEVEL

//#define FLUSH_TO_FILE
//#define FAST_FLUSH_TO_FILE
using namespace dif;
int32_t CHardrocRegisterWrite(struct ftdi_context *ftdic,uint32_t address, uint32_t data)
{
  printf(" FTDI %p \n",ftdic);
  int32_t ret=0;
  uint32_t taddress;
  unsigned char  ttampon[7];

  taddress=address&0x3FFF;						// keep only 14 LSB, write, so bit 14=0,register mode, so bit 15=0
  ttampon[0] = (taddress>>8)&0xFF;
  ttampon[1] = taddress&0xFF;
  ttampon[2] = (data>>24)&0xFF;
  ttampon[3] = (data>>16)&0xFF;
  ttampon[4] = (data>>8)&0xFF;
  ttampon[5] = data&0xFF;

  ret=ftdi_write_data(ftdic, ttampon,6);

  return ret;
}	

dif::driver::driver(char * deviceIdentifier,uint32_t productid )    : mdcc::FtdiUsbDriver(deviceIdentifier,productid) 
{


}


int32_t dif::driver::NbAsicsWrite(uint32_t tnumber,uint32_t l1,uint32_t l2,uint32_t l3,uint32_t l4 )    //throw (LocalHardwareException)
{
  uint32_t taddress=0x05;
  //printf ("nb of asics = %d\n",tnumber);
	
  tnumber += (l1<<8) + (l2<<14) + (l3<<20)+ (l4<<26);
  //	printf ("tnumber = %d\n",tnumber);
  UsbRegisterWrite2(taddress,tnumber);			
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -2; }
  return 0;
}	

int32_t dif::driver::UsbSetDIFID(uint32_t tnumber)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x01;
  UsbRegisterWrite(taddress,tnumber);			
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -2;}
  return 0;
}	

int32_t dif::driver::GetDIFID(uint32_t *tnumber)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x01;
  UsbRegisterRead(taddress,tnumber);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocSetGeneratorDivision(uint32_t tnumber)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x200;
  UsbRegisterWrite(taddress,tnumber);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::NbAsicsRead(uint32_t *tnumber)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x05;

  UsbRegisterRead(taddress,tnumber);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	



int32_t dif::driver::HardrocPwonDacDelayRead(uint32_t *tnumber)    
{
  uint32_t taddress=0x41;

  UsbRegisterRead(taddress,tnumber);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocPwonDacDelayWrite(uint32_t tnumber)    
{
  uint32_t taddress=0x41;
  UsbRegisterWrite(taddress,tnumber);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocPwonAEndDelayRead(uint32_t *tnumber)    
{
  uint32_t taddress=0x40;

  UsbRegisterRead(taddress,tnumber);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocPwonAEndDelayWrite(uint32_t tnumber)    
{
  uint32_t taddress=0x40;
  UsbRegisterWrite(taddress,tnumber);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocSLCStatusRead(uint32_t *tstatus)    
{
  uint32_t taddress=0x06;
  UsbRegisterRead(taddress,tstatus);

  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;

}	

int32_t dif::driver::HardrocSLCCRCStatusRead(void)    
{
  uint32_t taddress=0x06;
  uint32_t tdata;
  UsbRegisterRead(taddress,&tdata);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  if ((tdata&0x03)==0x01) // OK
    return 0;
  else
    return -1;	
}	

int32_t dif::driver::HardrocSLCLoadStatusRead(void)    
{
  uint32_t taddress=0x06;
  uint32_t tdata;

  UsbRegisterRead(taddress,&tdata);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  if ((tdata&0x0C)==0x04) // OK
    return 0;
  else
    return -1;	
}	

int32_t dif::driver::DIFMonitoringEnable(int32_t status)   
{
  uint32_t taddress=0x10;					
  uint32_t tstatus;
	
  UsbRegisterRead(taddress,&tstatus);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tstatus =(tstatus&0xFFFE) +(status&0x01);
  UsbRegisterWrite(taddress,tstatus);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFMonitoringSetDIFGain (int32_t gain)   
{
  uint32_t taddress=0x10;					
  uint32_t tstatus;
	
  UsbRegisterRead(taddress,&tstatus);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tstatus =(tstatus&0xFFFD) +((gain&0x01)<<1);
  UsbRegisterWrite(taddress,tstatus);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFMonitoringSetSlabGain(int32_t gain)   
{
  uint32_t taddress=0x10;					
  uint32_t tstatus;
	
  UsbRegisterRead(taddress,&tstatus);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tstatus =(tstatus&0xFFFB) +((gain&0x01)<<2);
  UsbRegisterWrite(taddress,tstatus);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFMonitoringSetSequencer(int32_t status)   
{
  uint32_t taddress=0x10;					
  uint32_t tstatus;
	
  UsbRegisterRead(taddress,&tstatus);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tstatus =(tstatus&0xFFF7) +((status&0x01)<<3);
  UsbRegisterWrite(taddress,tstatus);		
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFMonitoringSetAVDDshdn (int32_t status)   
{
  uint32_t taddress=0x10;					
  uint32_t tstatus;
	
  UsbRegisterRead(taddress,&tstatus);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tstatus =(tstatus&0xFFEF) +((status&0x01)<<4);
  UsbRegisterWrite(taddress,tstatus);		
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFMonitoringSetDVDDshdn (int32_t status)   
{
  uint32_t taddress=0x10;					
  uint32_t tstatus;
	
  UsbRegisterRead(taddress,&tstatus);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tstatus =(tstatus&0xFFDF) +((status&0x01)<<5);
  UsbRegisterWrite(taddress,tstatus);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFMonitoringSetConvertedChannels (int32_t channel)   
{
  uint32_t taddress=0x10;					
  uint32_t tstatus;
	
  UsbRegisterRead(taddress,&tstatus);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tstatus =(tstatus&0xFF3F) +((channel&0x03)<<6);
  UsbRegisterWrite(taddress,tstatus);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFMonitoringGetConfigRegister(uint32_t *status)   
{
  uint32_t taddress=0x10;					
  UsbRegisterRead(taddress,status);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFMonitoringGetTemperature(uint32_t *Temperature)

{
  uint32_t taddress;

  taddress=0x11;
  UsbRegisterRead(taddress,Temperature);
  if (!isOk())
    {
      *Temperature = 0;
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFMonitoringGetDIFCurrent(uint32_t *DIFCurrent)    
{
  uint32_t taddress=0x12;

  UsbRegisterRead(taddress,DIFCurrent);			
  if (!isOk())
    {
      *DIFCurrent = 0;
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFMonitoringGetSlabCurrent(uint32_t *SlabCurrent)    
{
  uint32_t taddress=0x13;

  UsbRegisterRead(taddress,SlabCurrent);			
  if (!isOk())
    {
      *SlabCurrent = 0;
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFMonitoringGetChannel4Monitoring(uint32_t *Ch4Value)    
{
  uint32_t taddress=0x14;

  UsbRegisterRead(taddress,Ch4Value);			
  if (!isOk())
    {
      *Ch4Value = 0;
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocCommandSLCWrite(void)    
{
  uint32_t taddress=0x01;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	
int32_t dif::driver::HardrocCommandSLCWriteLocal(void)    
{
  uint32_t taddress=0x11;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	
int32_t dif::driver::HardrocCommandSLCWriteByte(unsigned char  tbyte)    
{
  write(tbyte);		
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	


int32_t dif::driver::HardrocCommandSLCWriteCRC(unsigned char  *tbyte)    
{
  writeNb(tbyte,2);		
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::CommandSLCWriteSingleSLCFrame(unsigned char  *tbyte,uint32_t n)

{
  writeNb(tbyte,n);		
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}



int32_t dif::driver::HardrocCommandLemoPulse(void)

{
  uint32_t taddress= 0x06;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	


int32_t dif::driver::FT245Reset(void)

{
  resetBus();

  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

// OK HR2 et MR
int32_t dif::driver::FPGAReset(void)

{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tdata =tdata | 0x01; 
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  tdata =tdata & 0xFFFFFFFE;	
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

// OK
int32_t dif::driver::HardrocReset(void)

{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tdata =tdata | 0x02; 
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  tdata =tdata & 0xFFFFFFD;	
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::BCIDReset(void)   
{
  uint32_t taddress=0x03;
  uint32_t tdata;

  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tdata =tdata | 0x04; 
  UsbRegisterWrite(taddress,tdata);
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  tdata =tdata & 0xFFFFFFFB;	
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::SCReset(void)

{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tdata =tdata | 0x08; 
  UsbRegisterWrite(taddress,tdata);
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  tdata =tdata & 0xFFFFFFF7;	
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

// met select = 0 puis sc_sr_rest = 0
// puis sc_sr_reset = 1
// puis select = 1
int32_t dif::driver::SCSRReset(void)

{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  /*	
	for (;;)
	{
	for (int32_t i=0;i<32;i++)
	{
	printf ("i=%d\n",i);
	getchar();
	tdata =1<<i;
	UsbRegisterWrite(taddress,tdata);
	if (!isOk())
	{
	PM_ERROR(_logDif,"Error found");
	return -2;
	}
	tdata =0;
	UsbRegisterWrite(taddress,tdata);
	if (!isOk())
	{
	PM_ERROR(_logDif,"Error found");
	return -2;
	}
	}	
	}
  */	
  tdata =tdata & 0xFFFEFFFF; // select = 0
  UsbRegisterWrite(taddress,tdata);
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  tdata =tdata | 0x08;	// *sc_resetn=0
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  tdata =tdata & 0xFFFFFFF7;	 // *sc_resten = 1
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  tdata =tdata | 0x10000;	 //select = 1
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::SRReset(void)

{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tdata =tdata | 0x10; 
  UsbRegisterWrite(taddress,tdata);
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  tdata =tdata & 0xFFFFFFEF;	
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::SCReportReset(void)

{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tdata =tdata | 0x20; 
  UsbRegisterWrite(taddress,tdata);
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  tdata =tdata & 0xFFFFFFDF;	
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::DIFCptReset(void)

{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  tdata =tdata | 0x2000; 
  UsbRegisterWrite(taddress,tdata);
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -2; }
  tdata =tdata & 0xFFFFDFFF;	
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -2; }
  return 0;
}	

int32_t dif::driver::SetPowerAnalog(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata = ((tdata&0xFFFFFEFF) |(tstatus<<8));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -2; }
  return 0;
}	

int32_t dif::driver::GetPowerAnalog(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1;}
  *tstatus= (tdata >>8)&0x01;
  return 0;
}	


int32_t dif::driver::SetPowerADC(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFFFFFDFF) |(tstatus<<9));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1;}
  return 0;
}	

int32_t dif::driver::GetPowerADC(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1;}
  *tstatus= (tdata >>9)&0x01;
  return 0;
}	
/*cc 3011
  int32_t dif::driver::SetPowerSS(int32_t tstatus)    
  {
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFFFFFBFF) |(tstatus<<10));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
  }	

  int32_t dif::driver::GetPowerSS(int32_t *tstatus)    
  {
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>10)&0x01;
  return 0;
  }	
*/
int32_t dif::driver::SetPowerDigital(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1;}

  tdata =((tdata&0xFFFFF7FF) |(tstatus<<11));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1;}
  return 0;
}	

int32_t dif::driver::GetPowerDigital(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1;}
  *tstatus= (tdata >>11)&0x01;
  return 0;
}	

int32_t dif::driver::SetPowerDAC(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFFFFEFFF) |(tstatus<<12));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetPowerDAC(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>12)&0x01;
  return 0;
}	

int32_t dif::driver::ResetCounter(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata |=(1<<13);
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  tdata &=0xFFFFDFFF;
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::ClearAnalogSR(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata |=(1<<15);
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  tdata &=0xFFFF7FFF;
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::SetSCChoice(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFFFDFFFF) |(tstatus<<17));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetSCChoice(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>17)&0x01;
  return 0;
}	

int32_t dif::driver::SetCalibrationMode(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFFFBFFFF) |(tstatus<<18));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetCalibrationMode(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>18)&0x01;
  return 0;
}	

int32_t dif::driver::SetSetupWithCCC(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFFF7FFFF) |(tstatus<<19));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetSetupWithCCC(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>19)&0x01;
  return 0;
}	

int32_t dif::driver::SetSetupWithDCC(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFFEFFFFF) |(tstatus<<20));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetSetupWithDCC(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>20)&0x01;
  return 0;
}	

int32_t dif::driver::SetAcqTest(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFFDFFFFF) |(tstatus<<21));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetAcqTest(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>21)&0x01;
  return 0;
}	

int32_t dif::driver::Set4VforSC(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFFBFFFFF) |(tstatus<<22));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::Get4VforSC(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>22)&0x01;
  return 0;
}	

int32_t dif::driver::SetMode4VforSC(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFF7FFFFF) |(tstatus<<23));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetMode4VforSC(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>23)&0x01;
  return 0;
}	

int32_t dif::driver::SetModeDCCCCC(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFEFFFFFF) |(tstatus<<24));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetModeDCCCCC(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>24)&0x01;
  return 0;
}	





int32_t dif::driver::SetHold(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFDFFFFFF) |(tstatus<<25));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetHold(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>25)&0x01;
  return 0;
}	

int32_t dif::driver::SetTimeoutDigitalReadout(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xFBFFFFFF) |(tstatus<<26));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetTimeoutDigitalReadout(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>26)&0x01;
  return 0;
}	

int32_t dif::driver::SetPowerPulsing(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xF7FFFFFF) |(tstatus<<27));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetPowerPulsing(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>27)&0x01;
  return 0;
}

int32_t dif::driver::SetRealPowerPulsing(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xEFFFFFFF) |(tstatus<<28));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetRealPowerPulsing(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>28)&0x01;
  return 0;
}

int32_t dif::driver::SetDIFCommandsONOFF(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xDFFFFFFF) |(tstatus<<29));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetDIFCommandsONOFF(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>29)&0x01;
  return 0;
}

int32_t dif::driver::SetDROBtMode(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0xBFFFFFFF) |(tstatus<<30));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetDROBtMode(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>30)&0x01;
  return 0;
}

int32_t dif::driver::SetClockFrequency(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1;}
  tdata =((tdata&0x7FFFFFFF) |(tstatus<<31));
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())  { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::GetClockFrequency(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus= (tdata >>31)&0x01;
  return 0;
}




int32_t dif::driver::SetControlRegister(int32_t tvalue)    
{
  uint32_t taddress=0x03;
	
  UsbRegisterWrite(taddress,tvalue);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::GetControlRegister(uint32_t *tvalue)    
{
  uint32_t taddress=0x03;
	
  UsbRegisterRead(taddress,tvalue);	
  if (!isOk())
    {
      *tvalue=0;
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  return 0;
}	




int32_t dif::driver::HardrocSetPowerPulsing(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  tstatus=tstatus&0x01;
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  if (tstatus ==1)
    tdata =tdata |(tstatus<<27);
  else
    tdata = tdata&(~((!tstatus)<<27)) ;
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  return 0;
}	

int32_t dif::driver::HardrocGetPowerPulsing(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  *tstatus= (tdata >>27)&0x01;
  return 0;
}	

int32_t dif::driver::SetSCClockFrequency(int32_t tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  if (tstatus ==1)
    tdata =tdata |(tstatus<<31);
  else
    tdata = tdata&(~((!tstatus)<<31)) ;
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::GetSCClockFrequency(int32_t *tstatus)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  *tstatus= (tdata >>31)&0x01;
  return 0;
}	


// ************ digital readout ************
int32_t dif::driver ::HardrocFlushDigitalFIFO(void)
{
  return 0;
  unsigned char  tdata;
	
#ifdef FLUSH_TO_FILE
  char dateStr [20];
  time_t tm= time(NULL);
  strftime(dateStr,20,"%d%m%y_%H%M%S",localtime(&tm));
	
  sprintf (FlushFileName,"Results/flush_%s.dat",	dateStr);			// format is "flush_date_time_index_runnumber.dat" stored in results/
  std::cout<<"flushFileName= "<<FlushFileName<<std::endl;
  FlushFile.open(FlushFileName,std::ios_base::out);				

  // get timestamp at global header
  struct timeb tTimestamp;
  char tchaine[30];
  ftime(&tTimestamp);
  sprintf (tchaine, "%08lx%08x",(tTimestamp.time), tTimestamp.millitm);
  FlushFile <<tchaine;

#endif

  uint32_t tbytesread;
  uint32_t tbytestoread=1;

  std::cout<<"Flushing FIFO : "<<std::endl;

  while  (read(&tdata)!=0)
    {
      std::cout.width(2);
      std::cout.fill('0');
      std::cout<<std::hex<<"0x"<<(int)tdata<<" "<<std::flush;
#ifdef FLUSH_TO_FILE
      if (FlushFile.is_open())
	{		
	  sprintf (tchaine, "%02x",tdata);
	  FlushFile <<tchaine;
	}
#endif
    }
#ifdef FLUSH_TO_FILE
  FlushFile.flush();
  FlushFile.close();
#endif
  std::cout<<std::dec<<std::endl<<"Memory flushed"<<std::endl;
  return 0;
}	

int32_t dif::driver ::HardrocFastFlushDigitalFIFO(void)
{
  return 0; //Not implemented
  unsigned char  tdata;
	
#ifdef FAST_FLUSH_TO_FILE
  char dateStr [20];
  time_t tm= time(NULL);
  strftime(dateStr,20,"%d%m%y_%H%M%S",localtime(&tm));
  sprintf (FlushFileName,"Results/flush_%s.dat",	dateStr);			// format is "flush_date_time_index_runnumber.dat" stored in results/
  std::cout<<"flushFileName= "<<FlushFileName<<std::endl;
  FlushFile.open(FlushFileName,std::ios_base::out);				
  struct timeb tTimestamp;
  char tchaine[30];
  ftime(&tTimestamp);
  sprintf (tchaine, "%08lx%08x",(tTimestamp.time), tTimestamp.millitm);
  FlushFile <<tchaine;
#endif

  int32_t RXQueue;
  int32_t TXQueue;
  int32_t Event;
  uint32_t tbytesread;
  uint32_t tbytestoread=1;

  //	std::cout<<"Flushing FIFO : "<<std::endl;

  FT245GetStatus(&RXQueue,&TXQueue,&Event);
  for (int32_t i=0;i<RXQueue;i++)
    {
      read(&tdata);

#ifdef FAST_FLUSH_TO_FILE
      if (FlushFile.is_open())
	{		
	  sprintf (tchaine, "%02x",tdata);
	  FlushFile <<tchaine;
	}
#endif
    }
#ifdef FAST_FLUSH_TO_FILE
  FlushFile.flush();
  FlushFile.close();
#endif
  return 0;
}	

int32_t dif::driver::HardrocStartDigitalAcquisitionCommand(void)

{
  uint32_t taddress=0x02;
	
  UsbCommandWrite(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocStopDigitalAcquisitionCommand(void)

{
  uint32_t taddress=0x23;
	
  UsbCommandWrite(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	


int32_t dif::driver::HardrocStartDigitalReadoutCommand(void)

{
  uint32_t taddress=0x03;
	
  UsbCommandWrite(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocSendRamfullExtCommand(void)

{
  uint32_t taddress=0x21;
	
  UsbCommandWrite(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocSendExternalTriggerCommand(void)

{
  uint32_t taddress=0x22;
	
  UsbCommandWrite(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocSendMezzanine11Command(void)

{
  uint32_t taddress=0x50;
	
  UsbCommandWrite(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

// *********** analog readout ************

int32_t  dif::driver::HardrocSetTimerHoldRegister(int32_t thold)

{
  int32_t taddress=0x20;
  uint32_t tdata;

  tdata=thold&0x0F;
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}

int32_t  dif::driver::HardrocGetTimerHoldRegister(uint32_t *thold)    
{
  int32_t taddress=0x20;

  UsbRegisterRead(taddress,thold);	
  if (!isOk())
    {
      *thold=0;
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  (*thold)=(*thold)&0x0F;
  return 0;
}

int32_t  dif::driver::HardrocStartAnalogAcq(void)    
{
  uint32_t taddress=0x04;
	
  UsbCommandWrite(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}

int32_t  dif::driver::HardrocSoftwareTriggerAnalogAcq(void)    
{
  uint32_t taddress=0x41;
	
  UsbCommandWrite(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}

int32_t dif::driver::HardrocSetNumericalReadoutMode(int32_t tmode)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  if (tmode ==1)
    tdata =tdata |((tmode&0x01)<<6);
  else
    tdata = tdata&(~((!(tmode&0x01))<<6)) ;
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocGetNumericalReadoutMode(int32_t *tmode)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  *tmode=(tdata>>6)&0x01;
  return 0;
}	

int32_t dif::driver::HardrocSetNumericalReadoutStartMode(int32_t tmode)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  if (tmode ==1)
    tdata =tdata |((tmode&0x01)<<7);
  else
    tdata = tdata&(~((!(tmode&0x01))<<7)) ;
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocGetNumericalReadoutStartMode(int32_t *tmode)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      *tmode=0;
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  return 0;
}	

int32_t dif::driver::HardrocSetSCOverVoltageDefault(void)

{
  uint32_t taddress;
  uint32_t tdata;
	
  taddress=0x03;
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tdata = tdata&(~((!0x01)<<22)) ;
  tdata =tdata |((0x01)<<23);
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocGetSCOverVoltage(int32_t *tmode)

{
  uint32_t taddress;
  uint32_t tdata;
	
  taddress=0x03;
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  *tmode=(tdata>>22)&0x02;
  return 0;
}	

int32_t dif::driver::HardrocSetTestAllAsicsDefault(void)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tdata =tdata |((0x01)<<25);
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocGetTestAllAsics(int32_t *tmode)    
{
  uint32_t taddress =0x03;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  *tmode=(tdata>>25)&0x01;
  return 0;
}	

int32_t dif::driver::HardrocSetEnablePowerPulsing(int32_t tmode)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tdata =tdata |((tmode&0x01)<<27);
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocGetEnablePowerPulsing(int32_t *tmode)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  *tmode=(tdata>>27)&0x01;
  return 0;
}	

int32_t dif::driver::HardrocSetEnableTimeoutDigitalReadout(int32_t tmode)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  tdata =tdata |((tmode&0x01)<<26);
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocGetEnableTimeoutDigitalReadout(int32_t *tmode)    
{
  uint32_t taddress=0x03;
  uint32_t tdata;
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  *tmode=(tdata>>22)&0x01;
  return 0;
}	

int32_t dif::driver::HardrocGetStatusRegister(uint32_t *tstatus)    
{
  uint32_t taddress=0x04;
  UsbRegisterRead(taddress,tstatus);	
  if (!isOk()) { PM_ERROR(_logDif,"Error found"); return -1; }
  return 0;
}	

int32_t dif::driver::HardrocGetMemFull(uint32_t *tstatus)    
{
  uint32_t taddress;
	
  taddress=0x04;
  UsbRegisterRead(taddress,tstatus);	
  if (!isOk()) { *tstatus=0; PM_ERROR(_logDif,"Error found"); return -1; }
  *tstatus=(*tstatus)&0x01;
  return 0;
}	

int32_t dif::driver::HardrocGetRamFullCpt(uint32_t *tstatus)    
{
  uint32_t taddress = 0x09;
	
  UsbRegisterRead(taddress,tstatus);	
  if (!isOk())
    {
      *tstatus=0;
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  *tstatus=(*tstatus)&0xFFFFFFFF;
  return 0;
}	

int32_t dif::driver::HardrocSetSCDebugRegister(int32_t tvalue)    
{
  uint32_t taddress=0x19;
	
  UsbRegisterWrite(taddress,tvalue);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocGetSCDebugRegister(uint32_t *tvalue)    
{
  uint32_t taddress=0x19;
	
  UsbRegisterRead(taddress,tvalue);	
  if (!isOk())
    {
      *tvalue=0;
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  *tvalue=(*tvalue)&0xFF;
  return 0;
}	



int32_t dif::driver::SetChipTypeRegister(int32_t tvalue)    
{
  uint32_t taddress=0x0;
	
  UsbRegisterWrite(taddress,tvalue);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::GetChipTypeRegister(uint32_t *tvalue)    
{
  uint32_t taddress=0x0;
	
  UsbRegisterRead(taddress,tvalue);	
  if (!isOk())
    {
      *tvalue=0;
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  *tvalue=(*tvalue);
  return 0;
}	
/*cc 3011
  int32_t dif::driver::SetResetCounter(int32_t tvalue)    
  {
  uint32_t taddress=0x03;
  uint32_t tampon;
	
  UsbRegisterRead(taddress,&tampon);	
  if (!isOk()) {PM_ERROR(_logDif,"Error found"); return -1; }

  tampon&=0xDFFF;
  tampon |=tvalue<<13;
  UsbRegisterWrite(taddress,tampon);	
  if (!isOk()) {PM_ERROR(_logDif,"Error found"); return -2; }
  return 0;
  }	

  int32_t dif::driver::GetResetCounter(uint32_t *tvalue)    
  {
  uint32_t taddress=0x03;
	
  UsbRegisterRead(taddress,tvalue);	
  if (!isOk())
  {
  *tvalue=0;
  PM_ERROR(_logDif,"Error found");
  return -1;
  }
  return 0;
  }	
*/


int32_t dif::driver::SetPwrToPwrARegister(uint32_t tnumber)    
{
  uint32_t taddress=0x40;
  UsbRegisterWrite(taddress,tnumber);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::SetPwrAToPwrDRegister(uint32_t tnumber)    
{
  uint32_t taddress=0x41;
  UsbRegisterWrite(taddress,tnumber);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	
int32_t dif::driver::SetPwrDToDAQRegister(uint32_t tnumber)    
{
  uint32_t taddress=0x42;
  UsbRegisterWrite(taddress,tnumber);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	
int32_t dif::driver::SetDAQToPwrDRegister(uint32_t tnumber)    
{
  uint32_t taddress=0x43;
  UsbRegisterWrite(taddress,tnumber);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	
int32_t dif::driver::SetPwrDToPwrARegister(uint32_t tnumber)    
{
  uint32_t taddress=0x44;
  UsbRegisterWrite(taddress,tnumber);			
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::HardrocCommandAskDifTemperature(void)    
{
  uint32_t taddress=0x09;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	
int32_t dif::driver::HardrocCommandAskAsuTemperature(void)    
{
  uint32_t taddress=0x08;
	
  UsbCommandWrite	(taddress);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::GetDIFTemperature(uint32_t *tvalue)    				 	
{
  uint32_t taddress=0x11;
  *tvalue=0;	
	
  UsbRegisterRead(taddress,tvalue);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  return 0;	
}

int32_t dif::driver::SetTemperatureReadoutToAuto(uint32_t tvalue)    				 	
{
  uint32_t taddress=0x10;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;		
    }
  tdata &=~1;
  tdata |=(tvalue&0x01);	
  tdata |=(tvalue&0x08);	
	
  tdata = 0xF9; // test selon guillamue 02/04/12
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;		
    }
  return 0;	
}
int32_t dif::driver::GetTemperatureReadoutAutoStatus(uint32_t *tvalue)    				 	
{
  uint32_t taddress=0x10;
  uint32_t tdata;
	
  UsbRegisterRead(taddress,&tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;		
    }
  *tvalue=(tdata &0x01);
  return 0;	
}

int32_t dif::driver::GetASUTemperature(uint32_t *tvalue1,uint32_t *tvalue2)    						
{
  uint32_t taddress=0x52;
  *tvalue1=0;	
  *tvalue2=0;	
  UsbRegisterRead(taddress,tvalue1);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  taddress=0x53;
  UsbRegisterRead(taddress,tvalue2);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -1;
    }
  return 0;
}

int32_t dif::driver::SetEventsBetweenTemperatureReadout(uint32_t tdata)    
{
  uint32_t taddress=0x55;
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::GetEventsBetweenTemperatureReadout(uint32_t *tdata)    
{
  uint32_t taddress=0x55;
  UsbRegisterRead(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

int32_t dif::driver::SetAnalogConfigureRegister(uint32_t tdata)    
{
  uint32_t taddress=0x60;
  UsbRegisterWrite(taddress,tdata);	
  if (!isOk())
    {
      PM_ERROR(_logDif,"Error found");
      return -2;
    }
  return 0;
}	

