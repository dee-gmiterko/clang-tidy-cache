#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

std::string modificationTime(const fs::path& path);
std::string fileSignature(const fs::path& path);
std::string contentHash(const fs::path& path);
fs::path resolveAgainst(const fs::path& directory, const fs::path& path);
fs::path findUp(fs::path directory, const std::string& name);
fs::path findExecutable(const std::string& name);
std::string readFile(const fs::path& path);
