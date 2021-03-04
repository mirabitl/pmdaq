#include "baseServer.hh"
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;
#include <dlfcn.h>
 
baseServer::baseServer(utility::string_t url) : m_listener(url),req(0),_url(url)
{
 m_listener.support(methods::GET, std::bind(&baseServer::handle_get_or_post, this, std::placeholders::_1));
 m_listener.support(methods::POST, std::bind(&baseServer::handle_get_or_post, this, std::placeholders::_1));
}
 
void baseServer::handle_get_or_post(http_request message)
{
  ucout << "Method: " << message.method() << std::endl;
  ucout << "URL: " <<_url<<std::endl;
  ucout << "Absolute host: " <<message.absolute_uri().host() <<std::endl;
  ucout << "URI: " << http::uri::decode(message.relative_uri().path()) << std::endl;
  ucout << "Query: " << http::uri::decode(message.relative_uri().query()) << std::endl << std::endl;
  req++;
  ucout<<req<<std::endl;
  //message.reply(status_codes::OK, "1 ACCEPTED\n");
  ucout<<U("FullPath: ")<<U(uri::decode(message.relative_uri().path()))<<std::endl;
  // Registratin
  if (uri::decode(message.relative_uri().path()).compare("/REGISTER")==0)
    {
      auto query = uri::split_query(uri::decode(message.relative_uri().query()));
      ucout<<uri::decode(message.relative_uri().query())<<std::endl;
      for (auto it2 = query.begin(); it2 != query.end(); it2++)
	{
	  if (it2->first.compare("name")==0)
	    registerPlugin(it2->second,uri::decode(message.relative_uri().query()));
	  ucout << U("Query") << U(" ")<< it2->first << U(" ") << it2->second << std::endl;
	}
      message.reply(status_codes::OK);
    }
  else if (uri::decode(message.relative_uri().path()).compare("/REMOVE")==0)
      {
      auto querym = uri::split_query(uri::decode(message.relative_uri().query()));

      std::string _p_session("");
      std::string _p_name("");
      uint32_t _p_instance=9999;
      for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
	{
	  ucout << U("Query") << U(" ")
		<< it2->first << U(" ") << it2->second << std::endl;
	  if (it2->first.compare("session")==0)
	    _p_session.assign(it2->second);
	  if (it2->first.compare("name")==0)
	    _p_name.assign(it2->second);
	  if (it2->first.compare("instance")==0)
	    _p_instance=std::stoi(it2->second);
	}
      if (_p_instance ==9999)
	{
	  message.reply(status_codes::BadRequest, "{\"error\":\"Invalid instance\"}");return;}
      if (_p_name.length() ==0)
	{
	  message.reply(status_codes::BadRequest, "{\"error\":\"Invalid name\"}");return;}
      if (_p_session.length() ==0)
	{
	  message.reply(status_codes::BadRequest, "{\"error\":\"Invalid session\"}");return;}
      
      std::stringstream sb;
      sb<<"/"<<_p_session<<"/"<<_p_name<<"/"<<_p_instance<<"/";
      uint32_t lsb=sb.str().length();
      // Now find in pluggins all instance and remove it from the map
      for (auto it2 = _plugins.cbegin(); it2 != _plugins.cend() /* not hoisted */; /* no increment */)
	{
	  if (it2->first.compare(0,lsb,sb.str())==0)
	    {
	      ucout<<"removing "<<it2->first<<std::endl;
	      it2->second->terminate();
	      _plugins.erase(it2++);    // or "it = m.erase(it)" since C++11
	    }
	  else
	    {
	      ++it2;
	    }
	}

      message.reply(status_codes::OK);
    }
  else if (uri::decode(message.relative_uri().path()).compare("/SERVICES")==0)
    {
      ucout<<"On rentre dans SERVICES"<<std::endl;
       auto par = json::value();
       int np=0;
       for (auto it=_plugins.begin();it!=_plugins.end();it++)
	 {par[np]=json::value::string(U(it->first));np++;
	   ucout<<it->first<<std::endl;}
       message.reply(status_codes::OK,par);
    }
  else
    {
      auto itp=_plugins.find(uri::decode(message.relative_uri().path()));
      if (itp!=_plugins.end())
	itp->second->processRequest(message);
      else
	message.reply(status_codes::BadRequest, "{\"error\":\"Invalid PATH\"}");

    }

  return;
      
#ifdef DEBUG   
 auto paths = uri::split_path(uri::decode(message.relative_uri().path()));
 ucout<<paths.size()<<std::endl;
 for (auto it1 = paths.begin(); it1 != paths.end(); it1++)
   {
     ucout << U("Path") << U(" ")
	   << *it1 << std::endl;
   }

 auto query = uri::split_query(uri::decode(message.relative_uri().query()));
 for (auto it2 = query.begin(); it2 != query.end(); it2++)
   {
     ucout << U("Query") << U(" ")
	   << it2->first << U(" ") << it2->second << std::endl;
   }

 auto queryItr = query.find(U("request"));
 //message.reply(status_codes::OK, "2 ACCEPTED\n");
 if (queryItr!=query.end())
   {
     message.reply(status_codes::OK, "22 ACCEPTED\n");
   utility::string_t request = queryItr->second;
   ucout << U("Request") << U(" ") << request << std::endl;
   }
 //message.reply(status_codes::OK, "23 ACCEPTED\n");
 
 message.reply(status_codes::OK, "3 ACCEPTED\n");
#endif
}

void baseServer::registerPlugin(std::string name,std::string query)
{
  std::stringstream s;
  s << "lib" << name << ".so";
  ucout<<"1"<<s.str()<<std::endl;
  void *library = dlopen(s.str().c_str(), RTLD_NOW);
  ucout<<"2 open"<<s.str()<<std::endl;
  //printf("%s %x \n",dlerror(),(unsigned int) library);
  //ucout<<s.str()<<" Error " << dlerror() << " Library open address " << std::hex << library << std::dec<<endl;
  
  // Get the loadFilter function, for loading objects
  handlerPlugin *(*create)();
  create = (handlerPlugin * (*)()) dlsym(library, "loadProcessor");
  ucout<<"3 create"<<s.str()<<std::endl;
  //ucout<<" Error " << dlerror() << " file " << s.str() << " loads to processor address " << std::hex << create << std::dec<<endl;
  //printf("%s %x \n",dlerror(),(unsigned int) create);
  // printf("%s lods to %x \n",s.str().c_str(),(unsigned int) create);
  //void (*destroy)(Filter*);
  // destroy = (void (*)(Filter*))dlsym(library, "deleteFilter");
  // Get a new filter object
  handlerPlugin *a = (handlerPlugin *)create();
  ucout<<"4 called"<<s.str()<<std::endl;
  a->setUrl(url());
  for (auto x:a->getPaths(query) )
    {
      std::pair<std::string,handlerPlugin*> p(x,a);
      _plugins.insert(p);
    }
  

}

