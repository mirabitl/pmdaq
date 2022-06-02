#include  "Genesys.hh"
#include <boost/format.hpp> 
using namespace std;
using namespace genesys;
void genesys::GsDevice::setIos()
{
  struct termios oldtio,newtio;
  char buf[255];
  /* 
     Open modem device for reading and writing and not as controlling tty
     because we don't want to get killed if linenoise sends CTRL-C.
  */
            
  tcgetattr(fd1,&oldtio); /* save current serial port settings */
  bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
        
  /* 
     BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
     CRTSCTS : output hardware flow control (only used if the cable has
     all necessary lines. See sect. 7 of Serial-HOWTO)
     CS8     : 8n1 (8bit,no parity,1 stopbit)
     CLOCAL  : local connection, no modem contol
     CREAD   : enable receiving characters
  */
  newtio.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
	 
  /*
    IGNPAR  : ignore bytes with parity errors
    ICRNL   : map CR to NL (otherwise a CR input on the other computer
    will not terminate input)
    otherwise make device raw (no other input processing)
  */
  newtio.c_iflag = IGNPAR | ICRNL;
         
  /*
    Raw output.
  */
  newtio.c_oflag = 0;
         
  /*
    ICANON  : enable canonical input
    disable all echo functionality, and don't send signals to calling program
  */
  newtio.c_lflag = ICANON;
         
  /* 
     initialize all control characters 
     default values can be found in /usr/include/termios.h, and are given
     in the comments, but we don't need them here
  */
  newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
  newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
  newtio.c_cc[VERASE]   = 0;     /* del */
  newtio.c_cc[VKILL]    = 0;     /* @ */
  newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
  newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
  newtio.c_cc[VSWTC]    = 0;     /* '\0' */
  newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
  newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
  newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
  newtio.c_cc[VEOL]     = 0;     /* '\0' */
  newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
  newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
  newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
  newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
  newtio.c_cc[VEOL2]    = 0;     /* '\0' */
        
  /* 
     now clean the modem line and activate the settings for the port
  */
  tcflush(fd1, TCIOFLUSH);
  tcsetattr(fd1,TCSANOW,&newtio);
         
}

genesys::GsDevice::GsDevice(std::string device,uint32_t address)
{
  fprintf(stderr,"%s device %s address %d \n",__PRETTY_FUNCTION__,device.c_str(),address);
  fd1=open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
  fprintf(stderr,"%s device %s FD1 %d \n",__PRETTY_FUNCTION__,device.c_str(),fd1);
  if (fd1 == -1 )

    {
      std::cout<<"  Unable to open "<<device<<std::endl;
      return;
    }

  else

    {

      fcntl(fd1, F_SETFL,0);
      std::cout<<"  Successfull open of "<<device<<endl;
      //fprintf(stderr,"Port %d has been sucessfully opened and %d is the file description\n",address,fd1);
    }

  portstatus = 0;
#define OLDWAY
#ifdef OLDWAY
  struct termios options;
  // Get the current options for the port...
  tcgetattr(fd1, &options);
  
  // Set the baud rates to 115200...
  cfsetispeed(&options, B9600);
  cfsetospeed(&options, B9600);
  memset(&options,0,sizeof(options));
  
  options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  options.c_cc[VMIN] = 0;      /* block untill n bytes are received */
  options.c_cc[VTIME] = 0;     /* block untill a timer expires (n * 100 mSec.) */
  int error = tcsetattr(fd1, TCSANOW, &options);
  if (error==-1)
    {
      //fprintf(stderr,"Cannot set options \n");
      std::cout<<" Cannot set RS232 options "<<std::endl;
      portstatus=-1;
      return;
    }
  else
    portstatus=1;
  // Enable the receiver and set local mode...

#else
  setIos();
#endif

  char hadr[20];
  memset(hadr,0,20);
  sprintf(hadr,"ADR %2d\r\n",address);
  stringstream s;
  s<<"ADR "<<address<<"\r";

  this->readCommand(s.str());

  // Double first command on linux
  this->readCommand(s.str());
  std::cout<<" Value read "<<_value<<endl;
  // wr=write(fd1,s.str().c_str(),s.str().length());usleep(50000);
  // printf("%d Bytes sent are %d \n",portstatus,wr);
  
  s.str(std::string());
  s<<"RMT 1\r";
  this->readCommand(s.str());
  std::cout<<" Value read "<<_value<<endl;
  // wr=write(fd1,s.str().c_str(),s.str().length());usleep(50000);
  // printf("%d Bytes sent are %d \n",portstatus,wr);

std::cout<<" address set to "<<address<<endl;

}
genesys::GsDevice::~GsDevice()
{
  if (fd1>0)
    close(fd1);
}
void genesys::GsDevice::ON()
{
  readCommand("OUT 1\r");
  ::sleep(1);
  this->INFO();
}
void genesys::GsDevice::OFF()
{
  readCommand("OUT 0\r");
  ::sleep(1);
  this->INFO();
}

void genesys::GsDevice::readCommand(std::string cmd)
{
  if (fd1<0 || portstatus!=1)
    {
      std::cout<<" Device not open "<<std::endl;
      return;
    }
  //fprintf(stderr,"%s %s \n",__PRETTY_FUNCTION__,cmd.c_str());
  //PM_DEBUG(_logGenesys," command "<<cmd);
  memset(buff,0,1024);
  fd_set set;
  struct timeval timeout;
  int rv;

  FD_ZERO(&set); /* clear the set */
  FD_SET(fd1, &set); /* add our file descriptor to the set */

  timeout.tv_sec = 0;
  timeout.tv_usec = 2000;

  rv = select(fd1 + 1, &set, NULL, NULL, &timeout);
  if(rv == -1)
    {
      //perror("select"); /* an error accured */
      std::cout<<" select failed"<<std::endl;
    }
  else if(rv != 0)
    {
      read( fd1, buff, 100 ); /* there was data to read */
    
      //std::cout<<"Y avait "<<buff<<std::endl;
    }
  wr=write(fd1,cmd.c_str(),cmd.length());
  //fflush(fd1);
  //std::cout<<"sleep "<<wr<<std::endl;

  //sleep((int) 1);
  for (int i=0;i<100;i++) usleep(1000);
  memset(buff,0,1024);
  int32_t nchar=0,rd=0,ntry=0;
  while (1)
    {
      FD_ZERO(&set); /* clear the set */
      FD_SET(fd1, &set); /* add our file descriptor to the set */
	
      timeout.tv_sec = 0;
      timeout.tv_usec = 4800;

      //  fprintf(stderr,"waiting for select \n");
      rv = select(fd1 + 1, &set, NULL, NULL, &timeout);
      ntry++;
      if (ntry>100) break;
      if(rv == -1)
	{
	  cout<<" select failed"<<endl;
	  //perror("select"); /* an error accured */
	  //fprintf(stderr,"Bad select %d \n",rv); /* a timeout occured */
	}
      else if(rv == 0)
	{
	  //cout<<" select empty "<<ntry<<endl;
	  //fprintf(stderr,"Nothing in select \n"); /* a timeout occured */
	  break;
	}
      else
	{
	  //fprintf(stderr,"y a des donnees \n");
	  rd=read(fd1,&buff[nchar],100);
	  //printf(" rd = %d nchar %d %s\n",rd,nchar,buff);
	  if (rd>0)
	    nchar+=rd;
	}
      usleep(1);
     
    }

    
  //fprintf(stderr,"nchar %d OOOLLAA %s\n",nchar,buff);
  int istart=0;
  char bufr[100];
  memset(bufr,0,100);

  for (int i=0;i<nchar;i++)
    if (buff[i]<0x5f) {bufr[istart]=buff[i];istart++;}
  //memcpy(bufr,&buff[istart],nchar-istart);
  std::string toto;if (istart>1) toto.assign(bufr,istart-1);
  //fprintf(stderr," %d %d Corrected %s\n",istart,nchar,toto.c_str());
    
  _value=toto;
}
void genesys::GsDevice::INFO()
{
  int ntry=0;
  do
    {
      ::usleep(100000);
      this->readCommand("IDN?\r");
      ntry++;
      if (ntry>10) return;
      //std::cout<<boost::format(" Device %s \n") % _value;
    } while (_value.substr(0,6).compare("LAMBDA")!=0);
  
  std::cout<<" ID: "<<_value<<endl;
  this->readCommand("MODE?\r");
  //std::cout<<boost::format(" Status %s \n") % _value;
  std::cout<<" MODE: "<<_value;
  std::size_t found;
  do {
    this->readCommand("STT?\r");
    //std::cout<<boost::format("Full Status=>\n\t %s \n") % _value;
    std::cout<<" STATUS: "<<_value;
  
    found=_value.find("MV(");
    if (found == std::string::npos) continue;;
    sscanf( _value.substr(found+3,6).c_str(),"%f",&_vRead);
    found=_value.find("PV(");
    if (found == std::string::npos) continue;;
    sscanf(_value.substr(found+3,4).c_str(),"%f",&_vSet);
    found=_value.find("MC(");
    if (found == std::string::npos) continue;;
    sscanf(_value.substr(found+3,6).c_str(),"%f",&_iRead);
    found=_value.find("PC(");
    if (found == std::string::npos) continue;;
    sscanf(_value.substr(found+3,6).c_str(),"%f",&_iSet);
    std::cout<<boost::format("Vset %f Vread %f Iset %f I read %f  \n") % _vSet % _vRead % _iSet % _iRead;
  } while (found==std::string::npos);
  this->readCommand("OUT?\r");
  //std::cout<<boost::format("Output Status=>\n\t %s \n") % _value;
  std::cout<<" Out: "<<_value<<std::endl;
  if (_value.compare("ON")==0)
    _status=1;
  else
    _status=0;
  _lastInfo=time(0);
  /*
    wr=write(fd1,":MDL?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":VOL!;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":VOL?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":CUR?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
  */
}
float genesys::GsDevice::ReadVoltageSet()
{
  if (time(0)-_lastInfo > 5) this->INFO();
  return _vSet;
}
float genesys::GsDevice::ReadVoltageUsed()
{
  if (time(0)-_lastInfo > 5) this->INFO();
  return _vRead;
}
float genesys::GsDevice::ReadCurrentUsed()
{
  if (time(0)-_lastInfo > 5) this->INFO();
  return _iRead;

}
std::string genesys::GsDevice::readValue(){return _value;}

web::json::value genesys::GsDevice::Status()
{
  ::usleep(50000);
  this->INFO();
  web::json::value r;
  r["vset"]=web::json::value::number(_vSet);
  r["vout"]=web::json::value::number(_vRead);
  r["iset"]=web::json::value::number(_iSet);
  r["iout"]=web::json::value::number(_iRead);
  r["status"]=web::json::value::number(_status);
  return r;
}
