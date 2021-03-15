#ifndef _FEBV1_MANAGER_HH
#define _FEBV1_MANAGER_HH
#include "Febv1Interface.hh"
#include "TdcConfigAccess.hh"
#include "baseApplication.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "ReadoutLogger.hh"

namespace lydaq
{

class Febv1Manager : public zdaq::baseApplication
{
public:
  Febv1Manager(std::string name);
  ~Febv1Manager(){;}

    /// INITIALISE  handler
  void initialise(zdaq::fsmmessage *m);
  /// CONFIGURE  handler
  void configure(zdaq::fsmmessage *m);
  /// START  handler
  void start(zdaq::fsmmessage *m);
  /// STOP  handler
  void stop(zdaq::fsmmessage *m);
  /// DESTROY  handler
  void destroy(zdaq::fsmmessage *m);
  /// job log command  (obsolete)
  void c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// STATUS Command handler
  void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// DIFLIST Command handler
  void c_diflist(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SET6BDAC Command handler
  void c_cal6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_set6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETMASK Command handler
  void c_setMask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETMODE Command handler
  void c_setMode(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETDELAY Command handler
  void c_setDelay(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETDURATION Command handler
  void c_setDuration(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETVTHTIME Command Handler
  void c_setvthtime(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETONEVTHTIME Command handler
  void c_set1vthtime(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// DOWNLOADDB Command handler
  void c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_asics(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// GETLUT command handler
  void c_getLUT(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// TDC Calibration  command handlers
  void c_getCalibrationStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setCalibrationMask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setMeasurementMask(Mongoose::Request &request, Mongoose::JsonResponse &response);

  /// PR2 SCurve
  void c_scurve(Mongoose::Request &request, Mongoose::JsonResponse &response);

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
  void ScurveStep(fsmwebCaller* m,fsmwebCaller* b,int thmin,int thmax,int step);
  void Scurve(int mode,int thmin,int thmax,int step);
  fsmwebCaller* findMDCC(std::string name);
  void thrd_scurve();

  void configurePR2();


private:
  lydaq::TdcConfigAccess *_tca;


  lydaq::febv1::Interface* _mpi;



  zdaq::fsmweb* _fsm;
  uint32_t _run,_type;
  uint8_t _delay;
  uint8_t _duration;
Json::Value _jControl;

  zmq::context_t* _context;
   bool _running;
  // Scurve parameters
  int _sc_mode,_sc_thmin,_sc_thmax,_sc_step;
  bool _sc_running;
};
};
#endif
