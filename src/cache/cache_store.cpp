#include "cache/cache_store.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>

#include "helpers/hash.h"

namespace {

fs::path cacheDirectory() {
  const char* xdgCacheHome = std::getenv("XDG_CACHE_HOME");
  const char* home = std::getenv("HOME");
  fs::path base = (xdgCacheHome != nullptr && *xdgCacheHome != '\0')
                      ? fs::path(xdgCacheHome)
                      : fs::path(home != nullptr ? home : ".") / ".cache";
  return base / "clang-tidy-cache";
}

}  // namespace

CacheStore CacheStore::forProject(const fs::path& projectRoot) {
  fs::path directory = cacheDirectory();
  std::error_code errorCode;
  fs::create_directories(directory, errorCode);
  return CacheStore(directory / std::to_string(fnv1a(projectRoot.string())));
}

std::map<std::string, RunResult> CacheStore::load() const {
  std::map<std::string, RunResult> records;
  std::ifstream file(file_, std::ios::binary);
  std::string key;
  int exitCode = 0;
  size_t outSize = 0;
  size_t errSize = 0;
  while (file >> key >> exitCode >> outSize >> errSize) {
    file.get();
    RunResult result;
    result.exitCode = exitCode;
    result.out.resize(outSize);
    result.err.resize(errSize);
    file.read(result.out.data(), static_cast<std::streamsize>(outSize));
    file.read(result.err.data(), static_cast<std::streamsize>(errSize));
    if (!file) {
      break;
    }
    records[key] = std::move(result);
  }
  return records;
}

void CacheStore::append(const std::string& key, const RunResult& result) const {
  std::string record = key + ' ' + std::to_string(result.exitCode) + ' ' + std::to_string(result.out.size()) + ' ' +
                       std::to_string(result.err.size()) + '\n' + result.out + result.err;
  int fileDescriptor = open(file_.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
  if (fileDescriptor < 0) {
    return;
  }
  ssize_t written = write(fileDescriptor, record.data(), record.size());
  static_cast<void>(written);
  close(fileDescriptor);
}
