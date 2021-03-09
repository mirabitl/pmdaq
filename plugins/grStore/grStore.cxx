
#include "grStore.hh"
#include "graphite-client.h"
using namespace monitoring;
grStore::grStore() {} 
void grStore::loadParameters(web::json::value params)
{
    _params=params["grstore"];
}

void grStore::connect()
{

  graphite_init(_params["host"].as_string().c_str(),_params["port"].as_integer());
}

void grStore::store(std::string loc,std::string hw,uint32_t ti,web::json::value status)
{
  
  std::stringstream spath("");


  // BMP
  if (status["name"].as_string().compare("BMP")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".T";
      graphite_send_plain(spath.str().c_str(),status["temperature"].as_double(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".P";
      graphite_send_plain(spath.str().c_str(),status["pressure"].as_double(),ti);
      return;
    }
  // HIH8000
  if (status["name"].as_string().compare("HIH")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".H0";
      graphite_send_plain(spath.str().c_str(),status["humidity0"].as_double(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".T0";
      graphite_send_plain(spath.str().c_str(),status["temperature0"].as_double(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".H1";
      graphite_send_plain(spath.str().c_str(),status["humidity1"].as_double(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".T1";
      graphite_send_plain(spath.str().c_str(),status["temperature1"].as_double(),ti);
      
      return;
    }
  // Genesys
  if (status["name"].as_string().compare("GENESYS")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".VSET";
      graphite_send_plain(spath.str().c_str(),status["vset"].as_double(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".VOUT";
      graphite_send_plain(spath.str().c_str(),status["vout"].as_double(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".IOUT";
      graphite_send_plain(spath.str().c_str(),status["iout"].as_double(),ti);


      float ist=0;
      if (status["status"].as_string().compare("ON")==0) ist=1;
	
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".ON";
      graphite_send_plain(spath.str().c_str(),1,ti);
      return;
    }
  // Genesys
  if (status["name"].as_string().compare("ZUP")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".VSET";
      graphite_send_plain(spath.str().c_str(),status["vset"].as_double(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".VOUT";
      graphite_send_plain(spath.str().c_str(),status["vout"].as_double(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".ISET";
      graphite_send_plain(spath.str().c_str(),status["iset"].as_double(),ti);

      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".PWR";
      graphite_send_plain(spath.str().c_str(),status["pwrstatus"].as_integer(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".IOUT";
      graphite_send_plain(spath.str().c_str(),status["iout"].as_double(),ti);


      float ist=0;
      if (status["status"].as_string().compare("ON")==0)
	{
	  ist=1;
	  //LOG4CXX_WARN(_logPdaq,"status "<<status["status"].as_string()<<" "<<ist);
	}
	
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".ON";
      graphite_send_plain(spath.str().c_str(),ist,ti);
      return;
    }

  // WIENER
  if (status["name"].as_string().compare("ISEG")==0)
    {
      for (auto it = status["channels"].as_array().begin(); it != status["channels"].as_array().end(); ++it)
	{
	  json::object jsch = (*it).as_object();
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["id"].as_integer()<<".VSET";
	  graphite_send_plain(spath.str().c_str(),jsch["vset"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["id"].as_integer()<<".VOUT";
	  graphite_send_plain(spath.str().c_str(),jsch["vout"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["id"].as_integer()<<".ISET";
	  graphite_send_plain(spath.str().c_str(),jsch["vset"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["id"].as_integer()<<".IOUT";
	  //std::cout<<"IOUT"<<jsch["iout"].as_double()<<std::endl;
	  graphite_send_plain(spath.str().c_str(),jsch["iout"].as_double()*1E6,ti);

	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["id"].as_integer()<<".RAMP";
	  graphite_send_plain(spath.str().c_str(),jsch["rampup"].as_double(),ti);

	 }
      return;
    }

  // SYX27
  if (status["name"].as_string().compare("SYX27")==0)
    {
      for (auto it = status["channels"].as_array().begin(); it != status["channels"].as_array().end(); ++it)
	{
	  json::object jsch = (*it).as_object();
		  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].as_integer()<<"."<<jsch["channel"].as_integer()<<".VSET";
	  graphite_send_plain(spath.str().c_str(),jsch["vset"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].as_integer()<<"."<<jsch["channel"].as_integer()<<".VOUT";
	  graphite_send_plain(spath.str().c_str(),jsch["vout"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].as_integer()<<"."<<jsch["channel"].as_integer()<<".ISET";
	  graphite_send_plain(spath.str().c_str(),jsch["vset"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].as_integer()<<"."<<jsch["channel"].as_integer()<<".IOUT";
	  graphite_send_plain(spath.str().c_str(),jsch["iout"].as_double(),ti);

	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].as_integer()<<"."<<jsch["channel"].as_integer()<<".RAMP";
	  graphite_send_plain(spath.str().c_str(),jsch["rampup"].as_double(),ti);

	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].as_integer()<<"."<<jsch["channel"].as_integer()<<".STATUS";
	  graphite_send_plain(spath.str().c_str(),jsch["status"].as_double(),ti);

	 }
      return;
    }
}
extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  monitoring::monStore* loadStore(void)
    {
      return (new grStore);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteStore(monitoring::monStore* obj)
    {
      delete obj;
    }
}

