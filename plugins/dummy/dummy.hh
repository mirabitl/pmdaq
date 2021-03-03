#pragma once

#include "handlerPlugin.hh"
#include "stdafx.hh"
#include <thread>

typedef std::function<void (http_request&)> CMDFunctor;
class dummy : public handlerPlugin
  {
  public:

    /**
       \brief Constructor
       \param dire is the directory path for writing
     */
    dummy();
    

    /**
       \brief Start a run
       \param run is the run number
       \details It creates the file SMM_dayMonthYear_HourMinuteSeconde_run.dat
     */
    virtual std::vector<std::string> getPaths(std::string query);
    virtual void processRequest(http_request& message);
    virtual void initialise();
    virtual void terminate();

    void readEvent();
    void addCommand(std::string s,CMDFunctor f);
    void configure(http_request message);
    void start(http_request message);
    void status(http_request message);
    void stop(http_request message);
    void list(http_request message);

  private:
    std::string _basePath;
    std::map<std::string,CMDFunctor> _commands;
  protected:
    std::string _p_session;
    std::string _p_name;
    uint32_t _p_instance;
    std::thread _thr;
    uint32_t _event;
    bool _started;
  };


