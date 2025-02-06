#include "bufferHandler.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <boost/format.hpp>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <vector>
#include <map>

#include "utils.hh"
#include <err.h>

#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>

using namespace std;
using namespace wizcc;

// Netlink callbacks

wizcc::OnAccept::OnAccept() {}
void wizcc::OnAccept:: exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) 
{
  NL::Socket* newConnection = socket->accept();
  group->add(newConnection);
  PM_INFO(_logWizcc,"Connection " << newConnection->hostTo() << ":" << newConnection->portTo() << " added...");
}





wizcc::OnDisconnect::OnDisconnect() : _disconnect(false) {}
void wizcc::OnDisconnect::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  group->remove(socket);
  PM_INFO(_logWizcc,"Client " << socket->hostTo() << " disconnected...");
  _disconnect=true;
}

wizcc::OnClientDisconnect::OnClientDisconnect() : _disconnect(false) {}
void wizcc::OnClientDisconnect::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  if (!_disconnect)
    PM_INFO(_logWizcc,"Client " << socket->hostTo() << " disconnected...");
  _disconnect=true;
}



#undef DEBUGBUF

// On read method

wizcc::bufferHandler::bufferHandler(std::string directory) : _storeDir(directory),_npacket(0)
{
  _sockMap.clear();
 

}

void wizcc::bufferHandler::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  //cout << "\nREAD -- ";
  this->process_buffer(socket);


}

uint64_t wizcc::bufferHandler::socket_id(NL::Socket* socket)
{return ((uint64_t) utils::convertIP(socket->hostTo())<<32)|socket->portTo();}

void wizcc::bufferHandler::process_buffer(NL::Socket* socket) 
{
  // build id

  const std::lock_guard<std::mutex> lock(_sem);
  uint64_t id=socket_id(socket);
  PM_DEBUG(_logWizcc,"Message received from "<<socket->hostTo()<<":"<<socket->portTo()<<" =>"<<std::hex<<id<<std::dec);

  // Find data buffer where to store
  std::map<uint64_t, ptrBuf>::iterator itsock=_sockMap.find(id);

  // New socket
  if (itsock==_sockMap.end())
  {
    ptrBuf p(0,new unsigned char[0x100000]);
    std::pair<uint64_t, ptrBuf> p1(id,p);
    _sockMap.insert(p1);
    
    itsock=_sockMap.find(id);
    // Build subdir
    struct stat st = {0};
    std::stringstream s;
    s<<_storeDir<<"/"<<socket->hostTo();//<<"/"<<socket->portTo();
    std::cout<<"Creating "<<s.str()<<std::endl;
    if (stat(s.str().c_str(), &st) == -1) {
      mkdir(s.str().c_str(), 0700);
    }
    s<<"/"<<socket->portTo();
    std::cout<<"Creating "<<s.str()<<std::endl;
    if (stat(s.str().c_str(), &st) == -1) {
      mkdir(s.str().c_str(), 0700);
    }

  }




  
  ptrBuf &p=itsock->second;
  uint32_t* iptr=(uint32_t*) &p.second[0];
  uint16_t* sptr=(uint16_t*) &p.second[0];


  // Read full buffer from the socket/port

  
  size_t ier=0;
  uint32_t size_remain=16*1024;
  // uint8_t temp[8];
  // ier=socket->read(temp,8);
  // for (int ib=0;ib<8;ib++)
  //   printf("%.2x ",temp[ib]);
  // printf("\n");
  // //getchar();
  // return;
  while (size_remain>0)
  {
    try 
    {
      ier=socket->read(&p.second[0],size_remain);
    }
    catch (NL::Exception e)
    {
      printf("%s Error message when reading block %s \n",__PRETTY_FUNCTION__,e.msg().c_str());
      return;
    }
    if (ier<0)
      break;
    _npacket++;
    // if (_npacket%1000 ==1)
    PM_DEBUG(_logWizcc,"Packet "<<_npacket<<" receive "<<ier<<" bytes from"<<std::hex<<id<<std::dec);
    //fprintf(stderr,"%s Packet %ld Receive %ld bytes from %lx \n",__PRETTY_FUNCTION__,_npacket,ier,id);
    break;
    size_remain -=ier;
    
  }
  p.first=ier;


  // Now call the processors assoicated
  std::map<uint64_t,MPIFunctor >::iterator icmd=_handlers.find(id);
  if (icmd==_handlers.end())
    {
      PM_ERROR(_logWizcc,"Message received from "<<socket->hostTo()<<":"<<socket->portTo()<<" =>"<<std::hex<<id<<std::dec<<std::flush);
      fprintf(stderr,"%s No data handler for socket id %ld \n",__PRETTY_FUNCTION__,id);
      p.first=0;

      return;
          
    }
  icmd->second(id,p.first,(char*) p.second);
  p.first=0;
  return;
}
void wizcc::bufferHandler::remove_socket(NL::Socket* socket)
{

  std::map<uint64_t, ptrBuf>::iterator itsock=_sockMap.find(socket_id(socket));
  if (itsock==_sockMap.end()) return;
  delete itsock->second.second;
  _sockMap.erase(itsock);
}

void wizcc::bufferHandler::addHandler(uint64_t id,wizccFunctor f)
{
  std::pair<uint64_t,wizccFunctor> p(id,f);
  _handlers.insert(p);
}
