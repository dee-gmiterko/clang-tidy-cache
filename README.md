# clang-tidy-cache

A drop-in `clang-tidy` wrapper that caches results, so unchanged files are not re-analyzed.

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Usage

Use it in place of `clang-tidy`, with the same arguments:

```sh
clang-tidy-cache -p build src/main.cpp
```

Results are stored in cache `~/.cache/clang-tidy-cache`. Each project keeps at most about four entries per file of its compile database (oldest entries are dropped first), and projects whose cache has not been written for 30 days are deleted.

The cache key of a file covers the tool version, the source file, the `.clang-tidy` files, the compile command, the compiler, and every file the translation unit includes, system headers among them. Included files are identified by path and content. A file without a compile command, or whose preprocessor run fails, is never served from the cache.

## Contributing

Contributions are welcome. Please open an issue or a pull request. The project checks itself with its own tool: `make tidy` runs `clang-tidy-cache` on its own sources.

## License

MIT
