#pragma once
/**
 * @file MdccManager.hh
 * @author L.Mirabito
 * @brief MDCC Main application
 * @version 1.0
 * @date 2024-09-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "fsmw.hh"
#include "MdccHandler.hh"

using namespace std;
#include <sstream>
#include "stdafx.hh"
using namespace mdcc;
/**
 * 
 *@brief fsmw plugin to handle MDCC
 * 
 * The register accessible in the firmware are described in mdcc::MdccHandler
 * 
 * 
 * The Finite State Machine is defined as 
 * 
 * |States|
 * |------|
 * | CREATED|
 * | OPENED|
 * 
 * With the following transitions
 * 
 * |transitions| transition command|
 * |----------:|:------------------|
 * CREATED-OPENED| INITIALISE 
 * OPENED-CREATED| DESTROY
 * 
 * And the following commands are implemnted
 * 
 * | Command | Action| 
 * |--------:|:------|
 * | PAUSE | \b Software_veto=1 |
 * | RESUME | \b Software_veto=0 |
 * |RESET | Reset the counters Flip \b  Control=1 and then 0| 
 *ECALPAUSE | Obsolete 
 * ECALRESUME | Obsolete 
 * WRITEREG/SETREG | Write one register with \e  address (decimal) and \e  value parameters 
 * READREG/GETREG | Read value of one register at \e  address (decimal) parameter 
 * STATUS | Read and returns the value of all parameters in a JSON object 
 * SPILLON | Set \b  spillon  to \e  nclock parameter value 
 * SPILLOFF | Set \b  spilloff  to \e  nclock parameter value 
 * BEAMON | Set \b  beam  to \e  nclock parameter value (obsolete) 
 * RESETTDC/SETHARDRESET | Set \b  Rstdet to \e  value parameter (should be flip 1 to 0) 
 * CALIBON | Set \b  Calib  to 2 
 * CALIBOFF | Set \b  Calib  to 0 
 * RELOADCALIB | Do the full MDCC \e  per setting point sequence previously described 
 * SETCALIBCOUNT | Set \b  nb_windows  to \e  nclock parameter value 
 * SETSPILLREGISTER | Set \b  WindowConfig  to \e  value parameter 
 * SETCALIBREGISTER | Set \b  Calib to \e  value parameter 
 * SETTRIGEXT | Set \b  TrigExtDelay to \e  delay value and \b  TrigExtLength to \e  busy value 
 * SETEXTERNAL | \b  Enable_busy_on_trigger set to \e  value (0 or 1) parameter 
 *  
 * 
 */
class MdccManager : public fsmw
  {
  public:
  /**
   * @brief Construct a new Mdcc Manager object
   * 
   */
    MdccManager();
    /**
     * @brief Implementation on fsmw initialise
     * 
     * define states, transition and commands
     * 
     *
     * 
     */
    virtual void initialise();
    /**
     * @brief Cleanly stop the plugin
     * 
     */
    virtual void end();
    
    /**
     * @brief INITIALISE transition
     * 
     * @param m The http request containing the transition command, parameters and answer
     */
    void fsm_initialise(http_request m);
    /**
     * @brief DESTROY transition
     * 
     * @param m The http request containing the transition command, parameters and answer
     */
    void destroy(http_request m);

    /**
     * @brief Open the FtdiUsbDriver via the MdccHandler
     * 
     * @param s Ftdi device name
     */
    void doOpen(std::string s);
    /**
     * @brief Get the Mdcc Handler object
     * 
     * @return MdccHandler* pointer to the handler
     */
    MdccHandler* getMdccHandler(){  return _mdcc;}

    /**
     * @brief call back of STATUS command
     * 
     * @param m http request
     */
    void c_status(http_request m);
     /**
     * @brief call back of PAUSE command
     * 
     * @param m http request
     */
    void c_pause(http_request m);
     /**
     * @brief call back of RESUME command
     * 
     * @param m http request
     */
    void c_resume(http_request m);
     /**
     * @brief call back of ECALPAUSE command
     * 
     * @param m http request
     */
    void c_ecalpause(http_request m);
     /**
     * @brief call back of ECALRESUME command
     * 
     * @param m http request
     */
    void c_ecalresume(http_request m);
     /**
     * @brief call back of RESET (counters) command
     * 
     * @param m http request
     */
    void c_reset(http_request m);
     /**
     * @brief call back of READREG command
     * 
     * One parameter in the request, \b address is required 
     * 
     * @param m http request
     */
    void c_readreg(http_request m);
    /**
     * @brief call back of READREG command
     * 
     * Two parameters are required in the request,
     * 
     *  \b address  and \b value 
     * 
     * @param m http request
     */
    void c_writereg(http_request m);
    /**
     * @brief call back of SPILLON command
     * 
     * One parameter in the request, \b nclock is required 
     * 
     * @param m http request
     */
    void c_spillon(http_request m);
    /**
     * @brief call back of SPILLOFF command
     * 
     * One parameter in the request, \b nclock is required 
     * 
     * @param m http request
     */
    void c_spilloff(http_request m);
    /**
     * @brief call back of BEAMON command
     * 
     * One parameter in the request, \b nclock is required 
     * 
     * @param m http request
     * @deprecated Not implemmneted in firmware anymore
     */
    void c_beamon(http_request m);
    /**
     * @brief call back of SETCALIBCOUNT command
     * 
     * One parameter in the request, \b nclock is required 
     * 
     * It is the number of windows per calibration point
     * 
     * @param m http request
     */
    void c_setcalibcount(http_request m);
    /**
     * @brief call back of RELOADCALIB command
     *      
     * @param m http request
     */
    void c_reloadcalib(http_request m);
    /**
     * @brief call back of CALIBON command
     *      
     * @param m http request
     */
    void c_calibon(http_request m);
     /**
     * @brief call back of CALIBOFF command
     *     
     * @param m http request
     */
    void c_caliboff(http_request m);
    /**
     * @brief call back of RESETTDC command
     * 
     * \b value (0/1) is required    
     * @param m http request
     */
    void c_resettdc(http_request m);

    /**
     * @brief call back of SETCALIBREGISTER command
     * \b value (0/2/4) is required
     * @param m http request
     */
    void c_setcalibregister(http_request m);

    /**
     * @brief call back of SETSPILLREGISTER command
     *     
     * \b value (WindowConfig) is needed
     * @param m http request
     */
    void c_setspillregister(http_request m);
    /**
     * @brief call back of SETHARDRESET command
     *     
     * \b value (0/1) is required
     * 
     * @param m http request
     */
    void c_sethardreset(http_request m);
     /**
     * @brief call back of SETTRIGEXT command
     * 
     * \b delay and \b busy values are required
     *     
     * @param m http request
     */
    void c_settrigext(http_request m);
    /**
     * @brief call back of SETREGISTER command
     *     
     * \b address and \b value are required
     * @param m http request
     */
    void c_setregister(http_request m);
    /**
     * @brief call back of GETREGISTER command
     *     
     * \b address is required
     * @param m http request
     */
    void c_getregister(http_request m);
    /**
     * @brief call back of SETEXTERNAL command
     * 
     * \b value (0/1) is required
     *     
     * @param m http request
     */
    void c_setexternaltrigger(http_request m);
  private:
    /// @brief Pointer to the mdcc::MdccHandler
    mdcc::MdccHandler* _mdcc;
};

