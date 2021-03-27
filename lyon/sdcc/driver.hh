#pragma once
#include <iomanip>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdint.h>

// hardware access
#include "FtdiUsbDriver.hh"

#include <ftdi.h>
#include <string.h>

#include <ftdi.h>
#include <string.h>
#include "stdafx.hh"
#include "utils.hh"
static LoggerPtr _logSdcc(Logger::getLogger("PMDAQ_SDCC"));


#define FLUSH_TO_FILE 		// when flush function is sed , data are sent to a file

namespace sdcc
{
  class driver : public mdcc::FtdiUsbDriver
  {
  public:
    
    driver(	char * deviceIdentifier)     ;
    
    virtual ~driver()     ;
    
    
    // register access
    
    
    int32_t CCCCommandDIFReset(void)    																				;
    int32_t CCCCommandBCIDReset(void)     																			;
    int32_t CCCCommandStartAcquisitionAuto(void)     														;
    int32_t CCCCommandRamfullExt(void)     																			;
    int32_t CCCCommandTriggerExt(void)     																			;
    int32_t CCCCommandStopAcquisition(void)     																;
    int32_t CCCCommandDigitalReadout(void)    																	;
    int32_t CCCCommandTrigger(void)    																					;
    int32_t CCCCommandClearMemory(void)    																			;
    int32_t CCCCommandStartSingleAcquisition(void)    													;
    int32_t CCCCommandPulseLemo(void)    																			 	;
    int32_t CCCCommandRazChannel(void)  																		   	;
    //		int32_t CCCCommandNoCommand(void)   																				;
    int32_t CCCCommandResetCCC(void)    																		 		;
    int32_t CCCCommandSpillOn(void)    																					;
    int32_t CCCCommandSpillOff(void)    																					;
	
  protected:
    struct ftdi_context theFtdi;
    uint32_t timeOut;
	
				
  };
};

