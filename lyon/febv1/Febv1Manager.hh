#pragma once
#include "Febv1Interface.hh"
#include "Febv1ConfigAccess.hh"
#include "fsmw.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "stdafx.hh"
#include "utils.hh"

class Febv1Manager : public fsmw
{
public:
  Febv1Manager();

  virtual void initialise();
  virtual void end();

    /// INITIALISE  handler
  void fsm_initialise(http_request m);
  /// CONFIGURE  handler
  void configure(http_request m);
  /// START  handler
  void start(http_request m);
  /// STOP  handler
  void stop(http_request m);
  /// DESTROY  handler
  void destroy(http_request m);
  /// job log command  (obsolete)
  void c_joblog(http_request m );
  /// STATUS Command handler
  void c_status(http_request m );
  /// DIFLIST Command handler
  void c_diflist(http_request m );
  /// SET6BDAC Command handler
  void c_cal6bdac(http_request m );
  void c_set6bdac(http_request m );
  /// SETMASK Command handler
  void c_setMask(http_request m );
  /// SETMODE Command handler
  void c_setMode(http_request m );
  /// SETDELAY Command handler
  void c_setDelay(http_request m );
  /// SETDURATION Command handler
  void c_setDuration(http_request m );
  /// SETVTHTIME Command Handler
  void c_setvthtime(http_request m );
  /// SETONEVTHTIME Command handler
  void c_set1vthtime(http_request m );
  /// DOWNLOADDB Command handler
  void c_downloadDB(http_request m );
  void c_asics(http_request m );
  /// GETLUT command handler
  void c_getLUT(http_request m );
  /// TDC Calibration  command handlers
  void c_getCalibrationStatus(http_request m );
  void c_setCalibrationMask(http_request m );
  void c_setMeasurementMask(http_request m );

  /// PR2 SCurve
  void c_scurve(http_request m );

  /// Change 6BDAC (all FEBs,all Asics)
  void set6bDac(uint8_t dac);
  /// 
  void cal6bDac(uint32_t mask,int32_t dacShift);
  /// Change Mask (all febs, ASIC mask)
  void setMask(uint32_t mask, uint8_t asic = 255);
  /// Sof trigger (obsolete)
  void sendTrigger(uint32_t nt);
  /// Change VTHTIME (all FEBS)
  void setVthTime(uint32_t dac);
  /// Change VTHTIME (FEB and asic specified)
  void setSingleVthTime(uint32_t vth, uint32_t feb, uint32_t asic);
  /// Change Dead time
  void setDelay();
  /// Change active time
  void setDuration();
  /// Reuqires LUT of one TDC channel
  void getLUT(int chan);
  /// Get calibration status bits
  void getCalibrationStatus();
  /// Set Calibration Mask
  void setCalibrationMask(uint64_t mask);
  /// Set measurement mask
  void setMeasurementMask(uint64_t mask,uint32_t feb);
  /// Scurve
  void ScurveStep(std::string mdcc,std::string builder,int thmin,int thmax,int step);
  void Scurve(int mode,int thmin,int thmax,int step);

  void thrd_scurve();

  void configurePR2();


private:
  Febv1ConfigAccess *_tca;


  febv1::Interface* _mpi;



  
  uint32_t _run,_type;
  uint8_t _delay;
  uint8_t _duration;
  json::value _jControl;

  zmq::context_t* _context;
   bool _running;
  // Scurve parameters
  int _sc_mode,_sc_thmin,_sc_thmax,_sc_step;
  uint32_t _sc_spillon,_sc_spilloff,_sc_ntrg;
  bool _sc_running;
  std::thread* g_scurve;
};
