cmake_minimum_required(VERSION 3.16)
project(clang-tidy-cache CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
include(cmake/CPM.cmake)
CPMAddPackage("gh:nlohmann/json@3.11.3")

add_executable(clang-tidy-cache
  src/main.cpp
  src/cached_run.cpp
  src/invocation.cpp
  src/cache/cache_key.cpp
  src/cache/record_codec.cpp
  src/cache/file/file_store.cpp
  src/cache/file/stale_projects.cpp
  src/clang_tidy/clang_tidy.cpp
  src/compile_db/compile_db.cpp
  src/helpers/filesystem.cpp
  src/helpers/fingerprint.cpp
  src/helpers/hash.cpp
  src/helpers/json_utils.cpp
  src/helpers/process/process.cpp
  src/helpers/text/shell.cpp)
target_include_directories(clang-tidy-cache PRIVATE src)
target_link_libraries(clang-tidy-cache PRIVATE nlohmann_json::nlohmann_json)
target_compile_options(clang-tidy-cache PRIVATE -Wall -Wextra)
install(TARGETS clang-tidy-cache)
