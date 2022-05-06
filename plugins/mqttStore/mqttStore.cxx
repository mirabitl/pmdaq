
#include "mqttStore.hh"
#include "utils.hh"

//#include "graphite-client.h"
static LoggerPtr _logMqtt(Logger::getLogger("PMDAQ_GRAPHITE"));

using namespace monitoring;
mqttStore::mqttStore() : _mosq(NULL) {} 
void mqttStore::loadParameters(web::json::value params)
{
    _params=params["mqttstore"];
}


void mqttStore::mqtt_setup(){

  int keepalive = 0;
  bool clean_session = true;
  
  mosquitto_lib_init();
  _mosq = mosquitto_new(NULL, clean_session, NULL);
  if(!_mosq){
		fprintf(stderr, "Error: Out of memory.\n");
		exit(1);
	}
  
  //mosquitto_log_callback_set(mosq, mosq_log_callback);
  
  if(mosquitto_connect(_mosq,_params["host"].as_string().c_str() ,_params["port"].as_integer(), keepalive)){
		fprintf(stderr, "Unable to connect.\n");
		exit(1);
	}
  //int loop = mosquitto_loop_start(mosq);
  //if(loop != MOSQ_ERR_SUCCESS){
  //  fprintf(stderr, "Unable to start loop: %i\n", loop);
  //  exit(1);
  //}
}

int  mqttStore::mqtt_send(const char* topic,float val,uint32_t ti){
 
  std::stringstream ss;
  ss<<val;
  const char* msg=ss.str().c_str();
  return mosquitto_publish(_mosq, NULL, topic, strlen(msg),msg, 0, 0);
}

void mqttStore::connect()
{

  this->mqtt_setup();
}

void mqttStore::store(std::string loc,std::string hw,uint32_t ti,web::json::value status)
{
  
  std::stringstream spath("");


  // BMP
  if (status["name"].as_string().compare("BMP")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/T";
      mqtt_send(spath.str().c_str(),status["temperature"].as_double(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/P";
      mqtt_send(spath.str().c_str(),status["pressure"].as_double(),ti);
      return;
    }
  // HIH8000
  if (status["name"].as_string().compare("HIH")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/H0";
      mqtt_send(spath.str().c_str(),status["humidity0"].as_double(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/T0";
      mqtt_send(spath.str().c_str(),status["temperature0"].as_double(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/H1";
      mqtt_send(spath.str().c_str(),status["humidity1"].as_double(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/T1";
      mqtt_send(spath.str().c_str(),status["temperature1"].as_double(),ti);
      
      return;
    }
  // Genesys
  if (status["name"].as_string().compare("GENESYS")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/VSET";
      mqtt_send(spath.str().c_str(),status["vset"].as_double(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/VOUT";
      mqtt_send(spath.str().c_str(),status["vout"].as_double(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/IOUT";
      mqtt_send(spath.str().c_str(),status["iout"].as_double(),ti);


      float ist=0;
      if (status["status"].as_string().compare("ON")==0) ist=1;
	
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/ON";
      mqtt_send(spath.str().c_str(),1,ti);
      return;
    }
  // Genesys
  if (status["name"].as_string().compare("ZUP")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/VSET";
      mqtt_send(spath.str().c_str(),status["vset"].as_double(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/VOUT";
      mqtt_send(spath.str().c_str(),status["vout"].as_double(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/ISET";
      mqtt_send(spath.str().c_str(),status["iset"].as_double(),ti);

      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/PWR";
      mqtt_send(spath.str().c_str(),status["pwrstatus"].as_integer(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/IOUT";
      mqtt_send(spath.str().c_str(),status["iout"].as_double(),ti);


      float ist=0;
      if (status["status"].as_string().compare("ON")==0)
	{
	  ist=1;
	  //PMF_WARN(_logMqtt,"status "<<status["status"].as_string()<<" "<<ist);
	}
	
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"/"<<hw<<"/ON";
      mqtt_send(spath.str().c_str(),ist,ti);
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
	  spath<<loc<<"/"<<hw<<"/"<<jsch["id"].as_integer()<<"/VSET";
	  mqtt_send(spath.str().c_str(),jsch["vset"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"/"<<hw<<"/"<<jsch["id"].as_integer()<<"/VOUT";
	  mqtt_send(spath.str().c_str(),jsch["vout"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"/"<<hw<<"/"<<jsch["id"].as_integer()<<"/ISET";
	  mqtt_send(spath.str().c_str(),jsch["vset"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"/"<<hw<<"/"<<jsch["id"].as_integer()<<"/IOUT";
	  //std::cout<<"IOUT"<<jsch["iout"].as_double()<<std::endl;
	  mqtt_send(spath.str().c_str(),jsch["iout"].as_double()*1E6,ti);

	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"/"<<hw<<"/"<<jsch["id"].as_integer()<<"/RAMP";
	  mqtt_send(spath.str().c_str(),jsch["rampup"].as_double(),ti);

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
	  spath<<loc<<"/"<<hw<<"/"<<jsch["slot"].as_integer()<<"/"<<jsch["channel"].as_integer()<<"/VSET";
	  mqtt_send(spath.str().c_str(),jsch["vset"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"/"<<hw<<"/"<<jsch["slot"].as_integer()<<"/"<<jsch["channel"].as_integer()<<"/VOUT";
	  mqtt_send(spath.str().c_str(),jsch["vout"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"/"<<hw<<"/"<<jsch["slot"].as_integer()<<"/"<<jsch["channel"].as_integer()<<"/ISET";
	  mqtt_send(spath.str().c_str(),jsch["vset"].as_double(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"/"<<hw<<"/"<<jsch["slot"].as_integer()<<"/"<<jsch["channel"].as_integer()<<"/IOUT";
	  mqtt_send(spath.str().c_str(),jsch["iout"].as_double(),ti);

	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"/"<<hw<<"/"<<jsch["slot"].as_integer()<<"/"<<jsch["channel"].as_integer()<<"/RAMP";
	  mqtt_send(spath.str().c_str(),jsch["rampup"].as_double(),ti);

	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"/"<<hw<<"/"<<jsch["slot"].as_integer()<<"/"<<jsch["channel"].as_integer()<<"/STATUS";
	  mqtt_send(spath.str().c_str(),jsch["status"].as_double(),ti);

	 }
      return;
    }
}
extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  monitoring::monStore* loadStore(void)
    {
      return (new mqttStore);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteStore(monitoring::monStore* obj)
    {
      delete obj;
    }
}

