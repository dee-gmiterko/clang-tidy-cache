#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "helpers/filesystem.h"
#include "helpers/process/run_result.h"

enum class Echo : std::uint8_t { Off, On };

int runInherited(const fs::path& tool, const std::vector<std::string>& args);

RunResult runCaptured(const fs::path& tool, const std::vector<std::string>& args, Echo echo,
                      const fs::path& workingDirectory = {});
