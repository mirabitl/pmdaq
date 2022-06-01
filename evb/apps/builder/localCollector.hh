#pragma once
#include "fsmw.hh"
#include "pmMerger.hh"
#include "stdafx.hh"
#include <chrono>
#include "mqtt/client.h"
#include "mqtt/message.h"
#include "utils.hh"


namespace pm
{
  namespace builder {
    class collector : public fsmw {
    public:
      collector();
  /**
       \brief Start a run
       \param run is the run number
       \details It creates the file SMM_dayMonthYear_HourMinuteSeconde_run.dat
     */
      virtual void initialise();
      virtual void end();
      
      void configure(http_request message);
      void start(http_request message);
      void status(http_request message);
      void setheader(http_request message);
      void purge(http_request message);
      void stop(http_request message);
      void halt(http_request message);

      void monitorLoop(){;}
      void publish(std::string topic,std::string value);

    private:
      pm::pmMerger* _merger;
      bool _running,_readout;
      zmq::context_t* _context;
      std::shared_ptr<mqtt::client> _cli;
      std::thread g_loop;
      uint32_t _period;
      bool _looping;
      std::string _brokerid;
    };
  };
};



