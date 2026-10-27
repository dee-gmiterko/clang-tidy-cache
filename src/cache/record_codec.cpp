#include "cache/record_codec.h"

#include <fstream>

std::vector<Record> readRecords(const fs::path& path) {
  std::vector<Record> records;
  std::ifstream file(path, std::ios::binary);
  std::string key;
  int exitCode = 0;
  size_t outSize = 0;
  size_t errSize = 0;
  while (file >> key >> exitCode >> outSize >> errSize) {
    file.get();
    Record record{.key = key, .result = {}};
    record.result.exitCode = exitCode;
    record.result.out.resize(outSize);
    record.result.err.resize(errSize);
    file.read(record.result.out.data(), static_cast<std::streamsize>(outSize));
    file.read(record.result.err.data(), static_cast<std::streamsize>(errSize));
    if (!file) {
      break;
    }
    records.push_back(std::move(record));
  }
  return records;
}

std::string serialize(const std::string& key, const RunResult& result) {
  return key + ' ' + std::to_string(result.exitCode) + ' ' + std::to_string(result.out.size()) + ' ' +
         std::to_string(result.err.size()) + '\n' + result.out + result.err;
}
