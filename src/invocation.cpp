#include "invocation.h"

#include <array>
#include <string_view>
#include <utility>

#include "helpers/text/contains.h"

namespace {

constexpr auto kValuedOptions = std::to_array<std::string_view>(
    {"p", "config-file", "config", "checks", "header-filter", "exclude-header-filter", "format-style", "export-fixes",
     "line-filter", "load", "vfsoverlay", "store-check-profile", "warnings-as-errors", "extra-arg", "extra-arg-before",
     "removed-arg"});

constexpr auto kUncacheableOptions = std::to_array<std::string_view>(
    {"fix", "fix-errors", "fix-notes", "export-fixes", "store-check-profile", "enable-check-profile", "list-checks",
     "dump-config", "verify-config", "explain-config", "version", "help"});

std::string optionName(const std::string& arg) {
  size_t begin = arg.starts_with("--") ? 2 : 1;
  return arg.substr(begin);
}

std::string optionNameWithoutValue(const std::string& arg) {
  std::string name = optionName(arg);
  return name.substr(0, name.find('='));
}

std::string optionInlineValue(const std::string& arg) {
  std::string name = optionName(arg);
  size_t equals = name.find('=');
  return equals == std::string::npos ? std::string{} : name.substr(equals + 1);
}

}  // namespace

Invocation parseInvocation(int argc, char** argv) {
  Invocation parsed;
  bool afterDashDash = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    parsed.args.push_back(arg);
    parsed.fileIndexOfArg.push_back(-1);
    if (afterDashDash) {
      parsed.keyArgs += arg + '\n';
    } else if (arg == "--") {
      afterDashDash = true;
      parsed.keyArgs += arg + '\n';
    } else if (arg.starts_with('-')) {
      parsed.keyArgs += arg + '\n';
      std::string name = optionNameWithoutValue(arg);
      std::string value = optionInlineValue(arg);
      if (contains(kUncacheableOptions, name)) {
        parsed.bypassCache = true;
      }
      if (contains(kValuedOptions, optionName(arg)) && i + 1 < argc) {
        value = argv[++i];
        parsed.args.push_back(value);
        parsed.fileIndexOfArg.push_back(-1);
        parsed.keyArgs += value + '\n';
        name = optionName(arg);
      }
      if (name == "p") {
        parsed.buildDir = value;
      } else if (name == "config-file") {
        parsed.configFiles.push_back(fs::absolute(value).lexically_normal());
      }
    } else {
      parsed.fileIndexOfArg.back() = static_cast<int>(parsed.files.size());
      parsed.files.push_back(fs::absolute(arg).lexically_normal());
    }
  }
  return parsed;
}

std::vector<std::string> Invocation::argsForFile(size_t fileIndex) const {
  std::vector<std::string> selected;
  for (size_t i = 0; i < args.size(); ++i) {
    if (fileIndexOfArg[i] < 0 || std::cmp_equal(fileIndexOfArg[i], fileIndex)) {
      selected.push_back(args[i]);
    }
  }
  return selected;
}
