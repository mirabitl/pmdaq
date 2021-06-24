#pragma once


/**
 * \class LIBOARDManager
 *
 * \ingroup lydaq
 *
 * \brief SDHCAL LIBOARD managment class
 *
 * This class is a new implementation of the LIBOARDSupervisor
 * It is analog to GricManager, with its own access to DB
 *
 * \note Attempts at zen rarely work.
 *
 * \author L.Mirabito
 *
 * \version 1.0
 *
 * \date January 2020
 *
 *
 */

#include <iostream>

#include <string.h>
#include <stdio.h>
#include <thread>
#include "fsmw.hh"
#include "stdafx.hh"
#include "utils.hh"
#include "LiboardInterface.hh"

#include "LIROCConfigAccess.hh"

using namespace liboard;
#include <sstream>
#include <map>
#include <vector>
class LiboardManager  : public fsmw
  {
  
  public:
    /**
       Create a Liboard Manager class
       \param name the name of the process 
    */
    LiboardManager();
    virtual void initialise();
    virtual void end();
    
    /**
       \brief Transition is CREATED &rarr; SCANNED. It scans the FTDI network to find all Liboard connected
       
       \param m a zdaq::fsmmessage with \a command and \a content tag
       
       The answer contains a list of Liboard found
     */
    void scan(http_request  m);

    /**
       \brief Transition is SCANNED &rarr; INITIALISED. It initialises all Liboard already found in scan
       \param m a zdaq::fsmmessage with \a command and  \a content tag
       
       It also download the DB state specified in \a db tag parameters. 
       It initialised only Liboard found in the DB.

       Eventually it creates the data stream to the event builder specified in the \a publish tag

       \note Failure at this stage corresponds either to USB failure or missing clock or wrong firmware

       The answer contains a list of Liboard with their status
    */
    void fsm_initialise(http_request  m);

    /**
       \brief Transition is INITIALISED &rarr; CONFIGURED. It configures Liboard and associated ASICs
       \param m a zdaq::fsmmessage with \a command and  \a content tag
   

       \note Failure at this stage corresponds either to electric problem either on the ASU or on Liboard-ASU, ASU-ASU connectors. It may also come from a mismatch between ASIC list in the database and actual active chips on ASU.

       The answer contains a list of Liboard with their status
    */
    void configure(http_request  m);

    /**
       \brief Transition is CONFIGURED &rarr; RUNNING. It starts the readout threads for each Liboard
       \param m a zdaq::fsmmessage with \a command and  \a content tag
   

       \note Each Liboard has its own thread. 

       The answer contains a list of Liboard with their status
    */    
    void start(http_request  m);
    void stop(http_request  m);
    void destroy(http_request  m);

    void c_status(http_request m );
    void c_downloadDB(http_request m );
    void c_setchannelmask(http_request m );
    void c_setmask(http_request m );
    void c_setthreshold(http_request m );
    void c_setdcpa(http_request m );
    void c_setdaclocal(http_request m );
    void c_external(http_request m );
    /**
       Change the threshold on all Asics of the Liboard
       @param b0 First threshold
       @param b1 Second threshold
       @param b2 Third threshold
       @param idif The dif ID
     */
    void setThreshold(uint16_t v0,uint32_t idif=0);
    void setDC_pa(uint8_t gain);
    void setDAC_local(uint8_t dac);
    void setMask(uint64_t mask);
    void setChannelMask(uint16_t channel,uint16_t val);

    web::json::value configureLR();

    void prepareDevices();
    void startReadoutThread(LiboardInterface* d);
    // DimRpc interface
    std::map<uint32_t,liboard::FtdiDeviceInfo*>& getFtdiMap(){ return theFtdiDeviceInfoMap_;}
    std::map<uint32_t,LiboardInterface*>& getLiboardMap(){ return _LiboardInterfaceMap;}
      
    liboard::FtdiDeviceInfo* getFtdiDeviceInfo(uint32_t i) { if ( theFtdiDeviceInfoMap_.find(i)!=theFtdiDeviceInfoMap_.end()) return theFtdiDeviceInfoMap_[i]; else return NULL;}

    void joinThreads(){for (auto i=g_d.begin();i!=g_d.end();i++) (*i).join();}

    void setCtest(uint64_t mask);
    void ScurveStep(std::string builder,int thmin,int thmax,int step);
    void thrd_scurve() ;
    void Scurve(int mode,int thmin,int thmax,int step);

    void c_scurve(http_request m );
    void c_masktdcchannels(http_request m );
    void c_setlatchdelay(http_request m );
    void c_setlatchduration(http_request m );

    
    void c_mdccstatus(http_request m);
    void c_pause(http_request m);
    void c_resume(http_request m);
    void c_ecalpause(http_request m);
    void c_ecalresume(http_request m);
    void c_reset(http_request m);
    void c_readreg(http_request m);
    void c_writereg(http_request m);
    void c_spillon(http_request m);
    void c_spilloff(http_request m);
    void c_beamon(http_request m);
    void c_setcalibcount(http_request m);
    void c_reloadcalib(http_request m);
    void c_calibon(http_request m);
    void c_caliboff(http_request m);
    void c_resettdc(http_request m);
    void c_setcalibregister(http_request m);
    void c_setspillregister(http_request m);
    void c_sethardreset(http_request m);
    void c_settrigext(http_request m);
    void c_setexternaltrigger(http_request m);


    
  private:
    std::map<uint32_t,liboard::FtdiDeviceInfo*> theFtdiDeviceInfoMap_;	
    std::map<uint32_t,LiboardInterface*> _LiboardInterfaceMap;
    std::vector<LiboardInterface*> _vDif;
    LIROCConfigAccess* _hca;

    std::vector<std::thread> g_d;
    zmq::context_t* _context;

    bool _running;
    // Scurve parameters
    int _sc_mode,_sc_thmin,_sc_thmax,_sc_step;
    bool _sc_running;
    std::thread g_scurve;
    // MDCC
    liboard::LiboardDriver* _mdcc;
  };

