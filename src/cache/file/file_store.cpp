#include "cache/file/file_store.h"

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <vector>

#include "cache/file/stale_projects.h"
#include "cache/record_codec.h"

namespace {

constexpr size_t kEntriesPerFile = 4;
constexpr size_t kTargetPercent = 75;

fs::path cacheDirectory() {
  const char* xdgCacheHome = std::getenv("XDG_CACHE_HOME");
  const char* home = std::getenv("HOME");
  fs::path base = (xdgCacheHome != nullptr && *xdgCacheHome != '\0')
                      ? fs::path(xdgCacheHome)
                      : fs::path(home != nullptr ? home : ".") / ".cache";
  return base / "clang-tidy-cache";
}

}  // namespace

FileStore::FileStore(const fs::path& projectRoot) {
  fs::path directory = cacheDirectory();
  std::error_code errorCode;
  fs::create_directories(directory, errorCode);
  file_ = directory / projectId(projectRoot);
  for (Record& record : readRecords(file_)) {
    ++recordCount_;
    byKey_[record.key] = std::move(record.result);
  }
}

std::optional<RunResult> FileStore::lookup(const std::string& key) const {
  auto found = byKey_.find(key);
  if (found == byKey_.end()) {
    return std::nullopt;
  }
  return found->second;
}

void FileStore::store(const std::string& key, const RunResult& result) {
  std::string record = serialize(key, result);
  int fileDescriptor = open(file_.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
  if (fileDescriptor < 0) {
    return;
  }
  ssize_t written = write(fileDescriptor, record.data(), record.size());
  static_cast<void>(written);
  close(fileDescriptor);
  ++recordCount_;
  byKey_[key] = result;
}

void FileStore::finish(const std::set<std::string>& liveKeys, size_t projectFileCount) {
  size_t capacity = projectFileCount * kEntriesPerFile;
  if (capacity > 0 && recordCount_ > capacity) {
    compact(liveKeys, capacity);
  }
  removeStaleProjects(file_);
}

void FileStore::compact(const std::set<std::string>& liveKeys, size_t capacity) const {
  int lockDescriptor = open(file_.c_str(), O_RDONLY);
  if (lockDescriptor < 0) {
    return;
  }
  if (flock(lockDescriptor, LOCK_EX | LOCK_NB) != 0) {
    close(lockDescriptor);
    return;
  }
  std::vector<Record> records = readRecords(file_);
  std::map<std::string, size_t> lastIndex;
  for (size_t i = 0; i < records.size(); ++i) {
    lastIndex[records[i].key] = i;
  }
  std::vector<const Record*> unique;
  for (size_t i = 0; i < records.size(); ++i) {
    if (lastIndex[records[i].key] == i) {
      unique.push_back(&records[i]);
    }
  }
  size_t target = capacity * kTargetPercent / 100;
  size_t toEvict = unique.size() > target ? unique.size() - target : 0;
  fs::path temporary = file_;
  temporary += ".tmp";
  {
    std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
    for (const Record* record : unique) {
      if (toEvict > 0 && !liveKeys.contains(record->key)) {
        --toEvict;
        continue;
      }
      out << serialize(record->key, record->result);
    }
    out.flush();
    if (!out) {
      std::error_code errorCode;
      fs::remove(temporary, errorCode);
      close(lockDescriptor);
      return;
    }
  }
  std::error_code errorCode;
  fs::rename(temporary, file_, errorCode);
  close(lockDescriptor);
}
