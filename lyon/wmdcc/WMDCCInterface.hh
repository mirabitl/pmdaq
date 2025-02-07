#pragma once
/**
 * @file WMDCCInterface.hh
 * @author Laurent Mirabito
 * @brief 
 * @version 1.0
 * @date 2024-09-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//#include <netlink/socket.h>
//#include <netlink/socket_group.h>
#include "WizccInterface.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include <thread>
#include "stdafx.hh"
#include "utils.hh"
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
bit 1 : start of spill et duree programmable spillon
bit 2 : duree et delai programmable spillon /spilloff
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
x"001D"        Clock Enable
x"0020"        Enable_busy_on_trigger_register        RW        genere un busy quand on recoit un trigger
x"0021"        debounceBusy_register        RW        duree du ebounce des busy (minimum busy length)
x"0022"        Trigext Nb
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
x"0100"        version        RO        x"14060100" ;20-FRM-PT-06-001_06
 */
/*

LEMO Enable  Mask 0xA0
Bit 0 SOS
Bit 1 EOS
Bit 2 Trigext
Bit 3 SPS spill

0x200 FEB Trig ctrl envoiye sur pair 5 HDMI 10
Bit 0 Trigext PM
Bit 1 Trigger interne avec trigdelay et triglength depuis la fenetre

0x60  Utilise uniquement pour WMDCC en DCC
*/ 
static LoggerPtr _logWmdcc(Logger::getLogger("PMDAQ_WMDCC"));
/**
 * @brief Name space for WMDCC board
 * 
 * # Hardware
 * ## Latest Firmware Registers
 * The firmware registers are defined here:
 * 
 *------------------------------------
 *|Register Address | Name | Content  |
 *|----------------:|:----:|----------|
 *|0000 | test_reg | Read Write register to test the access (default 1234)| 
 *|0001 | ID_reg | Id of the board| 
 *|0002 | software_veto_reg | Bit 0 : generates a software busy| 
 *|0003 | spill_counter_reg | Number of spill| 
 *|0004 | Counters_reset_reg | Bit 0 reset trigger register| 
 *|0005 | spillon_reg | Length in 40 MHz clock of the window| 
 *|0006 | spilloff_reg | Delay in 40 MHz clock of the next window | 
 *|0008 | Calib_reg | bit 0 : not used | 
 *|   | | bit 1 :  Switch to calibration mode with windows count is decremented |
 *|   | | bit 2 : reload calib counter with nb_window |
 *|000A | nb_windows_reg | Number of window for the calibration acquisition| 
 *|000C | Rstdet_reg |send RESET on HDMI to front-end | 
 *|000D | WindowConfig_reg | -- bit 0 : start of spill to end of spill  |
 *|     | | -- bit 1 : start of spill and SpillOn length |
 *|     | | -- bit 2 : Internal counter|
 *|     | | -- bit 3 : calibration with SpillOn/Off controled by calib  |
 *|     | | -- bit 4 : SOS and internal count (obsolete usage) |
 *|     | | -- bit 5 : Internal count start on End Busy (obsolete usage) |
 *|     | | -- bit 6 : Internal count start on End Busy + SpillOff  (obsolete usage) |
 *|000E | TrigExtDelay_reg | delay to send the trigext | 
 *|000F | TrigExtLength_reg | Length of trig ext pulse| 
 *|0010 | trigextConfig_reg | Configuration of external trigger|  
 *|   | | bit 0 :Trig ext masked if board busy|
 *|   | | bit 1 :Trig ext stopped if busy arrives |
 *|0011 | trigextcounter_reg | External trigger count| 
 *|001D | clk_enable_reg | Bit Mask of enabled clock per HDMI (default 0x3FF)| 
 *|0020 | minBusy_reg | Minimal busy time| 
 *|0021 | channel_enable_reg | Bit mask of enabled HDMI channels| 
 *|0030 | TDC_Ctrl_reg | Unused (to do) | 
 *|0031 | TDC_CoarseCount_reg | Unused (to do) | 
 *|0032 | TDC_time1_reg | Unused (to do)| 
 *|0033 | TDC_clkcnt1_reg | Unused (to do)| 
 *|0034 | TDC_time2_reg | Unused (to do)| 
 *|0035 | TDC_clkcnt2_reg | Unused (to do)| 
 *|0036 | TDC_time3_reg | Unused (to do)| 
 *|0037 | TDC_clkcnt3_reg | Unused (to do)| 
 *|0038 | TDC_time4_reg | Unused (to do)| 
 *|0039 | TDC_clkcnt4_reg | Unused (to do) | 
 *|003a | TDC_time5_reg | Unused (to do)| 
 *|003b | TDC_clkcnt5_reg | Unused (to do)| 
 *|003c | TDC_time6_reg | Unused (to do)| 
 *|003d | TDC_clkcnt6_reg | Unused (to do)| 
 *|0040 | TDC_calib1_reg | Unused (to do)| 
 *|0041 | TDC_calib2_reg | Unused (to do)| 
 *|0050-5B | busy0/B_counter_reg | From 0 to 11 busy counters| 
 *|0070 | sps_spill_duration_reg | Length in clocks of SPS spill| 
 *|0071 | sps_spill_control_reg |  Bit 0 Enable SPS fill as Not Busy| 
 *|00A0  | LemoMask_reg | -- bit 0 : Start of spill  |
 *|      | | -- bit 1 :End of Spill |
 *|      | | -- bit 2 : External Trigger|
 *|      | | -- bit 3 : SPS Spill |
 *|0100 | version | Firmware version| 
 *|0200 | feb_trg_ctrl_reg | -- bit 0 : Normal external trigger sent |
 *|     | | --bit 1 :Trigger generated from the start of window|
 *|0201 | feb_trg_duration_reg | Length of FC7 trigger pulse| 
 *|0202 | feb_trg_delay_reg |  Delay of the FC7 trigger pulse| 
 * 
 * ## Bus
 * 
 * The communication is done via an TCP/IP socket to an address and port defined on the board
 * IP 192.168.100.205 Port 10002
 * 
 * # Software
 * 
 * The socket communication is based on NL::Socket (netlink C++ socket https://sourceforge.net/projects/netlinksockets/)
 * 
 *  
 * 
 */
  namespace wmdcc
  {
    // Gere la socket registre
    class registerHandler : public wizcc::socketProcessor
    {
    public:
            /**
       * @brief Firmware registers access
       * 
       */
      enum PORT { REGISTER=10002};
      enum Register {
        //! Test register
         TEST=0x0,
         //!Board Id
		     ID=0x1,
         //! Veto mask
		     MASK=0x2,
         //! Number of Spill
		     SPILL_CNT=0x3,
         //!  Bit 0 reset counters register
		     ACQ_CTRL=0x4,
         //! Windows ON length
		     SPILL_ON=0x5,
         //! Windows OFF length
		     SPILL_OFF=0x6,
         //! Calib control register
		     CALIB_CTRL=0x8,
         //! Calibration number of windows
		     CALIB_NWIN=0xA,
         //! Hard Reset of Front end Rstdet 
		     RESET_FE=0xC,
         //! window configuration
		     WIN_CTRL=0xD,
         //! Delay to send extrnal trigger
		     TRG_EXT_DELAY=0xE,
         //! trigger Pulse length
		     TRG_EXT_LEN=0xF,
         //! Configuration of external trigger
		     TRG_EXT_CONFIG=0x10,
         //! Counter of external trigger
		     TRG_EXT_COUNT=0x11,
         //! Bit Mask of enabled clock per HDMI (default 0x3FF) 
		     CLK_ENABLE=0x1D,
         //! Minimal busy time 
		     MIN_BUSY_LENGTH=0x20,
         //! Bit mask of enabled HDMI channels 
		     CHANNEL_ENABLE=0x21,
         //! obsolote
		     TRG_EXT_NB=0x22,
         //! obsolote
		     EN_BUSY_TRG=0x20,
         //! obsolote
		     DEBOUNCE_BUSY=0x21,
         //! To be implemented
		     TDC_CTRL=0x30,
         //! To be implemented
		     TDC_COARSE=0x31,
		     TDC_T1=0x32,TDC_CNT1=0x33,
		     TDC_T2=0x34,TDC_CNT2=0x35,
		     TDC_T3=0x36,TDC_CNT3=0x37,
		     TDC_T4=0x38,TDC_CNT4=0x39,
		     TDC_T5=0x3A,TDC_CNT5=0x3B,
		     TDC_T6=0x3C,TDC_CNT6=0x3D,
		     TDC_CAL1=0x40,TDC_CAL2=0x41,
         //! First counter of busy
		     BUSY_0=0x50, 
         //! Unused 
		     RESET_FSM=0x60,
         //! Duree du spill SPS
		     SPS_SPILL_DURATION=0x70, 
         //! Bit 0 Enable SPS fill as Not Busy
		     SPS_SPILL_CTRL=0x71, 
         //! Enable Mask des lemo pour eviter les parasites par defaut 0X0 tout est masque
		     LEMO_MASK=0xA0, 
         //! FEBV2 control register
		     RESYNC_MASK=0x200,
         //! Firmware version
		     VERSION=0x100};
		     

      registerHandler(std::string);
      virtual bool process_message();
      void writeRegister(uint16_t address,uint32_t value);
      uint32_t readRegister(uint16_t address);
    private:
      wizcc::Message* _msg;
      bool _noTransReply;
    };



  };
