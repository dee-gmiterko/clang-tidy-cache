#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "helpers/filesystem.h"

struct CompileCommand {
  fs::path directory;
  std::vector<std::string> arguments;
};

class CompileDatabase {
 public:
  static std::optional<CompileDatabase> load(const fs::path& jsonFile);

  [[nodiscard]] std::optional<CompileCommand> find(const fs::path& sourceFile) const;

 private:
  std::vector<std::pair<fs::path, CompileCommand>> entries_;
};
