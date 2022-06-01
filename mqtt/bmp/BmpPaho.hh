#include "PahoInterface.hh"
#include "bmp280.hh"

class BmpPaho : public PahoInterface {
    public:
    BmpPaho(std::string name, std::string process, uint32_t instance);
    ~BmpPaho();
    void registerCommands();
    virtual void status(web::json::value v);
  virtual void begin(web::json::value v);
  virtual void end(web::json::value v);
  virtual void destroy(web::json::value v);
   void lock() {_bsem.lock();}
  void unlock() {_bsem.unlock();}
    private:
    PahoInterface* _paho;
   bmp280* _bmp;
  std::mutex _bsem;
};
