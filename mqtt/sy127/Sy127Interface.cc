#include "Sy127Interface.hh"
using json = nlohmann::json;
using namespace std;

// ---------------- Parser ----------------

string Sy127Parser::clean(const string& text) {
  return regex_replace(text, regex("\x1b\\[[0-9;]*[A-Za-z]"), "");
}

map<string, json> Sy127Parser::parseBlock(const string& raw) {
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
      //cout<<ch<<":"<<ch<<" "<<v0<<endl;
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
Sy127Access::Sy127Access(string device, int baud, int mode) : mode(mode)
{
  if (sp_get_port_by_name(device.c_str(), &port) != SP_OK)
    throw runtime_error("Serial port not found");

  sp_open(port, SP_MODE_READ_WRITE);
  sp_set_baudrate(port, baud);
  sp_set_bits(port, 8);
  sp_set_parity(port, SP_PARITY_NONE);
  sp_set_stopbits(port, 1);
  
  cout << "Serial open\n";
  this->read();
  this->status();
  
}

void Sy127Access::sleep_ms(int ms) {
  this_thread::sleep_for(chrono::milliseconds(ms));
}
void Sy127Access::write_char(char c) {
  sp_blocking_write(port, &c, 1, 1000);
}

void Sy127Access::write(const std::string& s) {
  for (char c : s) {
    write_char(c);
    sleep_ms(150);  // ajustable
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

string Sy127Access::read() {
  //cout<<"on rentre dans read"<<endl;
  char buf[8192];
  int n = sp_blocking_read(port, buf, sizeof(buf), 1000);
  //cout<<string(buf, n > 0 ? n : 0)<<endl;
  return string(buf, n > 0 ? n : 0);
}

void Sy127Access::status() {
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
string  Sy127Access::json_status()
{
  json j;
  for (auto& kv : this->channels) {
    j[kv.first] = kv.second;
  }

  return j.dump();
}
double  Sy127Access::v0(std::string channel)
{
  auto it=this->channels.find(channel);
  if (it!=this->channels.end())
    return it->second["V0"];
  else
    return -1;
}
double Sy127Access::i0(std::string channel)
{
  auto it=this->channels.find(channel);
  if (it!=this->channels.end())
    return it->second["I0"];
  else
    return -1;
}    
double Sy127Access::vmon(std::string channel)
{
  auto it=this->channels.find(channel);
  if (it!=this->channels.end())
    return it->second["VMON"];
  else
    return -1;
}
double Sy127Access::imon(std::string channel)
{
  auto it=this->channels.find(channel);
  if (it!=this->channels.end())
    return it->second["IMON"];
  else
    return -1;
}
double Sy127Access::rup(std::string channel)
{
  auto it=this->channels.find(channel);
  if (it!=this->channels.end())
    return it->second["RUP"];
  else
    return -1;
}
double Sy127Access::rdw(std::string channel)
{
  auto it=this->channels.find(channel);
  if (it!=this->channels.end())
    return it->second["RDW"];
  else
    return -1;
}
string Sy127Access::status(std::string channel)
{
  auto it=this->channels.find(channel);
  if (it!=this->channels.end())
    return it->second["STATUS"];
  else
    return "UNK";
}

bool Sy127Access::modifyChannel(const string& ch) {
  if (channels.find(ch) == channels.end()) return false;

  write("1");
  write("B");
  write("A");
  write("A");
  write(ch + "\r\n");

  return true;
}

void Sy127Access::setV0(const string& ch, double val) {
  if (!modifyChannel(ch)) return;
  write("C");
  write(formatFloat(val));
  cout<<"New V0 "<<ch<<" "<<formatFloat(val)<<endl;
}

void Sy127Access::setI0(const string& ch, double val) {
  if (!modifyChannel(ch)) return;
  write("F");
  write(formatFloat(val));
}

void Sy127Access::setRup(const string& ch, double val) {
  if (!modifyChannel(ch)) return;
  write("I");
  write(formatFloat(val));
}

void Sy127Access::setRdw(const string& ch, double val) {
  if (!modifyChannel(ch)) return;
  write("J");
  write(formatFloat(val));
}

void Sy127Access::toggle(const string& ch) {
  if (!modifyChannel(ch)) return;
  write("N");
}


string Sy127Access::formatFloat(double v) {
  ostringstream oss;
  oss << fixed << setprecision(1) << v << "\r\n";
  return oss.str();
}
