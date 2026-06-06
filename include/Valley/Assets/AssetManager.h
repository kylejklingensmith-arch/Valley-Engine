#pragma once

#include "Valley/Assets/AssetRegistry.h"

#include <cstddef>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
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

    MeshHandle register_mesh(const std::string& virtual_path, MeshAsset resource);
    MaterialHandle register_material(const std::string& virtual_path, MaterialAsset resource);
    TextureHandle register_texture(const std::string& virtual_path, TextureAsset resource);

    MeshHandle import_gltf(const std::string& virtual_path, const std::string& source_path);
    TextureHandle import_png(const std::string& virtual_path, const std::string& source_path);

    std::shared_ptr<const MeshAsset> acquire_mesh(const MeshHandle& handle);
    std::shared_ptr<const MaterialAsset> acquire_material(const MaterialHandle& handle);
    std::shared_ptr<const TextureAsset> acquire_texture(const TextureHandle& handle);

    [[nodiscard]] const MeshAsset* find_mesh(const MeshHandle& handle) const;
    [[nodiscard]] const MaterialAsset* find_material(const MaterialHandle& handle) const;
    [[nodiscard]] const TextureAsset* find_texture(const TextureHandle& handle) const;
    [[nodiscard]] std::optional<AssetHandle> find_asset(const std::string& virtual_path) const;

    std::future<MeshHandle> import_gltf_async(const std::string& virtual_path, const std::string& source_path);
    std::future<TextureHandle> import_png_async(const std::string& virtual_path, const std::string& source_path);

    bool hot_reload(const MeshHandle& handle);
    bool hot_reload(const TextureHandle& handle);
    [[nodiscard]] std::vector<AssetBrowserEntry> browser_snapshot() const;
    [[nodiscard]] std::size_t mesh_cache_bytes() const;
    [[nodiscard]] std::size_t texture_cache_bytes() const;

private:
    struct MeshRecord { MeshAsset asset; std::shared_ptr<MeshAsset> cache; int ref_count = 0; };
    struct MaterialRecord { MaterialAsset asset; std::shared_ptr<MaterialAsset> cache; int ref_count = 0; };
    struct TextureRecord { TextureAsset asset; std::shared_ptr<TextureAsset> cache; int ref_count = 0; };

    mutable std::mutex m_mutex;
    AssetRegistry m_registry;
    std::unordered_map<std::string, MeshRecord> m_meshes;
    std::unordered_map<std::string, MaterialRecord> m_materials;
    std::unordered_map<std::string, TextureRecord> m_textures;

    static MeshAsset load_gltf_stub(const std::string& virtual_path, const std::string& source_path);
    static TextureAsset load_png_stub(const std::string& virtual_path, const std::string& source_path);
};

MeshAsset create_ground_plane_mesh_resource(std::string debug_name);
MeshAsset create_cube_mesh_resource(std::string debug_name);
MaterialAsset create_placeholder_material(std::string debug_name);

} // namespace Valley::Assets
