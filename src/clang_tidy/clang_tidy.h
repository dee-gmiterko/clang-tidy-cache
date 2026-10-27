#pragma once

#include <optional>
#include <string>
#include <vector>

#include "helpers/filesystem.h"
#include "helpers/fingerprint.h"
#include "helpers/process/run_result.h"
#include "invocation.h"

class ClangTidy {
 public:
  static std::optional<ClangTidy> locate();
  static fs::path projectRoot(const fs::path& startDirectory);
  static std::vector<fs::path> configFilesFor(const fs::path& sourceFile);
  static void reportProgress(const Invocation& invocation, size_t fileIndex);

  [[nodiscard]] Fingerprint fingerprint(const Invocation& invocation) const;
  [[nodiscard]] int passThrough(const Invocation& invocation) const;
  [[nodiscard]] RunResult run(const Invocation& invocation, size_t fileIndex) const;

 private:
  explicit ClangTidy(fs::path executable) : executable_(std::move(executable)) {}

  fs::path executable_;
};
