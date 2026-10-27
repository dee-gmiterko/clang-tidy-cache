#include "compile_db/compile_db.h"

#include <nlohmann/json.hpp>
#include <set>

#include "helpers/json_utils.h"
#include "helpers/text/shell.h"

namespace {

std::vector<std::string> argumentsOf(const nlohmann::json& entry) {
  auto arguments = entry.find("arguments");
  if (arguments != entry.end() && arguments->is_array()) {
    std::vector<std::string> result;
    result.reserve(arguments->size());
    for (const nlohmann::json& argument : *arguments) {
      if (argument.is_string()) {
        result.push_back(argument.get<std::string>());
      }
    }
    return result;
  }
  std::optional<std::string> command = stringMember(entry, "command");
  return command ? splitShellCommand(*command) : std::vector<std::string>{};
}

}  // namespace

std::optional<CompileDatabase> CompileDatabase::load(const fs::path& jsonFile) {
  nlohmann::json document = nlohmann::json::parse(readFile(jsonFile), nullptr, false);
  if (!document.is_array()) {
    return std::nullopt;
  }
  CompileDatabase database;
  for (const nlohmann::json& entry : document) {
    std::optional<std::string> directory = stringMember(entry, "directory");
    std::optional<std::string> file = stringMember(entry, "file");
    if (!directory || !file) {
      continue;
    }
    CompileCommand command{.directory = *directory, .arguments = argumentsOf(entry)};
    if (command.arguments.empty()) {
      continue;
    }
    database.entries_.emplace_back(resolveAgainst(command.directory, *file), std::move(command));
  }
  return database;
}

std::optional<CompileCommand> CompileDatabase::find(const fs::path& sourceFile) const {
  for (const auto& [path, command] : entries_) {
    if (path == sourceFile) {
      return command;
    }
  }
  return std::nullopt;
}

size_t CompileDatabase::fileCount() const {
  std::set<fs::path> files;
  for (const auto& entry : entries_) {
    files.insert(entry.first);
  }
  return files.size();
}
