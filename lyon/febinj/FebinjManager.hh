#ifndef _FebInjServer_h

#define _FebInjServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "stdafx.hh"
#include "utils.hh"
#include "fsmw.hh"

#include "febinj.hh"

using namespace std;
#include <sstream>


  class FebinjManager : public fsmw
  {
  public:
    virtual void initialise();
    virtual void end();
    enum TRIGGER_TYPE {INTSINGLE=1,INTMULTI=2,EXTSINGLE=4,EXTMULTI=8,SOFT=16,BYPASS=32};
    FebinjManager();
    // Transition
    void configure(http_request m);
    void destroy(http_request m);
    // Commande
    void c_set_mask(http_request m);
    void c_set_trigger_source(http_request m);
    void c_software_trigger(http_request m);
    void c_internal_trigger(http_request m);
    void c_pause_external_trigger(http_request m);
    void c_resume_external_trigger(http_request m);
    void c_set_number_of_trigger(http_request m);
    void c_set_delay(http_request m);
    void c_set_duration(http_request m);
    void c_set_pulse_height(http_request m);
    
  private:
    febinj::board* _inj;
  };

