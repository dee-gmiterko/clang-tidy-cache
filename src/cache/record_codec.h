#pragma once

#include <string>
#include <vector>

#include "helpers/filesystem.h"
#include "helpers/process/run_result.h"

struct Record {
  std::string key;
  RunResult result;
};

std::vector<Record> readRecords(const fs::path& path);
std::string serialize(const std::string& key, const RunResult& result);
