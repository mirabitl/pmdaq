#include "localProducer.hh"

using namespace pm;
using namespace pm::builder;

pm::builder::producer::producer() :  _running(false){;}

void pm::builder::producer::initialise()
{
  // Create the context and the merger
  _context = new zmq::context_t();

  _sources.clear();
  _stat.clear();
  _gthr.clear();

  // Register state
  this->addState("CONFIGURED");
  this->addState("RUNNING");

  // Register transitions
  this->addTransition("CONFIGURE", "CREATED", "CONFIGURED", std::bind(&pm::builder::producer::configure, this,std::placeholders::_1));
  this->addTransition("CONFIGURE", "CONFIGURED", "CONFIGURED", std::bind(&pm::builder::producer::configure, this,std::placeholders::_1));
  this->addTransition("START", "CONFIGURED", "RUNNING", std::bind(&pm::builder::producer::start, this,std::placeholders::_1));
  this->addTransition("STOP", "RUNNING", "CONFIGURED", std::bind(&pm::builder::producer::stop, this,std::placeholders::_1));
  this->addTransition("HALT", "RUNNING", "CREATED", std::bind(&pm::builder::producer::halt, this,std::placeholders::_1));
  this->addTransition("HALT", "CONFIGURED", "CREATED", std::bind(&pm::builder::producer::halt, this,std::placeholders::_1));

  // Standalone command
  this->addCommand("STATUS", std::bind(&pm::builder::producer::status, this,std::placeholders::_1));
  this->addCommand("GENERATE", std::bind(&pm::builder::producer::generate, this,std::placeholders::_1));


  for (int i=1;i<0x20000;i++) _plrand[i]= std::rand();
}
void pm::builder::producer::end()
{
  // Stop possible running thread
  _running=false;
  PMF_ERROR(_logPmex, "End method of producer "<<_gthr.size());
  for (auto it=_gthr.begin();it!=_gthr.end();it++)
    {
      std::cout<<"joining"<<std::endl;
      it->join();
    }
  _gthr.clear();
  PMF_ERROR(_logPmex, "Exiting end method of producer "<<_gthr.size());
}


void pm::builder::producer::configure(http_request m)
{
  auto par = json::value::object();
  for (std::vector<pm::pmSender*>::iterator it=_sources.begin();it!=_sources.end();it++)
    delete (*it);
  _sources.clear();
  // Clear statistics
  _stat.clear();
  // Update information if any
  // check parameters exist

   if (params().as_object().find("detid")==params().as_object().end())
    { 
      PMF_ERROR(_logPmex, "Missing detid");
      par["status"]=json::value::string(U("Missing detid "));
      Reply(status_codes::OK,par);
      return;  
    }
   if (params().as_object().find("sourceid")==params().as_object().end())
    { 
      PMF_ERROR(_logPmex, "Missing sourceid");
      par["status"]=json::value::string(U("Missing sourceid "));
      Reply(status_codes::OK,par);
      return;  
    }
   if (params().as_object().find("paysize")==params().as_object().end())
    { 
      PMF_ERROR(_logPmex, "Missing paysize");
      par["status"]=json::value::string(U("Missing paysize "));
      Reply(status_codes::OK,par);
      return;  
    }


  // declare source and create zmSenders

  int32_t det=params()["detid"].as_integer();
  _detid=det;

  json::value array_keys;uint32_t nds=0;
  for (auto it = params()["sourceid"].as_array().begin(); it != params()["sourceid"].as_array().end(); ++it)
    {
      json::value jsitem = *it;
      int32_t sid=(*it).as_integer();
      // rest as before
      PMF_INFO(_logPmex,"Creating data source "<<det<<" "<<sid);
      array_keys[nds++]=json::value::number((det<<16)|sid);
      pm::pmSender* ds= new pm::pmSender(_context,det,sid);
      //ds->connect(this->parameters()["pushdata"].asString());
      ds->autoDiscover(session(),"evb_builder","collectingPort");
      //for (uint32_t i=0;i<_mStream.size();i++)
      //	ds->connect(_mStream[i]);
      ds->collectorRegister();


      if (params().as_object().find("compress")!=params().as_object().end())
	ds->setCompress(params()["compress"].as_integer()==1);
      
      
      _sources.push_back(ds);
      _stat.insert(std::pair<uint32_t,uint32_t>((det<<16)|sid,0));
	
    }

  // Subscribe to the soft trigger source
    // Subscribe to a software trigger provider

  //
  par["status"]=json::value::string(U("CONFIGURED"));
  par["sources"]=array_keys;
  Reply(status_codes::OK,par);

}
/**
 * Thread process per zmSender: Fill and publish an event
 */
void pm::builder::producer::fillEvent(uint32_t event,uint64_t bx,pm::pmSender* ds,uint32_t eventSize)
{
  // randomize event size if not set
  if (eventSize==0)
    {
      eventSize=int(std::rand()*1.*0x10000/(RAND_MAX))-1;
      if (eventSize<10) eventSize=10;
      if (eventSize>0x10000-10) eventSize=0x10000-10;
    }
  // Payload address
  uint32_t* pld=(uint32_t*) ds->payload();
  // Copy Random data with tags at start and end of data payload
  memcpy(pld,_plrand,eventSize*sizeof(uint32_t));
  //for (int i=1;i<eventSize-1;i++) pld[i]= _plrand[i];
  pld[0]=event;
  pld[eventSize-1]=event;
  // Publish the data source
  ds->publish(bx,event,eventSize*sizeof(uint32_t));
  // Update statistics
  std::map<uint32_t,uint32_t>::iterator its=_stat.find((ds->buffer()->detectorId()<<16)|ds->buffer()->dataSourceId());
  if (its!=_stat.end())
    its->second=event;
	
}
/**
 * Standalone thread with no external trigger to publish continously data
 */
void pm::builder::producer::streamdata(pm::pmSender *ds)
{
  uint32_t last_evt=0,event=0;
  uint64_t bx=0;
  std::srand(std::time(0));
  PMF_INFO(_logPmex," Start of Thread of: "<<ds->buffer()->dataSourceId()<<" is running "<<_event<<" events and status is "<<_running);
  while (_running)
    {
      ::usleep(5000);
      //::sleep(2);
      if (event%1000==0)
        ::sleep(1);
      if (!_running) break;
      //if (event == last_evt && event!=0) continue;
      if (event%100==0)
	PMF_INFO(_logPmex," Thread of: "<<ds->buffer()->dataSourceId()<<" is running "<<event<<" events and status is "<<_running);
      // Just fun 
      // Create a dummy buffer of fix length depending on source id and random data
      // 
      uint32_t psi=1024;
      if (params().as_object().find("paysize")!=params().as_object().end())
      	psi=params()["paysize"].as_integer(); 
      this->fillEvent(event,bx,ds,0);
      last_evt=event;
      event++;
      bx++;
    }
  PMF_INFO(_logPmex," Thread of: "<<ds->buffer()->dataSourceId()<<" is exiting after "<<last_evt<<"events");
}
/**
 * Transition from CONFIGURED to RUNNING, starts one thread per data source in standalone mode
 */
void pm::builder::producer::start(http_request m)
{

  _event=0;
  _running=true;

  // Clear the stats
  for (auto it=_stat.begin();it!=_stat.end();it++) it->second=0;
  _gthr.clear();
  // Check mode
  // Standalone all datasources are publishing continously events of fixed size
  for (std::vector<pm::pmSender*>::iterator ids=_sources.begin();ids!=_sources.end();ids++)
    {
      //(*ids)->collectorRegister();
      _gthr.push_back(std::thread(std::bind(&pm::builder::producer::streamdata, this,(*ids))));
      ::usleep(500000);
    }
  auto par = json::value::object();
  par["status"]=json::value::string(U("STARTED"));
  Reply(status_codes::OK,par);

      
}
      
/**
 * RUNNING to CONFIGURED, Stop threads 
 */
void pm::builder::producer::stop(http_request m)
{
  
  

  
  // Stop running
  _running=false;
  ::sleep(1);

  for (auto it=_gthr.begin();it!=_gthr.end();it++)
    {
      std::cout<<"joining"<<std::endl;
      it->join();
    }
  _gthr.clear();
  auto par = json::value::object();
  par["status"]=json::value::string(U("STOPPED"));
  Reply(status_codes::OK,par);

}
/**
 * go back to CREATED, call stop and destroy sources
 */
void pm::builder::producer::halt(http_request m)
{
  
  

  if (_running)
    {
        _running=false;
	::sleep(1);
	
	for (auto it=_gthr.begin();it!=_gthr.end();it++)
	  {
	    std::cout<<"joining"<<std::endl;
	    it->join();
	  }
    }

  std::cout<<"Destroying"<<std::endl;
  //stop data sources
  for (std::vector<pm::pmSender*>::iterator it=_sources.begin();it!=_sources.end();it++)
    delete (*it);
  _sources.clear();
  _gthr.clear();

  auto par = json::value::object();
  par["status"]=json::value::string(U("HALTED"));
  Reply(status_codes::OK,par);

    
}

/**
 * Standalone command GENERATE, unused but it might be used to generate data to 
 * configure the hardware
 */
void pm::builder::producer::generate(http_request m)
{

    // Initialise random data packet
  for (int i=1;i<0x20000;i++) _plrand[i]= std::rand();
  //response["answer"]="DONE";
  auto par = json::value::object();
  par["status"]=json::value::string(U("done"));
  Reply(status_codes::OK,par);
}
/**
 * Standalone command LIST to get the statistics of each data source 
 */
void pm::builder::producer::status(http_request m)
{
  auto par = json::value::object(); 

  json::value array_keys;uint32_t nds=0;
  for (std::map<uint32_t,uint32_t>::iterator it=_stat.begin();it!=_stat.end();it++)
    {
      json::value js;
      js["detid"]=json::value::number((it->first>>16)&0xFFFF);
      js["sourceid"]=json::value::number(it->first&0xFFFF);
      js["event"]=json::value::number(it->second);
      array_keys[nds++]=js;
      //std::cout<<it->first<<" "<<it->second<<std::endl;
      //std::cout<<js<<std::endl;
      
    }
  std::cout<<array_keys<<std::endl;
  par["detector"]=json::value::number(_detid);
  par["zmSenders"]=array_keys;
  Reply(status_codes::OK,par);
}
extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  handlerPlugin* loadProcessor(void)
    {
      return (new  pm::builder::producer);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin* obj)
    {
      delete obj;
    }
}
