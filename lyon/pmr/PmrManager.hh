#pragma once

/**
 * \class PMRManager
 *
 * \ingroup lydaq
 *
 * \brief SDHCAL PMR managment class
 *
 * This class is a new implementation of the PMRSupervisor
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
#include "PmrInterface.hh"

#include "HR2ConfigAccess.hh"

using namespace pmr;
#include <sstream>
#include <map>
#include <vector>
class PmrManager : public fsmw
{

public:
  /**
     Create a Pmr Manager class
     \param name the name of the process
  */
  PmrManager();
  virtual void initialise();
  virtual void end();

  /**
     \brief Transition is CREATED &rarr; SCANNED. It scans the FTDI network to find all Pmr connected

     \param m a zdaq::fsmmessage with \a command and \a content tag

     The answer contains a list of Pmr found
  */
  void scan(http_request m);

  /**
     \brief Transition is SCANNED &rarr; INITIALISED. It initialises all Pmr already found in scan
     \param m a zdaq::fsmmessage with \a command and  \a content tag

     It also download the DB state specified in \a db tag parameters.
     It initialised only Pmr found in the DB.

     Eventually it creates the data stream to the event builder specified in the \a publish tag

     \note Failure at this stage corresponds either to USB failure or missing clock or wrong firmware

     The answer contains a list of Pmr with their status
  */
  void fsm_initialise(http_request m);

  /**
     \brief Transition is INITIALISED &rarr; CONFIGURED. It configures Pmr and associated ASICs
     \param m a zdaq::fsmmessage with \a command and  \a content tag


     \note Failure at this stage corresponds either to electric problem either on the ASU or on Pmr-ASU, ASU-ASU connectors. It may also come from a mismatch between ASIC list in the database and actual active chips on ASU.

     The answer contains a list of Pmr with their status
  */
  void configure(http_request m);

  /**
     \brief Transition is CONFIGURED &rarr; RUNNING. It starts the readout threads for each Pmr
     \param m a zdaq::fsmmessage with \a command and  \a content tag


     \note Each Pmr has its own thread.

     The answer contains a list of Pmr with their status
  */
  void start(http_request m);
  void stop(http_request m);
  void destroy(http_request m);

  void c_status(http_request m);
  void c_downloadDB(http_request m);
  void c_setchannelmask(http_request m);
  void c_setmask(http_request m);
  void c_setthresholds(http_request m);
  void c_setpagain(http_request m);
  void c_external(http_request m);
  /**
     Change the threshold on all Asics of the Pmr
     @param b0 First threshold
     @param b1 Second threshold
     @param b2 Third threshold
     @param idif The dif ID
  */
  void setThresholds(uint16_t b0, uint16_t b1, uint16_t b2, uint32_t idif = 0);
  void setGain(uint16_t gain);
  void setMask(uint32_t level, uint64_t mask);
  void setChannelMask(uint16_t level, uint16_t channel, uint16_t val);

  web::json::value configureHR2();

  void prepareDevices();
  void configureThread(PmrInterface *d,unsigned char* b,uint32_t nb);
  void startReadoutThread(PmrInterface *d);
  // DimRpc interface
  std::map<uint32_t, pmr::FtdiDeviceInfo *> &getFtdiMap() { return theFtdiDeviceInfoMap_; }
  std::map<uint32_t, PmrInterface *> &getPmrMap() { return _PmrInterfaceMap; }

  pmr::FtdiDeviceInfo *getFtdiDeviceInfo(uint32_t i)
  {
    if (theFtdiDeviceInfoMap_.find(i) != theFtdiDeviceInfoMap_.end())
      return theFtdiDeviceInfoMap_[i];
    else
      return NULL;
  }

  void joinThreads()
  {
    for (auto i = g_d.begin(); i != g_d.end(); i++)
      (*i).join();
  }
  void joinConfigureThreads();


  void setAllMasks(uint64_t mask);
  void setCTEST(uint64_t mask);
  void ScurveStep(std::string mdcc, std::string builder, int thmin, int thmax, int step);
  void thrd_scurve();
  void Scurve(int mode, int thmin, int thmax, int step);

  void c_scurve(http_request m);
  void GainCurveStep(std::string mdcc,std::string builder,int thmin,int thmax,int step,int thr);
  void thrd_gaincurve() ;
  void GainCurve(int mode,int thmin,int thmax,int step,int thr);
  void c_gaincurve(http_request m);
private:
  std::map<uint32_t, pmr::FtdiDeviceInfo *> theFtdiDeviceInfoMap_;
  std::map<uint32_t, PmrInterface *> _PmrInterfaceMap;
  std::vector<PmrInterface *> _vDif;
  HR2ConfigAccess *_hca;

  std::vector<std::thread> g_d;
  std::vector<std::thread> g_c;
  zmq::context_t *_context;

  bool _running;
  // Scurve parameters
  int _sc_mode, _sc_thmin, _sc_thmax, _sc_step, _sc_gmin, _sc_gmax, _sc_threshold, _sc_level;
  int _sc_win, _sc_ntrg;
  bool _sc_running;
  int _run_mode;
  std::thread g_scurve;
};
