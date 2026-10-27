#pragma once

#include "clang_tidy/clang_tidy.h"
#include "invocation.h"

int runCached(const ClangTidy& tool, const Invocation& invocation);
