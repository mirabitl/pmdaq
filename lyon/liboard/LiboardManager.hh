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

#include "HR2ConfigAccess.hh"

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
    void c_setthresholds(http_request m );
    void c_setpagain(http_request m );
    void c_external(http_request m );
    /**
       Change the threshold on all Asics of the Liboard
       @param b0 First threshold
       @param b1 Second threshold
       @param b2 Third threshold
       @param idif The dif ID
     */
    void setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif=0);
    void setGain(uint16_t gain);
    void setMask(uint32_t level,uint64_t mask);
    void setChannelMask(uint16_t level,uint16_t channel,uint16_t val);

    web::json::value configureHR2();

    void prepareDevices();
    void startReadoutThread(LiboardInterface* d);
    // DimRpc interface
    std::map<uint32_t,liboard::FtdiDeviceInfo*>& getFtdiMap(){ return theFtdiDeviceInfoMap_;}
    std::map<uint32_t,LiboardInterface*>& getLiboardMap(){ return _LiboardInterfaceMap;}
      
    liboard::FtdiDeviceInfo* getFtdiDeviceInfo(uint32_t i) { if ( theFtdiDeviceInfoMap_.find(i)!=theFtdiDeviceInfoMap_.end()) return theFtdiDeviceInfoMap_[i]; else return NULL;}

    void joinThreads(){for (auto i=g_d.begin();i!=g_d.end();i++) (*i).join();}

    void setAllMasks(uint64_t mask);
    void setCTEST(uint64_t mask);
    void ScurveStep(std::string mdcc,std::string builder,int thmin,int thmax,int step);
    void thrd_scurve() ;
    void Scurve(int mode,int thmin,int thmax,int step);

    void c_scurve(http_request m );
  private:
    std::map<uint32_t,liboard::FtdiDeviceInfo*> theFtdiDeviceInfoMap_;	
    std::map<uint32_t,LiboardInterface*> _LiboardInterfaceMap;
    std::vector<LiboardInterface*> _vDif;
    HR2ConfigAccess* _hca;

    std::vector<std::thread> g_d;
    zmq::context_t* _context;

    bool _running;
    // Scurve parameters
    int _sc_mode,_sc_thmin,_sc_thmax,_sc_step;
    bool _sc_running;
    std::thread g_scurve;
  };

