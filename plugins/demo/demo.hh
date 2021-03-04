#pragma once

#include "fsmw.hh"
#include "stdafx.hh"
#include <thread>


class demo : public fsmw
  {
  public:

    /**
       \brief Constructor
       \param dire is the directory path for writing
     */
    demo();
    

    /**
       \brief Start a run
       \param run is the run number
       \details It creates the file SMM_dayMonthYear_HourMinuteSeconde_run.dat
     */
    virtual void initialise();
    virtual void end();

    void readEvent();
    void configure(http_request message);
    void start(http_request message);
    void status(http_request message);
    void stop(http_request message);


  private:
    std::thread _thr;
    uint32_t _event;
    bool _started;
  };


