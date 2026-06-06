#pragma once

#include "Valley/Assets/AssetTypes.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Valley::Assets {

struct AssetRecord {
    AssetHandle handle;
    std::string virtual_path;
};

class AssetRegistry {
public:
    AssetHandle register_asset(AssetType type, const std::string& virtual_path);
    [[nodiscard]] std::optional<AssetHandle> find(const std::string& virtual_path) const;
    [[nodiscard]] std::optional<AssetRecord> find_by_id(const AssetId& id) const;
    [[nodiscard]] std::vector<std::string> all_virtual_paths() const;

private:
    std::unordered_map<std::string, AssetHandle> m_by_path;
    std::unordered_map<std::string, AssetRecord> m_by_id;
};

[[nodiscard]] std::string stable_asset_key(AssetType type, std::string_view virtual_path);

} // namespace Valley::Assets
