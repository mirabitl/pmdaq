#include "PahoInterface.hh"

#include "HVCaenInterface.hh"
class Sy1527Paho : public PahoInterface {
public:
  Sy1527Paho(std::string name, std::string process, uint32_t instance,std::string account,uint32_t first,uint32_t last);
  ~Sy1527Paho();
  void registerCommands();
  web::json::value channelStatus(uint32_t channel);
  web::json::value rangeStatus(int32_t first,int32_t last);

  virtual void status(web::json::value v);
  virtual void begin(web::json::value v);
  virtual void end(web::json::value v);
  virtual void destroy(web::json::value v);
  virtual void vset(web::json::value v);
  virtual void iset(web::json::value v);
  virtual void rampup(web::json::value v);
  virtual void on(web::json::value v);
  virtual void off(web::json::value v);
  virtual void clearalarm(web::json::value v);
  void lock() {_bsem.lock();}
  void unlock() {_bsem.unlock();}
private:
  PahoInterface* _paho;
  caen::HVCaenInterface* _hv;
  std::mutex _bsem;
  std::string _account;
  uint32_t _first,_last;
};
