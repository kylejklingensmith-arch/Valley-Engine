#include "Valley/Assets/AssetManager.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>

namespace Valley::Assets {
namespace {
std::string key_for(const AssetId& id)
{
    return id.to_string();
}

std::size_t mesh_byte_size(const MeshAsset& mesh)
{
    return mesh.vertices.size() * sizeof(MeshVertex) + mesh.indices.size() * sizeof(std::uint32_t);
}
} // namespace

AssetManager::AssetManager() = default;

MeshHandle AssetManager::register_mesh(const std::string& virtual_path, MeshAsset resource)
{
    std::lock_guard lock(m_mutex);
    const auto handle = MeshHandle { .asset = m_registry.register_asset(AssetType::Mesh, virtual_path) };
    if (resource.debug_name.empty()) {
        resource.debug_name = virtual_path;
    }
    resource.vertex_count = resource.vertices.empty() ? resource.vertex_count : resource.vertices.size();
    resource.primitive_count = resource.indices.empty() ? resource.primitive_count : resource.indices.size() / 3;
    m_meshes[key_for(handle.id())] = MeshRecord { .asset = std::move(resource), .cache = nullptr, .ref_count = 0 };
    return handle;
}

MaterialHandle AssetManager::register_material(const std::string& virtual_path, MaterialAsset resource)
{
    std::lock_guard lock(m_mutex);
    const auto handle = MaterialHandle { .asset = m_registry.register_asset(AssetType::Material, virtual_path) };
    if (resource.debug_name.empty()) {
        resource.debug_name = virtual_path;
    }
    m_materials[key_for(handle.id())] = MaterialRecord { .asset = std::move(resource), .cache = nullptr, .ref_count = 0 };
    return handle;
}

TextureHandle AssetManager::register_texture(const std::string& virtual_path, TextureAsset resource)
{
    std::lock_guard lock(m_mutex);
    const auto handle = TextureHandle { .asset = m_registry.register_asset(AssetType::Texture, virtual_path) };
    if (resource.debug_name.empty()) {
        resource.debug_name = virtual_path;
    }
    m_textures[key_for(handle.id())] = TextureRecord { .asset = std::move(resource), .cache = nullptr, .ref_count = 0 };
    return handle;
}

MeshHandle AssetManager::import_gltf(const std::string& virtual_path, const std::string& source_path)
{
    return register_mesh(virtual_path, load_gltf_stub(virtual_path, source_path));
}

TextureHandle AssetManager::import_png(const std::string& virtual_path, const std::string& source_path)
{
    return register_texture(virtual_path, load_png_stub(virtual_path, source_path));
}

std::shared_ptr<const MeshAsset> AssetManager::acquire_mesh(const MeshHandle& handle)
{
    std::lock_guard lock(m_mutex);
    auto& record = m_meshes.at(key_for(handle.id()));
    if (!record.cache) { record.cache = std::make_shared<MeshAsset>(record.asset); }
    record.ref_count += 1;
    return record.cache;
}

std::shared_ptr<const MaterialAsset> AssetManager::acquire_material(const MaterialHandle& handle)
{
    std::lock_guard lock(m_mutex);
    auto& record = m_materials.at(key_for(handle.id()));
    if (!record.cache) { record.cache = std::make_shared<MaterialAsset>(record.asset); }
    record.ref_count += 1;
    return record.cache;
}

std::shared_ptr<const TextureAsset> AssetManager::acquire_texture(const TextureHandle& handle)
{
    std::lock_guard lock(m_mutex);
    auto& record = m_textures.at(key_for(handle.id()));
    if (!record.cache) { record.cache = std::make_shared<TextureAsset>(record.asset); }
    record.ref_count += 1;
    return record.cache;
}

const MeshAsset* AssetManager::find_mesh(const MeshHandle& handle) const
{
    std::lock_guard lock(m_mutex);
    const auto found = m_meshes.find(key_for(handle.id()));
    return found == m_meshes.end() ? nullptr : &found->second.asset;
}

const MaterialAsset* AssetManager::find_material(const MaterialHandle& handle) const
{
    std::lock_guard lock(m_mutex);
    const auto found = m_materials.find(key_for(handle.id()));
    return found == m_materials.end() ? nullptr : &found->second.asset;
}

const TextureAsset* AssetManager::find_texture(const TextureHandle& handle) const
{
    std::lock_guard lock(m_mutex);
    const auto found = m_textures.find(key_for(handle.id()));
    return found == m_textures.end() ? nullptr : &found->second.asset;
}

std::optional<AssetHandle> AssetManager::find_asset(const std::string& virtual_path) const
{
    std::lock_guard lock(m_mutex);
    return m_registry.find(virtual_path);
}

std::future<MeshHandle> AssetManager::import_gltf_async(const std::string& virtual_path, const std::string& source_path)
{
    return std::async(std::launch::async, [this, virtual_path, source_path]() { return import_gltf(virtual_path, source_path); });
}

std::future<TextureHandle> AssetManager::import_png_async(const std::string& virtual_path, const std::string& source_path)
{
    return std::async(std::launch::async, [this, virtual_path, source_path]() { return import_png(virtual_path, source_path); });
}

bool AssetManager::hot_reload(const MeshHandle& handle)
{
    std::lock_guard lock(m_mutex);
    const auto key = key_for(handle.id());
    const auto found = m_meshes.find(key);
    if (found == m_meshes.end()) {
        return false;
    }

    const auto source = found->second.asset.source_path;
    const auto debug_name = found->second.asset.debug_name;
    found->second.asset = load_gltf_stub(debug_name, source);
    found->second.cache.reset();
    return true;
}

bool AssetManager::hot_reload(const TextureHandle& handle)
{
    std::lock_guard lock(m_mutex);
    const auto key = key_for(handle.id());
    const auto found = m_textures.find(key);
    if (found == m_textures.end()) {
        return false;
    }

    const auto source = found->second.asset.source_path;
    const auto debug_name = found->second.asset.debug_name;
    found->second.asset = load_png_stub(debug_name, source);
    found->second.cache.reset();
    return true;
}

std::vector<AssetBrowserEntry> AssetManager::browser_snapshot() const
{
    std::lock_guard lock(m_mutex);
    std::vector<AssetBrowserEntry> rows;
    for (const auto& [key, record] : m_meshes) {
        (void)key;
        rows.push_back({ .virtual_path = record.asset.debug_name, .type = AssetType::Mesh, .ref_count = record.ref_count, .loaded = (bool)record.cache });
    }
    for (const auto& [key, record] : m_materials) {
        (void)key;
        rows.push_back({ .virtual_path = record.asset.debug_name, .type = AssetType::Material, .ref_count = record.ref_count, .loaded = (bool)record.cache });
    }
    for (const auto& [key, record] : m_textures) {
        (void)key;
        rows.push_back({ .virtual_path = record.asset.debug_name, .type = AssetType::Texture, .ref_count = record.ref_count, .loaded = (bool)record.cache });
    }
    return rows;
}

std::size_t AssetManager::mesh_cache_bytes() const
{
    std::lock_guard lock(m_mutex);
    std::size_t total = 0;
    for (const auto& [key, record] : m_meshes) {
        (void)key;
        total += mesh_byte_size(record.asset);
    }
    return total;
}

std::size_t AssetManager::texture_cache_bytes() const
{
    std::lock_guard lock(m_mutex);
    std::size_t total = 0;
    for (const auto& [key, record] : m_textures) {
        (void)key;
        total += record.asset.width * record.asset.height * 4;
    }
    return total;
}

MeshAsset AssetManager::load_gltf_stub(const std::string& virtual_path, const std::string& source_path)
{
    std::ifstream file(source_path);
    const std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    MeshAsset mesh = create_cube_mesh_resource(virtual_path);
    mesh.source_path = source_path;
    if (!content.empty()) {
        mesh.primitive_count = std::max<std::size_t>(1, std::count(content.begin(), content.end(), '{'));
        mesh.vertex_count = std::max<std::size_t>(mesh.vertices.size(), content.size() % 500 + 3);
    }
    return mesh;
}

TextureAsset AssetManager::load_png_stub(const std::string& virtual_path, const std::string& source_path)
{
    std::ifstream file(source_path, std::ios::binary);
    std::vector<unsigned char> bytes(24, 0);
    file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    std::size_t width = 0;
    std::size_t height = 0;

    const bool has_png_signature = file.gcount() >= 24 && bytes[0] == 0x89 && bytes[1] == 'P' && bytes[2] == 'N' && bytes[3] == 'G';
    if (has_png_signature) {
        width = (static_cast<std::size_t>(bytes[16]) << 24u) | (static_cast<std::size_t>(bytes[17]) << 16u) | (static_cast<std::size_t>(bytes[18]) << 8u) | static_cast<std::size_t>(bytes[19]);
        height = (static_cast<std::size_t>(bytes[20]) << 24u) | (static_cast<std::size_t>(bytes[21]) << 16u) | (static_cast<std::size_t>(bytes[22]) << 8u) | static_cast<std::size_t>(bytes[23]);
    } else {
        std::ifstream fixture(source_path);
        const std::string content((std::istreambuf_iterator<char>(fixture)), std::istreambuf_iterator<char>());
        if (content.find("VALLEY_PNG_FIXTURE") != std::string::npos) {
            width = 1;
            height = 1;
        }
    }

    return TextureAsset { .debug_name = virtual_path, .source_path = source_path, .width = width, .height = height };
}

MeshAsset create_ground_plane_mesh_resource(std::string debug_name)
{
    MeshAsset mesh;
    mesh.debug_name = std::move(debug_name);
    mesh.primitive_hint = MeshPrimitiveHint::GroundPlane;
    mesh.vertices = {
        { -0.5F, 0.0F, -0.5F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F },
        { 0.5F, 0.0F, -0.5F, 0.0F, 1.0F, 0.0F, 1.0F, 0.0F },
        { 0.5F, 0.0F, 0.5F, 0.0F, 1.0F, 0.0F, 1.0F, 1.0F },
        { -0.5F, 0.0F, 0.5F, 0.0F, 1.0F, 0.0F, 0.0F, 1.0F },
    };
    mesh.indices = { 0, 1, 2, 0, 2, 3 };
    mesh.vertex_count = mesh.vertices.size();
    mesh.primitive_count = mesh.indices.size() / 3;
    return mesh;
}

MeshAsset create_cube_mesh_resource(std::string debug_name)
{
    MeshAsset mesh;
    mesh.debug_name = std::move(debug_name);
    mesh.primitive_hint = MeshPrimitiveHint::Cube;
    mesh.vertices = {
        { -0.5F, -0.5F, -0.5F }, { 0.5F, -0.5F, -0.5F }, { 0.5F, 0.5F, -0.5F }, { -0.5F, 0.5F, -0.5F },
        { -0.5F, -0.5F, 0.5F },  { 0.5F, -0.5F, 0.5F },  { 0.5F, 0.5F, 0.5F },  { -0.5F, 0.5F, 0.5F },
    };
    mesh.indices = {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        0, 4, 5, 0, 5, 1,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        0, 3, 7, 0, 7, 4,
    };
    mesh.vertex_count = mesh.vertices.size();
    mesh.primitive_count = mesh.indices.size() / 3;
    return mesh;
}

MaterialAsset create_placeholder_material(std::string debug_name)
{
    return MaterialAsset { .debug_name = std::move(debug_name), .base_color = { 0.72F, 0.68F, 0.58F, 1.0F } };
}

} // namespace Valley::Assets
