#include "cache/cache_key.h"

#include <algorithm>
#include <array>
#include <string_view>

#include "helpers/hash.h"

namespace {

fs::path resolveCompiler(const CompileCommand& command) {
  const std::string& name = command.arguments.front();
  if (name.find('/') == std::string::npos) {
    return findExecutable(name);
  }
  fs::path path = name;
  return path.is_absolute() ? path : (command.directory / path).lexically_normal();
}

uint64_t headerFingerprint(const ProjectPaths& paths) {
  constexpr std::array<std::string_view, 6> headerExtensions{".h", ".hh", ".hpp", ".hxx", ".inc", ".inl"};
  uint64_t fingerprint = 0;
  std::error_code errorCode;
  fs::recursive_directory_iterator entryIterator(paths.root, fs::directory_options::skip_permission_denied, errorCode);
  const fs::recursive_directory_iterator end;
  for (; entryIterator != end; entryIterator.increment(errorCode)) {
    const fs::path& path = entryIterator->path();
    if (entryIterator->is_directory(errorCode)) {
      if (path.filename().string().starts_with('.') || path == paths.buildDir) {
        entryIterator.disable_recursion_pending();
      }
    } else if (std::ranges::find(headerExtensions, path.extension().string()) != headerExtensions.end()) {
      fingerprint += fnv1a(path.string() + modificationTime(path));
    }
  }
  return fingerprint;
}

}  // namespace

CacheKeyBuilder::CacheKeyBuilder(Fingerprint toolFingerprint, ProjectPaths paths)
    : toolFingerprint_(std::move(toolFingerprint)), paths_(std::move(paths)) {}

const std::string& CacheKeyBuilder::contentHashOf(const fs::path& path) {
  auto found = contentHashes_.find(path);
  if (found == contentHashes_.end()) {
    found = contentHashes_.emplace(path, contentHash(path)).first;
  }
  return found->second;
}

const CompileDatabase* CacheKeyBuilder::databaseFor(const fs::path& sourceFile) {
  fs::path databaseFile = paths_.buildDir.empty() ? findUp(sourceFile.parent_path(), "compile_commands.json")
                                                  : paths_.buildDir / "compile_commands.json";
  if (databaseFile.empty()) {
    return nullptr;
  }
  auto found = databases_.find(databaseFile);
  if (found == databases_.end()) {
    found = databases_.emplace(databaseFile, CompileDatabase::load(databaseFile)).first;
  }
  return found->second ? &*found->second : nullptr;
}

void CacheKeyBuilder::appendDependencies(Fingerprint& key, const std::optional<CompileCommand>& command) {
  if (command) {
    fs::path compiler = resolveCompiler(*command);
    if (!compiler.empty()) {
      key.append(compiler.string());
      key.append(fileSignature(compiler));
    }
  }
  if (!headerFingerprint_) {
    headerFingerprint_ = std::to_string(headerFingerprint(paths_));
  }
  key.append(*headerFingerprint_);
}

std::string CacheKeyBuilder::keyFor(const fs::path& sourceFile, const std::vector<fs::path>& configFiles) {
  Fingerprint key = toolFingerprint_;
  key.append(sourceFile.string());
  key.append(contentHashOf(sourceFile));
  for (const fs::path& configFile : configFiles) {
    key.append(configFile.string());
    key.append(contentHashOf(configFile));
  }

  std::optional<CompileCommand> command;
  if (const CompileDatabase* database = databaseFor(sourceFile); database != nullptr) {
    command = database->find(sourceFile);
  }
  if (command) {
    key.append(command->directory.string());
    for (const std::string& argument : command->arguments) {
      key.append(argument);
    }
  }
  appendDependencies(key, command);
  return key.digest();
}
