#include "baseServer.hh"
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;
#include <dlfcn.h>

baseServer::baseServer(utility::string_t urla) : m_listener(urla), req(0), _url(urla)
{

  m_listener.support(methods::GET, std::bind(&baseServer::handle_get_or_post, this, std::placeholders::_1));
  m_listener.support(methods::POST, std::bind(&baseServer::handle_get_or_post, this, std::placeholders::_1));
  _url.assign(urla);
  PM_INFO(_logPdaq, "Listenig on: " << this->url());
}

void baseServer::handle_get_or_post(http_request message)
{
  PM_DEBUG(_logPdaq, "Method: " << message.method());
  PM_DEBUG(_logPdaq, "URL: " << _url);
  PM_DEBUG(_logPdaq, "Absolute host: " << message.absolute_uri().host());
  PM_DEBUG(_logPdaq, "URI: " << http::uri::decode(message.relative_uri().path()));
  PM_DEBUG(_logPdaq, "Query: " << http::uri::decode(message.relative_uri().query()));
  req++;
  PM_DEBUG(_logPdaq, req);
  //message.reply(status_codes::OK, "1 ACCEPTED\n");
  PM_DEBUG(_logPdaq, U("FullPath: ") << U(uri::decode(message.relative_uri().path())));
  // Registratin
  if (uri::decode(message.relative_uri().path()).compare("/REGISTER") == 0)
  {
    auto query = uri::split_query(uri::decode(message.relative_uri().query()));
    PM_DEBUG(_logPdaq, uri::decode(message.relative_uri().query()));
    for (auto it2 = query.begin(); it2 != query.end(); it2++)
    {
      if (it2->first.compare("name") == 0)
        registerPlugin(it2->second, uri::decode(message.relative_uri().query()));
      PM_INFO(_logPdaq, U("Registering Query") << U(" ") << it2->first << U(" ") << it2->second);
    }
    auto rep = json::value::string(U(uri::decode(message.relative_uri().query())));
    message.reply(status_codes::OK, rep);
  }
  else if (uri::decode(message.relative_uri().path()).compare("/REMOVE") == 0)
  {
    auto querym = uri::split_query(uri::decode(message.relative_uri().query()));

    std::string _p_session("");
    std::string _p_name("");
    uint32_t _p_instance = 9999;
    for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
      PM_INFO(_logPdaq, U("Query") << U(" ")
                                   << it2->first << U(" ") << it2->second);
      if (it2->first.compare("session") == 0)
        _p_session.assign(it2->second);
      if (it2->first.compare("name") == 0)
        _p_name.assign(it2->second);
      if (it2->first.compare("instance") == 0)
        _p_instance = std::stoi(it2->second);
    }
    if (_p_instance == 9999)
    {
      PM_ERROR(_logPdaq, " No instance given for remove");
      message.reply(status_codes::BadRequest, "{\"error\":\"Invalid instance\"}");
      return;
    }
    if (_p_name.length() == 0)
    {
      PM_ERROR(_logPdaq, " No name given for remove");
      message.reply(status_codes::BadRequest, "{\"error\":\"Invalid name\"}");
      return;
    }
    if (_p_session.length() == 0)
    {
      PM_ERROR(_logPdaq, " No session given for remove");
      message.reply(status_codes::BadRequest, "{\"error\":\"Invalid session\"}");
      return;
    }

    std::stringstream sb;
    sb << "/" << _p_session << "/" << _p_name << "/" << _p_instance << "/";
    uint32_t lsb = sb.str().length();
    auto rep = json::value::string(U(sb.str()));
    #ifdef OLDPLUGIN
        for (auto it2 = _plugins.cbegin(); it2 != _plugins.cend() /* not hoisted */; /* no increment */)
    {
      if (it2->first.compare(0, lsb, sb.str()) == 0)
      {
        PM_INFO(_logPdaq, "removing " << it2->first);
        //it2->second->terminate();
	it2->second->terminate();
	PM_INFO(_logPdaq, "terminated " << it2->first);
      
        _plugins.erase(it2++); // or "it = m.erase(it)" since C++11
      }
      else
      {
        ++it2;
      }
    }

    #else
    // Now find in pluggins all instance and remove it from the map
    pluginInfo<handlerPlugin>* pluginfo=NULL;
    for (auto it2 = _plugins.cbegin(); it2 != _plugins.cend() /* not hoisted */; /* no increment */)
    {
      if (it2->first.compare(0, lsb, sb.str()) == 0)
      {
        PM_INFO(_logPdaq, "removing " << it2->first);
        //it2->second->terminate();
	if (it2->second->isAlived())
	  {
	    it2->second->ptr()->terminate();
	    PM_INFO(_logPdaq, "terminated " << it2->first);
	    it2->second->close();
	    PM_INFO(_logPdaq, "closed " << it2->first);
	    pluginfo=it2->second;
	  }
        _plugins.erase(it2++); // or "it = m.erase(it)" since C++11
      }
      else
      {
        ++it2;
      }
    }
    PM_INFO(_logPdaq, "deleting pluginfo ");
    if (pluginfo!=NULL)
      {delete pluginfo;
    	pluginfo=NULL;}
    #endif
    
    PM_INFO(_logPdaq, "deleted pluginfo ");
    message.reply(status_codes::OK, rep);
    PM_INFO(_logPdaq, "Message replied ");
  }
  else if (uri::decode(message.relative_uri().path()).compare("/SERVICES") == 0)
  {
    PM_DEBUG(_logPdaq, "On rentre dans SERVICES");
    auto par = json::value();
    int np = 0;
    for (auto it = _plugins.begin(); it != _plugins.end(); it++)
    {
      par[np] = json::value::string(U(it->first));
      np++;
      PM_DEBUG(_logPdaq, it->first);
    }
    message.reply(status_codes::OK, par);
  }
  else
  {
    auto itp = _plugins.find(uri::decode(message.relative_uri().path()));
    if (itp != _plugins.end())
#ifdef OLDPLUGIN
      itp->second->processRequest(message);
#else
      itp->second->ptr()->processRequest(message);
#endif
    else
    {
      PM_ERROR(_logPdaq, " Invalid request path");
      message.reply(status_codes::BadRequest, "{\"error\":\"Invalid PATH\"}");
    }
  }

  return;

#ifdef DEBUG
  auto paths = uri::split_path(uri::decode(message.relative_uri().path()));
  PM_INFO(_logPdaq, paths.size());
  for (auto it1 = paths.begin(); it1 != paths.end(); it1++)
  {
    PM_INFO(_logPdaq, U("Path") << U(" ")
                                << *it1);
  }

  auto query = uri::split_query(uri::decode(message.relative_uri().query()));
  for (auto it2 = query.begin(); it2 != query.end(); it2++)
  {
    PM_INFO(_logPdaq, U("Query") << U(" ")
                                 << it2->first << U(" ") << it2->second);
  }

  auto queryItr = query.find(U("request"));
  //message.reply(status_codes::OK, "2 ACCEPTED\n");
  if (queryItr != query.end())
  {
    message.reply(status_codes::OK, "22 ACCEPTED\n");
    utility::string_t request = queryItr->second;
    PM_INFO(_logPdaq, U("Request") << U(" ") << request);
  }
  //message.reply(status_codes::OK, "23 ACCEPTED\n");

  message.reply(status_codes::OK, "3 ACCEPTED\n");
#endif
}

void baseServer::registerPlugin(std::string name, std::string query)
{
  #ifdef OLDPLUGIN
  std::stringstream s;
  s << "lib" << name << ".so";
  PM_INFO(_logPdaq, "1" << s.str());
  void *library = dlopen(s.str().c_str(), RTLD_NOW);
  PM_INFO(_logPdaq, "2 open" << s.str());
  //printf("%s %x \n",dlerror(),(unsigned int) library);
  //PM_INFO(_logPdaq,s.str()<<" Error " << dlerror() << " Library open address " << std::hex << library << std::dec<<endl;

  // Get the loadFilter function, for loading objects
  handlerPlugin *(*create)();
  create = (handlerPlugin * (*)()) dlsym(library, "loadProcessor");
  PM_INFO(_logPdaq, " create" << s.str());
  ucout << " Error " << dlerror() << " file " << s.str() << " loads to processor address " << std::hex << create << std::dec << endl;
  //printf("%s %x \n",dlerror(),(unsigned int) create);
  // printf("%s lods to %x \n",s.str().c_str(),(unsigned int) create);
  //void (*destroy)(Filter*);
  // destroy = (void (*)(Filter*))dlsym(library, "deleteFilter");
  // Get a new filter object
  handlerPlugin *a = (handlerPlugin *)create();
  PM_INFO(_logPdaq, " Create called" << s.str());
  a->setUrl(url());
  for (auto x : a->getPaths(query))
  {
    std::pair<std::string, handlerPlugin *> p(x, a);
    _plugins.insert(p);
  }
  #else
  pluginInfo<handlerPlugin>* p_info = new  pluginInfo<handlerPlugin>(name,"loadProcessor","deleteProcessor");
  p_info->ptr()->setUrl(url());
  for (auto x : p_info->ptr()->getPaths(query))
  {
    std::pair<std::string, pluginInfo<handlerPlugin>* > p(x, p_info);
    _plugins.insert(p);
  }
  #endif
}
