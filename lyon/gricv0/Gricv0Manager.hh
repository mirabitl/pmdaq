#pragma once
#include "Gricv0Interface.hh"
#include "HR2ConfigAccess.hh"
#include "fsmw.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>


class Gricv0Manager : public fsmw
{
public:
  Gricv0Manager();
  ~Gricv0Manager(){;}
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
  void c_startacq(http_request m);
  void c_stopacq(http_request m);

  void c_storesc(http_request m);
  void c_loadsc(http_request m);
  void c_readsc(http_request m);
  void c_lastabcid(http_request m);
  void c_lastgtc(http_request m);


  void c_pulse(http_request m);
  void c_close(http_request m);

  void fsm_initialise(http_request m);
  void configureHR2();
  void configure(http_request m);
  void setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif=0);
  void shiftThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif=0);
  void setGain(uint16_t gain);
  void setMask(uint32_t level,uint64_t mask);
  void setChannelMask(uint16_t level,uint16_t channel,uint16_t val);
  void setAllMasks(uint64_t mask);
  void setCTEST(uint64_t mask);
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

  web::json::value build_status();

private:
  HR2ConfigAccess* _hca;
  gricv0::Interface* _mpi;




  uint32_t _run,_type;
  uint8_t _delay;
  uint8_t _duration;

  zmq::context_t* _context;
  bool _running;
 // Scurve parameters
  uint32_t _running_mode;
  int _sc_mode, _sc_thmin, _sc_thmax, _sc_step, _sc_gmin, _sc_gmax, _sc_threshold, _sc_level,_sc_channel;
  int _sc_win, _sc_ntrg;
  bool _sc_running;
  std::thread g_scurve;

};
