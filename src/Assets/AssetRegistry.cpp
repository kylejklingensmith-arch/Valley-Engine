#include "Valley/Assets/AssetRegistry.h"

#include <stdexcept>

namespace Valley::Assets {

std::string stable_asset_key(AssetType type, std::string_view virtual_path)
{
    const char* type_name = "unknown";
    switch (type) {
    case AssetType::Mesh: type_name = "mesh"; break;
    case AssetType::Material: type_name = "material"; break;
    case AssetType::Texture: type_name = "texture"; break;
    }

    std::string key(type_name);
    key.push_back(':');
    key.append(virtual_path);
    return key;
}

AssetHandle AssetRegistry::register_asset(AssetType type, const std::string& virtual_path)
{
    const auto found = m_by_path.find(virtual_path);
    if (found != m_by_path.end()) {
        if (found->second.type != type) {
            throw std::logic_error("Asset path was already registered with a different resource type.");
        }
        return found->second;
    }

    AssetHandle handle { .id = AssetUuid::from_stable_string(stable_asset_key(type, virtual_path)), .type = type };
    m_by_path.emplace(virtual_path, handle);
    m_by_id.emplace(handle.id.to_string(), AssetRecord { .handle = handle, .virtual_path = virtual_path });
    return handle;
}

std::optional<AssetHandle> AssetRegistry::find(const std::string& virtual_path) const
{
    const auto found = m_by_path.find(virtual_path);
    if (found == m_by_path.end()) {
        return std::nullopt;
    }
    return found->second;
}

std::optional<AssetRecord> AssetRegistry::find_by_id(const AssetId& id) const
{
    const auto found = m_by_id.find(id.to_string());
    if (found == m_by_id.end()) {
        return std::nullopt;
    }
    return found->second;
}

std::vector<std::string> AssetRegistry::all_virtual_paths() const
{
    std::vector<std::string> paths;
    paths.reserve(m_by_path.size());
    for (const auto& [path, _] : m_by_path) {
        paths.push_back(path);
    }
    return paths;
}

} // namespace Valley::Assets
