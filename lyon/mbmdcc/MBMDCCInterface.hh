#pragma once

#include "MpiMessageHandler.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include <thread>
#include "stdafx.hh"
#include "utils.hh"
#define MAX_BUFFER_LEN 0x4000
#define MBSIZE 0x40000
/**
x"0001"        ID_register        RO        ID of board
x"0002"        software_veto_register        RW        16 bits pour 16 veto soft differents, veto si =1
x"0003"        spillNb_register        RO        nombre de spills
x"0004"        Control_register        RW        control register
bit 0 : reset spillNb et cpt busy
x"0005"        spillon_register        RW        duree du spillon en clk 40M
x"0006"        spilloff_register        RW        duree du spilloff en clk 40M
x"0007"        channel_enable_register        RW        activation de chaque canal
bit0 channel 1
bit1 channel 2
bit2 channel 3
bit3 channel 4
bit4 channel 5
bit5 channel 6
bit6 channel 7
bit7 channel 8
bit8 channel 9
bit9 channel 10
bit10 channel 11( non utilise)
bit11 channel 12 ( non utilise)
x"0008"        Calib_register        RW        configuration de calibration
bit 0 : active la calibration pour un certain nombre de triggers, n'est plus utilise
bit 1 : active la calibratio pour un certain nombre de fenetres
bit 2 : reloard nombre de fenetre ou de triggers
x"000A"        nb_windows_register        RW        nombre de fenetre pour la calibration
x"000C"        Rstdet_register        RW        reset detecteur
bit 0 : reset envoye aux FE
x"000D"        WindowConfig <file:///Users/combaret/Downloads/tiddlywiki_5.1.14.html#WindowConfig>_register        RW        configuration de type de fenetre generee
bit 0 : strat of spill, end of spill
bit 1 : start of spill et duree programmable
bit 2 : duree et delai programmable
bit 3 : calibration avec spillon/spilloff interne declenché par calib_reg(1),
bit 4 : compteur interne mais seulement sur start_of_spill et duree programmable
bit 5 : compteur interne avec debut comptage sur finbusy
bit 6 : compteur interne avec debut comptage sur finbusy et cptoff apres
x"000E"        TrigExtDelay <file:///Users/combaret/Downloads/tiddlywiki_5.1.14.html#TrigExtDelay>_register        RW        delai du trig ext en clk 40M
x"000F"        TrigExtLength <file:///Users/combaret/Downloads/tiddlywiki_5.1.14.html#TrigExtLength>_register        RW        duree du trig ext en clk 40M
x"0011"        busy1Nb_register        RO        number of busy on ch 1
x"0012"        busy2Nb_register        RO        number of busy on ch 2
x"0013"        busy3Nb_register        RO        number of busy on ch 3
x"0014"        busy4Nb_register        RO        number of busy on ch 4
x"0015"        busy5Nb_register        RO        number of busy on ch 5
x"0016"        busy6Nb_register        RO        number of busy on ch 6
x"0017"        busy7Nb_register        RO        number of busy on ch 7
x"0018"        busy8Nb_register        RO        number of busy on ch 8
x"0019"        busy9Nb_register        RO        number of busy on ch 9
x"001A"        busy10Nb_register        RO        number of busy on ch 10
x"001B"        busy11Nb_register        RO        number of busy on ch 11
x"001C"        busy12Nb_register        RO        number of busy on ch 12
x"0020"        Enable_busy_on_trigger_register        RW        genere un busy quand on recoit un trigger
x"0021"        debounceBusy_register        RW        duree du ebounce des busy (minimum busy length)
x"0030"        TDC_Ctrl_register                Todo
x"0031"        TDC_CoarseCount_register                Todo
x"0032"        TDC_time1_register                Todo
x"0033"        TDC_clkcnt1_register                Todo
x"0034"        TDC_time2_register                Todo
x"0035"        TDC_clkcnt2_register                Todo
x"0036"        TDC_time3_register                Todo
x"0037"        TDC_clkcnt3_register                Todo
x"0038"        TDC_time4_register                Todo
x"0039"        TDC_clkcnt4_register                Todo
x"003a"        TDC_time5_register                Todo
x"003b"        TDC_clkcnt5_register                Todo
x"003c"        TDC_time6_register                Todo
x"003d"        TDC_clkcnt6_register                Todo
x"0040"        TDC_calib1_register                Todo
x"0041"        TDC_calib2_register                Todo
x"0100"        version        RO        x"14060100" ;20-FRM-PT-06-001_00
 */


static LoggerPtr _logMbmdcc(Logger::getLogger("PMDAQ_MBMDCC"));
  namespace mbmdcc
  {
    class board;
    class Message {
    public:
      enum Fmt {HEADER=0,LEN=1,TRANS=3,CMD=4,PAYLOAD=6};
      enum  command { WRITEREG=1,READREG=2,SLC=4,DATA=8,ACKNOWLEDGE=16,ERROR=32};
      enum Register {TEST=0x0,ID=0x1,
		     MASK=0x2,
		     SPILL_CNT=0x3,
		     ACQ_CTRL=0x4,
		     SPILL_ON=0x5,
		     SPILL_OFF=0x6,
		     CHANNEL_ENABLE=0x7,
		     CALIB_CTRL=0x8,
		     CALIB_NWIN=0xA,
		     RESET_FE=0xC,
		     WIN_CTRL=0xD,
		     TRG_EXT_DELAY=0xE,
		     TRG_EXT_LEN=0xF,
		     BUSY_0=0x10,
		     EN_BUSY_TRG=0x20,
		     DEBOUNCE_BUSY=0x21,
		     TDC_CTRL=0x30,
		     TDC_COARSE=0x31,
		     TDC_T1=0x32,TDC_CNT1=0x33,
		     TDC_T2=0x34,TDC_CNT2=0x35,
		     TDC_T3=0x36,TDC_CNT3=0x37,
		     TDC_T4=0x38,TDC_CNT4=0x39,
		     TDC_T5=0x3A,TDC_CNT5=0x3B,
		     TDC_T6=0x3C,TDC_CNT6=0x3D,
		     TDC_CAL1=0x40,TDC_CAL2=0x41,
		     VERSION=0x100};
		     

      Message(): _address(0),_length(2) {;}
      inline uint64_t address(){return _address;}
      inline uint16_t length(){return _length;}
      inline uint8_t* ptr(){return _buf;}
      inline void setLength(uint16_t l){_length=l;}
      inline void setAddress(uint64_t a){_address=a;}
      inline void setAddress(std::string address,uint16_t port){_address=( (uint64_t) utils::convertIP(address)<<32)|port;}
    private:
      uint64_t _address;
      uint16_t _length;
      uint8_t _buf[MAX_BUFFER_LEN];
    
    };


    /// Gere les connections aux socket et le select
    
    class Interface 
    {
    public:
      enum PORT { REGISTER=10002};
      Interface();
      ~Interface(){;}
      void initialise();
      void addDevice(std::string address);
      void listen();

      inline std::map<std::string,mbmdcc::board*>& boards(){ return _boards;}
      inline mbmdcc::board* getBoard(std::string ip) {return _boards[ip];} 
      void close();
      void terminate();
    private:
      void dolisten();
      std::map<std::string,mbmdcc::board*> _boards;

      NL::SocketGroup* _group;
 
      mpi::MpiMessageHandler* _msh;
      mpi::OnRead* _onRead;
      mpi::OnAccept* _onAccept;
      mpi::OnClientDisconnect* _onClientDisconnect;
      mpi::OnDisconnect* _onDisconnect;
      std::thread g_store;
      bool _running;
    };

    // Gere chaque socket
    // processPacket est virtuel
    class socketHandler
    {
    public:
      socketHandler(std::string,uint32_t port);
      int16_t checkBuffer(uint8_t* b,uint32_t maxidx);
      uint32_t sendMessage(mbmdcc::Message* wmsg);
      virtual void processBuffer(uint64_t id, uint16_t l,char* b);
      void purgeBuffer();
      virtual bool processPacket()=0;
      NL::Socket* socket(){return _sock;}
      uint64_t id() {return _id;}
      uint32_t ipid() {return (_id>>32)&0xFFFFFFFF;}
      uint32_t sourceid() {return (ipid()>>16)&0XFFFF;}
      //uint32_t sourceid() {return ipid();}
      inline uint8_t* answer(uint8_t tr){return _answ[tr];}
      uint32_t transaction() {return _transaction;}

      void clear();
    protected:
      uint32_t _idx;
      uint8_t _buf[MBSIZE];
    private:
      uint64_t _id;
      
      NL::Socket* _sock;
      // temporary buffer to collect reply
      uint8_t _b[MBSIZE];
      // Command answers pointers
      std::map<uint8_t,uint8_t*> _answ;
      uint32_t _transaction;

    };
    // Gere la socket registre
    class registerHandler : public socketHandler
    {
    public:
      registerHandler(std::string);
      virtual bool processPacket();
      void writeRegister(uint16_t address,uint32_t value);
      uint32_t readRegister(uint16_t address);
      void processReply(uint32_t tr,uint32_t* rep=0);
      inline void useTransactionId(){_noTransReply=false;}
    private:
      mbmdcc::Message* _msg;
      bool _noTransReply;
    };


    /// Un board => 1 socket
    class board
    {
    public:
      board(std::string ip);
      inline mbmdcc::registerHandler* reg(){return _regh;}
      inline std::string ipAddress(){return _ip;}
    private:
      std::string _ip;
      mbmdcc::registerHandler* _regh;
      
    };

  };
