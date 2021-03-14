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
