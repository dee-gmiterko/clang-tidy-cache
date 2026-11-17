#include "cache/cache_key.h"

#include "compile_db/dependencies.h"

namespace {

fs::path resolveCompiler(const CompileCommand& command) {
  const std::string& name = command.arguments.front();
  if (name.find('/') == std::string::npos) {
    return findExecutable(name);
  }
  fs::path path = name;
  return path.is_absolute() ? path : (command.directory / path).lexically_normal();
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
  const auto& database = found->second;
  return database ? &*database : nullptr;
}

bool CacheKeyBuilder::appendDependencies(Fingerprint& key, const std::optional<CompileCommand>& command) {
  if (!command) {
    return false;
  }
  fs::path compiler = resolveCompiler(*command);
  if (compiler.empty()) {
    return false;
  }
  std::optional<std::vector<fs::path>> dependencies = includedFiles(compiler, *command);
  if (!dependencies) {
    return false;
  }
  key.append(compiler.string());
  key.append(fileSignature(compiler));
  for (const fs::path& dependency : *dependencies) {
    key.append(dependency.string());
    key.append(contentHashOf(dependency));
  }
  return true;
}

std::optional<std::string> CacheKeyBuilder::keyFor(const fs::path& sourceFile, const std::vector<fs::path>& configFiles) {
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
  if (!appendDependencies(key, command)) {
    return std::nullopt;
  }
  return key.digest();
}

size_t CacheKeyBuilder::projectFileCount(const fs::path& sourceFile) {
  const CompileDatabase* database = databaseFor(sourceFile);
  return database != nullptr ? database->fileCount() : 0;
}
