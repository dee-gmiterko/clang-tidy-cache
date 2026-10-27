#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

std::optional<std::string> stringMember(const nlohmann::json& entry, const char* name);
