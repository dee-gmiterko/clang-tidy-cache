#pragma once

#include <optional>
#include <set>
#include <string>

#include "helpers/filesystem.h"
#include "helpers/hash.h"
#include "helpers/process/run_result.h"

inline std::string projectId(const fs::path& projectRoot) { return std::to_string(fnv1a(projectRoot.string())); }

class CacheStore {
 public:
  CacheStore() = default;
  CacheStore(const CacheStore&) = delete;
  CacheStore& operator=(const CacheStore&) = delete;
  virtual ~CacheStore() = default;

  [[nodiscard]] virtual std::optional<RunResult> lookup(const std::string& key) const = 0;
  virtual void store(const std::string& key, const RunResult& result) = 0;
  virtual void finish(const std::set<std::string>& liveKeys, size_t projectFileCount) = 0;
};
