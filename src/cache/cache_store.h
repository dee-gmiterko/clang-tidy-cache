#pragma once

#include <map>
#include <string>

#include "helpers/filesystem.h"
#include "helpers/process/run_result.h"

class CacheStore {
 public:
  explicit CacheStore(fs::path file) : file_(std::move(file)) {}

  static CacheStore forProject(const fs::path& projectRoot);

  [[nodiscard]] std::map<std::string, RunResult> load() const;
  void append(const std::string& key, const RunResult& result) const;

 private:
  fs::path file_;
};
