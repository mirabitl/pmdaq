#include "PicoInterface.hh"

#include "HVCaenInterface.hh"
#include <time.h>
class Sy1527Pico : public PicoInterface {
public:
  Sy1527Pico(std::string id, std::string subid,std::string account,uint32_t first,uint32_t last);
  ~Sy1527Pico();
  void registerCommands();
  web::json::value channelStatus(uint32_t channel);
  web::json::value rangeStatus(int32_t first,int32_t last);
  void opensocket();
  void closesocket();
  virtual void status(web::json::value v);
  virtual void reset(web::json::value v);
  virtual void vset(web::json::value v);
  virtual void iset(web::json::value v);
  virtual void rampup(web::json::value v);
  virtual void on(web::json::value v);
  virtual void off(web::json::value v);
  virtual void clearalarm(web::json::value v);
  void lock() {_bsem.lock();}
  void unlock() {_bsem.unlock();}
private:
  caen::HVCaenInterface* _hv;
  std::mutex _bsem;
  std::string _account,_host,_name,_pwd;
  uint32_t _first,_last;
  time_t _last_connect;
};
