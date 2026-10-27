#include "cached_run.h"
#include "clang_tidy/clang_tidy.h"
#include "helpers/process/run_result.h"
#include "invocation.h"

int main(int argc, char** argv) {
  Invocation invocation = parseInvocation(argc, argv);

  std::optional<ClangTidy> tool = ClangTidy::locate();
  if (!tool) {
    return kExecFailedStatus;
  }
  if (invocation.files.empty() || invocation.bypassCache) {
    return tool->passThrough(invocation);
  }
  return runCached(*tool, invocation);
}
