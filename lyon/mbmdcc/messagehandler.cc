#include "MBMDCCInterface.hh"
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

//using namespace mpi;

mbmdcc::messageHandler::messageHandler() : _npacket(0) {_sockMap.clear();}

uint64_t mbmdcc::messageHandler::Id(NL::Socket* socket)
{
  // struct hostent *he;
  // struct in_addr **addr_list;
  // int i;
  // char ip[100];
  // if ((he = gethostbyname(socket->hostTo().c_str())) == NULL)
  // {
  //   return 0;
  // }

  // addr_list = (struct in_addr **)he->h_addr_list;

  // for (i = 0; addr_list[i] != NULL; i++)
  // {
  //   //Return the first one;
  //   strcpy(ip, inet_ntoa(*addr_list[i]));
  //   break;
  // }
  in_addr_t ls1 = inet_addr(socket->hostTo().c_str());
  //  in_addr_t ls1=inet_addr(ip_address.c_str());
  //uint32_t ls1=utils::convertIP(socket->hostTo());
  return ((uint64_t)ls1<<32)|((uint64_t) socket->portTo());

}
void mbmdcc::messageHandler::processMessage(NL::Socket* socket) 
{
  // build id

  const std::lock_guard<std::mutex> lock(_sem);
  uint64_t id=this->Id(socket);
  PM_INFO(_logMbmdcc,"Message received from "<<socket->hostTo()<<":"<<socket->portTo()<<" =>"<<std::hex<<id<<std::dec<<" Read Size "<<socket->nextReadSize());
  std::map<uint64_t, ptrBuf>::iterator itsock=_sockMap.find(id);

  if (itsock==_sockMap.end())
  {
    ptrBuf p(0,new unsigned char[0x100000]);
    std::pair<uint64_t, ptrBuf> p1(id,p);
    _sockMap.insert(p1);
    
    itsock=_sockMap.find(id);
    // Build subdir
    struct stat st = {0};
    std::stringstream s;
    s<<"/dev/shm/"<<socket->hostTo();//<<"/"<<socket->portTo();
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
  // Check


  
  size_t ier=0;
  uint32_t size_remain=16*1024;
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
    PM_DEBUG(_logMbmdcc,"Packet "<<_npacket<<" receive "<<ier<<" bytes from"<<std::hex<<id<<std::dec);
    //fprintf(stderr,"%s Packet %ld Receive %ld bytes from %lx \n",__PRETTY_FUNCTION__,_npacket,ier,id);
    break;
    size_remain -=ier;
    
  }
  p.first=ier;
  std::map<uint64_t,MPIFunctor >::iterator icmd=_handlers.find(id);
  if (icmd==_handlers.end())
    {
      uint64_t idn=this->Id(socket);
      PM_ERROR(_logMbmdcc,"Message received from "<<socket->hostTo()<<":"<<socket->portTo()<<" =>"<<std::hex<<utils::convertIP(socket->hostTo())<<" ID "<<id<<" et maintenant "<< idn <<std::dec<<std::flush);
      fprintf(stderr,"%s No data handler for socket id %ld \n",__PRETTY_FUNCTION__,id);
      p.first=0;

      return;

    }
  icmd->second(id,p.first,(char*) p.second);
  p.first=0;
  return;
}
void mbmdcc::messageHandler::removeSocket(NL::Socket* socket)
{
  uint64_t id=this->Id(socket);
  std::map<uint64_t, ptrBuf>::iterator itsock=_sockMap.find(id);
  if (itsock==_sockMap.end()) return;
  delete itsock->second.second;
  _sockMap.erase(itsock);
}

void mbmdcc::messageHandler::addHandler(uint64_t id,MPIFunctor f)
{
  std::pair<uint64_t,MPIFunctor> p(id,f);
  _handlers.insert(p);
}


mbmdcc::OnAccept::OnAccept(messageHandler* msh) : _msh(msh) {}
void mbmdcc::OnAccept:: exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) 
{
  NL::Socket* newConnection = socket->accept();
  group->add(newConnection);
  cout << "\nConnection " << newConnection->hostTo() << ":" << newConnection->portTo() << " added...";
  cout.flush();
}




mbmdcc::OnRead::OnRead(messageHandler* msh) : _msh(msh) {}
void mbmdcc::OnRead::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  //cout << "\nREAD -- ";
  _msh->processMessage(socket);


    cout.flush();
  /*    unsigned char buffer[256];
    buffer[255] = '\0';
    memset(theReadBuffer_,0,0x10000);
    //socket->read(buffer, 255);
    size_t msgLen =socket->read(theReadBuffer_,0x10000);
    std::string sread(buffer);
    cout << "Message from " << socket->hostTo() << ":" << socket->portTo() << ". Text received: " << sread<<std::endl;
    cout.flush();
		
    for(unsigned i=1; i < (unsigned) group->size(); ++i)
    {
    if(group->get(i) != socket)
    {
    printf(" \t sending %d  %d %s \n",i,sread.size(),sread.c_str());
    group->get(i)->send(sread.c_str(), sread.size());
    }
    }
  */
}

mbmdcc::OnDisconnect::OnDisconnect(messageHandler* msh) : _msh(msh),_disconnect(false) {}
void mbmdcc::OnDisconnect::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  group->remove(socket);
  cout << "\nClient " << socket->hostTo() << " disconnected...";
  //_msh->removeSocket(socket);
  cout.flush();
  _disconnect=true;
  //delete socket;
}

mbmdcc::OnClientDisconnect::OnClientDisconnect() : _disconnect(false) {}
void mbmdcc::OnClientDisconnect::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  if (!_disconnect)
    cout <<__PRETTY_FUNCTION__<< "\nClient " << socket->hostTo() << " disconnected..."<<std::flush;
  _disconnect=true;
  //uint32_t* i =(uint32_t*) reference;
  //(*i)=0xDEAD;
}
