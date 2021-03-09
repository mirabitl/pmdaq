#pragma once
#include <iostream>

#include <string.h>
#include<stdio.h>
#include <mongoc.h>

#include "monitor.hh"
#include "stdafx.hh"


using namespace std;
#include <sstream>


  class mgStore : public monitoring::monStore
  {
  public:
    mgStore();
    virtual void connect();
    virtual void store(std::string loc,std::string hw,uint32_t ti,web::json::value status);
    virtual  void loadParameters(web::json::value params);
    // Access to the interface
  private:
    //zdaq::fsm* _fsm;
    web::json::value _params;
    std::string _dbaccount,_dbname;
    mongoc_uri_t *_uri;
    mongoc_client_t *_client;
    mongoc_database_t *_database;
    mongoc_collection_t *_measitems;
  };

