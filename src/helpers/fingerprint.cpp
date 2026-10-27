#include "helpers/fingerprint.h"

#include "helpers/hash.h"

namespace {

constexpr char kFieldSeparator = '\0';

}  // namespace

void Fingerprint::append(const std::string& field) {
  data_ += field;
  data_ += kFieldSeparator;
}

std::string Fingerprint::digest() const { return toHex(fnv1a(data_)); }
