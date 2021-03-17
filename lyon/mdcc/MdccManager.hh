#pragma once
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "fsmw.hh"
#include "MdccHandler.hh"

using namespace std;
#include <sstream>
#include "stdafx.hh"
using namespace mdcc;
class MdccManager : public fsmw
  {
  public:
    MdccManager();
    virtual void initialise();
    virtual void end();
    
    void fsm_initialise(http_request m);
    void destroy(http_request m);

    
    void doOpen(std::string s);
    MdccHandler* getMdccHandler(){  return _mdcc;}

    void c_status(http_request m);
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
    void c_setregister(http_request m);
    void c_getregister(http_request m);
    void c_setexternaltrigger(http_request m);
  private:
    mdcc::MdccHandler* _mdcc;
};

