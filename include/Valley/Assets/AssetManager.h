#pragma once

#include "Valley/Assets/AssetRegistry.h"

#include <future>
#include <memory>
#include <optional>
#include <cstddef>
#include <unordered_map>

namespace Valley::Assets {

struct AssetBrowserEntry {
    std::string virtual_path;
    AssetType type = AssetType::Mesh;
    int ref_count = 0;
    bool loaded = false;
};

class AssetManager {
public:
    AssetManager();

    AssetHandle import_gltf(const std::string& virtual_path, const std::string& source_path);
    AssetHandle import_png(const std::string& virtual_path, const std::string& source_path);

    std::shared_ptr<const MeshAsset> acquire_mesh(const AssetHandle& handle);
    std::shared_ptr<const TextureAsset> acquire_texture(const AssetHandle& handle);

    std::future<AssetHandle> import_gltf_async(const std::string& virtual_path, const std::string& source_path);
    std::future<AssetHandle> import_png_async(const std::string& virtual_path, const std::string& source_path);

    bool hot_reload(const AssetHandle& handle);
    [[nodiscard]] std::vector<AssetBrowserEntry> browser_snapshot() const;
    [[nodiscard]] std::size_t mesh_cache_bytes() const;
    [[nodiscard]] std::size_t texture_cache_bytes() const;

private:
    struct MeshRecord { MeshAsset asset; std::shared_ptr<MeshAsset> cache; int ref_count = 0; };
    struct TextureRecord { TextureAsset asset; std::shared_ptr<TextureAsset> cache; int ref_count = 0; };

    AssetRegistry m_registry;
    std::unordered_map<std::string, AssetHandle> m_sources;
    std::unordered_map<std::string, MeshRecord> m_meshes;
    std::unordered_map<std::string, TextureRecord> m_textures;

    static MeshAsset load_gltf_stub(const std::string& virtual_path, const std::string& source_path);
    static TextureAsset load_png_stub(const std::string& virtual_path, const std::string& source_path);
};

} // namespace Valley::Assets
