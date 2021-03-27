#include "DIFInterface.hh"
#include <unistd.h>
#include <stdint.h>
lydaq::DIFInterface::DIFInterface(FtdiDeviceInfo* ftd) : _rd(NULL),_state("CREATED"),_dsData(NULL),_detid(100)
{
  // Creation of data structure
  memcpy(&_ftd,ftd,sizeof(FtdiDeviceInfo));
  _status = new DIFStatus();
  memset(_status,0,sizeof(DIFStatus));
  _status->id=_ftd.id;

   gethostname(_status->host,80);
  _dbdif = new DIFDbInfo();
  _readoutStarted=false;
  _readoutCompleted=true;
}
void lydaq::DIFInterface::setTransport(zdaq::zmSender* p)
{
_dsData=p;
printf("DSDATA is %x\n",_dsData);
}
lydaq::DIFInterface::~DIFInterface()
{

  delete _dbdif;
  if (_dsData!=NULL)
    {
      delete _dsData;
      _dsData=NULL;
    }
  if (_rd!=NULL)
    {
      delete _rd;
      _rd=NULL;
    }
}
void lydaq::DIFInterface::writeRegister(uint32_t adr,uint32_t reg)
{
  LOG4CXX_INFO(_logLdaq,"Writing "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg);
  try 
    {
      _rd->UsbRegisterWrite(adr,reg);
    }
  catch (LocalHardwareException e)
    {
	
      LOG4CXX_ERROR(_logLdaq,"Cannot write register "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);
    }
  LOG4CXX_INFO(_logLdaq,"Wrote "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);
}
void lydaq::DIFInterface::readRegister(uint32_t adr,uint32_t &reg)
{

  LOG4CXX_INFO(_logLdaq,"Reading "<<_status->id<<" ["<<std::hex<<adr<<"]<-");
  try 
    {
      _rd->UsbRegisterRead(adr,&reg);
    }
  catch (LocalHardwareException e)
    {
	
      LOG4CXX_ERROR(_logLdaq,"Cannot read register "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);
    }
  LOG4CXX_INFO(_logLdaq,"Got "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);

}
void lydaq::DIFInterface::start()
{

  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("START_FAILED");
      return;
    }
  try 
    {
      _rd->start();
      this->publishState("STARTED");
      LOG4CXX_INFO(_logLdaq,"DIF "<<_status->id<<" is started");
      _status->bytes=0;
      _running=true;
      
    }
  catch (LocalHardwareException e)
    {
      LOG4CXX_ERROR(_logLdaq,"Start failed "<<_status->id);
      this->publishState("START_FAILED");
    }
  
}
void lydaq::DIFInterface::readout()
{
  LOG4CXX_INFO(_logLdaq,"Thread of dif "<<_status->id<<" is started");
  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("READOUT_FAILED");
      _readoutStarted=false;
      return;
    }


  unsigned char cbuf[MAX_EVENT_SIZE];
  _readoutCompleted=false;
  while (_readoutStarted)
    {
      if (!_running) {usleep((uint32_t) 100000);continue;}
      usleep((uint32_t) 100);
		
		
      try 
	{
	  //printf("Trying to read \n");fflush(stdout);
	  uint32_t nread=_rd->DoHardrocV2ReadoutDigitalData(cbuf);
	  //printf(" Je lis %d => %d \n",_status->id,nread);
	  if (nread==0) continue;
	  //printf(" Je lis %d bytes => %d %x\n",_status->id,nread,_dsData);fflush(stdout);
	  if (_dsData==NULL) continue;;
	  memcpy((unsigned char*) _dsData->payload(),cbuf,nread);
	  //this->publishData(nread);
	 
	  _status->gtc=lydaq::DIFInterface::getBufferDTC(cbuf);
	  _status->bcid=lydaq::DIFInterface::getBufferABCID(cbuf);
	  _status->bytes+=nread;
	  //printf(" Je envoie %d => %d  avec %x \n",_status->id,nread,_dsData);fflush(stdout);
	  _dsData->publish(_status->bcid,_status->gtc,nread);

	  
	}
      catch (LocalHardwareException e)
	{
	  LOG4CXX_ERROR(_logLdaq,"DIF "<<_status->id<<" cannot read events"<<e.what() );

	}
		
    }
  _readoutCompleted=true;
  LOG4CXX_INFO(_logLdaq,"Thread of dif "<<_status->id<<" is stopped"<<_readoutStarted);
  _status->status=0XFFFF;
}
void lydaq::DIFInterface::stop()
{
  _running=false;
  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("STOP_FAILED");
      return;
    }
   try 
     {
       _rd->stop();
       this->publishState("STOPPED");
     }
   catch (LocalHardwareException e)
     {
       this->publishState("STOP_FAILED");
       
       LOG4CXX_ERROR(_logLdaq,"Stop failed "<<_status->id);
     }
}
void lydaq::DIFInterface::destroy()
{
  if (_readoutStarted)
    {
      _readoutStarted=false;
      uint32_t ntry=0;
      while (!_readoutCompleted && ntry<100)
	{usleep((uint32_t) 200000);ntry++;}
    }
  if (_rd!=NULL)
    {
      try
	{
	  delete _rd;
	  _rd=NULL;
	}
      catch (LocalHardwareException& e)
	{
	  LOG4CXX_FATAL(_logLdaq,"Destroy failed for "<<_status->id<<" "<<e.what());

	  this->publishState("DESTROY_FAILED");
	  
	}
      this->publishState("CREATED");
    }
  if (_dsData!=NULL)
    {
      LOG4CXX_INFO(_logLdaq," Deleting dim services ");
      delete _dsData;
      _dsData=NULL;
    }

}
void lydaq::DIFInterface::difConfigure(uint32_t ctrlreg,uint32_t p2pa,uint32_t pa2pd,uint32_t pd2daq,uint32_t daq2dr,uint32_t d2ar)
{
  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("DIF_CONFIGURE_FAILED");
      return;
    }
  
  //_rd->setPowerManagment(0x8c52, 0x3e6,0xd640,0x4e,0x4e);// Start decale de 36000 clock (8b68 a la place de 43 ECAL needs)
  _rd->setPowerManagment(p2pa, pa2pd,pd2daq,daq2dr,d2ar);// Start decale de 36000 clock (8b68 a la place de 43 ECAL needs)
  //_rd->setPowerManagment(0x4e, 0x3e6,0x4e,0x4e,0x4e);// old value
  _rd->setControlRegister(ctrlreg);
  try
    {
      _rd->configureRegisters();
    }
  catch(...)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") cannot write registers");
      this->publishState("DIF_CONFIGURE_FAILED");
      return;
    }
  this->publishState("DIF_CONFIGURED");
  LOG4CXX_INFO(_logLdaq, "DIF   id ("<<_status->id << ") has writen registers");

}
void lydaq::DIFInterface::chipConfigure()
{
  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("CHIP_CONFIGURE_FAILED");
      return;
    }
  if (_dbdif->id !=_status->id)
    {
      LOG4CXX_ERROR(_logLdaq, "DB info for DIF   id ("<<_status->id << ") is not available");
      this->publishState("CHIP_CONFIGURE_FAILED");
      return;
    }
  try
    {
      if (_dbdif->nbasic!=48)
	{
	  _rd->setNumberOfAsics(_dbdif->nbasic);
	  _rd->configureRegisters();
	}
      _status->slc=_rd->configureChips(_dbdif->slow);
    }
  catch(...)
    {
       LOG4CXX_ERROR(_logLdaq, "DB info for DIF   id ("<<_status->id << ") cannot configure chips");
      this->publishState("CHIP_CONFIGURE_FAILED");
      return;
    }
  this->publishState("CHIP_CONFIGURED");
  LOG4CXX_INFO(_logLdaq, "DIF   id ("<<_status->id << ") has programmed ASICs");
		

}
void lydaq::DIFInterface::configure(uint32_t ctrlreg,uint32_t l1,uint32_t l2,uint32_t l3,uint32_t l4,uint32_t l5)
{
  this->difConfigure(ctrlreg,l1,l2,l3,l4,l5);
  if (_state.compare("DIF_CONFIGURED")!=0)
    {
      _status->slc=0;
      return;
    }
  this->chipConfigure();
  if (_state.compare("CHIP_CONFIGURED")!=0)
    {
      return;
    }
  bool bad=false;
  std::stringstream s0;
  s0.str(std::string());
  s0<<"CONFIGURED => ";
  if ((_status->slc&0x0003)==0x01) s0<<"SLC CRC OK       - ";
  else
    { 
      if ((_status->slc&0x0003)==0x02) 
	s0<<"SLC CRC Failed   - ";
      else 
	s0<<"SLC CRC forb  - ";
      bad=true;
    }
  if ((_status->slc&0x000C)==0x04) s0<<"All OK      - ";
  else 
    {
      if ((_status->slc&0x000C)==0x08) 
	s0<<"All Failed  - ";
      else  
	s0<<"All forb - ";
      bad=true;
    }
  if ((_status->slc&0x0030)==0x10) s0<<"L1 OK     - ";
  else 
    {
      if ((_status->slc&0x0030)==0x20) s0<<"L1 Failed - ";
      else s0<<"L1 forb   - ";
      bad=true;
    }
  //std::cout<<s0.str()<<std::endl;
  if (bad)
    LOG4CXX_ERROR(_logLdaq,"Configure failed on "<<_status->id<<s0.str()<<" SLC="<<_status->slc);

  this->publishState(s0.str());
  LOG4CXX_INFO(_logLdaq, "DIF   id ("<<_status->id << ") ="<<s0.str());
}
void lydaq::DIFInterface::initialise(zdaq::zmSender* p)
{
  uint32_t difid=_ftd.id;
  //  create services
  if (p!=NULL) this->setTransport(p);


  try
    {
      std::string s(_ftd.name);
      _rd = new DIFReadout(s,_ftd.productid);
      
      }
  catch (...)
    {
      if (_rd!=NULL)
	{
	  delete _rd;
	  _rd=NULL;
	}
      LOG4CXX_FATAL(_logLdaq,"cannot create DIFReadout for  "<<difid);
      this->publishState("INIT_RD_FAILED");
      return;
    }
  try
    {
      _rd->checkReadWrite(0x1234,100);
    }
  catch (LocalHardwareException& e)
    {
      LOG4CXX_FATAL(_logLdaq," Unable to read USB register (check clock) "<<e.message());
      this->publishState("INIT_FAILED");
      return;
    }
  try
    {
      _rd->checkReadWrite(0x1234,100);
    }
  catch (LocalHardwareException& e)
    {
      LOG4CXX_FATAL(_logLdaq," Second check read write failed "<<e.message());
      this->publishState("INIT_FAILED");
      return;
    }
  this->publishState("INITIALISED");
}
 


uint32_t lydaq::DIFInterface::getBufferDIF(unsigned char* cb,uint32_t idx)
{
  return cb[idx+DIF_ID_SHIFT];
}
uint32_t lydaq::DIFInterface::getBufferDTC(unsigned char* cb,uint32_t idx)
{
  return (cb[idx+DIF_DTC_SHIFT]<<24)+(cb[idx+DIF_DTC_SHIFT+1]<<16)+(cb[idx+DIF_DTC_SHIFT+2]<<8)+cb[idx+DIF_DTC_SHIFT+3];
}
uint32_t lydaq::DIFInterface::getBufferGTC(unsigned char* cb,uint32_t idx)
{
  return (cb[idx+DIF_GTC_SHIFT]<<24)+(cb[idx+DIF_GTC_SHIFT+1]<<16)+(cb[idx+DIF_GTC_SHIFT+2]<<8)+cb[idx+DIF_GTC_SHIFT+3];
}
unsigned long long lydaq::DIFInterface::getBufferABCID(unsigned char* cb,uint32_t idx)
{
  unsigned long long Shift=16777216ULL;//to shift the value from the 24 first bits
  unsigned long long LBC= ( (cb[idx+DIF_BCID_SHIFT]<<16) | (cb[idx+DIF_BCID_SHIFT+1]<<8) | (cb[idx+DIF_BCID_SHIFT+2]))*Shift+( (cb[idx+DIF_BCID_SHIFT+3]<<16) | (cb[idx+DIF_BCID_SHIFT+4]<<8) | (cb[idx+DIF_BCID_SHIFT+5]));
  return LBC;
}
