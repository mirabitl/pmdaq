#include <baseServer.hh>
#include <cpprest/uri.h>
#include <cpprest/http_listener.h>
#include <cpprest/asyncrt_utils.h>
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

#pragma comment(lib, "cpprest_2_7.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "httpapi.lib")
 
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;
 
 
int main(int argc, char** argv)
{

printf("parsing the config file \n");
DOMConfigurator::configure("/etc/Log4cxxConfig.xml");
//_logger->setLevel(log4cxx::Level::getInfo());
PM_INFO (_logPdaq, "this is a info message, after parsing configuration file");


  
  std::string host="localhost";
  uint32_t port =8888;
  char c;

  // while ( (c = getopt (argc, argv, "H:P:")) != -1 ) {
  // switch (c) {
  // case 'H':
  //   host.assign(optarg);
  //   //cout<<optarg<<endl; //I tried printing the value but it only prints the second  flag value
  //   break;
  // case 'P':
  //     port = atoi(optarg);
  //     std::cout<<port<<std::endl;
  //     break;
  // case '?':
  //   default:
  //     std::cout<<"Usage:  [-H host] [-P port].\n"<<std::endl;
   
  //   }
  // }
  char* wp=getenv("PNS_NAME");
  if (wp!=NULL)      host.assign(std::string(wp));
  char* wp1=getenv("PNS_PORT");
  if (wp1!=NULL)      port=std::stoi(std::string(wp1));

 try
 {
   std::stringstream sadr;
   sadr<<"http://"<<host<<":"<<port;
   utility::string_t address = U(sadr.str());
  uri_builder uri(address);
  auto addr = uri.to_uri().to_string();
  baseServer handler(addr);
  handler.open().wait();
  PM_INFO (_logPdaq,utility::string_t(U(" PNS Listening for requests at: ")) << addr<<" "<<handler.url() );
  // ucout << U("Press ENTER key to quit...") << std::endl;
  // std::string line;
  // std::getline(std::cin, line);
  handler.registerPlugin("pns","");
  while (1)
    ::sleep(100);
  handler.close().wait();
 }
 catch (std::exception& ex)
 {
  ucout << U("Exception: ") << ex.what() << std::endl;
  ucout << U("Press ENTER key to quit...") << std::endl;
  std::string line;
  std::getline(std::cin, line);
 }
 return 0;
}
