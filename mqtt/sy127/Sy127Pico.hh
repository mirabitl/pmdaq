#include "PicoInterface.hh"

#include "Sy127Interface.hh"
#include <time.h>
class Sy127Pico : public PicoInterface {
public:
  Sy127Pico(std::string id, std::string subid,std::string device,uint32_t mode);
  ~Sy127Pico();
  void registerCommands();
  virtual void status(web::json::value v);
  virtual void reset(web::json::value v);
  virtual void vset(web::json::value v);
  virtual void iset(web::json::value v);
  virtual void rampup(web::json::value v);
  virtual void rampdown(web::json::value v);
  virtual void on(web::json::value v);
  virtual void off(web::json::value v);
  virtual void clearalarm(web::json::value v);
  void lock() {_bsem.lock();}
  void unlock() {_bsem.unlock();}
private:
  Sy127Access* _hv;
  std::mutex _bsem;
  std::string _device;
  uint32_t _mode;
  time_t _last_connect;

};
