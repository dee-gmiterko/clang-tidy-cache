#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

template <size_t Count>
bool contains(const std::array<std::string_view, Count>& options, const std::string& name) {
  return std::ranges::find(options, name) != options.end();
}
