#pragma once

#include <string>
#include <vector>

#include "helpers/filesystem.h"

struct Invocation {
  std::vector<std::string> args;
  std::vector<int> fileIndexOfArg;
  std::vector<fs::path> files;
  std::string keyArgs;
  std::string buildDir;
  std::vector<fs::path> configFiles;
  bool bypassCache = false;

  [[nodiscard]] std::vector<std::string> argsForFile(size_t fileIndex) const;
};

Invocation parseInvocation(int argc, char** argv);
