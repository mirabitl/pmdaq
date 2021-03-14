
#include "hihPlugin.hh"
#include <jsoncpp/json/json.h>
static LoggerPtr _logHih(Logger::getLogger("PMDAQ_HIH"));


using namespace lydaq;
hihPlugin::hihPlugin(): _hih(NULL){} 
void hihPlugin::open()
{

  PMF_INFO(_logHih," CMD: Open ");

  std::cout<<"calling open "<<std::endl;
  if (_hih!=NULL)
    delete _hih;
  
 
  _hih= new lydaq::hih8000();

}
void hihPlugin::close()
{
  PMF_INFO(_logHih," CMD: closing Hih");
  if (_hih==NULL)
    {
       PMF_ERROR(_logHih,"No HVHihInterface opened");
       return;
    }
  
  delete _hih;
  _hih=NULL;
}

web::json::value hihPlugin::status()
{
  auto r=json::value::object();
  r["name"]=json::value::string(U(this->hardware()));
  r["status"]=json::value::string(U("UNKNOWN"));
   if (_hih==NULL)
    {
      PMF_ERROR(_logHih,"No Hih Interface opened");
       return r;
    }
   _hih->Read();
   r["temperature0"]=json::value::number(_hih->temperature(0));
   r["temperature1"]=json::value::number(_hih->temperature(1));
   r["humidity0"]=json::value::number(_hih->humidity(0));
   r["humidity1"]=json::value::number(_hih->humidity(1));

   r["status"]=json::value::string(U("READ"));


   return r;
}


void hihPlugin::c_status(http_request m)
{
  auto par = json::value::object();
  if (_hih==NULL)
  {
    PMF_ERROR(_logHih,"No Hih opened");
    par["status"]=json::value::string(U("No Device"));
    Reply(status_codes::OK,par);

    return;
  }

  par["status"] =this->status();
  Reply(status_codes::OK,par);

}


void hihPlugin::registerCommands() 
{

 addCommand("HIH_STATUS",std::bind(&hihPlugin::c_status,this,std::placeholders::_1));

   
}



extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new hihPlugin);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
