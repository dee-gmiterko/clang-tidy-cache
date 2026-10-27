#include "helpers/filesystem.h"

#include <unistd.h>

#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string_view>

#include "helpers/hash.h"

std::string modificationTime(const fs::path& path) {
  std::error_code errorCode;
  auto time = fs::last_write_time(path, errorCode);
  return errorCode ? "0" : std::to_string(time.time_since_epoch().count());
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

fs::path findExecutable(const std::string& name) {
  const char* envPath = std::getenv("PATH");
  std::string searchPath = envPath != nullptr ? envPath : "";
  size_t begin = 0;
  while (begin <= searchPath.size()) {
    size_t end = searchPath.find(':', begin);
    if (end == std::string::npos) {
      end = searchPath.size();
    }
    fs::path candidate = fs::path(searchPath.substr(begin, end - begin)) / name;
    std::error_code errorCode;
    if (fs::is_regular_file(candidate, errorCode) && access(candidate.c_str(), X_OK) == 0) {
      return candidate;
    }
    begin = end + 1;
  }
  return {};
}

std::string readFile(const fs::path& path) {
  std::ifstream file(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(file), {}};
}
