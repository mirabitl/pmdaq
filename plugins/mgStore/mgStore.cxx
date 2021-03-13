
#include "mgStore.hh"
mgStore::mgStore(): _dbaccount(""),_uri(0),_client(0),_database(0),_measitems(0) {} 
void mgStore::loadParameters(web::json::value params)
{
    _params=params["mgstore"];
  
}

void mgStore::connect()
{
   bson_t *command, reply;
  bson_error_t error;
  char *str;
  std::string login("");
  if  (_params.as_object().find("login")!=_params.as_object().end())
    {
      login=_params["login"].as_string();
    }
  else
    {
      char* wp=getenv("MGDBLOGIN");
      if (wp!=NULL)  
	login=std::string(wp);
      else
	{
	  PM_ERROR(_logPdaq," MGDBLOGIN is not set");
	  return;
	}
    }
   std::size_t current, previous = 0;
  current = login.find("@");
  std::vector<string> cont;
  while (current != std::string::npos) {
    cont.push_back(login.substr(previous, current - previous));
    previous = current + 1;
    current = login.find("@", previous);
  }

  cont.push_back(login.substr(previous, current - previous));
  std::string userinfo=cont[0];
  std::stringstream hostinfo("");
  hostinfo<<"mongodb://"<<cont[1];
  std::string uri_string=hostinfo.str();
  _dbname=cont[2];
  /*
    userinfo=login.split("@")[0]
    hostinfo=login.split("@")[1]
    dbname=login.split("@")[2]
    user=userinfo.split("/")[0]
    pwd=userinfo.split("/")[1]
    host=hostinfo.split(":")[0]
    port=int(hostinfo.split(":")[1])

  */
  // mongoc internal
  mongoc_init ();

  //find uri and dbname

  
  _uri = mongoc_uri_new_with_error (uri_string.c_str(), &error);
  if (!_uri) {
    fprintf (stderr,
	     "failed to parse URI: %s\n"
	     "error message:       %s\n",
	     uri_string.c_str(),
	     error.message);
    return;
  }

  /*
   * Create a new client instance
   */
  _client = mongoc_client_new_from_uri (_uri);
  if (!_client) {
    return;
  }

  /*
   * Register the application name so we can track it in the profile logs
   * on the server. This can also be done from the URI (see other examples).
   */
  mongoc_client_set_appname (_client, "connect-slowcontrol");

  /*
   * Get a handle on the database "db_name" and collection "coll_name"
   */
  _database = mongoc_client_get_database (_client,_dbname.c_str());
  _measitems = mongoc_client_get_collection (_client,_dbname.c_str(),"MONITORED_ITEMS");

  /*
   * Do work. This example pings the database, prints the result as JSON and
   * performs an insert
   */
  command = BCON_NEW ("ping", BCON_INT32 (1));

  int32_t retval = mongoc_client_command_simple (
						 _client, "admin", command, NULL, &reply, &error);

  if (!retval) {
    fprintf (stderr, "%s\n", error.message);
    PM_ERROR(_logPdaq," Cannot ping to dbaccount :"<<error.message);
    return;
  }

  str = bson_as_json (&reply, NULL);
  PM_INFO(_logPdaq," Ping to "<<login<<" => "<<std::string(str));
  bson_destroy (&reply);
  bson_destroy (command);
  bson_free (str);
}

void mgStore::store(std::string loc,std::string hw,uint32_t ti,web::json::value status)
{
  bson_error_t error;
  //fprintf(stderr,"Entry time %d \n",time(0));  
  auto  mondoc=web::json::value::object();
  std::stringstream spath("");
  spath<<","<<loc<<","<<hw<<",";
  mondoc["path"]=json::value::string(U(spath.str()));
  mondoc["ctime"]=json::value::number(ti);
  mondoc["status"]=status;
  //Json::FastWriter fastWriter;
  //std::string scont= fastWriter.write(mondoc);

  std::stringstream sobj("");
  web::json::value v=mondoc;
  sobj<<v;
  bson_t      *doc;
  char        *string;


  doc = bson_new_from_json ((const uint8_t *)sobj.str().c_str(), -1, &error);

  if (!doc) {
    fprintf (stderr, "%s\n", error.message);
    return;
  }
  if (!mongoc_collection_insert_one (_measitems, doc, NULL, NULL, &error)) {
    fprintf (stderr, "%s\n", error.message);
  }
  bson_destroy (doc);
  //fprintf(stderr,"End time %d \n",time(0));  
}
extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  monitoring::monStore* loadStore(void)
    {
      return (new mgStore);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteStore(monitoring::monStore*  obj)
    {
      delete obj;
    }
}

