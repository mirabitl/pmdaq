

#include <stdio.h>
#include <ftdi.h>
#include <string.h>

#include "LiboardDriver.hh"

using namespace liboard;

liboard::LiboardDriver::LiboardDriver(char * deviceIdentifier, uint32_t productid ) 
{
  _productId=productid;
  memset(_deviceId,0,12);
  memcpy(_deviceId,deviceIdentifier,8);
  sscanf(deviceIdentifier,"LI_%d",&_difId);
  this->setup();
  unlock();
}

liboard::LiboardDriver::~LiboardDriver()     
{
  int ret=0;
  
  if ((ret = ftdi_usb_close(&theFtdi)) < 0)
    {
      fprintf(stderr, "unable to close ftdi device: %d (%s)\n", ret, ftdi_get_error_string(&theFtdi));
      PM_ERROR(_logLiboard,"unable to close ftdi device: "<<_deviceId);
      ftdi_deinit(&theFtdi);
      
    }

  ftdi_deinit(&theFtdi);



}	


int32_t liboard::LiboardDriver::open(char * deviceIdentifier, uint32_t productid ) 
{
  int32_t ret;


  if (ftdi_init(&theFtdi) < 0)
    {
      PM_FATAL(_logLiboard,"FTDI_INIT failed");
      return -1;
    }
  ret=ftdi_set_interface(&theFtdi,INTERFACE_A);

  if ((ret = ftdi_usb_open_desc(&theFtdi, 0x0403,productid,NULL,deviceIdentifier)) < 0)
    {
      PM_FATAL(_logLiboard,"Cannot open ftdi device "<<deviceIdentifier<<" id "<<std::hex<<productid<<std::dec<<" rc:"<<ret);
      return -2;   
    }

  ret=ftdi_usb_reset(&theFtdi); 
  printf("Reset %d %s %x \n",ret,deviceIdentifier,productid);
  ftdi_disable_bitbang(&theFtdi); 	
  ftdi_setflowctrl(&theFtdi,SIO_DISABLE_FLOW_CTRL);
  ftdi_set_latency_timer(&theFtdi,1); // ca marchait avec0x200 on remet 2

  ftdi_write_data_set_chunksize (&theFtdi,65535);
  ftdi_read_data_set_chunksize (&theFtdi,65535);
  if (_productId==0x6014) ftdi_set_bitmode(&theFtdi,0,0);
  ftdi_usb_purge_buffers(&theFtdi);

  
  PM_INFO(_logLiboard,"Access open to ftdi device "<<deviceIdentifier<<" id "<<std::hex<<productid<<std::dec<<" rc:"<<ret);
  return 0;
}

int32_t liboard::LiboardDriver::writeNBytes( unsigned char  *cdata, uint32_t nb)
{
  lock();
  int32_t tbytestowrite=nb;
  int32_t ret=ftdi_write_data(&theFtdi,cdata,tbytestowrite);
  if( ret==-666)
    {
      PM_ERROR(_logLiboard,"USB device unavailable");
    }
  if( ret<0)
    {
      PM_ERROR(_logLiboard,"usb_bulk_write error = "<<ret);
    }

  if (ret != nb) 
    {
      PM_ERROR(_logLiboard,"Timeout occured while writing on FT245");
    }
  unlock();
  return ret;
}

int32_t   liboard::LiboardDriver::readNBytes( unsigned char  *resultPtr,uint32_t nbbytes )
{
  lock();
  uint32_t tbytesread=0;	

  int32_t ret= 0;
  int32_t ntry=0;
  int32_t timeOut =10;
	
  int32_t b0;
	
  while (tbytesread!=nbbytes)
    {
      ret=ftdi_read_data(&theFtdi,resultPtr,nbbytes-tbytesread); 
      if (ret==0) 
	{
	  ntry++;
	  usleep(1);
	}
      if (ret>0) {tbytesread+=ret;ntry=1;}
      if (ntry>timeOut) break;
    }
  if( ret==-666)
    {

      PM_ERROR(_logLiboard,"USB device unavailable");
    }
  if( ret<0)
    {
      PM_ERROR(_logLiboard,"usb_bulk_read error = "<<ret);
    }
  if (ret==0)
    {
      PM_ERROR(_logLiboard,"No data after ntry = "<<ntry);
      memset(resultPtr,0,nbbytes);
    }

  if (tbytesread != (int32_t) nbbytes) 
    {
      memset(resultPtr,0,nbbytes);
      PM_ERROR(_logLiboard,"Only  "<<ret<<" Bytes found, "<<nbbytes<<" required, buffer cleared");

    }
  unlock();
  return ret;
}

int32_t liboard::LiboardDriver::registerWrite(uint32_t address, uint32_t data)
{
  uint32_t taddress;
  unsigned char  ttampon[7];

  taddress=address&0x7FFF;						// keep only 15 LSB, write, so bit 15=0
  ttampon[0] = (taddress>>8)&0xFF;
  ttampon[1] = taddress&0xFF;
  ttampon[2] = (data>>24)&0xFF;
  ttampon[3] = (data>>16)&0xFF;
  ttampon[4] = (data>>8)&0xFF;
  ttampon[5] = data&0xFF;
  writeNBytes(ttampon,6);					// write address and data all in one
  return 0;
}	

int32_t liboard::LiboardDriver::registerRead(uint32_t address, uint32_t *data)
{
  uint32_t taddress;
  unsigned char  ttampon[5];
  //	printf ("add avant = %x\n",address);
	
  taddress=address|0x8000;						// read, so bit 15=1
  ttampon[0] = (taddress>>8)&0xFF;
  ttampon[1] = taddress&0xFF;
  if (writeNBytes(ttampon,2) !=2)		// write address 
    {
      (*data)=0;
      PM_ERROR(_logLiboard,"write adress error");	
      return -2;
    }

  if (readNBytes(ttampon,4)!=4)
    {
      (*data)=0;
      PM_ERROR(_logLiboard,"read error");
      return -1;
    }
  //		printf("ttampon[%d]=%02x\n",0,ttampon[0]);
  //		printf("ttampon[%d]=%02x\n",1,ttampon[1]);
  //		printf("ttampon[%d]=%02x\n",2,ttampon[2]);
  //		printf("ttampon[%d]=%02x\n",3,ttampon[3]);
  (*data)=ttampon[0]<<24;
  (*data)|=ttampon[1]<<16;
  (*data)|=ttampon[2]<<8;
  (*data)|=ttampon[3];
  return 0;
}	

void liboard::LiboardDriver::lock() {_bsem.lock();}
void liboard::LiboardDriver::unlock() {_bsem.unlock();}

uint32_t liboard::LiboardDriver::registerRead(uint32_t address)
{
  uint32_t tdata=0;
  this->registerRead(address,&tdata);
  return tdata;
}
int32_t liboard::LiboardDriver::setup()
{
  int32_t ret;
  uint32_t tdata;
  unsigned char toto;
 

  this->open(_deviceId,_productId);
  //	FtdiOpen("DCCCCC01",0x6001);
  // empty mem
  while (ftdi_read_data(&theFtdi,&toto,1)>0) {printf ("%02x",toto);}

  // Check test Register
  ret=registerWrite(LIBOARD_TEST_REG,0x12345678);
  //printf ("test ....reg write, ret=%d\n",ret);
  ret=registerRead(LIBOARD_TEST_REG,&tdata);
  //printf ("test ....reg read, ret=%d data = 0x%x\n",ret,tdata);

  if (tdata!=0x12345678)
    PM_ERROR(_logLiboard,"Invalid Test register test "<<std::hex<<tdata<<std::dec);
  ret=registerWrite(LIBOARD_TEST_REG,0xABCD1234);
  //printf ("test ....reg write, ret=%d\n",ret);
  ret=registerRead(LIBOARD_TEST_REG,&tdata);
  //  printf ("test ....reg read, ret=%d , data =0x%x\n",ret,tdata);
  if (tdata!=0xABCD1234)
    PM_ERROR(_logLiboard,"Invalid Test register test 2 "<<std::hex<<tdata<<std::dec);
  // Unset PowerPulsing

  ret=registerWrite(LIBOARD_ID_REG,_difId);
  return 0;
}

int32_t liboard::LiboardDriver::loadSLC(uint32_t* SLC,uint32_t slc_size)
{
  int32_t ret;
  uint32_t taddr;
  uint32_t tdata;
  int32_t i;
  uint32_t nb_asic=slc_size/139;

  // // SLC Size 

  // tdata = slc_size; //0x0A;
  // ret=registerWrite(LIBOARD_SLC_SIZE_REG, tdata);// SLc size
  // printf ("SLc size reg write(0x%08x at 0x%x), ret=%d\n",tdata, taddr, ret);

  // SLC Control Enable LOAD
  tdata = 0x01;
  ret=registerWrite(LIBOARD_SLC_CONTROL_REG, tdata);// enable SLc load
  //	printf ("SLc ctrl reg write( 0x%x at 0x%x), ret=%d\n",tdata, taddr,ret);

  //getchar();

  for (i = 0;i<slc_size; i++)
    {
      printf ("0x%x\n",SLC[i]);
      tdata=SLC[i];
      ret=registerWrite(LIBOARD_SLC_DATA_REG,tdata);
      printf ("%d reg write 0x%08x at 0x%x, ret=%d\n",i,tdata,taddr, ret);
      // getchar();
    }

  // SLC Control disable LOAD
  tdata = 0x00;
  ret=registerWrite(LIBOARD_SLC_CONTROL_REG,tdata);// disable SLc load
  //	printf ("disable SLC load( 0x%08x at 0x%x), ret=%d\n",tdata, taddr,ret);

  // SLC Control SLC to ASIC
  tdata = 0x02;
  ret=registerWrite(LIBOARD_SLC_CONTROL_REG, tdata);// start SLC to asic
  //  printf ("start slc load reg write (0x%08x at 0x%x), ret=%d\n",tdata, taddr, ret);
	
  
  // Reset SLC to ASIC
  tdata = 0x00;
  ret=registerWrite(LIBOARD_SLC_CONTROL_REG, tdata);// reset start SLC to asic
  //	printf ("reset start slc load reg write (0x%08x at 0x%x), ret=%d\n",tdata, taddr, ret);

  //}
  usleep(500000);// attendre la fin su slc pour lire status...
  
  ret=registerRead(LIBOARD_SLC_STATUS_REG,&tdata);
  printf ("slc status read, ret=%d data = 0x%08x\n",ret,tdata);
  if ((tdata&0x01)==1)
    printf ("           SLC OK\n");
  else
    printf ("           SLC fail\n");

  return tdata;
}





int32_t liboard::LiboardDriver::setAcquisitionMode(bool active,bool autoreset,bool external)
{
  int32_t ret;
  uint32_t tdata;

  tdata=0;
  if (active)
    tdata = 0x01;
  if (!autoreset)
    tdata |=0x2;
  if (external)
    tdata |=0x4;
  ret=registerWrite(LIBOARD_RO_CONTROL_REG, tdata);
  printf ("enable acq reg write (0x%08x at 0x%x), ret=%d\n",tdata, LIBOARD_RO_CONTROL_REG, ret);fflush(stdout);

  return 0;
}

int32_t liboard::LiboardDriver::resetFSM()
{
  int32_t ret;
  ret=registerWrite(LIBOARD_RO_RESET_REG, 0x1);
  ::usleep(100000);
  ret=registerWrite(LIBOARD_RO_RESET_REG, 0x0);
  
  //	printf ("reset_FSM write (0x%08x at 0x%x), ret=%d\n",tdata, taddr, ret);

  return 0;
}
void liboard::LiboardDriver::maskTdcChannels(uint64_t mask)
{
  uint32_t lsb=(uint32_t) (mask&0xFFFFFFFF);
  uint32_t msb=(uint32_t) ((mask>>32)&0xFFFFFFFF);
  int32_t ret;
  ret=registerWrite(LIBOARD_MASK_LSB_REG, lsb);
  ret=registerWrite(LIBOARD_MASK_MSB_REG, msb);
  
}

void liboard::LiboardDriver::setLatchDelay(uint32_t delay)
{int ret=registerWrite(LIBOARD_LATCH_DELAY_REG, delay);}
void liboard::LiboardDriver::setLatchDuration(uint32_t delay)
{int ret=registerWrite(LIBOARD_LATCH_DURATION_REG, delay);}
int32_t liboard::LiboardDriver::readData(unsigned char* tro,uint32_t size)
{
  uint32_t ret=ftdi_read_data(&theFtdi,tro,size);
  return ret;
}

uint32_t liboard::LiboardDriver::readOneEvent(unsigned char* cbuf)
{
  lock();
  int32_t tret=0;
  int32_t header_size=0,idx=0,frame_size=0,trailer=0;
  // Read Header (16 bytes)
  //fprintf(stderr,"On rentre dans readOne\n");
  while (header_size<LIBOARD_HEADER_SIZE)
    {

      tret=ftdi_read_data(&theFtdi,&cbuf[idx],LIBOARD_HEADER_SIZE);
      //fprintf(stderr," tret header %d \n",tret);
      if (tret==0)
	{unlock();return 0;} // No data on bus
      header_size+=tret;
      idx+=tret;
    }
  // for (int i=0;i<LIBOARD_HEADER_SIZE;i++)
  //   fprintf(stderr,"%.2x ",cbuf[i]);
  // fprintf(stderr,"\n")
    ;
  
  // Read Frames
  for (;;)
    {
      // Read on frame or A0
      while(frame_size <LIBOARD_FRAME_SIZE)
	{
	  tret=ftdi_read_data(&theFtdi,&cbuf[idx],LIBOARD_FRAME_SIZE);
	  /*fprintf(stderr," tret frame %d \n",tret);
	   for (int i=idx;i<idx+tret;i++)
	     fprintf(stderr,"%.2x ",cbuf[i]);
	  */
	  frame_size+=tret;
	  idx+=tret;
	  //fprintf(stderr," Stop %.2x \n",cbuf[idx-4]);
	  if ((tret<LIBOARD_FRAME_SIZE) && (cbuf[idx-4]==LIBOARD_EVENT_STOP))
	    {
	      trailer=1;					
	      break;
	    }	
	}
      // Continue to next frame
      if (trailer ==0) 
	{
	  frame_size=0;
	}	
      else	
	{
	  // Exit
	  trailer=0;
	  break;
	}
    }
  unlock();
  return idx;
}

uint32_t liboard::LiboardDriver::version(){return this->registerRead(0x100);}
uint32_t liboard::LiboardDriver::mask(){return this->registerRead(LIBOARD_MDCC_SHIFT+0x2);}
void liboard::LiboardDriver::maskTrigger(){this->registerWrite(LIBOARD_MDCC_SHIFT+0x2,0x1);}
void liboard::LiboardDriver::unmaskTrigger(){this->registerWrite(LIBOARD_MDCC_SHIFT+0x2,0x0);}
uint32_t liboard::LiboardDriver::spillCount(){return this->registerRead(LIBOARD_MDCC_SHIFT+0x3);}
void liboard::LiboardDriver::resetCounter(){this->registerWrite(LIBOARD_MDCC_SHIFT+0x4,0x1);this->registerWrite(LIBOARD_MDCC_SHIFT+0x4,0x0);}
uint32_t liboard::LiboardDriver::spillOn(){return this->registerRead(LIBOARD_MDCC_SHIFT+0x5);}
uint32_t liboard::LiboardDriver::spillOff(){return this->registerRead(LIBOARD_MDCC_SHIFT+0x6);}
void liboard::LiboardDriver::setSpillOn(uint32_t nc){this->registerWrite(LIBOARD_MDCC_SHIFT+0x5,nc);}
void liboard::LiboardDriver::setSpillOff(uint32_t nc){this->registerWrite(LIBOARD_MDCC_SHIFT+0x6,nc);}
uint32_t liboard::LiboardDriver::beam(){return this->registerRead(LIBOARD_MDCC_SHIFT+0x7);}
void liboard::LiboardDriver::setBeam(uint32_t nc){this->registerWrite(LIBOARD_MDCC_SHIFT+0x7,nc);}
void liboard::LiboardDriver::calibOn(){this->registerWrite(LIBOARD_MDCC_SHIFT+0x8,0x2);}
void liboard::LiboardDriver::calibOff(){this->registerWrite(LIBOARD_MDCC_SHIFT+0x8,0x0);}
uint32_t liboard::LiboardDriver::calibCount(){return this->registerRead(LIBOARD_MDCC_SHIFT+0xa);}
void liboard::LiboardDriver::setCalibCount(uint32_t nc){this->registerWrite(LIBOARD_MDCC_SHIFT+0xa,nc);}

void liboard::LiboardDriver::setCalibRegister(uint32_t nc){this->registerWrite(LIBOARD_MDCC_SHIFT+0x8,nc);}

uint32_t liboard::LiboardDriver::hardReset(){return this->registerRead(LIBOARD_MDCC_SHIFT+0xc);}
void liboard::LiboardDriver::setHardReset(uint32_t nc){this->registerWrite(LIBOARD_MDCC_SHIFT+0xc,nc);}

void liboard::LiboardDriver::setSpillRegister(uint32_t nc){this->registerWrite(LIBOARD_MDCC_SHIFT+0xD,nc);}
uint32_t liboard::LiboardDriver::spillRegister(){return this->registerRead(LIBOARD_MDCC_SHIFT+0xD);}
void liboard::LiboardDriver::useSPSSpill(bool t)
{
  uint32_t reg=this->spillRegister();
  if (t)
    this->setSpillRegister(reg|1);
  else
    this->setSpillRegister(reg&~1);
}
void liboard::LiboardDriver::useTrigExt(bool t)
{
  uint32_t reg=this->spillRegister();
  if (t)
    this->setSpillRegister(reg|2);
  else
    this->setSpillRegister(reg&~2);
}

void liboard::LiboardDriver::setTriggerDelay(uint32_t nc){this->registerWrite(LIBOARD_MDCC_SHIFT+0xE,nc);}
uint32_t liboard::LiboardDriver::triggerDelay(){return this->registerRead(LIBOARD_MDCC_SHIFT+0xE);}
void liboard::LiboardDriver::setTriggerBusy(uint32_t nc){this->registerWrite(LIBOARD_MDCC_SHIFT+0xF,nc);}
uint32_t liboard::LiboardDriver::triggerBusy(){return this->registerRead(LIBOARD_MDCC_SHIFT+0xF);}

void liboard::LiboardDriver::setExternalTrigger(uint32_t nc){this->registerWrite(LIBOARD_MDCC_SHIFT+0x1A,nc);}
uint32_t liboard::LiboardDriver::externalTrigger(){return this->registerRead(LIBOARD_MDCC_SHIFT+0x1A);}

void liboard::LiboardDriver::reloadCalibCount(){

  this->maskTrigger();
  this->registerWrite(LIBOARD_MDCC_SHIFT+0xD,0x8);
  this->registerWrite(LIBOARD_MDCC_SHIFT+0x8,0x4);
  // sleep(1);
  // this->registerWrite(LIBOARD_MDCC_SHIFT+0x8,0x0);
  // sleep(1);
  this->unmaskTrigger();
  this->calibOn();


}




uint32_t liboard::LiboardDriver::ecalmask(){return this->registerRead(LIBOARD_MDCC_SHIFT+0xB);}
void liboard::LiboardDriver::maskEcal(){this->registerWrite(LIBOARD_MDCC_SHIFT+0xB,0x1);}
void liboard::LiboardDriver::unmaskEcal(){this->registerWrite(LIBOARD_MDCC_SHIFT+0xB,0x0);}
void liboard::LiboardDriver::resetTDC(uint8_t b){this->registerWrite(LIBOARD_MDCC_SHIFT+0xC,b);}
uint32_t liboard::LiboardDriver::busyCount(uint8_t b){return this->registerRead(LIBOARD_MDCC_SHIFT+0x10+(b&0xF));}


