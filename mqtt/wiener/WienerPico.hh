#include "PicoInterface.hh"
#include "WienerSnmp.hh"

class WienerPico : public PicoInterface {
public:
  WienerPico(std::string id, std::string subid,std::string ipadr,uint32_t first,uint32_t last);
  ~WienerPico();
  void registerCommands();
  web::json::value channelStatus(uint32_t channel);
  web::json::value rangeStatus(int32_t first,int32_t last);

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
  wiener::WienerDevice* _hv;
  std::mutex _bsem;
  std::string _ipaddress;
  uint32_t _first,_last;
};
