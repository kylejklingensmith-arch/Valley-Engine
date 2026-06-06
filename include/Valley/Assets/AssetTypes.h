#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace Valley::Assets {

enum class AssetType { Mesh, Texture };

struct AssetUuid {
    std::array<std::uint8_t, 16> bytes {};
    [[nodiscard]] static AssetUuid from_stable_string(std::string_view source);
    [[nodiscard]] std::string to_string() const;
    friend bool operator==(const AssetUuid&, const AssetUuid&) = default;
};

struct AssetHandle {
    AssetUuid uuid {};
    AssetType type = AssetType::Mesh;
    friend bool operator==(const AssetHandle&, const AssetHandle&) = default;
};

struct MeshAsset {
    std::string debug_name;
    std::string source_path;
    std::size_t primitive_count = 0;
    std::size_t vertex_count = 0;
};

struct TextureAsset {
    std::string debug_name;
    std::string source_path;
    std::size_t width = 0;
    std::size_t height = 0;
};

} // namespace Valley::Assets
