#pragma once

#include "Valley/Assets/AssetManager.h"
#include "Valley/Renderer/Math.h"

#include <cstddef>
#include <string>
#include <vector>

namespace Valley::Renderer {

struct Transform {
    Vec3 position;
    Vec3 rotation;
    Vec3 scale { 1.0, 1.0, 1.0 };
};

struct RenderEntity {
    std::string name;
    Transform transform;
    Assets::MeshHandle mesh;
    Assets::MaterialHandle material;
};

struct DirectionalLight {
    Vec3 direction { -0.35, -1.0, -0.2 };
    Vec3 color { 1.0, 0.95, 0.82 };
    double intensity = 1.0;
};

class RenderScene {
public:
    explicit RenderScene(const Assets::AssetManager* assets = nullptr);

    void set_asset_manager(const Assets::AssetManager* assets);
    RenderEntity& add_entity(RenderEntity entity);
    void set_directional_light(const DirectionalLight& light);

    [[nodiscard]] const Assets::AssetManager* asset_manager() const;
    [[nodiscard]] const Assets::MeshAsset* mesh_resource(const Assets::MeshHandle& handle) const;
    [[nodiscard]] const Assets::MaterialAsset* material_resource(const Assets::MaterialHandle& handle) const;
    [[nodiscard]] const std::vector<RenderEntity>& entities() const;
    [[nodiscard]] const DirectionalLight& directional_light() const;
    [[nodiscard]] std::size_t entity_count() const;

private:
    const Assets::AssetManager* m_assets = nullptr;
    std::vector<RenderEntity> m_entities;
    DirectionalLight m_directional_light;
};

RenderScene create_basic_renderer_test_scene(Assets::AssetManager& assets);

} // namespace Valley::Renderer
