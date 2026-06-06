#include "Valley/Assets/AssetRegistry.h"

namespace Valley::Assets {

AssetHandle AssetRegistry::register_asset(AssetType type, const std::string& virtual_path)
{
    const auto found = m_by_path.find(virtual_path);
    if (found != m_by_path.end()) {
        return found->second;
    }

    AssetHandle handle { .uuid = AssetUuid::from_stable_string(virtual_path), .type = type };
    m_by_path.emplace(virtual_path, handle);
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
