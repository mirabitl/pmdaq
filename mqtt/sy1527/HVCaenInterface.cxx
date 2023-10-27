#include "HVCaenInterface.hh"
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //for exit(0);
#include<sys/socket.h>
#include<errno.h> //For errno - the error number
#include<netdb.h> //hostent
#include<arpa/inet.h>
#include "CAENHVWrapper.h"
#include <iostream>
using namespace caen;

caen::HVCaenInterface::HVCaenInterface(std::string host,std::string user,std::string pwd) :theHost_(host),theUser_(user),thePassword_(pwd)
{
  theHandle_=-1;

  theIp_="";
  struct hostent *he;
  struct in_addr **addr_list;
  int i;
  char ip[30];

  if ( (he = gethostbyname( theHost_.c_str() ) ) == NULL) 
    {
      // get the host info
      herror("gethostbyname");
      return ;
    }
 
  addr_list = (struct in_addr **) he->h_addr_list;
     
  for(i = 0; addr_list[i] != NULL; i++) 
    {
      //Return the first one;
      strcpy(ip , inet_ntoa(*addr_list[i]) );
      break;
    }
  theIp_.assign(ip);
  connected_=false;
  this->Connect();
}

caen::HVCaenInterface::~HVCaenInterface()
{
  if (theHandle_!=-1) Disconnect();
}
void caen::HVCaenInterface::Disconnect()
{
  //if (theHandle_==-1) return;

  int ret = CAENHV_DeinitSystem(theHandle_);
  //if( ret == CAENHV_OK )
  //  printf("CAENHV_DeinitSystem: Connection closed (num. %d)\n\n", ret);
  if( ret != CAENHV_OK )
    {
      printf("CAENHV_DeinitSystem: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
      connected_=false;
      theHandle_=-1;
      return;
    }
  else
    printf("CAENHV_DeinitSystem: Connection closed (num. %d)\n\n", ret);
  connected_=false;
  theHandle_=-1;
}


void caen::HVCaenInterface::Connect()
{

  //if (connected_) return;
  // Now connect to the CAEN crate
  CAENHVRESULT ret;
  int sysHndl;
  int sysType=0;
  //sysType=2; //SY4527
  int link=LINKTYPE_TCPIP;
  int ntry=0;
 tryconnect:
  ret = CAENHV_InitSystem((CAENHV_SYSTEM_TYPE_t)sysType, link,(char*) theIp_.c_str(),theUser_.c_str(),thePassword_.c_str(), &sysHndl);

  std::cout<<theIp_<<"<- IP "<<theUser_<<"<-User "<<thePassword_<<"<- Pwd "<<std::endl;
  if( ret == CAENHV_OK )
    {
      theID_=ret;
      theHandle_=sysHndl;
      printf("Connection done %d \n",theHandle_);
      connected_=true;
      return;
    }
  else
    if (ret== CAENHV_DEVALREADYOPEN)
    {
      connected_=true;
      return;
    }
    else
    {
      fprintf(stderr,"\nCAENHV_InitSystem: %s (num. %d) handle %d \n\n", CAENHV_GetError(sysHndl), ret,sysHndl);    
  connected_=false;
  theHandle_=-1;
  ntry++;
  if (ntry>2) return;
  ::sleep(15);
  goto tryconnect;
    }
  
}

void caen::HVCaenInterface::SetOff(uint32_t channel)
{
  if (!isConnected()) return;
  this->SetIntValue("Pw",BoardSlot(channel),BoardChannel(channel),0);return;
  char ParName[16];
  sprintf(ParName,"%s","Pw");
  int param[1];
  param[0] = 0;

 
  uint16_t ChNum=1,ChList[1];
  ChList[0]=BoardChannel(channel);

  CAENHVRESULT ret=CAENHV_SetChParam(theHandle_,BoardSlot(channel),ParName, ChNum, ChList,param);
  //printf("CAENHV_SetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
  if( ret != CAENHV_OK )
    {
       printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      this->Disconnect();
      ::sleep((unsigned int) 2);
      this->Connect();
    }

}
void caen::HVCaenInterface::SetOn(uint32_t channel)
{
  if (!isConnected()) return;
  this->SetIntValue("Pw",BoardSlot(channel),BoardChannel(channel),1);return;
  char ParName[16];
  sprintf(ParName,"%s","Pw");
  int param[1];
  param[0] = 1;

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=BoardChannel(channel);

  CAENHVRESULT ret=CAENHV_SetChParam(theHandle_,BoardSlot(channel),ParName, ChNum, ChList,param);
  //  printf("CAENHV_SetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
  if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      this->Disconnect();
      ::sleep((unsigned int) 2);
      this->Connect();
    }
  
}
void caen::HVCaenInterface::SetCurrent(uint32_t channel,float imax)
{
  if (!isConnected()) return;
  this->SetFloatValue("I0Set",BoardSlot(channel),BoardChannel(channel),imax);return;
  char ParName[16];
  sprintf(ParName,"%s","I0Set");
  float param[1];
  param[0] = imax;

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=BoardChannel(channel);

  CAENHVRESULT ret=CAENHV_SetChParam(theHandle_,BoardSlot(channel),ParName, ChNum, ChList,param);
  //printf("CAENHV_SetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
  if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      this->Disconnect();
      ::sleep((unsigned int) 2);
      this->Connect();
    }

}
void caen::HVCaenInterface::SetVoltage(uint32_t channel,float v0)
{
  if (!isConnected()) return;
  this->SetFloatValue("V0Set",BoardSlot(channel),BoardChannel(channel),v0);return;
  char ParName[16];
  sprintf(ParName,"%s","V0Set");
  float param[1];
  param[0] = v0;

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=BoardChannel(channel);


  //printf("%d %d %d %f \n",channel,slot,ChList[0],v0);
  CAENHVRESULT ret=CAENHV_SetChParam(theHandle_,BoardSlot(channel),ParName, ChNum, ChList,param);
  //printf("CAENHV_SetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
  if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      this->Disconnect();
      ::sleep((unsigned int) 2);
      this->Connect();
    }

}
void caen::HVCaenInterface::SetVoltageRampUp(uint32_t channel,float v0)
{
  if (!isConnected()) return;
  this->SetFloatValue("RUp",BoardSlot(channel),BoardChannel(channel),v0);return;
  char ParName[16];
  sprintf(ParName,"%s","RUp");
  float param[1];
  param[0] = v0;

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=BoardChannel(channel);


  //printf("%d %d %d %f \n",channel,slot,ChList[0],v0);
  CAENHVRESULT ret=CAENHV_SetChParam(theHandle_,BoardSlot(channel),ParName, ChNum, ChList,param);
  //printf("CAENHV_SetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
    if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      this->Disconnect();
      ::sleep((unsigned int) 2);
      this->Connect();
    }

}
float caen::HVCaenInterface::GetCurrentSet(uint32_t channel)
{
  if (!isConnected()) return -1;
  return this->GetFloatValue("I0Set",BoardSlot(channel),BoardChannel(channel));
  char ParName[16];
  sprintf(ParName,"%s","I0Set");
  float param[1];
  

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=BoardChannel(channel);

  CAENHVRESULT ret=CAENHV_GetChParam(theHandle_,BoardSlot(channel),ParName, ChNum, ChList,param);
  //printf("CAENHV_GetChParam: %s (num. %d)\n %f\n", CAENHV_GetError(theHandle_), ret,param[0]) ;
    if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      this->Disconnect();
      ::sleep((unsigned int) 2);
      this->Connect();
    }

  return param[0];
}
std::string caen::HVCaenInterface::GetName(uint32_t channel)
{
  if (!isConnected()) return "SY_DISC";
  return this->GetStringValue("Name",BoardSlot(channel),BoardChannel(channel));
  char ParName[16];
  sprintf(ParName,"%s","Name");
  char param[1][MAX_CH_NAME];
  

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=BoardChannel(channel);

  CAENHVRESULT ret=  CAENHV_GetChName(theHandle_,BoardSlot(channel),ChNum, ChList,param); 
  //printf("CAENHV_GetChParam: %s (num. %d)\n %f\n", CAENHV_GetError(theHandle_), ret,param[0]) ;
    if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      this->Disconnect();
      ::sleep((unsigned int) 2);
      this->Connect();
    }

  return std::string(param[0]);
}
float caen::HVCaenInterface::GetVoltageSet(uint32_t channel)
{
  if (!isConnected()) return -1;
  return this->GetFloatValue("V0Set",BoardSlot(channel),BoardChannel(channel));
  char ParName[16];
  sprintf(ParName,"%s","V0Set");
  float param[1];
  

  uint16_t slot=channel/6; 
  uint16_t ChNum=1,ChList[1];
  ChList[0]=BoardChannel(channel);
  
  CAENHVRESULT ret=CAENHV_GetChParam(theHandle_,BoardSlot(channel),ParName, ChNum, ChList,param);
  //printf("CAENHV_GetChParam: %s (num. %d)\n %f \n", CAENHV_GetError(theHandle_), ret,param[0]) ;
    if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      this->Disconnect();
      ::sleep((unsigned int) 2);
      this->Connect();
    }

  return param[0];
}
float caen::HVCaenInterface::GetCurrentRead(uint32_t channel)
{
  if (!isConnected()) return -1;
  return this->GetFloatValue("IMon",BoardSlot(channel),BoardChannel(channel));
  char ParName[16];
  sprintf(ParName,"%s","IMon");
  float param[1];
  

  uint16_t slot=channel/6;
  slot=(slot+1)*2;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=BoardChannel(channel);

  CAENHVRESULT ret=CAENHV_GetChParam(theHandle_,BoardSlot(channel),ParName, ChNum, ChList,param);
  //printf("CAENHV_GetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret) ;
    if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %x) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      //this->Disconnect();
      //::sleep((unsigned int) 2);
      // this->Connect();
    }

  return param[0];
}
float caen::HVCaenInterface::GetVoltageRead(uint32_t channel)
{
  if (!isConnected()) return -1;
  return this->GetFloatValue("VMon",BoardSlot(channel),BoardChannel(channel));
  char ParName[16];
  sprintf(ParName,"%s","VMon");
  float param[1];
  

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  slot=(slot+1)*2;
  ChList[0]=BoardChannel(channel);

  CAENHVRESULT ret=CAENHV_GetChParam(theHandle_,BoardSlot(channel),ParName, ChNum, ChList,param);
  //printf("CAENHV_GetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret) ;
    if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %x) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      // this->Disconnect();
      //::sleep((unsigned int) 2);
      // this->Connect();
    }

  return param[0];
}
float caen::HVCaenInterface::GetVoltageRampUp(uint32_t channel)
{
  if (!isConnected()) return -1;
  return this->GetFloatValue("RUp",BoardSlot(channel),BoardChannel(channel));
  char ParName[16];
  sprintf(ParName,"%s","RUp");
  float param[1];
  

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=BoardChannel(channel);

  CAENHVRESULT ret=CAENHV_GetChParam(theHandle_,BoardSlot(channel),ParName, ChNum, ChList,param);
  printf("CAENHV_GetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret) ;
    if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      this->Disconnect();
      ::sleep((unsigned int) 2);
      this->Connect();
    }

  return param[0];
}
uint32_t caen::HVCaenInterface::GetStatus(uint32_t channel)
{
  if (!isConnected()) return 9999;
  return this->GetIntValue("Status",BoardSlot(channel),BoardChannel(channel));
}

void caen::HVCaenInterface::SetFloatValue(std::string name,uint32_t slot,uint32_t channel,float val)
{
  
  float param[1];
  param[0] = val;

  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel;


  //printf("%d %d %d %f \n",channel,slot,ChList[0],v0);
  CAENHVRESULT ret=CAENHV_SetChParam(theHandle_,slot,name.c_str(), ChNum, ChList,param);
  //printf("CAENHV_SetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
  if( ret != CAENHV_OK  )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      
    }

}
void caen::HVCaenInterface::SetIntValue(std::string name,uint32_t slot,uint32_t channel,int32_t val)
{
  
  int32_t param[1];
  param[0] = val;

  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel;


  //printf("%d %d %d %f \n",channel,slot,ChList[0],v0);
  CAENHVRESULT ret=CAENHV_SetChParam(theHandle_,slot,name.c_str(), ChNum, ChList,param);
  //printf("CAENHV_SetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
  if( ret != CAENHV_OK  )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
      
    }

}


float caen::HVCaenInterface::GetFloatValue(std::string name,uint32_t slot,uint32_t channel)
{
  float param[1];
  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel;

  CAENHVRESULT ret=CAENHV_GetChParam(theHandle_,slot,name.c_str(), ChNum, ChList,param);
  //printf("CAENHV_GetChParam: %s (num. %d)\n %f %d %d \n", CAENHV_GetError(theHandle_), ret,param[0],slot,channel) ;
  //printf("CAENHV_GetChParam: %s (num. %d)\n %f\n", CAENHV_GetError(theHandle_), ret,param[0]) ;
    if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
    }

  return param[0];
}
int32_t caen::HVCaenInterface::GetIntValue(std::string name,uint32_t slot,uint32_t channel)
{
  int32_t param[1];
  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel;

  CAENHVRESULT ret=CAENHV_GetChParam(theHandle_,slot,name.c_str(), ChNum, ChList,param);
  //printf("CAENHV_GetChParam: %s (num. %d)\n %d %d %d \n", CAENHV_GetError(theHandle_), ret,param[0],slot,channel) ;
    if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
    }

  return param[0];
}

std::string caen::HVCaenInterface::GetStringValue(std::string name,uint32_t slot,uint32_t channel)
{
  char ParName[16];
  sprintf(ParName,"%s",name.c_str());
  char param[1][MAX_CH_NAME];
  


  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel;

  CAENHVRESULT ret=  CAENHV_GetChName(theHandle_,slot,ChNum, ChList,param); 
  //printf("CAENHV_GetChParam: %s (num. %d)\n %f\n", CAENHV_GetError(theHandle_), ret,param[0]) ;
    if( ret != CAENHV_OK )
    {
      printf("\nCAENHV access: %s (num. %d) handle %d \n\n", CAENHV_GetError(theHandle_), ret,theHandle_); 
    
    }

  return std::string(param[0]);
}

web::json::value  caen::HVCaenInterface::ChannelInfo(uint32_t slot,uint32_t channel)
{
  if (isConnected())
    {
      web::json::value rep;
      rep["chname"]=web::json::value::string(U(GetStringValue("Name",slot,channel)));
      rep["vset"]=web::json::value::number(GetFloatValue("V0Set",slot,channel));
      rep["iset"]=web::json::value::number(GetFloatValue("I0Set",slot,channel));
      rep["vout"]=web::json::value::number(GetFloatValue("VMon",slot,channel));
      rep["iout"]=web::json::value::number(GetFloatValue("IMon",slot,channel));
      rep["rampup"]=web::json::value::number(GetFloatValue("RUp",slot,channel));
      rep["status"]=web::json::value::number(GetIntValue("Status",slot,channel));
      return rep;
    }
  else
    {
      web::json::value rep;
      rep["name"]=web::json::value::string(U("SY_DISC"));
      rep["vset"]=web::json::value::number(-1.0);
      rep["vmon"]=web::json::value::number(-1.0);
      rep["imon"]=web::json::value::number(-1.0);
      rep["rup"]=web::json::value::number(-1.0);
      rep["status"]=web::json::value::number(0xFF);
      return rep;

    }
}
