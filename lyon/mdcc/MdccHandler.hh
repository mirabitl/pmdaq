#pragma once
/**
 * @file MdccHandler.hh
 * @author Laurent Mirabito
 * @brief 
 * @version 1.0
 * @date 2024-09-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "FtdiUsbDriver.hh"

#include "stdafx.hh"
#include "utils.hh"
static LoggerPtr _logMdcc(Logger::getLogger("PMDAQ_MDCC"));

namespace mdcc
{
/**
* @brief Getter and setter function of the MDCC
* 
* The register accessible in the firmware are :

*------------------------------------
*|Register Address | Name | Content  |
*|----------------:|:----:|----------|
*|0001  |   ID | Id of the board |
*|  0002  |   software_veto | Bit 0 : generates a software busy |
*|  0003  |   spillNb | Number of spill | 
*|  0004  |   Control | Bit 0 reset trigger register |
*|  0005  |   spillon | Length in 40 MHz clock of the window |
*|  0006  |   spilloff | Delay in 40 MHz clock of the next window |
*|  0007  |   beam | (obsolete) Length of beam spill |
*|  0008  |  Calib | bit 0 : not used | 
*|        |   | bit 1 :  Switch to calibration mode with windows count is decremented |
*|        |         | bit 2 : reload calib counter with nb_window |
*|  0009  |   Calib_Counter | Number of window for calibration |
*|  000A  |   nb_windows | Number of window for the acquisition |
*|  000B  |   software_ECALveto | (obsolete) second software veto | 
*|  000C  |   Rstdet | send RESET on HDMI to front-end |  
*|  000D  |  WindowConfig |  -- bit 0 : start of spill to end of spill  |
*|        | | -- bit 1 : start of spill and SpillOn length |
*|        | |-- bit 2 : Internal counter|
*|        | | -- bit 3 : calibration with SpillOn/Off controled by calib  |
*|        | | -- bit 4 : SOS and internal count (obsolete usage)|
*|        | | -- bit 5 : Internal count start on End Busy (obsolete usage) |
*|        | |-- bit 6 : Internal count start on End Busy + SpillOff  (obsolete usage) |
*|  000E  |   TrigExtDelay | delay to send the trigext |
*|  000F  |   TrigExtLength| Length of trig ext pulse |  
*|  0011  |   busy1Nb| Busy count HDMI 1|
*|  0012  |   busy2Nb| Busy count HDMI 2|
*|  0013  |   busy3Nb| Busy count HDMI 3|
*|  0014  |   busy4Nb| Busy count HDMI 4|
*|  0015  |   busy5Nb| Busy count HDMI 5|
*|  0016  |   busy6Nb| Busy count HDMI 6|
*|  0017  |   busy7Nb| Busy count HDMI 7|
*|  0018  |   busy8Nb| Busy count HDMI 8|
*|  0019  |   busy9Nb| Busy count HDMI 9|
*|  001A  |  Enable_busy_on_trigger | Generate a busy on external trigger | 
*|  0020  |   debounceBusy | Tuning of minimal busy length | 
*|  0100  |   version | Firmware  version |       
   * 
   */
  class MdccHandler 
  {
  public:
  /**
   * @brief Construct a new Mdcc Handler object
   * 
   * @param name Device identifier
   * @param productid FTDI product Id if not 0x6001
   */
    MdccHandler (std::string name,uint32_t productid=0x6001);
    /**
     * @brief Destroy the Mdcc Handler object
     * 
     */
    ~MdccHandler();
    /**
     * @brief Create access to ftdi using libftdi
     * 
     */
    void open();
    /**
     * @brief Close FTDI access
     * 
     */
    void close();
    /**
     * @brief Write 32 bits data to register
     * 
     * @param addr Register address
     * @param data Data to write
     */
    void writeRegister(uint32_t addr,uint32_t data);
    /**
     * @brief Read register
     * 
     * @param addr Register address
     * @return uint32_t Register value
     */
    uint32_t readRegister(uint32_t addr);
    /**
     * @brief Pause the windows  
     * 
     * \b software_veto set to 1
     * 
     */
    void maskTrigger();
    /**
     * @brief Resume windows
     * 
     * \b software_veto set to 0
     * 
     */
    void unmaskTrigger();
    /**
     * @brief Second software veto
     * 
     * \b software_ECALveto set to 1
     * 
     * @deprecated
     * 
     */
    void maskEcal();
     /**
     * @brief Resume second software veto
     * 
     * \b software_ECALveto set to 0
     * 
     * @deprecated
     * 
     */
    void unmaskEcal();
    /**
     * @brief Reset all counters
     * 
     * \b Control set to 1 then to 0
     * 
     */
    void resetCounter();
    /**
     * @brief Set b on \b Rstdet 
     * 
     * @param b Byte to write
     */
    void resetTDC(uint8_t b);
    /**
     * @brief Read the \b software_veto
     * 
     * @return uint32_t Value of the register
     */
    uint32_t mask();
    /**
     * @brief Read \b software_ECALveto
     * 
     * @return uint32_t 
     */
    uint32_t ecalmask();
    /**
     * @brief Read \b spillNb register
     * 
     * @return uint32_t The spill counter
     */
    uint32_t spillCount();
    /**
     * @brief Read the busy counter \b busy1-9Nb 
     * 
     * @param b HDMI number
     * @return uint32_t Number of busy
     */
    uint32_t busyCount(uint8_t b);
    /**
     * @brief Read the \b spillon register
     * 
     * @return uint32_t Spil ON length 
     */
    uint32_t spillOn();
    /**
     * @brief Read the \b spilloff register
     * 
     * @return uint32_t Spil OFF length 
     */
    uint32_t spillOff();
    /**
     * @brief Set the SpillOn length
     * 
     * @param nc Clock length
     */
    void setSpillOn(uint32_t nc);
    /**
     * @brief Set the Spill Off length
     * 
     * @param nc Clock length
     */
    void setSpillOff(uint32_t nc);
    /**
     * @brief Read the \b beam register
     * 
     * @return uint32_t register value
     * @deprecated
     * 
     */
    uint32_t beam();
    /**
     * @brief Set the \b beam register
     * 
     * @param nc Clock length
     * @deprecated
     */
    void setBeam(uint32_t nc);
     /**
      * @brief Read \b Rstdet register
      * 
      * @return uint32_t Register value
      */
    uint32_t hardReset();
    /**
     * @brief Set the \b Rstdet register
     * 
     * @param nc Value to write
     */
    void setHardReset(uint32_t nc);
    /**
     * @brief Read \b version register
     * 
     * @return uint32_t Firmware version
     */
    uint32_t version();
    /**
     * @brief Read \b ID register
     * 
     * @return uint32_t Boar Id
     */
    uint32_t id();
    /**
     * @brief Set calibration ON
     * 
     */
    void calibOn();
    /**
     * @brief set calibration Off
     * 
     */
    void calibOff();
    /**
     * @brief Complete procedure to reload calib
     * 
     *  \b software_veto 1 | \b WindowConfig 8
     * 
     * \b Calib 4 | \b Calib 2 | \b software_veto 0 
     * 
     * 
     * 
     */
    void reloadCalibCount();
    /**
     * @brief Read the \b Calib_Counter register
     * 
     * @return uint32_t Number of calibration windows
     */
    uint32_t calibCount();
    /**
     * @brief Set the \b Calib_Counter register
     * 
     * @param nc Number of claibration windows
     */
    void setCalibCount(uint32_t nc);
    /**
     * @brief Set the \b Calib register 
     * 
     * @param nc 2 or 4 (see firmware registers)
     */
    void setCalibRegister(uint32_t nc);
    /**
     * @brief Set the \b WindowConfig register
     * 
     * @param nc window config register (see firmware registers)
     */
    void setSpillRegister(uint32_t nc);
    /**
     * @brief Read the \b WindowConfig register
     * 
     * @return uint32_t 
     */
    uint32_t spillRegister();
    /**
     * @brief Change \b WindowConfig  to use SPS spill
     * 
     * @param t True SPS spill is used
     */
    void useSPSSpill(bool t);
    /**
     * @brief Change \b WindowConfig to use external trigger
     * 
     * @param t True external trigger is used
     */
    void useTrigExt(bool t);
    /**
     * @brief Set the \b TrigExtDelay register
     * 
     * @param nc delay of the trigger
     */
    void setTriggerDelay(uint32_t nc);
    /**
     * @brief Read the \b TrigExtDelay register
     * 
     * @return uint32_t delay in clock number
     */
    uint32_t triggerDelay();
    /**
     * @brief Set the \b TrigExtLength register
     * 
     * @param nc length of the trigger pulse
     */
    void setTriggerBusy(uint32_t nc);
    /**
     * @brief read the \b TrigExtLength register
     * 
     * @return uint32_t Length of trigger pulse
     */
    uint32_t triggerBusy();
    /**
     * @brief Set the \b Enable_busy_on_trigger register
     * 
     * @param nc 0 or 1
     */
    void setExternalTrigger(uint32_t nc);
    /**
     * @brief read   the \b Enable_busy_on_trigger register
     * 
     * @return uint32_t Boolean 1 or 0
     */
    uint32_t externalTrigger();
  
  private : 
    mdcc::FtdiUsbDriver* _driver;
    std::string _name;
    uint32_t _productid,_version,_id;

  };
};

