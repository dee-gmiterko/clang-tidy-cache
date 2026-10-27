#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "compile_db/compile_db.h"
#include "helpers/filesystem.h"
#include "helpers/fingerprint.h"

struct ProjectPaths {
  fs::path root;
  fs::path buildDir;
};

class CacheKeyBuilder {
 public:
  CacheKeyBuilder(Fingerprint toolFingerprint, ProjectPaths paths);

  std::string keyFor(const fs::path& sourceFile, const std::vector<fs::path>& configFiles);
  size_t projectFileCount(const fs::path& sourceFile);

 private:
  const std::string& contentHashOf(const fs::path& path);
  const CompileDatabase* databaseFor(const fs::path& sourceFile);
  void appendDependencies(Fingerprint& key, const std::optional<CompileCommand>& command);

  Fingerprint toolFingerprint_;
  ProjectPaths paths_;
  std::optional<std::string> headerFingerprint_;
  std::map<fs::path, std::string> contentHashes_;
  std::map<fs::path, std::optional<CompileDatabase>> databases_;
};
