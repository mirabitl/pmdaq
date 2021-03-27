#include "interface.hh"
#include <unistd.h>
#include <stdint.h>
dif::interface::interface(FtdiDeviceInfo* ftd) : _rd(NULL),_state("CREATED"),_dsData(NULL),_detid(100)
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
void dif::interface::setTransport(zdaq::zmSender* p)
{
  _dsData=p;
  //printf("DSDATA is %x\n",_dsData);
}
dif::interface::~interface()
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
void dif::interface::writeRegister(uint32_t adr,uint32_t reg)
{
  PM_INFO(_logDif,"Writing "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg);
  _rd->UsbRegisterWrite(adr,reg);

  if(!_rd->isOk())
    {
	
      PM_ERROR(_logDif,"Cannot write register "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);
    }
  PM_INFO(_logDif,"Wrote "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);
}
void dif::interface::readRegister(uint32_t adr,uint32_t &reg)
{

  PM_INFO(_logDif,"Reading "<<_status->id<<" ["<<std::hex<<adr<<"]<-");
  _rd->UsbRegisterRead(adr,&reg);

  if(!_rd->isOk())
    {
	
      PM_ERROR(_logDif,"Cannot read register "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);
    }
  PM_INFO(_logDif,"Got "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);

}
void dif::interface::start()
{

  if (_rd==NULL)
    {
      PM_ERROR(_logDif, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("START_FAILED");
      return;
    }
  _rd->start();
  this->publishState("STARTED");
  PM_INFO(_logDif,"DIF "<<_status->id<<" is started");
  _status->bytes=0;
  _running=true;
      

  if(!_rd->isOk())
    {
      PM_ERROR(_logDif,"Start failed "<<_status->id);
      this->publishState("START_FAILED");
    }
  
}
void dif::interface::readout()
{
  PM_INFO(_logDif,"Thread of dif "<<_status->id<<" is started");
  if (_rd==NULL)
    {
      PM_ERROR(_logDif, "DIF   id ("<<_status->id << ") is not initialised");
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
		
		
      //printf("Trying to read \n");fflush(stdout);
      uint32_t nread=_rd->DoHardrocV2ReadoutDigitalData(cbuf);
      //printf(" Je lis %d => %d \n",_status->id,nread);
      if (nread==0) continue;
      //printf(" Je lis %d bytes => %d %x\n",_status->id,nread,_dsData);fflush(stdout);
      if (_dsData==NULL) continue;;
      memcpy((unsigned char*) _dsData->payload(),cbuf,nread);
      //this->publishData(nread);
	 
      _status->gtc=dif::interface::getBufferDTC(cbuf);
      _status->bcid=dif::interface::getBufferABCID(cbuf);
      _status->bytes+=nread;
      //printf(" Je envoie %d => %d  avec %x \n",_status->id,nread,_dsData);fflush(stdout);
      _dsData->publish(_status->bcid,_status->gtc,nread);

      if(!_rd->isOk())
	PM_ERROR(_logDif,"DIF "<<_status->id<<" cannot read events"<<e.what() );


		
    }
  _readoutCompleted=true;
  PM_INFO(_logDif,"Thread of dif "<<_status->id<<" is stopped"<<_readoutStarted);
  _status->status=0XFFFF;
}
void dif::interface::stop()
{
  _running=false;
  if (_rd==NULL)
    {
      PM_ERROR(_logDif, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("STOP_FAILED");
      return;
    }
  _rd->stop();
  this->publishState("STOPPED");
  if(!_rd->isOk())
    {
      this->publishState("STOP_FAILED");
       
      PM_ERROR(_logDif,"Stop failed "<<_status->id);
    }
}
void dif::interface::destroy()
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
      delete _rd;
      _rd=NULL;
      this->publishState("CREATED");
    }
  if (_dsData!=NULL)
    {
      PM_INFO(_logDif," Deleting dim services ");
      delete _dsData;
      _dsData=NULL;
    }

}
void dif::interface::difConfigure(uint32_t ctrlreg,uint32_t p2pa,uint32_t pa2pd,uint32_t pd2daq,uint32_t daq2dr,uint32_t d2ar)
{
  if (_rd==NULL)
    {
      PM_ERROR(_logDif, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("DIF_CONFIGURE_FAILED");
      return;
    }
  
  //_rd->setPowerManagment(0x8c52, 0x3e6,0xd640,0x4e,0x4e);// Start decale de 36000 clock (8b68 a la place de 43 ECAL needs)
  _rd->setPowerManagment(p2pa, pa2pd,pd2daq,daq2dr,d2ar);// Start decale de 36000 clock (8b68 a la place de 43 ECAL needs)
  //_rd->setPowerManagment(0x4e, 0x3e6,0x4e,0x4e,0x4e);// old value
  _rd->setControlRegister(ctrlreg);
  _rd->configureRegisters();

  if(!_rd->isOk())
    {
      PM_ERROR(_logDif, "DIF   id ("<<_status->id << ") cannot write registers");
      this->publishState("DIF_CONFIGURE_FAILED");
      return;
    }
  this->publishState("DIF_CONFIGURED");
  PM_INFO(_logDif, "DIF   id ("<<_status->id << ") has writen registers");

}
void dif::interface::chipConfigure()
{
  if (_rd==NULL)
    {
      PM_ERROR(_logDif, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("CHIP_CONFIGURE_FAILED");
      return;
    }
  if (_dbdif->id !=_status->id)
    {
      PM_ERROR(_logDif, "DB info for DIF   id ("<<_status->id << ") is not available");
      this->publishState("CHIP_CONFIGURE_FAILED");
      return;
    }
  if (_dbdif->nbasic!=48)
    {
      _rd->setNumberOfAsics(_dbdif->nbasic);
      _rd->configureRegisters();
    }
  _status->slc=_rd->configureChips(_dbdif->slow);
    
  if(!_rd->isOk())
    {
      PM_ERROR(_logDif, "DB info for DIF   id ("<<_status->id << ") cannot configure chips");
      this->publishState("CHIP_CONFIGURE_FAILED");
      return;
    }
  this->publishState("CHIP_CONFIGURED");
  PM_INFO(_logDif, "DIF   id ("<<_status->id << ") has programmed ASICs");
		

}
void dif::interface::configure(uint32_t ctrlreg,uint32_t l1,uint32_t l2,uint32_t l3,uint32_t l4,uint32_t l5)
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
    PM_ERROR(_logDif,"Configure failed on "<<_status->id<<s0.str()<<" SLC="<<_status->slc);

  this->publishState(s0.str());
  PM_INFO(_logDif, "DIF   id ("<<_status->id << ") ="<<s0.str());
}
void dif::interface::initialise(zdaq::zmSender* p)
{
  uint32_t difid=_ftd.id;
  //  create services
  if (p!=NULL) this->setTransport(p);


  std::string s(_ftd.name);
  _rd = new DIFReadout(s,_ftd.productid);
      
  
  _rd->checkReadWrite(0x1234,100);
    
  if (!_rd->isOk())
    {
      PM_FATAL(_logDif," Unable to read USB register (check clock) "<<e.message());
      this->publishState("INIT_FAILED");
      return;
    }
  _rd->checkReadWrite(0x1234,100);
    


  if (!_rd->isOk())
    {
      PM_FATAL(_logDif," Second check read write failed "<<e.message());
      this->publishState("INIT_FAILED");
      return;
    }
  this->publishState("INITIALISED");
}
 


uint32_t dif::interface::getBufferDIF(unsigned char* cb,uint32_t idx)
{
  return cb[idx+DIF_ID_SHIFT];
}
uint32_t dif::interface::getBufferDTC(unsigned char* cb,uint32_t idx)
{
  return (cb[idx+DIF_DTC_SHIFT]<<24)+(cb[idx+DIF_DTC_SHIFT+1]<<16)+(cb[idx+DIF_DTC_SHIFT+2]<<8)+cb[idx+DIF_DTC_SHIFT+3];
}
uint32_t dif::interface::getBufferGTC(unsigned char* cb,uint32_t idx)
{
  return (cb[idx+DIF_GTC_SHIFT]<<24)+(cb[idx+DIF_GTC_SHIFT+1]<<16)+(cb[idx+DIF_GTC_SHIFT+2]<<8)+cb[idx+DIF_GTC_SHIFT+3];
}
unsigned long long dif::interface::getBufferABCID(unsigned char* cb,uint32_t idx)
{
  unsigned long long Shift=16777216ULL;//to shift the value from the 24 first bits
  unsigned long long LBC= ( (cb[idx+DIF_BCID_SHIFT]<<16) | (cb[idx+DIF_BCID_SHIFT+1]<<8) | (cb[idx+DIF_BCID_SHIFT+2]))*Shift+( (cb[idx+DIF_BCID_SHIFT+3]<<16) | (cb[idx+DIF_BCID_SHIFT+4]<<8) | (cb[idx+DIF_BCID_SHIFT+5]));
  return LBC;
}
