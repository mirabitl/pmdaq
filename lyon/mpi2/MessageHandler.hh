#pragma once

#include <cstdint>
#include <exception>
#include <functional>
#include <string>
#include <utility>

namespace mpi2 {

using MPIFunctor = std::function<void(uint64_t, uint16_t, char*)>;

struct MpiException : public std::exception {
  explicit MpiException(std::string message) : _message(std::move(message)) {}
  const char* what() const noexcept override { return _message.c_str(); }

private:
  std::string _message;
};

class MessageHandler {
public:
  virtual ~MessageHandler() = default;
  virtual void processMessage(uint64_t id, uint16_t length, char* data) = 0;
};

}  // namespace mpi2
