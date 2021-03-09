#include "baseServer.hh"
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;
#include <dlfcn.h>
 
baseServer::baseServer(utility::string_t urla) : m_listener(urla),req(0),_url(urla) {
    
 m_listener.support(methods::GET, std::bind(&baseServer::handle_get_or_post, this, std::placeholders::_1));
 m_listener.support(methods::POST, std::bind(&baseServer::handle_get_or_post, this, std::placeholders::_1));
 _url.assign(urla);
 LOG4CXX_INFO(_logPdaq,__PRETTY_FUNCTION__<< "Listenig on: " << this->url());
}
 
void baseServer::handle_get_or_post(http_request message)
{
  LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<< "Method: " << message.method() );
  LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<< "URL: " <<_url);
  LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<< "Absolute host: " <<message.absolute_uri().host() );
  LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<< "URI: " << http::uri::decode(message.relative_uri().path()) );
  LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<< "Query: " << http::uri::decode(message.relative_uri().query()));
  req++;
  LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<<req);
  //message.reply(status_codes::OK, "1 ACCEPTED\n");
  LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<<U("FullPath: ")<<U(uri::decode(message.relative_uri().path())));
  // Registratin
  if (uri::decode(message.relative_uri().path()).compare("/REGISTER")==0)
    {
      auto query = uri::split_query(uri::decode(message.relative_uri().query()));
      LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<<uri::decode(message.relative_uri().query()));
      for (auto it2 = query.begin(); it2 != query.end(); it2++)
	{
	  if (it2->first.compare("name")==0)
	    registerPlugin(it2->second,uri::decode(message.relative_uri().query()));
	  LOG4CXX_INFO(_logPdaq,__PRETTY_FUNCTION__<< U("Registering Query") << U(" ")<< it2->first << U(" ") << it2->second );
	}
      auto rep = json::value::string(U(uri::decode(message.relative_uri().query())));
      message.reply(status_codes::OK,rep);
    }
  else if (uri::decode(message.relative_uri().path()).compare("/REMOVE")==0)
      {
      auto querym = uri::split_query(uri::decode(message.relative_uri().query()));

      std::string _p_session("");
      std::string _p_name("");
      uint32_t _p_instance=9999;
      for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
	{
	  LOG4CXX_INFO(_logPdaq,__PRETTY_FUNCTION__<< U("Query") << U(" ")
		<< it2->first << U(" ") << it2->second );
	  if (it2->first.compare("session")==0)
	    _p_session.assign(it2->second);
	  if (it2->first.compare("name")==0)
	    _p_name.assign(it2->second);
	  if (it2->first.compare("instance")==0)
	    _p_instance=std::stoi(it2->second);
	}
      if (_p_instance ==9999)
	{
	  LOG4CXX_ERROR(_logPdaq,__PRETTY_FUNCTION__<<" No instance given for remove");
	  message.reply(status_codes::BadRequest, "{\"error\":\"Invalid instance\"}");return;}
      if (_p_name.length() ==0)
	{
	  LOG4CXX_ERROR(_logPdaq,__PRETTY_FUNCTION__<<" No name given for remove");
	  message.reply(status_codes::BadRequest, "{\"error\":\"Invalid name\"}");return;}
      if (_p_session.length() ==0)
	{
	  LOG4CXX_ERROR(_logPdaq,__PRETTY_FUNCTION__<<" No session given for remove");
	  message.reply(status_codes::BadRequest, "{\"error\":\"Invalid session\"}");return;}
      
      std::stringstream sb;
      sb<<"/"<<_p_session<<"/"<<_p_name<<"/"<<_p_instance<<"/";
      uint32_t lsb=sb.str().length();
      // Now find in pluggins all instance and remove it from the map
      for (auto it2 = _plugins.cbegin(); it2 != _plugins.cend() /* not hoisted */; /* no increment */)
	{
	  if (it2->first.compare(0,lsb,sb.str())==0)
	    {
	      LOG4CXX_INFO(_logPdaq,__PRETTY_FUNCTION__<<"removing "<<it2->first);
	      it2->second->terminate();
	      _plugins.erase(it2++);    // or "it = m.erase(it)" since C++11
	    }
	  else
	    {
	      ++it2;
	    }
	}
      auto rep = json::value::string(U(sb.str()));
      message.reply(status_codes::OK,rep);
    }
  else if (uri::decode(message.relative_uri().path()).compare("/SERVICES")==0)
    {
      LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<<"On rentre dans SERVICES");
       auto par = json::value();
       int np=0;
       for (auto it=_plugins.begin();it!=_plugins.end();it++)
	 {par[np]=json::value::string(U(it->first));np++;
	   LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<<it->first);}
       message.reply(status_codes::OK,par);
    }
  else
    {
      auto itp=_plugins.find(uri::decode(message.relative_uri().path()));
      if (itp!=_plugins.end())
	itp->second->processRequest(message);
      else
	{
	  LOG4CXX_ERROR(_logPdaq,__PRETTY_FUNCTION__<<" Invalid request path");
	message.reply(status_codes::BadRequest, "{\"error\":\"Invalid PATH\"}");
	}
    }

  return;
      
#ifdef DEBUG   
 auto paths = uri::split_path(uri::decode(message.relative_uri().path()));
 LOG4CXX_INFO(_logPdaq,__PRETTY_FUNCTION__<<paths.size());
 for (auto it1 = paths.begin(); it1 != paths.end(); it1++)
   {
     LOG4CXX_INFO(_logPdaq,__PRETTY_FUNCTION__<< U("Path") << U(" ")
	   << *it1 );
   }

 auto query = uri::split_query(uri::decode(message.relative_uri().query()));
 for (auto it2 = query.begin(); it2 != query.end(); it2++)
   {
     LOG4CXX_INFO(_logPdaq,__PRETTY_FUNCTION__<< U("Query") << U(" ")
	   << it2->first << U(" ") << it2->second );
   }

 auto queryItr = query.find(U("request"));
 //message.reply(status_codes::OK, "2 ACCEPTED\n");
 if (queryItr!=query.end())
   {
     message.reply(status_codes::OK, "22 ACCEPTED\n");
   utility::string_t request = queryItr->second;
   LOG4CXX_INFO(_logPdaq,__PRETTY_FUNCTION__<< U("Request") << U(" ") << request );
   }
 //message.reply(status_codes::OK, "23 ACCEPTED\n");
 
 message.reply(status_codes::OK, "3 ACCEPTED\n");
#endif
}

void baseServer::registerPlugin(std::string name,std::string query)
{
  std::stringstream s;
  s << "lib" << name << ".so";
  LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<<"1"<<s.str());
  void *library = dlopen(s.str().c_str(), RTLD_NOW);
  LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<<"2 open"<<s.str());
  //printf("%s %x \n",dlerror(),(unsigned int) library);
  //LOG4CXX_INFO(_logPdaq,__PRETTY_FUNCTION__<<s.str()<<" Error " << dlerror() << " Library open address " << std::hex << library << std::dec<<endl;
  
  // Get the loadFilter function, for loading objects
  handlerPlugin *(*create)();
  create = (handlerPlugin * (*)()) dlsym(library, "loadProcessor");
  LOG4CXX_DEBUG(_logPdaq,__PRETTY_FUNCTION__<<" create"<<s.str());
  ucout<<" Error " << dlerror() << " file " << s.str() << " loads to processor address " << std::hex << create << std::dec<<endl;
  //printf("%s %x \n",dlerror(),(unsigned int) create);
  // printf("%s lods to %x \n",s.str().c_str(),(unsigned int) create);
  //void (*destroy)(Filter*);
  // destroy = (void (*)(Filter*))dlsym(library, "deleteFilter");
  // Get a new filter object
  handlerPlugin *a = (handlerPlugin *)create();
  LOG4CXX_INFO(_logPdaq,__PRETTY_FUNCTION__<<" Create called"<<s.str());
  a->setUrl(url());
  for (auto x:a->getPaths(query) )
    {
      std::pair<std::string,handlerPlugin*> p(x,a);
      _plugins.insert(p);
    }
  

}

