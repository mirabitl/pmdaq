#pragma once

#include "fsmw.hh"
#include "stdafx.hh"
#include <thread>
#include "pluginInfo.hh"
namespace monitoring {
  /**
     \class monstore
     \brief Purely virtual interface to process data

     \details A zmonStore is accesible as a pluggin it must implement 2 methods
     - loadProcessor
     - deleteProcessor

     \b Example

     extern "C" 
     {

     monitoring::monStore* loadStore(void)
     {
     return (new myprocessor);
     }

     void deleteStore(zdaq::zmonStore* obj)
     {
     delete obj;
     }

     }

     \author    Laurent Mirabito
     \version   1.0
     \date      January 2019
     \copyright GNU Public License.
  */
  class monStore
  {
  public:
    virtual void connect()=0;
    virtual void store(std::string loc,std::string hw,uint32_t ti,web::json::value status)=0;
    /**
       \brief Parameter setting interface
       \param params is a Json::value where parameters are stored
     */
    virtual  void loadParameters(web::json::value params)=0;
  };


  
    class supervisor : public fsmw {
    public:
      
      supervisor();

      
      virtual void initialise();
      virtual void end();

      // Monitoring virtuals
      virtual void registerCommands();
      virtual web::json::value status();
      virtual void open();
      virtual void close();
      virtual std::string hardware();
      // Implemented part
      void monitor();
      void configure(http_request message);
      void start(http_request message);
      void stop(http_request message);
      void halt(http_request message);
      void c_status(http_request message);
      void registerStore(std::string name);
    private:

      bool _running,_readout;
      uint32_t _period;

      //std::vector<monitoring::monStore* > _stores;
      std::vector<pluginInfo<monitoring::monStore> > _stores;
      std::thread _thr;


    };
};


