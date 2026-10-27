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

Results are stored in cache `~/.cache/clang-tidy-cache`.

## Contributing

Contributions are welcome. Please open an issue or a pull request. The project checks itself with its own tool: `make tidy` runs `clang-tidy-cache` on its own sources.

## License

MIT
