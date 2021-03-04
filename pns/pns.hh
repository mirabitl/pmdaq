#pragma once

#include "handlerPlugin.hh"
#include "stdafx.hh"
#include <thread>


class pns : public handlerPlugin
  {
  public:

    /**
       \brief Constructor
       \param dire is the directory path for writing
     */
    pns();
    

    /**
       \brief Start a run
       \param run is the run number
       \details It creates the file SMM_dayMonthYear_HourMinuteSeconde_run.dat
     */
    virtual std::vector<std::string> getPaths(std::string query);
    virtual void processRequest(http_request& message);
    virtual void terminate();
    // PNS web services
    void list(http_request message);
    void update(http_request message);
    void remove(http_request message);
    void purge(http_request message);
    web::json::value registered();

    std::string host(){return _host;}
    uint32_t port(){return _port;}
    std::string path(){return _basePath;}

  private:
    std::map<std::string,std::string> _services;

  protected:
    std::string _host;
    uint32_t _port;
    std::string _basePath;    
  };


