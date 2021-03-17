#ifndef _C4I_MANAGER_HH
#define _C4I_MANAGER_HH
#include "C4iInterface.hh"
#include "HR2ConfigAccess.hh"
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

class C4iManager : public zdaq::baseApplication
{
public:
  C4iManager(std::string name);
  ~C4iManager(){;}

  void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setthresholds(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setpagain(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setmask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setchannelmask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_readreg(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_writereg(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_readbme(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void initialise(zdaq::fsmmessage* m);
  void configureHR2();
  void configure(zdaq::fsmmessage* m);
  void setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif=0);
  void setGain(uint16_t gain);
  void setAllMasks(uint64_t mask);
  void setCTEST(uint64_t mask);
  void setMask(uint32_t level,uint64_t mask);
  void setChannelMask(uint16_t level,uint16_t channel,uint16_t val);
  void start(zdaq::fsmmessage* m);
  void stop(zdaq::fsmmessage* m);
  void destroy(zdaq::fsmmessage* m);
  void ScurveStep(fsmwebCaller* mdcc,fsmwebCaller* builder,int thmin,int thmax,int step);
  void thrd_scurve() ;
  void Scurve(int mode,int thmin,int thmax,int step);
  fsmwebCaller* findMDCC(std::string appname);
  void c_scurve(Mongoose::Request &request, Mongoose::JsonResponse &response);
private:
  lydaq::HR2ConfigAccess* _hca;
  lydaq::c4i::Interface* _mpi;



  zdaq::fsmweb* _fsm;
  uint32_t _run,_type;
  uint8_t _delay;
  uint8_t _duration;

  zmq::context_t* _context;

  bool _running;
 // Scurve parameters
  int _sc_mode,_sc_thmin,_sc_thmax,_sc_step;
  bool _sc_running;

  
};
};
#endif
