#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Valley::Assets {

enum class AssetType { Mesh, Material, Texture };

struct AssetUuid {
    std::array<std::uint8_t, 16> bytes {};
    [[nodiscard]] static AssetUuid from_stable_string(std::string_view source);
    [[nodiscard]] std::string to_string() const;
    friend bool operator==(const AssetUuid&, const AssetUuid&) = default;
};

using AssetId = AssetUuid;

struct AssetHandle {
    AssetId id {};
    AssetType type = AssetType::Mesh;
    friend bool operator==(const AssetHandle&, const AssetHandle&) = default;
};

struct MeshHandle {
    AssetHandle asset {};
    [[nodiscard]] const AssetId& id() const { return asset.id; }
    [[nodiscard]] bool valid() const { return asset.id != AssetId {}; }
    friend bool operator==(const MeshHandle&, const MeshHandle&) = default;
};

struct MaterialHandle {
    AssetHandle asset { .type = AssetType::Material };
    [[nodiscard]] const AssetId& id() const { return asset.id; }
    [[nodiscard]] bool valid() const { return asset.id != AssetId {}; }
    friend bool operator==(const MaterialHandle&, const MaterialHandle&) = default;
};

struct TextureHandle {
    AssetHandle asset { .type = AssetType::Texture };
    [[nodiscard]] const AssetId& id() const { return asset.id; }
    [[nodiscard]] bool valid() const { return asset.id != AssetId {}; }
    friend bool operator==(const TextureHandle&, const TextureHandle&) = default;
};

enum class MeshPrimitiveHint {
    Unknown,
    GroundPlane,
    Cube,
};

struct MeshVertex {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
    float nx = 0.0F;
    float ny = 1.0F;
    float nz = 0.0F;
    float u = 0.0F;
    float v = 0.0F;
};

struct MeshAsset {
    std::string debug_name;
    std::string source_path;
    MeshPrimitiveHint primitive_hint = MeshPrimitiveHint::Unknown;
    std::vector<MeshVertex> vertices;
    std::vector<std::uint32_t> indices;
    std::size_t primitive_count = 0;
    std::size_t vertex_count = 0;
};

struct MaterialAsset {
    std::string debug_name;
    std::array<float, 4> base_color { 1.0F, 1.0F, 1.0F, 1.0F };
    TextureHandle albedo_texture {};
    bool uses_texture = false;
};

struct TextureAsset {
    std::string debug_name;
    std::string source_path;
    std::size_t width = 0;
    std::size_t height = 0;
};

} // namespace Valley::Assets
