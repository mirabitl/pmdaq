#pragma once
#include "Wtricv1Interface.hh"
#include "HR2ConfigAccess.hh"
#include "fsmw.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>

using namespace wtricv1;

class Wtricv1Manager : public fsmw
{
public:
  Wtricv1Manager();
  ~Wtricv1Manager(){;}
  virtual void initialise();
  virtual void end();

  
  void c_status(http_request m);
  void c_reset(http_request m);
  void c_setthresholds(http_request m);
  void c_shiftthresholds(http_request m);
  void c_setpagain(http_request m);
  void c_setmask(http_request m);
  void c_setchannelmask(http_request m);
  void c_downloadDB(http_request m);
  void c_readreg(http_request m);
  void c_writereg(http_request m);
  void c_readbme(http_request m);
  void fsm_initialise(http_request m);
  void configureHR2();
  void configure(http_request m);
  void setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif=0);
  void shiftThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif=0);
  void setGain(uint16_t gain);
  void setAllMasks(uint64_t mask);
  void setCTEST(uint64_t mask);
  void setMask(uint32_t level,uint64_t mask);
  void setChannelMask(uint16_t level,uint16_t channel,uint16_t val);
  void start(http_request m);
  void stop(http_request m);
  void destroy(http_request m);
  void ScurveStep(std::string mdcc,std::string builder,int thmin,int thmax,int step);
  void thrd_scurve() ;
  void Scurve(int mode,int thmin,int thmax,int step);

  void c_scurve(http_request m);
  void GainCurveStep(std::string mdcc,std::string builder,int thmin,int thmax,int step,int thr);
  void thrd_gaincurve() ;
  void GainCurve(int mode,int thmin,int thmax,int step,int thr);
  void c_gaincurve(http_request m);
  void c_ctest(http_request m);
  void c_setrunmode(http_request m);
  web::json::value build_status();
  inline wtricv1::dataHandler* cast_data(wizcc::socketProcessor* s) {return ( wtricv1::dataHandler*) s;}
  inline wtricv1::registerHandler* cast_reg(wizcc::socketProcessor* s) {return ( wtricv1::registerHandler*) s;}
  inline wtricv1::slcHandler* cast_slc(wizcc::socketProcessor* s) {return ( wtricv1::slcHandler*) s;}
private:
  HR2ConfigAccess* _hca;
  wizcc::Controller* _mpi;



  
  uint32_t _run,_type;
  uint8_t _delay;
  uint8_t _duration;

  zmq::context_t* _context;

  bool _running;
 // Scurve parameters
  uint32_t _running_mode;

  int _sc_mode,_sc_thmin,_sc_thmax,_sc_step,_sc_gmin,_sc_gmax,_sc_threshold,_sc_level,_sc_channel;
  int _sc_win,_sc_ntrg;
  bool _sc_running;
  int _run_mode;
std::thread g_scurve;
  
};
