#pragma once

#include <string>

constexpr int kExecFailedStatus = 127;
constexpr int kSignalStatusBase = 128;

struct RunResult {
  int exitCode = 0;
  std::string out;
  std::string err;
};

inline RunResult execFailed() {
  RunResult result;
  result.exitCode = kExecFailedStatus;
  return result;
}
