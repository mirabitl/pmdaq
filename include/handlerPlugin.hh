#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <string>
#include "stdafx.hh"
    /**
     \class handlerPlugin
     \brief Purely virtual interface to process data

     \details A handlerPlugin is accesible as a pluggin it must implement 2 methods
     - loadProcessor
     - deleteProcessor

     \b Example

     extern "C" 
     {

     zdaq::handlerPlugin* loadStore(void)
     {
     return (new myprocessor);
     }

     void deleteStore(zdaq::handlerPlugin* obj)
     {
     delete obj;
     }

     }

     \author    Laurent Mirabito
     \version   1.0
     \date      January 2019
     \copyright GNU Public License.
  */
  class handlerPlugin
  {
  public:
    virtual std::vector<std::string> getPaths(std::string query)=0;
    virtual void processRequest(http_request& message)=0;
    virtual void terminate()=0;
    
  };


