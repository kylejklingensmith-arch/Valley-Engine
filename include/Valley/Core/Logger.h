#pragma once

#include <string_view>

namespace Valley::Core::Log {

void info(std::string_view area, std::string_view message);
void warn(std::string_view area, std::string_view message);
void error(std::string_view area, std::string_view message);

} // namespace Valley::Core::Log
