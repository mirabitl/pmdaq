
#include "bmpPlugin.hh"
#include <jsoncpp/json/json.h>

static LoggerPtr _logBmp(Logger::getLogger("PMDAQ_BMP"));


bmpPlugin::bmpPlugin(): _bmp(NULL){} 
void bmpPlugin::open()
{

  PMF_INFO(_logBmp," CMD: Open ");
  if (_bmp!=NULL)
    delete _bmp;
  
  
  _bmp= new bmp280();

  unlock();

}
void bmpPlugin::close()
{
  PMF_INFO(_logBmp," CMD: closing Bmp");
  if (_bmp==NULL)
    {
       PMF_ERROR(_logBmp,"No HVBmpInterface opened");
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
      PMF_ERROR(_logBmp,"No Bmp Interface opened");
       return r;
    }
   lock();
   float t,p;
   _bmp->TemperaturePressionRead(&t,&p);
   ::usleep(100000);
   unlock();
   r["pressure"]=json::value::number(p);
   r["temperature"]=json::value::number(t);
   r["status"]=json::value::string(U("READ"));


   return r;
}


void bmpPlugin::c_status(http_request m)
{
  auto par = json::value::object();
  if (_bmp==NULL)
  {
    PMF_ERROR(_logBmp,"No Bmp opened");
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
