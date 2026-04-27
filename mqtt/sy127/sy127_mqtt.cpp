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

class SY127Parser {
public:
    regex pattern;

    SY127Parser() :
        pattern(R"(^(CH\d+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+(\d+)\s+(\w+)$)") {}

    string clean(const string& text) {
        return regex_replace(text, regex("\x1b\\[[0-9;]*[A-Za-z]"), "");
    }

    map<string, json> parseBlock(const string& raw) {
        map<string, json> result;

        string cleanText = clean(raw);
	//cout<<cleanText<<endl;
        istringstream iss(cleanText);
        string line;

        while (getline(iss, line)) {
	  string ch, status;
	  double vmon, imon, v0, v1, i0, i1, rup, rdw;
	  int trip;
	  istringstream iss1(line);
	  if (iss1 >> ch >> vmon >> imon >> v0 >> v1 >> i0 >> i1 >> rup >> rdw >> trip >> status) {
	    // OK
	    cout<<ch<<":"<<ch<<" "<<v0<<endl;
	    result[ch]={
                    {"VMON", vmon},
                    {"IMON",imon},
                    {"V0", v0},
                    {"V1", v1},
                    {"I0",i0},
                    {"I1", i1},
                    {"RUP",rup},
                    {"RDW", rdw},
                    {"TRIP", trip},
                    {"STATUS", status}
	    };
	  }
        }
        return result;
    }
};

// ---------------- Serial Driver ----------------

class Sy127Access {
public:
    struct sp_port* port;
    SY127Parser parser;
    map<string, json> channels;
    int mode;

    Sy127Access(string device="/dev/ttyUSB0", int baud=9600, int mode=1) : mode(mode) {
        if (sp_get_port_by_name(device.c_str(), &port) != SP_OK)
            throw runtime_error("Serial port not found");

        sp_open(port, SP_MODE_READ_WRITE);
        sp_set_baudrate(port, baud);
        sp_set_bits(port, 8);
        sp_set_parity(port, SP_PARITY_NONE);
        sp_set_stopbits(port, 1);

        cout << "Serial open\n";

        write("1");
        //sleep_ms(200);
        write("A");
        //sleep_ms(200);
	//cout<<this->read()<<endl;

    }

  void sleep_ms(int ms) {
        this_thread::sleep_for(chrono::milliseconds(ms));
  }
  void write_char(char c) {
    sp_blocking_write(port, &c, 1, 1000);
  }

  void write(const std::string& s) {
    for (char c : s) {
      write_char(c);
      sleep_ms(120);  // ajustable
    }
  }
    /*    void write(const string& s) {
      for (int i=0;i<s.length();i++)
	{
	  sp_blocking_write(port, s.c_str()[i],1, 1000);
	  sleep_ms(150);
	}
    }
    */

    string read() {
      cout<<"on rentre dans read"<<endl;
        char buf[8192];
        int n = sp_blocking_read(port, buf, sizeof(buf), 1000);
	//cout<<string(buf, n > 0 ? n : 0)<<endl;
        return string(buf, n > 0 ? n : 0);
    }

    void status() {
        write("1");
        write("A");

        channels.clear();
        bool chread = true;

        while (chread) {
            string raw = read();
            auto parsed = parser.parseBlock(raw);

            for (auto& kv : parsed) {
                if (channels.find(kv.first) == channels.end()) {
                    channels[kv.first] = kv.second;
                } else {
                    chread = false;
                    break;
                }
            }

            if (chread)
                write(mode == 1 ? "p" : "q");
        }
    }

    bool modifyChannel(const string& ch) {
        if (channels.find(ch) == channels.end()) return false;

        write("1");
        write("B");
        write("A");
        write("A");
        write(ch + "\r\n");

        return true;
    }

    void setV0(const string& ch, double val) {
        if (!modifyChannel(ch)) return;
        write("C");
        write(formatFloat(val));
	cout<<"New V0 "<<ch<<" "<<formatFloat(val)<<endl;
    }

    void setI0(const string& ch, double val) {
        if (!modifyChannel(ch)) return;
        write("F");
        write(formatFloat(val));
    }

    void setRup(const string& ch, double val) {
        if (!modifyChannel(ch)) return;
        write("I");
        write(formatFloat(val));
    }

    void setRdw(const string& ch, double val) {
        if (!modifyChannel(ch)) return;
        write("J");
        write(formatFloat(val));
    }

    void toggle(const string& ch) {
        if (!modifyChannel(ch)) return;
        write("N");
    }

private:
    string formatFloat(double v) {
        ostringstream oss;
        oss << fixed << setprecision(1) << v << "\r\n";
        return oss.str();
    }
};

// ---------------- MQTT Bridge ----------------

class Bridge {
public:
    Sy127Access& crate;
    struct mosquitto* mosq;
    mutex mtx;

    Bridge(Sy127Access& c, const string& host) : crate(c) {
        mosquitto_lib_init();

        mosq = mosquitto_new("sy127", true, this);
        if (!mosq) throw runtime_error("MQTT init failed");

        mosquitto_message_callback_set(mosq, onMessageStatic);

        mosquitto_connect(mosq, host.c_str(), 1883, 60);
        mosquitto_subscribe(mosq, NULL, "sy127/cmd", 0);
	cout<<"Mosquitto connected"<<endl;
    }

    static void onMessageStatic(struct mosquitto* m, void* obj, const struct mosquitto_message* msg) {
        ((Bridge*)obj)->onMessage(msg);
    }

    void onMessage(const struct mosquitto_message* msg) {
        try {
            string payload((char*)msg->payload);
            auto j = json::parse(payload);

            lock_guard<mutex> lock(mtx);

            string cmd = j["cmd"];
            string ch = j.value("channel", "");
            double val = j.value("value", 0.0);

            if (cmd == "set_v0") crate.setV0(ch, val);
            else if (cmd == "set_i0") crate.setI0(ch, val);
            else if (cmd == "set_rup") crate.setRup(ch, val);
            else if (cmd == "set_rdw") crate.setRdw(ch, val);
            else if (cmd == "toggle") crate.toggle(ch);
            else cout << "Unknown cmd\n";

        } catch (exception& e) {
            cerr << "MQTT parse error: " << e.what() << endl;
        }
    }

    void publish() {
        json j;
        for (auto& kv : crate.channels) {
            j[kv.first] = kv.second;
        }

        string payload = j.dump();

        mosquitto_publish(mosq, NULL,
                          "sy127/status",
                          payload.size(),
                          payload.c_str(),
                          0, false);
    }

    void run() {

        thread statusThread([&]() {
            while (true) {
                {
		  cout<<"cathc mutex"<<endl;
                    lock_guard<mutex> lock(mtx);
                    crate.status();
                    publish();
                }
                this_thread::sleep_for(chrono::seconds(15));
            }
        });

	mosquitto_loop_forever(mosq, -1, 1);
        statusThread.join();
    }
};

// ---------------- MAIN ----------------

int main() {
    try {
        Sy127Access crate("/dev/ttyUSB0", 9600, 0);
        Bridge bridge(crate, "lyoilcdaq01");

        bridge.run();
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
}
