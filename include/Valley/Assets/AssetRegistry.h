#pragma once

#include "Valley/Assets/AssetTypes.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Valley::Assets {

class AssetRegistry {
public:
    AssetHandle register_asset(AssetType type, const std::string& virtual_path);
    [[nodiscard]] std::optional<AssetHandle> find(const std::string& virtual_path) const;
    [[nodiscard]] std::vector<std::string> all_virtual_paths() const;

private:
    std::unordered_map<std::string, AssetHandle> m_by_path;
};

} // namespace Valley::Assets
