
#include "bmpPlugin.hh"
#include <jsoncpp/json/json.h>
using namespace lydaq;
bmpPlugin::bmpPlugin(): _bmp(NULL){} 
void bmpPlugin::open()
{

  PM_INFO(_logPdaq," CMD: Open ");
  if (_bmp!=NULL)
    delete _bmp;
  
  
#ifdef BMP183  
  _bmp= new lydaq::BMP183();
#else
  _bmp= new lydaq::BMP280();
#endif


}
void bmpPlugin::close()
{
  PM_INFO(_logPdaq," CMD: closing Bmp");
  if (_bmp==NULL)
    {
       PM_ERROR(_logPdaq,"No HVBmpInterface opened");
       return;
    }
  
  delete _bmp;
  _bmp=NULL;
}

web::json::value bmpPlugin::status()
{
  auto r=json::value::object();
  r["name"]=json::value::string(U(this->hardware()));
  r["status"]=json::value::string(U("UNKNOWN"));
   if (_bmp==NULL)
    {
      PM_ERROR(_logPdaq,"No Bmp Interface opened");
       return r;
    }
#ifdef BMP183
   r["pressure"]=json::value::number(_bmp->BMP183PressionRead());
   r["temperature"]=json::value::number(_bmp->BMP183TemperatureRead());
#else
   float t,p;
   _bmp->TemperaturePressionRead(&t,&p);
   r["pressure"]=json::value::number(p);
   r["temperature"]=json::value::number(t);
#endif
   r["status"]=json::value::string(U("READ"));


   return r;
}


void bmpPlugin::c_status(http_request m)
{
  auto par = json::value::object();
  if (_bmp==NULL)
  {
    PM_ERROR(_logPdaq,"No Bmp opened");
    par["status"]=json::value::string(U("No Device"));
    Reply(status_codes::OK,par);

    return;
  }

  par["status"] =this->status();
  Reply(status_codes::OK,par);

}


void bmpPlugin::registerCommands() 
{

 addCommand("BMP_STATUS",std::bind(&bmpPlugin::c_status,this,std::placeholders::_1));

   
}



extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new bmpPlugin);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
