#pragma once

#include <cstdint>
#include <string>

constexpr uint64_t kFnvOffsetBasis = 1469598103934665603ULL;

uint64_t fnv1a(const std::string& data, uint64_t hash = kFnvOffsetBasis);
std::string toHex(uint64_t value);
