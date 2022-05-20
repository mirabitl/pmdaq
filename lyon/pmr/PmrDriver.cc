

#include <stdio.h>
#include <ftdi.h>
#include <string.h>

#include "PmrDriver.hh"

using namespace pmr;

pmr::PmrDriver::PmrDriver(char * deviceIdentifier, uint32_t productid ) 
{
  _productId=productid;
  memset(_deviceId,0,12);
  memcpy(_deviceId,deviceIdentifier,8);
  sscanf(deviceIdentifier,"FT101%d",&_difId);
  this->setup();
}

pmr::PmrDriver::~PmrDriver()     
{
  int ret=0;
  
  if ((ret = ftdi_usb_close(&theFtdi)) < 0)
    {
      fprintf(stderr, "unable to close ftdi device: %d (%s)\n", ret, ftdi_get_error_string(&theFtdi));
      PM_ERROR(_logPmr,"unable to close ftdi device: "<<_deviceId);
      ftdi_deinit(&theFtdi);
      
    }

  ftdi_deinit(&theFtdi);



}	


int32_t pmr::PmrDriver::open(char * deviceIdentifier, uint32_t productid ) 
{
  int32_t ret;


  if (ftdi_init(&theFtdi) < 0)
    {
      PM_FATAL(_logPmr,"FTDI_INIT failed");
      return -1;
    }

  if ((ret = ftdi_usb_open_desc(&theFtdi, 0x0403,productid,NULL,deviceIdentifier)) < 0)
    {
      PM_FATAL(_logPmr,"Cannot open ftdi device "<<deviceIdentifier<<" id "<<std::hex<<productid<<std::dec<<" rc:"<<ret);
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

  
  PM_INFO(_logPmr,"Access open to ftdi device "<<deviceIdentifier<<" id "<<std::hex<<productid<<std::dec<<" rc:"<<ret);
  return 0;
}

int32_t pmr::PmrDriver::writeNBytes( unsigned char  *cdata, uint32_t nb)
{
  int32_t tbytestowrite=nb;
  int32_t ret=ftdi_write_data(&theFtdi,cdata,tbytestowrite);
  if( ret==-666)
    {
      PM_ERROR(_logPmr,"USB device unavailable");
    }
  if( ret<0)
    {
      PM_ERROR(_logPmr,"usb_bulk_write error = "<<ret);
    }

  if (ret != nb) 
    {
      PM_ERROR(_logPmr,"Timeout occured while writing on FT245");
    }
  return ret;
}

int32_t   pmr::PmrDriver::readNBytes( unsigned char  *resultPtr,uint32_t nbbytes )
{
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

      PM_ERROR(_logPmr,"USB device unavailable");
    }
  if( ret<0)
    {
      PM_ERROR(_logPmr,"usb_bulk_read error = "<<ret);
    }
  if (ret==0)
    {
      PM_ERROR(_logPmr,"No data after ntry = "<<ntry);
      memset(resultPtr,0,nbbytes);
    }

  if (tbytesread != (int32_t) nbbytes) 
    {
      memset(resultPtr,0,nbbytes);
      PM_ERROR(_logPmr,"Only  "<<ret<<" Bytes found, "<<nbbytes<<" required, buffer cleared");

    }
  return ret;
}

int32_t pmr::PmrDriver::registerWrite(uint32_t address, uint32_t data)
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

int32_t pmr::PmrDriver::registerRead(uint32_t address, uint32_t *data)
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
      PM_ERROR(_logPmr,"write adress error");	
      return -2;
    }

  if (readNBytes(ttampon,4)!=4)
    {
      (*data)=0;
      PM_ERROR(_logPmr,"read error");
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

int32_t pmr::PmrDriver::setup()
{
  int32_t ret;
  uint32_t tdata;
  unsigned char toto;
 

  this->open(_deviceId,_productId);
  //	FtdiOpen("DCCCCC01",0x6001);
  // empty mem
  while (ftdi_read_data(&theFtdi,&toto,1)>0) {printf ("%02x",toto);}

  // Check test Register
  ret=registerWrite(PMR_TEST_REG,0x12345678);
  printf ("test ....reg write, ret=%d\n",ret);
  ret=registerRead(PMR_TEST_REG,&tdata);
  printf ("test ....reg read, ret=%d data = 0x%x\n",ret,tdata);

  if (tdata!=0x12345678)
    PM_ERROR(_logPmr,"Invalid Test register test "<<std::hex<<tdata<<std::dec);
  ret=registerWrite(PMR_TEST_REG,0xABCD1234);
  printf ("test ....reg write, ret=%d\n",ret);
  ret=registerRead(PMR_TEST_REG,&tdata);
  printf ("test ....reg read, ret=%d , data =0x%x\n",ret,tdata);
  if (tdata!=0xABCD1234)
    PM_ERROR(_logPmr,"Invalid Test register test 2 "<<std::hex<<tdata<<std::dec);
  // Unset PowerPulsing

  ret=registerWrite(PMR_ID_REG,_difId);
  this->setPowerPulsing(false);	
  return 0;
}

int32_t pmr::PmrDriver::loadSLC(unsigned char* SLC,uint32_t slc_size)
{
  int32_t ret;
  uint32_t taddr;
  uint32_t tdata;
  int32_t i;
  uint32_t nb_asic=slc_size/109;

  // SLC Size 
  //ret=registerRead(PMR_TEST_REG,&tdata);
  //printf ("LOADSLC ....reg read, ret=%d , data =0x%x\n",ret,tdata);
  tdata = nb_asic; //0x0A;
  ret=registerWrite(PMR_NBASIC_REG, tdata);// SLc size
  printf ("NBASIC reg write(0x%08x at 0x%x), ret=%d\n",tdata, taddr, ret);
  
  tdata = slc_size; //0x0A;
  ret=registerWrite(PMR_SLC_SIZE_REG, tdata);// SLc size
  printf ("SLc size reg write(0x%08x at 0x%x), ret=%d\n",tdata, taddr, ret);

  // SLC Control Enable LOAD
  tdata = 0x01;
  ret=registerWrite(PMR_SLC_CONTROL_REG, tdata);// enable SLc load
  //	printf ("SLc ctrl reg write( 0x%x at 0x%x), ret=%d\n",tdata, taddr,ret);

  //getchar();

  for (i = 0;i<slc_size; i=i+4)
    {
      
      if ((i+3) >= (slc_size))
	tdata = (SLC[i]<<24)+ (SLC[i+1]<<16)+(SLC[i+2]<<8);		
      else if ((i+2)>= (slc_size))
	tdata = (SLC[i]<<24)+ (SLC[i+1]<<16);		
      else 	if ((i+1)>= (slc_size))
	tdata = (SLC[i]<<24);		
      else
	tdata = (SLC[i]<<24)+ (SLC[i+1]<<16)+(SLC[i+2]<<8)+SLC[i+3];		
      ret=registerWrite(PMR_SLC_DATA_REG,tdata);
      if (ret!=0)
	{
	  printf ("0x%02x 0x%02x 0x%02x 0x%02x\n",SLC[i],SLC[i+1],SLC[i+2], SLC[i+3]); 
	  printf ("%d reg write 0x%08x at 0x%x, ret=%d\n",i,tdata,taddr, ret);
	}
      // getchar();
    }

  // SLC Control disable LOAD
  tdata = 0x00;
  ret=registerWrite(PMR_SLC_CONTROL_REG,tdata);// disable SLc load
  //	printf ("disable SLC load( 0x%08x at 0x%x), ret=%d\n",tdata, taddr,ret);

  // SLC Control SLC to ASIC
  tdata = 0x02;
  ret=registerWrite(PMR_SLC_CONTROL_REG, tdata);// start SLC to asic
  //  printf ("start slc load reg write (0x%08x at 0x%x), ret=%d\n",tdata, taddr, ret);
	
  
  // Reset SLC to ASIC
  tdata = 0x00;
  ret=registerWrite(PMR_SLC_CONTROL_REG, tdata);// reset start SLC to asic
  //	printf ("reset start slc load reg write (0x%08x at 0x%x), ret=%d\n",tdata, taddr, ret);

  //}
  usleep(700000);// attendre la fin su slc pour lire status...

  ret=registerRead(PMR_SLC_STATUS_REG,&tdata);
  printf ("slc status read, @%x ret=%d data = 0x%08x\n",PMR_SLC_STATUS_REG,ret,tdata);
  if ((tdata&0x03)==3)
    printf ("           SLC OK\n");
  else
    printf ("           SLC fail\n");
  //ret=registerRead(PMR_TEST_REG,&tdata);
  //printf ("LOADSLC  Apres reg read, ret=%d , data =0x%x\n",ret,tdata);
  return tdata;
}


int32_t pmr::PmrDriver::setPowerPulsing(bool enable,uint32_t an2d,uint32_t d2ac,uint32_t ac2d,uint32_t d2an)
{
  int32_t ret;
  ret=registerWrite(PMR_PP_AN_2_DIG_REG, an2d);
  ret=registerWrite(PMR_PP_DIG_2_ACQ_REG, d2ac);
  ret=registerWrite(PMR_PP_ACQ_2_DIG_REG, ac2d);
  ret=registerWrite(PMR_PP_DIG_2_AN_REG, d2an);

  if (enable)
    ret=registerWrite(PMR_PP_CONTROL_REG, 1);
  else
    ret=registerWrite(PMR_PP_CONTROL_REG, 0);


  return 0;
}



int32_t pmr::PmrDriver::setAcquisitionMode(bool active,bool autoreset,bool external)
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
  ret=registerWrite(PMR_RO_CONTROL_REG, tdata);
  printf ("enable acq reg write (0x%08x at 0x%x), ret=%d\n",tdata, PMR_RO_CONTROL_REG, ret);fflush(stdout);

  return 0;
}

int32_t pmr::PmrDriver::resetFSM()
{
  int32_t ret;
  ret=registerWrite(PMR_RO_RESET_REG, 0x1);
  ::usleep(100000);
  ret=registerWrite(PMR_RO_RESET_REG, 0x0);
  
  //	printf ("reset_FSM write (0x%08x at 0x%x), ret=%d\n",tdata, taddr, ret);

  return 0;
}
int32_t pmr::PmrDriver::readData(unsigned char* tro,uint32_t size)
{
  uint32_t ret=ftdi_read_data(&theFtdi,tro,size);
  return ret;
}

uint32_t pmr::PmrDriver::readOneEvent(unsigned char* cbuf)
{
  int32_t tret=0;
  int32_t header_size=0,idx=0,frame_size=0,trailer=0;
  // Read Header (16 bytes)
  //fprintf(stderr,"On rentre dans readOne\n");
  while (header_size<PMR_HEADER_SIZE)
    {
      tret=ftdi_read_data(&theFtdi,&cbuf[idx],PMR_HEADER_SIZE);
      
      if (tret==0) return 0; // No data on bus
      //fprintf(stderr,"%d tret header %d \n",_difId,tret);
      header_size+=tret;
      idx+=tret;
    }
  // Read Frames
  uint32_t nf=0;
  for (;;)
    {
      // Read on frame or A0
      while(frame_size <PMR_FRAME_SIZE)
	{
	  tret=ftdi_read_data(&theFtdi,&cbuf[idx],PMR_FRAME_SIZE);
	  if (tret!=0 &&nf%10==11)
	    fprintf(stderr," %d %d tret frame %d \n",_difId,nf++,tret);
	  frame_size+=tret;
	  idx+=tret;
	  if ((tret<PMR_FRAME_SIZE) && (cbuf[idx-4]==PMR_EVENT_STOP))
	    {
	      //fprintf(stderr," %d trailer seen \n",_difId);
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
  return idx;
}
