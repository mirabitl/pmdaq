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
 * @brief fsmw plugin to handle MDCC
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
    void c_resettdc(http_request m);
    void c_setcalibregister(http_request m);
    void c_setspillregister(http_request m);
    void c_sethardreset(http_request m);
    void c_settrigext(http_request m);
    void c_setregister(http_request m);
    void c_getregister(http_request m);
    void c_setexternaltrigger(http_request m);
  private:
    /// @brief Pointer to the mdcc::MdccHandler
    mdcc::MdccHandler* _mdcc;
};

