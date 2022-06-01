#include "Zup.hh"

#include <string>
#include <iostream>
#include <sstream>
#include <bitset>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <boost/format.hpp>

using namespace zup;
zup::Zup::Zup(std::string device,uint32_t address)
{

  fd1=open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

  if (fd1 == -1 )

    {

      perror("open_port: Unable to open /dev/ttyS0 – ");

    }

  else

    {

      fcntl(fd1, F_SETFL,0);
      printf("Port 1 has been sucessfully opened and %d is the file description\n",fd1);
    }


  portstatus = 0;

  struct termios options;
  // Get the current options for the port...
  tcgetattr(fd1, &options);
  // Set the baud rates to 115200...
  cfsetispeed(&options, B9600);
  cfsetospeed(&options, B9600);
  // Enable the receiver and set local mode...
  options.c_cflag |= (CLOCAL | CREAD);

  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;
  //options.c_cflag |= SerialDataBitsInterp(8);           /* CS8 - Selects 8 data bits */
  options.c_cflag &= ~CRTSCTS;                            // disable hardware flow control
  options.c_iflag &= ~(IXON | IXOFF | IXANY);           // disable XON XOFF (for transmit and receive)
  //options.c_cflag |= CRTSCTS;                     /* enable hardware flow control */


  options.c_cc[VMIN] = 1;     //min carachters to be read
  options.c_cc[VTIME] = 1;    //Time to wait for data (tenths of seconds)


  // Set the new options for the port...
  tcsetattr(fd1, TCSANOW, &options);


  //Set the new options for the port...
  tcflush(fd1, TCIFLUSH);
  if (tcsetattr(fd1, TCSANOW, &options)==-1)
    {
      perror("On tcsetattr:");
      portstatus = -1;
    }
  else
    portstatus = 1;


  char hadr[20];
  memset(hadr,0,20);
  sprintf(hadr,":ADR%.2d;\n",address);
  std::stringstream s;
  s<<":ADR"<<address<<";\r";
  //this->readCommand(s.str());
  
  wr=write(fd1,hadr,7);usleep(50000);
  wr=write(fd1,hadr,7);usleep(50000);

  printf("%d Bytes sent are %d \n",portstatus,wr);
  //this->readCommand(s.str());
  std::cout<<" Value read "<<_value<<std::endl;
}
zup::Zup::~Zup()
{
  if (fd1>0)
    close(fd1);
}
void zup::Zup::readCommand(std::string cmd)
{
  if (fd1<0 || portstatus!=1)
    {
      std::cout<<" Device not open \n";
      return;
    }
  //fprintf(stderr,"%s %s \n",__PRETTY_FUNCTION__,cmd.c_str());
  //PM_DEBUG(_logZup," command "<<cmd);
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
      std::cout<<" select failed\n";
    }
  else if(rv != 0)
    {
      read( fd1, buff, 100 ); /* there was data to read */
    
      std::cout<<"Y avait "<<buff<<std::endl;
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
      if (ntry>10) break;
      if(rv == -1)
	{
	  std::cout<<" select failed\n";
	  //perror("select"); /* an error accured */
	  //fprintf(stderr,"Bad select %d \n",rv); /* a timeout occured */
	}
      else if(rv == 0)
	{
	  std::cout<<" select empty "<<ntry<<std::endl;
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

  //printf(" try %d rd = %d nchar %d %s\n",ntry,rd,nchar,buff);
  memset(&buff[nchar-1],0,1024-(nchar-1));
  buff[nchar-1]=0;
  std::string ret(buff);
  //std::cout<<ret<<std::endl;
  
//   //printf("%s\n",buff);

  //fprintf(stderr,"nchar %d OOOLLAA %s\n",nchar,buff);
  int istart=0;
  char bufr[100];
  memset(bufr,0,100);

  for (int i=0;i<nchar;i++)
    if (buff[i]<0x5f) {bufr[istart]=buff[i];istart++;}
  //memcpy(bufr,&buff[istart],nchar-istart);
  std::string toto;if (istart>1) toto.assign(bufr,istart-1);
  //fprintf(stderr," %d %d Corrected %s\n",istart,nchar,toto.c_str());




  
  _value=ret;
}
void zup::Zup::ON()
{
    
  //wr=write(fd1,":OUT1;",6);usleep(50000);
  readCommand(":OUT1;");
  // ::usleep(50000);
  // this->INFO();
  //printf("Bytes sent are %d \n",wr);
}
void zup::Zup::OFF()
{
  readCommand(":OUT0;");
  // ::usleep(50000);
  // this->INFO();
  //wr=write(fd1,":OUT0;",6);usleep(50000);
  //printf("Bytes sent are %d \n",wr);

}
web::json::value zup::Zup::Status()
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
void zup::Zup::INFO()
{
  std::size_t found;
  ::usleep(50000);
  this->readCommand(":STT?;\r");
  std::cout<<"Status ----> "<<_value<<std::endl;
  found=_value.find("OS");
  if (found != std::string::npos) {
    std::bitset<8> bsta (_value.substr(found+2,8));
    //std::cout<<"Status="<<bsta<<std::endl;
    _status=bsta.to_ulong();
  }
  found=_value.find("SV");
  if (found != std::string::npos) {
    sscanf(_value.substr(found+2,5).c_str(),"%f",&_vSet);
    //std::cout<<"Vset="<<_vSet<<std::endl;

  }
  found=_value.find("SA");
  if (found != std::string::npos) {
    sscanf(_value.substr(found+2,5).c_str(),"%f",&_iSet);
    //std::cout<<"iset="<<_iSet<<std::endl;

  }
  found=_value.find("AV");
  if (found != std::string::npos) {
    sscanf(_value.substr(found+2,5).c_str(),"%f",&_vRead);
    //std::cout<<"VRead="<<_vRead<<std::endl;

  }
  found=_value.find("AA");
  if (found != std::string::npos) {
     sscanf(_value.substr(found+2,5).c_str(),"%f",&_iRead);
     //std::cout<<"iRead="<<_iRead<<std::endl;
  }

  return;

  
  this->readCommand(":MDL?;\r");
  std::cout<<_value<<std::endl;
  ::usleep(50000);
  this->readCommand(":STT?;\r");
  std::cout<<_value<<std::endl;
  found=_value.find("OS");
  if (found != std::string::npos) {
    std::cout<<_value.substr(found+2,8)<<std::endl;
    std::bitset<8> bsta (_value.substr(found+2,8));
    std::cout<<bsta<<std::endl;
    std::cout<<bsta.to_ulong()<<std::endl;
  }
  ::usleep(50000);
  this->readCommand(":VOL!;");
  std::cout<<_value<<std::endl;
  ::usleep(50000);
  this->readCommand(":VOL?;");
  ::usleep(50000);
  std::cout<<_value<<std::endl;
  this->readCommand(":CUR?;");
  std::cout<<_value<<std::endl;
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
float zup::Zup::ReadVoltageSet()
{
  this->INFO();
  return _vSet;
}
float zup::Zup::ReadVoltageUsed()
{
  this->INFO();
  return _vRead;
}
float zup::Zup::ReadCurrentUsed()
{
  this->INFO();
  return _iRead;
}
