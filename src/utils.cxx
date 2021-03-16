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
#include "utils.hh"
#include <err.h>


uint32_t utils::convertIP(std::string hname)
{
  struct hostent *he;
  struct in_addr **addr_list;
  int i;
  char ip[100];
  if ((he = gethostbyname(hname.c_str())) == NULL)
  {
    return 0;
  }

  addr_list = (struct in_addr **)he->h_addr_list;

  for (i = 0; addr_list[i] != NULL; i++)
  {
    //Return the first one;
    strcpy(ip, inet_ntoa(*addr_list[i]));
    break;
  }

  in_addr_t ls1 = inet_addr(ip);
  return (uint32_t)ls1;
}

uint64_t utils::asicTag(std::string hname,uint32_t header)
{
  uint64_t eid = ((uint64_t) utils::convertIP(hname)) << 32 | header;
  return eid;
}
uint64_t utils::asicTag(uint32_t ipa,uint32_t header)
{
  uint64_t eid = ((uint64_t) ipa) << 32 | header;
  return eid;
}

http_response utils::requesturl(std::string address)
{
  http::uri uri = http::uri(address);
  web::http::client::http_client_config cfg;
  cfg.set_timeout(std::chrono::seconds(1));
  http_client client(uri, cfg);
  return client.request(methods::GET).get();
}

http_response utils::request(std::string host, uint32_t port, std::string path,
                             web::json::value par)
{
  std::stringstream address("");
  address << U("http://") << host << ":" << port;
  http::uri uri = http::uri(address.str());
  web::http::client::http_client_config cfg;
  cfg.set_timeout(std::chrono::seconds(1));
  http_client client(http::uri_builder(uri).append_path(U(path)).to_uri(), cfg);

  if (par.is_object())
  {
    utility::ostringstream_t buf;
    uint32_t np = 0;
    for (auto iter = par.as_object().begin(); iter != par.as_object().end(); ++iter)
    {
      if (np == 0)
        buf << "?" << U(iter->first) << "=" << iter->second;
      else
        buf << "&" << U(iter->first) << "=" << iter->second;
      np++;
    }

    return client.request(methods::GET, U(buf.str())).get();
  }
  else
    return client.request(methods::GET).get();
}
std::vector<std::string> utils::split(const std::string &s, char delim)
{
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim))
  {
    elems.push_back(item);
    // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
  }
  return elems;
}

std::string utils::pns_name()
{
  std::string address;
  char *wp = getenv("PNS_NAME");
  if (wp != NULL)
    address.append(std::string(wp));
  else
    address.append("localhost");
  return address;
}
bool utils::checkpns(int port)
{
  struct hostent *h;
  struct sockaddr_in servaddr;

  int sd, rval;
  std::string address = utils::pns_name();

  h = gethostbyname(address.c_str());
  if (h == NULL)
  {
    PM_ERROR(_logPdaq, __PRETTY_FUNCTION__ << "Error when using gethostbyname " << address);
    std::exit(-1);
  }
  // std::cout << inet_ntoa(*((struct in_addr *)h->h_addr)) << std::endl;

  sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sd == -1)
  {
    PM_ERROR(_logPdaq, __PRETTY_FUNCTION__ << "Error when trying to create socket !");
    return false;
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);

  rval = connect(sd, (struct sockaddr *)&servaddr, sizeof(servaddr));

  if (rval == -1)
  {
    PM_ERROR(_logPdaq, __PRETTY_FUNCTION__ << "Port " << port << " is closed for: " << address);
    close(sd);
    return false;
  }

  else
  {
    PM_DEBUG(_logPdaq, __PRETTY_FUNCTION__ << "Opened port " << port << " :" << inet_ntoa(*((struct in_addr *)h->h_addr)));
    close(sd);
    return true;
  }
}

static int gr_n = 0;
static int gr_sockfd = 0;
#define MAX_MSG_PATH 100
#define MAX_MSG_LEN_PLAIN 130

int utils::graphite_init(const char *host, int port)
{
  struct sockaddr_in serv_addr;
  if ((gr_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket");
    return 1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0)
  {
    perror("inet_pton");
    return 1;
  }

  if (connect(gr_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("connect");
    return 1;
  }

  return 0;
}

void utils::graphite_finalize()
{
  if (gr_sockfd != -1)
  {
    close(gr_sockfd);
    gr_sockfd = -1;
  }
}

void utils::graphite_send(const char *message)
{
  gr_n = write(gr_sockfd, message, strlen(message));
  if (gr_n < 0)
  {
    perror("write");
    exit(1);
  }
}

void utils::graphite_send_plain(const char *path, float value, unsigned long timestamp)
{
  char spath[MAX_MSG_PATH];
  char message[MAX_MSG_LEN_PLAIN]; /* size = path + (value + timestamp) */

  /* make sure that path has a restricted length so it does not push the value + timestamp out of the message */
  snprintf(spath, MAX_MSG_PATH, "%s", path);

  /* format message as: <metric path> <metric value> <metric timestamp> */
  snprintf(message, MAX_MSG_LEN_PLAIN, "%s %.2f %lu\n", spath, value, timestamp);

  /* send to message to graphite */
  utils::graphite_send(message);
  printf("Sending %s\n", message);
}




std::string utils::lmexec(const char* cmd) {
  FILE* pipe = popen(cmd, "r");
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while (!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
  
}



std::map<uint32_t,std::string> utils::scanNetwork(std::string base)
{
  std::map<uint32_t,std::string> m;
  std::stringstream ss;
  ss<<"echo $(seq 254) | xargs -P255 -I% -d\" \" ping -W 1 -c 1 "<<base<<"% | grep -E \"[0-1].*?:\" | awk '{print $4}' | awk 'BEGIN{FS=\":\"}{print $1}'";

  std::cout<<ss.str()<<std::endl;
  std::string res= utils::lmexec( ss.str().c_str() );

  std::cout<<"Ethernet board on "<<base <<" \n"<<res;
  //getchar();
  std::stringstream ss1(res.c_str());
  std::string to;
  std::vector<std::string> host_list;
  if (res.c_str() != NULL)
  {
    while(std::getline(ss1,to,'\n')){
      host_list.push_back(to);
    }
  }
  
  
  //std::cout<<host_list.size()<<std::endl;
  for (auto x: host_list)
  {
    //std::cout<<x<<std::endl;
    
    struct in_addr ip;
    struct hostent *hp;
    
    if (!inet_aton(x.c_str(), &ip))
      errx(1, "can't parse IP address %s", x.c_str());
    
    if ((hp = gethostbyaddr((const void *)&ip, sizeof ip, AF_INET)) == NULL)
      printf("\t  %s is not known on the DNS \n", x.c_str());
    else
    {
      printf("%s is %x  %s\n", x.c_str(),ip.s_addr, hp->h_name);
      //m.insert(std::pair<uint32_t,std::string>(ip.s_addr,std::string(hp->h_name)));
      m.insert(std::pair<uint32_t,std::string>(ip.s_addr,x));
    }
      
      
      
      
      
  }
  return m;
}



std::string  utils::findUrl(std::string session, std::string appname,uint32_t appinstance)
{

  // Request PNS session process with process name = appname
  if (!utils::checkpns())
  {
    printf("No Process Name Server found\n");
    return "";
  }

  http_response rep = utils::request(utils::pns_name(), 8888, "/PNS/LIST", json::value::null());

  auto reg_list = rep.extract_json();
  auto serv_list = reg_list.get().as_object()["REGISTERED"];
  if (serv_list.is_null())
  {
    printf("No Process registered in PNS\n");
    return "";
  }
  for (auto it = serv_list.as_array().begin(); it != serv_list.as_array().end(); ++it)
  {
    std::string rec = (*it).as_string();
    //PM_DEBUG(_logPdaq, "PNS Service: " << rec);
    auto v = utils::split(rec, ':');
    std::string phost = v[0];
    uint32_t pport = std::stoi(v[1]);
    std::string ppath = v[2];
    //PM_DEBUG(_logPdaq, "PNS Service: " << phost << " " << pport << " " << ppath);
    auto vp0 = utils::split(ppath, '?');
    auto vp = utils::split(vp0[0], '/');
    //PM_DEBUG(_logPdaq, "VP size: " << vp.size());
    //PM_DEBUG(_logPdaq, "VP :  [1]" << vp[1] << " " << session << " [2]" << vp[2] << " " << appname);
    if (vp[1].compare(session) != 0)
      continue;
    if (vp[2].compare(appname) != 0)
      continue;
    uint32_t instance = std::stoi(vp[3]);
    if (instance!=appinstance)
      continue;
    std::stringstream surl("");
    surl<<"http://"<<phost<<":"<<pport<<vp0[0];
    return surl.str();
  }
  // Connect to the specified streams
  return "";
}



http_response utils::sendCommand(std::string url, std::string command,web::json::value par)
{
  std::stringstream address("");
  address << U(url);
  http::uri uri = http::uri(address.str());
  web::http::client::http_client_config cfg;
  cfg.set_timeout(std::chrono::seconds(1));
  http_client client(http::uri_builder(uri).append_path(U(command)).to_uri(), cfg);

  if (par.is_object())
  {
    utility::ostringstream_t buf;
    uint32_t np = 0;
    for (auto iter = par.as_object().begin(); iter != par.as_object().end(); ++iter)
    {
      if (np == 0)
        buf << "?" << U(iter->first) << "=" << iter->second;
      else
        buf << "&" << U(iter->first) << "=" << iter->second;
      np++;
    }

    return client.request(methods::GET, U(buf.str())).get();
  }
  else
    return client.request(methods::GET).get();
}
