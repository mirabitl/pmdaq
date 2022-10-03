#include "PahoInterface.hh"
#include "hihdriver.hh"

class HihPaho : public PahoInterface {
    public:
    HihPaho(std::string name, std::string process, uint32_t instance);
    ~HihPaho();
    void registerCommands();
    virtual void status(web::json::value v);
  virtual void begin(web::json::value v);
  virtual void end(web::json::value v);
  virtual void destroy(web::json::value v);
   void lock() {_bsem.lock();}
  void unlock() {_bsem.unlock();}
    private:
    PahoInterface* _paho;
   hihdriver* _Hih;
  std::mutex _bsem;
};
