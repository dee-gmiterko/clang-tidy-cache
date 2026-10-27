#include "helpers/hash.h"

#include <format>

namespace {
constexpr uint64_t kFnvPrime = 1099511628211ULL;
}

uint64_t fnv1a(const std::string& data, uint64_t hash) {
  for (unsigned char byte : data) {
    hash ^= byte;
    hash *= kFnvPrime;
  }
  return hash;
}

std::string toHex(uint64_t value) { return std::format("{:016x}", value); }
