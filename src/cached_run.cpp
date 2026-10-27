#include "cached_run.h"

#include <algorithm>
#include <iostream>

#include "cache/cache_key.h"
#include "cache/cache_store.h"

int runCached(const ClangTidy& tool, const Invocation& invocation) {
  fs::path projectRoot = ClangTidy::projectRoot(fs::current_path());
  fs::path buildDir = invocation.buildDir.empty() ? fs::path{} : fs::absolute(invocation.buildDir).lexically_normal();
  CacheKeyBuilder keyBuilder(tool.fingerprint(invocation), ProjectPaths{.root = projectRoot, .buildDir = buildDir});

  CacheStore store = CacheStore::forProject(projectRoot);
  const auto cached = store.load();

  int worstExitCode = 0;
  for (size_t i = 0; i < invocation.files.size(); ++i) {
    std::string key = keyBuilder.keyFor(invocation.files[i], ClangTidy::configFilesFor(invocation.files[i]));
    ClangTidy::reportProgress(invocation, i);
    RunResult result;
    auto hit = cached.find(key);
    if (hit != cached.end()) {
      result = hit->second;
      std::cout << result.out << std::flush;
      std::cerr << result.err << std::flush;
    } else {
      result = tool.run(invocation, i);
      store.append(key, result);
    }
    worstExitCode = std::max(worstExitCode, result.exitCode);
  }
  return worstExitCode;
}
