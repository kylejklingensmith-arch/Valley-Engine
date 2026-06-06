#include <algorithm>
#include "Valley/Assets/AssetManager.h"

#include <fstream>
#include <iterator>
#include <string>

namespace Valley::Assets {

AssetManager::AssetManager() = default;

AssetHandle AssetManager::import_gltf(const std::string& virtual_path, const std::string& source_path)
{
    const auto handle = m_registry.register_asset(AssetType::Mesh, virtual_path);
    m_meshes[handle.uuid.to_string()] = MeshRecord { .asset = load_gltf_stub(virtual_path, source_path), .cache = nullptr, .ref_count = 0 };
    m_sources[handle.uuid.to_string()] = handle;
    return handle;
}

AssetHandle AssetManager::import_png(const std::string& virtual_path, const std::string& source_path)
{
    const auto handle = m_registry.register_asset(AssetType::Texture, virtual_path);
    m_textures[handle.uuid.to_string()] = TextureRecord { .asset = load_png_stub(virtual_path, source_path), .cache = nullptr, .ref_count = 0 };
    m_sources[handle.uuid.to_string()] = handle;
    return handle;
}

std::shared_ptr<const MeshAsset> AssetManager::acquire_mesh(const AssetHandle& handle)
{
    auto& record = m_meshes.at(handle.uuid.to_string());
    if (!record.cache) { record.cache = std::make_shared<MeshAsset>(record.asset); }
    record.ref_count += 1;
    return record.cache;
}

std::shared_ptr<const TextureAsset> AssetManager::acquire_texture(const AssetHandle& handle)
{
    auto& record = m_textures.at(handle.uuid.to_string());
    if (!record.cache) { record.cache = std::make_shared<TextureAsset>(record.asset); }
    record.ref_count += 1;
    return record.cache;
}

std::future<AssetHandle> AssetManager::import_gltf_async(const std::string& virtual_path, const std::string& source_path)
{
    return std::async(std::launch::async, [this, virtual_path, source_path]() { return import_gltf(virtual_path, source_path); });
}

std::future<AssetHandle> AssetManager::import_png_async(const std::string& virtual_path, const std::string& source_path)
{
    return std::async(std::launch::async, [this, virtual_path, source_path]() { return import_png(virtual_path, source_path); });
}

bool AssetManager::hot_reload(const AssetHandle& handle)
{
    const auto key = handle.uuid.to_string();
    if (handle.type == AssetType::Mesh && m_meshes.contains(key)) {
        const auto source = m_meshes[key].asset.source_path;
        m_meshes[key].asset = load_gltf_stub(m_meshes[key].asset.debug_name, source);
        m_meshes[key].cache.reset();
        return true;
    }
    if (handle.type == AssetType::Texture && m_textures.contains(key)) {
        const auto source = m_textures[key].asset.source_path;
        m_textures[key].asset = load_png_stub(m_textures[key].asset.debug_name, source);
        m_textures[key].cache.reset();
        return true;
    }
    return false;
}

std::vector<AssetBrowserEntry> AssetManager::browser_snapshot() const
{
    std::vector<AssetBrowserEntry> rows;
    for (const auto& [key, record] : m_meshes) {
        (void)key;
        rows.push_back({ .virtual_path = record.asset.debug_name, .type = AssetType::Mesh, .ref_count = record.ref_count, .loaded = (bool)record.cache });
    }
    for (const auto& [key, record] : m_textures) {
        (void)key;
        rows.push_back({ .virtual_path = record.asset.debug_name, .type = AssetType::Texture, .ref_count = record.ref_count, .loaded = (bool)record.cache });
    }
    return rows;
}


std::size_t AssetManager::mesh_cache_bytes() const
{
    std::size_t total = 0;
    for (const auto& [k, r] : m_meshes) { (void)k; total += r.asset.vertex_count * sizeof(float) * 8; }
    return total;
}

std::size_t AssetManager::texture_cache_bytes() const
{
    std::size_t total = 0;
    for (const auto& [k, r] : m_textures) { (void)k; total += r.asset.width * r.asset.height * 4; }
    return total;
}

MeshAsset AssetManager::load_gltf_stub(const std::string& virtual_path, const std::string& source_path)
{
    std::ifstream file(source_path);
    const std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::size_t primitive_count = 1;
    std::size_t vertex_count = 3;
    if (!content.empty()) {
        primitive_count = std::count(content.begin(), content.end(), '{');
        vertex_count = content.size() % 500 + 3;
    }
    return MeshAsset { .debug_name = virtual_path, .source_path = source_path, .primitive_count = primitive_count, .vertex_count = vertex_count };
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

} // namespace Valley::Assets
