#include "Valley/Assets/AssetTypes.h"

#include <iomanip>
#include <sstream>

namespace Valley::Assets {

AssetUuid AssetUuid::from_stable_string(std::string_view source)
{
    AssetUuid uuid;
    std::uint64_t h = 1469598103934665603ull;
    for (const char ch : source) {
        h ^= static_cast<std::uint8_t>(ch);
        h *= 1099511628211ull;
    }
    for (std::size_t i = 0; i < uuid.bytes.size(); ++i) {
        h ^= (h >> 33);
        h *= 0xff51afd7ed558ccdULL;
        uuid.bytes[i] = static_cast<std::uint8_t>((h >> ((i % 8) * 8)) & 0xffu);
    }
    return uuid;
}

std::string AssetUuid::to_string() const
{
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (const auto byte : bytes) {
        out << std::setw(2) << static_cast<int>(byte);
    }
    return out.str();
}

} // namespace Valley::Assets
