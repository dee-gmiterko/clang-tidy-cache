#include "helpers/filesystem.h"

#include <unistd.h>

#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string_view>
#include <vector>

#include "helpers/hash.h"

std::string modificationTime(const fs::path& path) {
  std::error_code errorCode;
  auto time = fs::last_write_time(path, errorCode);
  return errorCode ? "0" : std::to_string(static_cast<long long>(time.time_since_epoch().count()));
}

std::string fileSignature(const fs::path& path) {
  std::error_code errorCode;
  auto size = fs::file_size(path, errorCode);
  return modificationTime(path) + ':' + (errorCode ? "0" : std::to_string(size));
}

std::string contentHash(const fs::path& path) { return toHex(fnv1a(readFile(path))); }

fs::path resolveAgainst(const fs::path& directory, const fs::path& path) {
  return (path.is_absolute() ? path : directory / path).lexically_normal();
}

fs::path findUp(fs::path directory, const std::string& name) {
  std::error_code errorCode;
  for (;; directory = directory.parent_path()) {
    if (fs::exists(directory / name, errorCode)) {
      return directory / name;
    }
    if (directory == directory.parent_path()) {
      return {};
    }
  }
}

namespace {

std::vector<fs::path> searchDirectories() {
  const char* envPath = std::getenv("PATH");
  std::string searchPath = envPath != nullptr ? envPath : "";
  std::vector<fs::path> directories;
  size_t begin = 0;
  while (begin <= searchPath.size()) {
    size_t end = searchPath.find(':', begin);
    if (end == std::string::npos) {
      end = searchPath.size();
    }
    directories.emplace_back(searchPath.substr(begin, end - begin));
    begin = end + 1;
  }
  return directories;
}

bool isExecutableFile(const fs::path& path) {
  std::error_code errorCode;
  return fs::is_regular_file(path, errorCode) && access(path.c_str(), X_OK) == 0;
}

fs::path findVersionedExecutable(const std::string& name) {
  const std::string prefix = name + "-";
  fs::path best;
  unsigned long bestVersion = 0;
  for (const fs::path& directory : searchDirectories()) {
    std::error_code errorCode;
    for (fs::directory_iterator it(directory, errorCode), last; !errorCode && it != last; it.increment(errorCode)) {
      std::string filename = it->path().filename().string();
      std::string suffix = filename.starts_with(prefix) ? filename.substr(prefix.size()) : "";
      if (suffix.empty() || suffix.find_first_not_of("0123456789") != std::string::npos || suffix.size() > 9) {
        continue;
      }
      unsigned long version = std::stoul(suffix);
      if (version > bestVersion && isExecutableFile(it->path())) {
        best = it->path();
        bestVersion = version;
      }
    }
  }
  return best;
}

}  // namespace

fs::path findExecutable(const std::string& name) {
  for (const fs::path& directory : searchDirectories()) {
    fs::path candidate = directory / name;
    if (isExecutableFile(candidate)) {
      return candidate;
    }
  }
  return findVersionedExecutable(name);
}

std::string readFile(const fs::path& path) {
  std::ifstream file(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(file), {}};
}
