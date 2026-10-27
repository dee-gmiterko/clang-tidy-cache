#include "cache/file/stale_projects.h"

#include <chrono>

namespace {

constexpr auto kMaxAge = std::chrono::hours(24 * 30);

bool isCacheFileName(const std::string& name) {
  std::string stem = name.substr(0, name.find('.'));
  return !stem.empty() && stem.find_first_not_of("0123456789") == std::string::npos;
}

}  // namespace

void removeStaleProjects(const fs::path& currentFile) {
  std::error_code errorCode;
  const auto cutoff = fs::file_time_type::clock::now() - kMaxAge;
  for (fs::directory_iterator it(currentFile.parent_path(), errorCode), end; !errorCode && it != end; it.increment(errorCode)) {
    const fs::path& path = it->path();
    if (path == currentFile || !isCacheFileName(path.filename().string())) {
      continue;
    }
    std::error_code entryError;
    if (fs::last_write_time(path, entryError) < cutoff && !entryError) {
      fs::remove(path, entryError);
    }
  }
}
