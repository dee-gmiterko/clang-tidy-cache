#pragma once

#include <map>

#include "cache/cache_store.h"

class FileStore : public CacheStore {
 public:
  explicit FileStore(const fs::path& projectRoot);

  [[nodiscard]] std::optional<RunResult> lookup(const std::string& key) const override;
  void store(const std::string& key, const RunResult& result) override;
  void finish(const std::set<std::string>& liveKeys, size_t projectFileCount) override;

 private:
  void compact(const std::set<std::string>& liveKeys, size_t capacity) const;

  fs::path file_;
  std::map<std::string, RunResult> byKey_;
  size_t recordCount_ = 0;
};
