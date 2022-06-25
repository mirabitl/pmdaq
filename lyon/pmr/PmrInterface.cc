#include "PmrInterface.hh"
#include <unistd.h>
#include <stdint.h>
pmr::PmrInterface::PmrInterface(pmr::FtdiDeviceInfo* ftd) : _rd(NULL),_state("CREATED"),_dsData(NULL),_detid(150),_external(true)
{
  // Creation of data structure

  memcpy(&_ftd,ftd,sizeof(pmr::FtdiDeviceInfo));

  _status = new pmr::DIFStatus();
  memset(_status,0,sizeof(pmr::DIFStatus));

  sscanf(ftd->name,"FT101%d",&(_status->id));
  _readoutStarted=false;
  _readoutCompleted=true;
  _sem.unlock();

}
void pmr::PmrInterface::setExternalTrigger(bool t) {_external=t;}
void pmr::PmrInterface::setTransport(pm::pmSender* p)
{
  _dsData=p;
  printf("DSDATA is %p\n", _dsData);
}
pmr::PmrInterface::~PmrInterface()
{


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
void pmr::PmrInterface::initialise(pm::pmSender* p)
{
  uint32_t difid=_ftd.id;
  //  create services
  if (p!=NULL) this->setTransport(p);


  try
    {
      PM_INFO(_logPmr,"creating Driver for  "<<difid);
      _rd = new PmrDriver(_ftd.name,_ftd.productid);
      _status->id= _rd->difId();
    }
  catch (...)
    {
      if (_rd!=NULL)
	{
	  delete _rd;
	  _rd=NULL;
	}
      PM_FATAL(_logPmr,"cannot create PmrDriver for  "<<difid);
      this->publishState("INIT_RD_FAILED");
      return;
    }
 
  this->publishState("INITIALISED");
}
 

void pmr::PmrInterface::start()
{

  if (_rd==NULL)
    {
      PM_ERROR(_logPmr, "Pmr   id ("<<_status->id << ") is not initialised");
      this->publishState("START_FAILED");
      return;
    }
  _sem.lock();
  _rd->setAcquisitionMode(true,false,_external);
  _sem.unlock();
  // _rd->setAcquisitionMode(true,true,_external);
  this->publishState("STARTED");
  PM_INFO(_logPmr,"Pmr "<<_status->id<<" is started");
  _status->bytes=0;
  _running=true;
      
  
}
void pmr::PmrInterface::stop()
{
  _running=false;
  if (_rd==NULL)
    {
      PM_ERROR(_logPmr, "Pmr   id ("<<_status->id << ") is not initialised");
      this->publishState("STOP_FAILED");
      return;
    }
  _sem.lock();
  _rd->setAcquisitionMode(false,false,_external);
  _sem.unlock();
  this->publishState("STOPPED");
  
}
void pmr::PmrInterface::setRunning(bool t){_running=t;}
void pmr::PmrInterface::readout()
{
  PM_INFO(_logPmr,"Thread of dif "<<_status->id<<" is started");
  if (_rd==NULL)
    {
      PM_ERROR(_logPmr, "Pmr   id ("<<_status->id << ") is not initialised");
      this->publishState("READOUT_FAILED");
      _readoutStarted=false;
      return;
    }
  _sem.lock();
  _rd->resetFSM();
  _sem.unlock();
  //unsigned char cbuf[64*128*20+8];
  _readoutCompleted=false;
  while (_readoutStarted)
    {
      //printf("On rentre dans la boucle \n");fflush(stdout);
      if (!_running) {usleep((uint32_t) 100000);continue;}
      usleep((uint32_t) 100);
		
		
      //printf("Trying to read \n");fflush(stdout);
      _sem.lock();
      uint32_t nread=_rd->readOneEvent(_cbuf);
      _sem.unlock();
      //printf(" Je lis %d => %d \n",_status->id,nread);
      if (nread==0) continue;
      _sem.lock();
      _rd->resetFSM();
      _sem.unlock();
      //printf(" Je lis %d bytes => %d %x\n",_status->id,nread,_dsData);fflush(stdout);
      //this->publishData(nread);
      
      _status->gtc=PmrGTC(_cbuf);
      //unsigned long long Shift=16777216ULL;//to shift the value from the 24 first bits
      //unsigned long long LBC= ( (_cbuf[Pmr_ABCID_SHIFT]<<16) | (_cbuf[Pmr_ABCID_SHIFT+1]<<8) | (_cbuf[PMR_ABCID_SHIFT+2]))*Shift+( (_cbuf[PMR_ABCID_SHIFT+3]<<16) | (_cbuf[PMR_ABCID_SHIFT+4]<<8) | (_cbuf[PMR_ABCID_SHIFT+5]));
      uint64_t LBC=((uint64_t)_cbuf[PMR_ABCID_SHIFT]<<40) |
	((uint64_t) _cbuf[PMR_ABCID_SHIFT+1]<<32) |
	((uint64_t) _cbuf[PMR_ABCID_SHIFT+2]<<24)|
        ((uint64_t) _cbuf[PMR_ABCID_SHIFT+3]<<16 )|
	((uint64_t) _cbuf[PMR_ABCID_SHIFT+4]<<8)  |
	((uint64_t) _cbuf[PMR_ABCID_SHIFT+5]);

      _status->bcid=LBC;//PmrABCID(_cbuf);
      //fprintf(stderr,"ABCID %lx \n",LBC);
      _status->bytes+=nread;
      _status->published++;
      if (_dsData==NULL) continue;;
      memcpy((unsigned char*) _dsData->payload(),_cbuf,nread);
      //printf(" Je envoie %d => %d  avec %x \n",_status->id,nread,_dsData);fflush(stdout);
      _dsData->publish(_status->bcid,_status->gtc,nread);
      if (_status->gtc%10 ==0)
	PM_INFO(_logPmr,"dif "<<_status->id<<" bcid "<<std::hex<<
		_status->bcid<<std::dec<<
		" gtc "<<_status->gtc<<
		" size"<<nread);
      //fprintf(stderr,"ICI Je publie ABCID  %lx \n",_status->bcid);
      //fprintf(stderr,"ICI Je publie  GTC %d \n",_status->gtc);
      //fprintf(stderr,"ICI Je publie  NREAD %d \n",nread);

      //_rd->resetFSM();
    }
  _readoutCompleted=true;
  PM_INFO(_logPmr,"Thread of dif "<<_status->id<<" is stopped"<<_readoutStarted);
  _status->status=0XFFFF;
}
void pmr::PmrInterface::destroy()
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
      PM_INFO(_logPmr," Deleting dim services ");
      delete _dsData;
      _dsData=NULL;
    }

}

void pmr::PmrInterface::configure(unsigned char* b,uint32_t nb)
{
  _sem.lock();
  //fprintf(stderr,"Debug Interface 1 %d\n",nb);

  _rd->loadSLC(b,nb);
  //fprintf(stderr,"Debug Interface 2\n");
  uint32_t tdata;
  _rd->registerRead(PMR_SLC_STATUS_REG,&tdata);
  //fprintf(stderr,"Debug Interface 3\n");
  _sem.unlock();
  _status->slc=tdata;
  this->publishState("CONFIGURED");
  //fprintf(stderr,"Debug Interface 4");
  PM_INFO(_logPmr, "Pmr   id ("<<_status->id << ") ="<<tdata);
  return;
}


