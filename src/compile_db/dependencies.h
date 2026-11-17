#pragma once

#include <optional>
#include <string>
#include <vector>

#include "compile_db/compile_db.h"
#include "helpers/filesystem.h"

std::vector<std::string> dependencyArguments(const CompileCommand& command);
std::vector<std::string> parseDependencyList(const std::string& text);

std::optional<std::vector<fs::path>> includedFiles(const fs::path& compiler, const CompileCommand& command);
