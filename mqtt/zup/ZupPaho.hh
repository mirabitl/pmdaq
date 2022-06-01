#include "PahoInterface.hh"
#include "Zup.hh"

class ZupPaho : public PahoInterface {
public:
  ZupPaho(std::string name, std::string process, uint32_t instance,std::string device,uint32_t port);
  ~ZupPaho();
  void registerCommands();

  virtual void status(web::json::value v);
  virtual void begin(web::json::value v);
  virtual void end(web::json::value v);
  virtual void destroy(web::json::value v);
  virtual void on(web::json::value v);
  virtual void off(web::json::value v);
  void lock() {_bsem.lock();}
  void unlock() {_bsem.unlock();}
private:
  PahoInterface* _paho;
  zup::Zup* _lv;
  std::mutex _bsem;
  std::string _device;
  uint32_t _port;
};
