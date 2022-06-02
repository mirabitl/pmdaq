#include "PahoInterface.hh"
#include "Genesys.hh"

class GenesysPaho : public PahoInterface {
public:
  GenesysPaho(std::string name, std::string process, uint32_t instance,std::string device,uint32_t port);
  ~GenesysPaho();
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
  genesys::GsDevice* _lv;
  std::mutex _bsem;
  std::string _device;
  uint32_t _port;
};
