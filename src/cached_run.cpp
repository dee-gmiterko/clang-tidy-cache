#include "cached_run.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <set>

#include "cache/cache_key.h"
#include "cache/cache_store.h"
#include "cache/file/file_store.h"

int runCached(const ClangTidy& tool, const Invocation& invocation) {
  fs::path projectRoot = ClangTidy::projectRoot(fs::current_path());
  fs::path buildDir = invocation.buildDir.empty() ? fs::path{} : fs::absolute(invocation.buildDir).lexically_normal();
  CacheKeyBuilder keyBuilder(tool.fingerprint(invocation), ProjectPaths{.root = projectRoot, .buildDir = buildDir});

  std::unique_ptr<CacheStore> store = std::make_unique<FileStore>(projectRoot);
  std::set<std::string> liveKeys;

  int worstExitCode = 0;
  for (size_t i = 0; i < invocation.files.size(); ++i) {
    std::string key = keyBuilder.keyFor(invocation.files[i], ClangTidy::configFilesFor(invocation.files[i]));
    liveKeys.insert(key);
    ClangTidy::reportProgress(invocation, i);
    RunResult result;
    if (std::optional<RunResult> hit = store->lookup(key)) {
      result = std::move(*hit);
      std::cout << result.out << std::flush;
      std::cerr << result.err << std::flush;
    } else {
      result = tool.run(invocation, i);
      store->store(key, result);
    }
    worstExitCode = std::max(worstExitCode, result.exitCode);
  }
  store->finish(liveKeys, keyBuilder.projectFileCount(invocation.files.front()));
  return worstExitCode;
}
