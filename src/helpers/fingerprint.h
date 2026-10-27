#pragma once

#include <string>

class Fingerprint {
 public:
  void append(const std::string& field);

  [[nodiscard]] std::string digest() const;

 private:
  std::string data_;
};
