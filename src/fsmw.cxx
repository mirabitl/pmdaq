#include "fsmw.hh"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
static LoggerPtr _logFsm(Logger::getLogger("PMDAQ_FSM"));

std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
    // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
  }
  return elems;
}

bool checkpns(int port=8888)
{
  struct hostent *h;
  struct sockaddr_in servaddr;

  int sd, rval;
  std::string address;
  char* wp=getenv("PNS_NAME");
  if (wp!=NULL)
    address.append(std::string(wp));
  else
    address.append("localhost");

  h=gethostbyname(address.c_str());
  if (h== NULL) {
    PM_ERROR(_logFsm,"Error when using gethostbyname " <<address);
    std::exit(-1);
  }
        // std::cout << inet_ntoa(*((struct in_addr *)h->h_addr)) << std::endl;

  sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sd == -1) {
    PM_ERROR(_logFsm,"Error when trying to create socket !");
    return false;
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  
  memcpy(&servaddr.sin_addr, h -> h_addr, h -> h_length);
      
  rval = connect(sd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  if (rval == -1) {
    PM_ERROR(_logFsm,"Port "<<port<<" is closed for: "<<address);
    close(sd);
    return false;
  }

  else {
    PM_DEBUG(_logFsm,"Opened port "<<port <<" :" << inet_ntoa(*((struct in_addr *)h->h_addr)));
    close(sd);
    return true;
  }
    

}

fsmw::fsmw() : _host(""),_port(0),_p_session(""),_p_name(""),_p_instance(0),_state("CREATED") {
  _states.clear();
  _transitions.clear();
  _commands.clear();
  addState("CREATED");
  addState("DEAD");
  setState("CREATED");
}

std::vector<std::string> fsmw::getPaths(std::string query)
{
  if (_port==0)
    {
      //ucout<<url() <<std::endl;
      // remove http:// and trailing / so 
      auto v=split(url().substr(7,url().length()-8),':');
      _host.assign(v[0]);
      _port=std::stoi(v[1]);

      PM_INFO (_logFsm," HOST found "<<_host<<" Port:"<<_port);

    }

  
  auto querym = uri::split_query(query);
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    {
     PM_DEBUG(_logFsm, U("Query") << U(" ")
		   << it2->first << U(" ") << it2->second);
      if (it2->first.compare("session")==0)
	_p_session.assign(it2->second);
      if (it2->first.compare("name")==0)
	_p_name.assign(it2->second);
      if (it2->first.compare("instance")==0)
	_p_instance=std::stoi(it2->second);
      if (it2->first.compare("params")==0)
	{
	  std::error_code  errorCode;
	  auto jval=web::json::value::parse(std::string(it2->second),errorCode);
	  _params=jval;
	  PM_DEBUG(_logFsm,"Parameters "<<_params);
	}
    }
  std::stringstream sb;
  sb<<"/"<<_p_session<<"/"<<_p_name<<"/"<<_p_instance<<"/";
  _basePath=sb.str();
  _params["path"]=json::value::string(U(_basePath));
  this->initialise();
  this->addCommand("INFO",std::bind(&fsmw::info,this,std::placeholders::_1));
  this->addCommand("PARAMS",std::bind(&fsmw::getparams,this,std::placeholders::_1));
  this->addCommand("SETPARAMS",std::bind(&fsmw::setparams,this,std::placeholders::_1));
  // Construct list of commands
  std::vector<std::string> v;
  for (auto it=_commands.begin();it!=_commands.end();it++)
    {v.push_back(it->first);}
  for (auto it=_transitions.begin();it!=_transitions.end();it++)
    {v.push_back(it->first);}
  this->publishState();
  return v;
}

void fsmw::processRequest(http_request& message)
{
  _response.set_status_code(status_codes::OK);
  json::value jrep=json::value::string(U("Not Set"));
  _response.set_body(jrep);

  auto icf=_commands.find(uri::decode(message.relative_uri().path()));
  if (icf!=_commands.end())
    {
      icf->second(message);
      message.reply(_response);
    }
  else
    {
      
      auto itf=_transitions.find(uri::decode(message.relative_uri().path()));
      if (itf==_transitions.end())
	{
	  json::value jrep;
	  jrep["status"]=json::value::string(U("FAILED"));
	  std::stringstream s0;
	  s0.str(std::string());  
	  s0<<uri::decode(message.relative_uri().path()) << "not found in transitions/commands list ";
	  jrep["comment"]=json::value::string(U(s0.str()));
	  message.reply(status_codes::NotImplemented, jrep);
	}
      else
	{
	  // loop on vector of transition
	  std::vector<fsmTransition> &vp=itf->second;
	  for (auto ift=vp.begin();ift!=vp.end();ift++)
	    if (ift->initialState().compare(_state)==0)
	      {
		
#define DEBUG	       
#ifdef DEBUG
		std::cout<<"calling callback"<<ift->finalState()<<"\n";
#endif
		ift->callback()(message);
#ifdef DEBUG
		std::cout<<"Message processed\n";
#endif
		_state=ift->finalState();
		this->publishState();
		std::cout<<"publish called\n";
		message.reply(_response);
		return;
	      }
	  // No initialState corresponding to _state
	  //if (it->second.initialState().compare(_state)!=0)
	  //  {
	  json::value jrep;
	  jrep["status"]=json::value::string(U("FAILED"));
	  std::stringstream s0;
	  s0.str(std::string());  
	  s0<<_state << "invalid intial state  ";
	  jrep["comment"]=json::value::string(U(s0.str()));
	  message.reply(status_codes::MethodNotAllowed, jrep);
	  
	  return;
	}
    }

}
void fsmw::addCommand(std::string s,CMDFunctor f)
{
  std::string cmd=_basePath+s;
  auto itf=_commands.find(cmd);
  if (itf==_commands.end())
    {
      std::pair<std::string,CMDFunctor> p(cmd,f);
      _commands.insert(p);
    }

}
void fsmw::initialise()
{
}
void fsmw::end(){}
void fsmw::terminate()
{
  this->end();
  setState("DEAD");
  publishState();
  
}
void fsmw::info(http_request message)
{
  auto par = json::value();
  int np=0;
  for (auto it=_commands.begin();it!=_commands.end();it++)
    {par[np]=json::value::string(U(it->first));np++;}

  auto rep = json::value();
  rep["COMMANDS"]=par;
  rep["TRANSITIONS"]=transitionsList();
  rep["ALLOWED"]=allowList();
  rep["STATE"]=json::value::string(U(_state));
  message.reply(status_codes::OK,rep);
}
void fsmw::getparams(http_request message)
{
  message.reply(status_codes::OK,_params);
}
void fsmw::setparams(http_request message)
{
  PMF_INFO (_logFsm,uri::decode(message.relative_uri().query()));
  auto querym = uri::split_query(uri::decode(message.relative_uri().query()));
  for (auto it2 = querym.begin(); it2 != querym.end(); it2++)
    if (it2->first.compare("params")==0)
      {
	std::error_code  errorCode;
	auto p=web::json::value::parse(std::string(it2->second),errorCode);
	for(auto iter = p.as_object().begin(); iter != p.as_object().end(); ++iter)
	  _params[iter->first]=iter->second;
	PM_DEBUG(_logFsm,"Parameters sets "<<_params);
      }
    
  message.reply(status_codes::OK,_params);
}

void fsmw::addState(std::string statename) 
{
  _states.push_back(statename);
}
  
void fsmw::addTransition(std::string s,std::string istate,std::string fstate,CMDFunctor f)
{
  std::string cmd=_basePath+s;
  //auto itf=_commands.find(cmd);
  if (_transitions.find(cmd)!=_transitions.end())
    {
      std::map<std::string,std::vector<fsmTransition> >::iterator iv=_transitions.find(cmd);
      bool found=false;
      for (std::vector<fsmTransition>::iterator it=iv->second.begin();it!=iv->second.end();it++)
	{
	  if (it->initialState().compare(istate)==0)
	    {/*already stor */ return;}
	}
      fsmTransition t(istate,fstate,f);;
      iv->second.push_back(t);

    }
  else
    {
      fsmTransition t(istate,fstate,f);
      std::vector<fsmTransition> vp;
      vp.push_back(t);
      std::pair<std::string,std::vector<fsmTransition> > p(cmd,vp);
      _transitions.insert(p);
    }
}

void fsmw::setState(std::string s){ _state=s;}
std::string fsmw::state(){return _state;}
void fsmw::publishState() {

  if (!checkpns())
    {  PMF_FATAL(_logFsm,"Invalid DNS Cannot publish state");return;}
  PM_DEBUG(_logFsm,"Entering publishState");

  
  utility::string_t address = U("http://");
  char* wp=getenv("PNS_NAME");
  if (wp!=NULL)      address.append(std::string(wp));
  else
    address.append("localhost");
  address.append(":8888");
  //ucout<<"Build address :"<<std::endl;
  http::uri uri = http::uri(address);
  web::http::client::http_client_config cfg; cfg.set_timeout(std::chrono::seconds(1));
  http_client client(http::uri_builder(uri).append_path(U("/PNS/UPDATE")).to_uri(),cfg);

  utility::ostringstream_t buf;
  buf << U("?host=")<<U(host())
      << U("&port=")<<U(port())
      << U("&path=")<<U(path())
      << U("&state=")<<U(state());
  
  http_response  response = client.request(methods::GET, buf.str()).get();
  PM_DEBUG(_logFsm,"reponse " <<response.to_string());
}
web::json::value fsmw::transitionsList()
{
  web::json::value jrep;
  int np=0;
  for( std::map<std::string,std::vector<fsmTransition> >::iterator it=_transitions.begin();it!=_transitions.end();it++)
    {
      //web::json::value jc;
      //jc["name"]=json::value::string(U(it->first));
      jrep[np]=json::value::string(U(it->first));
      np++;
    }
  return jrep;
}
web::json::value fsmw::allowList()
{
  int np=0;
  web::json::value jrep;
  for( std::map<std::string,std::vector<fsmTransition> >::iterator it=_transitions.begin();it!=_transitions.end();it++)
    {
      bool allowed=false;
      std::vector<fsmTransition> &vp=it->second;
      for (std::vector<fsmTransition>::iterator ift=vp.begin();ift!=vp.end();ift++)
	if (ift->initialState().compare(_state)==0)
	  {allowed=true;break;}
      if (allowed)
	{
	  jrep[np]=json::value::string(U(it->first));
	  np++;
	}
    }
  return jrep;
}
void fsmw::Reply(http::status_code code,const json::value &body_data)
{
  _response.set_status_code(code);
  _response.set_body(body_data);
}
