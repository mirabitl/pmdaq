#pragma once
#include "fsmw.hh"
#include "pmSender.hh"
#include "stdafx.hh"


namespace pm
{
  namespace builder {
    class producer : public fsmw {
    public:
      producer();
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
      void generate(http_request message);
      void stop(http_request message);
      void halt(http_request message);

      void incrementEvent() {_event++;_bx++;}
      bool running(){return _running;}
      void fillEvent(uint32_t event,uint64_t bx,pm::pmSender* ds,uint32_t eventSize);
      
      void streamdata(pm::pmSender*);

    private:
      std::vector<pm::pmSender*> _sources;
      std::map<uint32_t,uint32_t> _stat;
      uint32_t _detid;
      bool _running,_readout;
      zmq::context_t* _context;
      std::vector<std::thread> _gthr;
      uint32_t _event;
      uint64_t _bx;
      uint32_t _plrand[0x20000];
      
    };
  };
};



