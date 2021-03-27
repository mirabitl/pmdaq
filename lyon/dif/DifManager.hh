#ifndef _DIFManager_h

#define _DIFManager_h
/**
 * \class DIFManager
 *
 * \ingroup lydaq
 *
 * \brief SDHCAL DIF managment class
 *
 * This class is a new implementation of the DIFSupervisor
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
#include<stdio.h>
#include "baseApplication.hh"
#include "DIFInterface.hh"
#include "DIFReadoutConstant.hh"
#include "HR2ConfigAccess.hh"

using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "ReadoutLogger.hh"
namespace lydaq
{
  class DIFManager  : zdaq::baseApplication
  {
  
  public:
    /**
       Create a DIF Manager class
       \param name the name of the process 
    */
    DIFManager(std::string name);
    
    /**
       \brief Transition is CREATED &rarr; SCANNED. It scans the FTDI network to find all DIF connected
       
       \param m a zdaq::fsmmessage with \a command and \a content tag
       
       The answer contains a list of DIF found
     */
    void scan(zdaq::fsmmessage* m);

    /**
       \brief Transition is SCANNED &rarr; INITIALISED. It initialises all DIF already found in scan
       \param m a zdaq::fsmmessage with \a command and  \a content tag
       
       It also download the DB state specified in \a db tag parameters. 
       It initialised only DIF found in the DB.

       Eventually it creates the data stream to the event builder specified in the \a publish tag

       \note Failure at this stage corresponds either to USB failure or missing clock or wrong firmware

       The answer contains a list of DIF with their status
    */
    void initialise(zdaq::fsmmessage* m);

    /**
       \brief Transition is INITIALISED &rarr; CONFIGURED. It configures DIF and associated ASICs
       \param m a zdaq::fsmmessage with \a command and  \a content tag
   

       \note Failure at this stage corresponds either to electric problem either on the ASU or on DIF-ASU, ASU-ASU connectors. It may also come from a mismatch between ASIC list in the database and actual active chips on ASU.

       The answer contains a list of DIF with their status
    */
    void configure(zdaq::fsmmessage* m);

    /**
       \brief Transition is CONFIGURED &rarr; RUNNING. It starts the readout threads for each DIF
       \param m a zdaq::fsmmessage with \a command and  \a content tag
   

       \note Each DIF has its own thread. 

       The answer contains a list of DIF with their status
    */    
    void start(zdaq::fsmmessage* m);
    void stop(zdaq::fsmmessage* m);
    void destroy(zdaq::fsmmessage* m);

    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setchannelmask(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setmask(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setthresholds(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setpagain(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_ctrlreg(Mongoose::Request &request, Mongoose::JsonResponse &response);
    /**
       Change the threshold on all Asics of the DIF
       @param b0 First threshold
       @param b1 Second threshold
       @param b2 Third threshold
       @param idif The dif ID
     */
    void setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif);
    void setGain(uint16_t gain);
    void setMask(uint32_t level,uint64_t mask);
    void setChannelMask(uint16_t level,uint16_t channel,uint16_t val);

    Json::Value configureHR2();

    void prepareDevices();
    void startDIFThread(DIFInterface* d);
    // DimRpc interface
    std::map<uint32_t,FtdiDeviceInfo*>& getFtdiMap(){ return theFtdiDeviceInfoMap_;}
    std::map<uint32_t,DIFInterface*>& getDIFMap(){ return _DIFInterfaceMap;}
      
    FtdiDeviceInfo* getFtdiDeviceInfo(uint32_t i) { if ( theFtdiDeviceInfoMap_.find(i)!=theFtdiDeviceInfoMap_.end()) return theFtdiDeviceInfoMap_[i]; else return NULL;}

    void joinThreads(){g_d.join_all();}

  private:
    std::map<uint32_t,FtdiDeviceInfo*> theFtdiDeviceInfoMap_;	
    std::map<uint32_t,lydaq::DIFInterface*> _DIFInterfaceMap;
    std::vector<lydaq::DIFInterface*> _vDif;
    lydaq::HR2ConfigAccess* _hca;
    zdaq::fsmweb* _fsm;
    boost::thread_group g_d;
    zmq::context_t* _context;

  };
};
#endif

