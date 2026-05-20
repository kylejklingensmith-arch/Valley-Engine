#include "Valley/Core/Logger.h"

#include <iostream>

namespace Valley::Core::Log {
namespace {
void write(std::string_view level, std::string_view area, std::string_view message)
{
    std::cout << '[' << level << "][" << area << "] " << message << '\n';
}
} // namespace

void info(std::string_view area, std::string_view message)
{
    write("Info", area, message);
}

void warn(std::string_view area, std::string_view message)
{
    write("Warn", area, message);
}

void error(std::string_view area, std::string_view message)
{
    write("Error", area, message);
}

} // namespace Valley::Core::Log
