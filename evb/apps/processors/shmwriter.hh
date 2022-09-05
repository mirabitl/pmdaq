#pragma once
#include <stdint.h>
#include <stdlib.h>
#include "pmBuffer.hh"
#include "pmMerger.hh"
#include <vector>
#include <map>
#include <string>
#include "stdafx.hh"
namespace pm
{
  class shmwriter : public pm::evbprocessor
  {
  public:
    /**
           \brief Constructor
           \param dire is the directory path for writing
         */
    shmwriter(std::string dire = "/dev/shm/shmwriter");

    /**
       \brief Start a run
       \param run is the run number
       \details It creates the file SMM_dayMonthYear_HourMinuteSeconde_run.dat
     */
    virtual void start(uint32_t run);

    virtual void stop(); ///< Close the run and the file

    /**
       \brief Process the list of zda::buffer

       \param key  is the trigger ID
       \param dss is the list of pmdaq::buffer
     */
    virtual void processEvent(uint32_t key, std::vector<pm::buffer *> dss);

    /**
       \brief Store the run header

       \param header a (256 max) vector of int to be stored

       \details It copies the vectore in a pmdaq::buffer with detectorId 255 and eventId 0 and store it on disk
     */
    virtual void processRunHeader(std::vector<uint32_t> header);

    /**
       \brief Set parameters with a JSON descriptor

       \params a Json::Value object conating parameters values
     */
    virtual void loadParameters(json::value params);
    void store(uint32_t detid, uint32_t sourceid, uint32_t eventid, uint64_t bxid, void *ptr, uint32_t size, std::string destdir);
    static void ls(std::string sourcedir, std::vector<std::string> &res);
    static void pull(std::string name,pm::buffer* buf,std::string sourcedir);
  private:
    std::string _filepath;
    uint32_t _run, _event;
    bool _started;
  };
};
