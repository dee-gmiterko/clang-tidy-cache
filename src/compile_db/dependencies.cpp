#include "compile_db/dependencies.h"

#include <algorithm>
#include <array>
#include <string_view>

#include "helpers/process/process.h"

namespace {

constexpr std::array<std::string_view, 5> kValueOptions{"-o", "-MF", "-MT", "-MQ", "-MJ"};
constexpr std::array<std::string_view, 8> kDroppedOptions{"-c", "-S", "-E", "-M", "-MM", "-MD", "-MMD", "-MP"};
constexpr std::array<std::string_view, 4> kJoinedValueOptions{"-MF", "-MT", "-MQ", "-MJ"};

bool isOneOf(const std::string& argument, const auto& options) {
  return std::ranges::find(options, argument) != options.end();
}

constexpr std::array<std::string_view, 4> kLongOptionsStartingWithO{"-objc", "-object", "-openmp", "-opt"};

bool isJoinedOutput(const std::string& argument) {
  if (!argument.starts_with("-o") || argument.size() == 2) {
    return false;
  }
  return std::ranges::none_of(kLongOptionsStartingWithO,
                              [&](std::string_view option) { return argument.starts_with(option); });
}

bool hasJoinedValue(const std::string& argument) {
  if (isJoinedOutput(argument)) {
    return true;
  }
  return std::ranges::any_of(kJoinedValueOptions, [&](std::string_view option) { return argument.starts_with(option); });
}

bool isLineContinuation(const std::string& text, size_t position) {
  if (text[position] != '\\' || position + 1 >= text.size()) {
    return false;
  }
  return text[position + 1] == '\n' || (text[position + 1] == '\r' && position + 2 < text.size() && text[position + 2] == '\n');
}

}  // namespace

std::vector<std::string> dependencyArguments(const CompileCommand& command) {
  std::vector<std::string> result;
  for (size_t i = 1; i < command.arguments.size(); ++i) {
    const std::string& argument = command.arguments[i];
    if (isOneOf(argument, kValueOptions)) {
      ++i;
    } else if (!isOneOf(argument, kDroppedOptions) && !hasJoinedValue(argument)) {
      result.push_back(argument);
    }
  }
  result.emplace_back("-M");
  result.emplace_back("-E");
  return result;
}

std::vector<std::string> parseDependencyList(const std::string& text) {
  std::vector<std::string> tokens;
  std::string current;
  bool inToken = false;
  auto finish = [&] {
    if (inToken) {
      tokens.push_back(std::move(current));
      current.clear();
      inToken = false;
    }
  };
  for (size_t i = 0; i < text.size(); ++i) {
    char character = text[i];
    if (isLineContinuation(text, i)) {
      finish();
      i += text[i + 1] == '\r' ? 2 : 1;
    } else if (character == ' ' || character == '\t' || character == '\n' || character == '\r') {
      finish();
    } else if (character == '\\' && i + 1 < text.size() && (text[i + 1] == ' ' || text[i + 1] == '#' || text[i + 1] == '\\')) {
      current += text[++i];
      inToken = true;
    } else if (character == '$' && i + 1 < text.size() && text[i + 1] == '$') {
      current += '$';
      ++i;
      inToken = true;
    } else {
      current += character;
      inToken = true;
    }
  }
  finish();
  if (!tokens.empty()) {
    tokens.erase(tokens.begin());
  }
  return tokens;
}

std::optional<std::vector<fs::path>> includedFiles(const fs::path& compiler, const CompileCommand& command) {
  RunResult result = runCaptured(compiler, dependencyArguments(command), Echo::Off, command.directory);
  if (result.exitCode != 0) {
    return std::nullopt;
  }
  std::vector<fs::path> files;
  for (const std::string& dependency : parseDependencyList(result.out)) {
    files.push_back(resolveAgainst(command.directory, dependency));
  }
  std::ranges::sort(files);
  files.erase(std::ranges::unique(files).begin(), files.end());
  return files;
}
