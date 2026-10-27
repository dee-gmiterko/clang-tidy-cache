#include "helpers/json_utils.h"

std::optional<std::string> stringMember(const nlohmann::json& entry, const char* name) {
  auto member = entry.find(name);
  if (member == entry.end() || !member->is_string()) {
    return std::nullopt;
  }
  return member->get<std::string>();
}
