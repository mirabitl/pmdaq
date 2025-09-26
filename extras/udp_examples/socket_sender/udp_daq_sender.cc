// Version C++ Multiclasse avec CLI et Contrôle UDP ou HTTP (cpprest)

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <cstring>
#include <random>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <string>
#include <map>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// Logger indépendant
class Logger {
public:
    Logger() {
        std::ostringstream filename;
        filename << "/tmp/febv2debug" << getpid() << ".log";
        logfile.open(filename.str(), std::ios::out);
    }

    ~Logger() { if (logfile.is_open()) logfile.close(); }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex);
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logfile << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S")
                << " " << message << std::endl;
        std::cout << message << std::endl;
    }

private:
    std::ofstream logfile;
    std::mutex mutex;
};

// Acquisition Class DummySocket
class DummySocket {
public:
    DummySocket(Logger& logger) : log(logger), running(false), s_pause(false) {
        detectorId = 201;
        sourceId = 10;
        tokens = 100000;
        throttle_low_limit = 10000;
        throttle_high_limit = 20000;
        run = 0;
        nacq = 0;
        gtc = 0;
        throttle_start = false;
        state = "CREATED";
    }

    void initialise() {
        log.log("[INFO] Dummy socket init");
        state = "INITIALISED";
    }

    void configure() {
        tokens = 100000;
        throttle_low_limit = 10000;
        throttle_high_limit = 20000;
        run = 0;
        nacq = 0;
        state = "CONFIGURED";
        log.log("[INFO] Dummy socket configured");
    }

    void start(int run_number = 0) {
        if (running) return;
        running = true;
        run = run_number;
        producer_thread = std::thread(&DummySocket::acquiring_data, this);
        log.log("[INFO] DAQ started, Run number: " + std::to_string(run));
        state = "RUNNING";
    }

    void stop() {
        running = false;
        if (producer_thread.joinable())
            producer_thread.join();
        log.log("[INFO] DAQ stopped");
        state = "STOPPED";
    }

    void pause() { s_pause = true; }
    void resume() { s_pause = false; }

    void addTokens(int n) { tokens += n; }

    json::value status() {
        json::value obj;
        obj["tokens"] = json::value::number(tokens.load());
        if (running) {
            obj["state"] = json::value::string("running");
            obj["run"] = json::value::number(run);
            obj["event"] = json::value::number(gtc);
        } else {
            obj["state"] = json::value::string("stopped");
        }
        return obj;
    }

private:
    Logger& log;
    int detectorId, sourceId;
    std::atomic<bool> running;
    std::atomic<int> tokens;
    int throttle_low_limit, throttle_high_limit;
    int run, nacq, gtc;
    bool throttle_start, s_pause;
    std::string state;
    std::thread producer_thread;

    void acquiring_data() {
        const char* UDP_IP = "192.168.100.1";
        const int UDP_PORT = 8765;
        const int PACKET_SIZE = 1472;

        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        sockaddr_in servaddr{};
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(UDP_PORT);
        inet_pton(AF_INET, UDP_IP, &servaddr.sin_addr);

        std::vector<uint8_t> packet(PACKET_SIZE, 0);
        for (int i = 20; i < 1400; ++i)
            packet[i] = i % 255;

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(1, 247);
        std::vector<int> vs(100000);
        for (auto& v : vs) v = dist(rng);

        nacq = 0;
        gtc = 0;
        int offset = 0, elen = 0, rlen = 0;
        double t0 = current_time();

        while (running) {
            if (throttle(offset)) continue;
            if (s_pause) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                continue;
            }

            if (offset == 0) {
                elen = vs[gtc % 100000] * PACKET_SIZE;
                gtc++;
                rlen = elen;
            }

            nacq++;
            writeInt(packet.data(), nacq, 0);
            writeInt(packet.data(), offset, 4);
            writeInt(packet.data(), elen, 8);
            writeInt(packet.data(), gtc, 12);
            writeInt(packet.data(), tokens, 16);

            rlen -= PACKET_SIZE;
            offset += PACKET_SIZE;
            if (rlen < 1) offset = 0;

            if (nacq % 3000 == 1 && nacq > 1) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                double t = current_time();
                double MB = nacq * PACKET_SIZE / (1024.0 * 1024.0);
                double dt = t - t0;
                double MBS = (MB / dt) * 8;
                log.log("[INFO] Event " + std::to_string(gtc) + " Packet " + std::to_string(nacq) +
                        " MB " + format(MB) + " Throughput Mbit/s " + format(MBS));
            }

            sendto(sockfd, packet.data(), PACKET_SIZE, 0,
                   (sockaddr*)&servaddr, sizeof(servaddr));
            tokens--;
        }

        close(sockfd);
        log.log("[INFO] Acquisition thread finished, Run " + std::to_string(run));
    }

    bool throttle(int offset) {
        if (offset != 0) return false;
        if (tokens < throttle_low_limit) {
            throttle_start = true;
            return true;
        }
        return throttle_start && tokens < throttle_high_limit;
    }

    static void writeInt(uint8_t* buffer, int value, int offset) {
        buffer[offset] = (value >> 24) & 0xFF;
        buffer[offset + 1] = (value >> 16) & 0xFF;
        buffer[offset + 2] = (value >> 8) & 0xFF;
        buffer[offset + 3] = value & 0xFF;
    }

    static double current_time() {
        using namespace std::chrono;
        return duration_cast<duration<double>>(steady_clock::now().time_since_epoch()).count();
    }

    static std::string format(double value) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << value;
        return oss.str();
    }
};

// HTTP Serveur REST
void start_http_server(DummySocket& daq, Logger& log)
{
  utility::string_t address = U("http://127.0.0.1:8091");
  uri_builder uri(address);
  auto addr = uri.to_uri().to_string();
  std::cout<<addr<<std::endl;
  http_listener listener(addr);

  listener.support(methods::GET, [&](http_request request) {
    auto path = uri::decode(request.relative_uri().path());

    if (path == "/status") {
            request.reply(status_codes::OK, daq.status());
        }
        else if (path == "/addtokens") {
            auto queries = uri::split_query(request.request_uri().query());
            if (queries.find("count") != queries.end()) {
                try {
                    int n = std::stoi(queries["count"]);
                    daq.addTokens(n);
                    request.reply(status_codes::OK, "Tokens added via GET");
                } catch (...) {
                    request.reply(status_codes::BadRequest, "Invalid 'count' parameter");
                }
            } else {
                request.reply(status_codes::BadRequest, "Missing 'count' parameter");
            }
        }
        else {
            request.reply(status_codes::NotFound, "Unknown GET endpoint");
        }
  });

  listener.support(methods::POST, [&](http_request request) {
    auto path = uri::decode(request.relative_uri().path());
    
    if (path == "/start") {
      daq.start(1);
      request.reply(status_codes::OK, "Started");
    } else if (path == "/stop") {
      daq.stop();
      request.reply(status_codes::OK, "Stopped");
    } else if (path == "/pause") {
      daq.pause();
      request.reply(status_codes::OK, "Paused");
    } else if (path == "/resume") {
      daq.resume();
      request.reply(status_codes::OK, "Resumed");
    } else if (path == "/addtokens") {
      request.extract_json().then([&](pplx::task<json::value> task) {
	try {
	  auto body = task.get();
	  if (body.has_field("count")) {
	    int n = body["count"].as_integer();
	    daq.addTokens(n);
	    request.reply(status_codes::OK, "Tokens added via POST");
	  } else {
	    request.reply(status_codes::BadRequest, "Missing 'count' field");
	  }
	} catch (...) {
	  request.reply(status_codes::BadRequest, "Invalid JSON");
	}
      });
    } else {
      request.reply(status_codes::NotFound, "Unknown POST endpoint");
    }
  });
  std::cout<<" avant wait " <<std::endl;
  listener.open().wait();
  log.log("[INFO] HTTP control server started on port 8080");
  std::cin.get(); // bloque pour garder le serveur actif
  listener.close().wait();
  std::cout<<" apres wait " <<std::endl;
}

int main() {
    Logger logger;
    DummySocket daq(logger);

    daq.initialise();
    daq.configure();

    std::thread server_thread(start_http_server, std::ref(daq), std::ref(logger));

    std::cout << "HTTP control available on port 8080 (GET /status, /addtokens?count=XXX, POST /start, /stop, /pause, /resume, /addtokens)" << std::endl;
    while (1)
      ::sleep(10);
    server_thread.join();
    return 0;
}
