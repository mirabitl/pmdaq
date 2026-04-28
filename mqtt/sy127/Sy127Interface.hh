// sy127_mqtt.cpp

#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <thread>
#include <mutex>
#include <chrono>
#include <sstream>

#include <libserialport.h>
#include <mosquitto.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

// ---------------- Parser ----------------

class Sy127Parser {
public:
  regex pattern;
  
  Sy127Parser() :
        pattern(R"(^(CH\d+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+(\d+)\s+(\w+)$)") {}

  string clean(const string& text);
  map<string, json> parseBlock(const string& raw);
};

// ---------------- Serial Driver ----------------

class Sy127Access {
public:
    struct sp_port* port;
    Sy127Parser parser;
    map<string, json> channels;
    int mode;

  Sy127Access(string device="/dev/ttyUSB0", int baud=9600, int mode=1);
  void sleep_ms(int ms);
  void write_char(char c);
  void write(const std::string& s);
  string read();

  void status();
  string json_status();
  
  bool modifyChannel(const string& ch);
  
  void setV0(const string& ch, double val);

  void setI0(const string& ch, double val);

  void setRup(const string& ch, double val);

  void setRdw(const string& ch, double val);
  void toggle(const string& ch);

  double v0(std::string channel);
  double i0(std::string channel);
  double vmon(std::string channel);
  double imon(std::string channel);
  double rup(std::string channel);
  double rdw(std::string channel);
  string status(std::string channel);
private:
  string formatFloat(double v); 
};

/** ---------------- MAIN ----------------

int main() {
    try {
        Sy127Access crate("/dev/ttyUSB0", 9600, 0);
        Bridge bridge(crate, "lyoilcdaq01");

        bridge.run();
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
}

**/
