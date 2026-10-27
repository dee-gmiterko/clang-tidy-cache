#include "clang_tidy/clang_tidy.h"

#include <array>
#include <cstdlib>
#include <iostream>

#include "helpers/fingerprint.h"
#include "helpers/process/process.h"

namespace {

constexpr const char* kExecutableName = "clang-tidy";
constexpr std::array<const char*, 5> kIncludeEnvironment{"CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH",
                                                         "OBJC_INCLUDE_PATH", "SDKROOT"};
constexpr std::array<const char*, 2> kConfigNames{".clang-tidy", "_clang-tidy"};

}  // namespace

std::optional<ClangTidy> ClangTidy::locate() {
  fs::path executable = findExecutable(kExecutableName);
  if (executable.empty()) {
    return std::nullopt;
  }
  return ClangTidy(std::move(executable));
}

fs::path ClangTidy::projectRoot(const fs::path& startDirectory) {
  fs::path configFile = findUp(startDirectory, kConfigNames.front());
  return configFile.empty() ? startDirectory : configFile.parent_path();
}

std::vector<fs::path> ClangTidy::configFilesFor(const fs::path& sourceFile) {
  std::vector<fs::path> configFiles;
  std::error_code errorCode;
  for (fs::path directory = sourceFile.parent_path();; directory = directory.parent_path()) {
    for (const char* name : kConfigNames) {
      fs::path config = directory / name;
      if (fs::is_regular_file(config, errorCode)) {
        configFiles.push_back(std::move(config));
      }
    }
    if (directory == directory.parent_path()) {
      return configFiles;
    }
  }
}

void ClangTidy::reportProgress(const Invocation& invocation, size_t fileIndex) {
  if (invocation.files.size() > 1) {
    std::cerr << "[" << fileIndex + 1 << "/" << invocation.files.size() << "] Processing file "
              << invocation.files[fileIndex].string() << ".\n"
              << std::flush;
  }
}

Fingerprint ClangTidy::fingerprint(const Invocation& invocation) const {
  Fingerprint result;
  result.append(invocation.keyArgs);
  result.append(fs::current_path().string());
  result.append(executable_.string());
  result.append(fileSignature(executable_));
  for (const char* variable : kIncludeEnvironment) {
    const char* value = std::getenv(variable);
    result.append(value != nullptr ? value : "");
  }
  for (const fs::path& configFile : invocation.configFiles) {
    result.append(configFile.string());
    result.append(contentHash(configFile));
  }
  return result;
}

int ClangTidy::passThrough(const Invocation& invocation) const { return runInherited(executable_, invocation.args); }

RunResult ClangTidy::run(const Invocation& invocation, size_t fileIndex) const {
  return runCaptured(executable_, invocation.argsForFile(fileIndex), Echo::On);
}
