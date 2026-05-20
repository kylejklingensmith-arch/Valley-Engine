#pragma once

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

enum class MeshPrimitive {
    GroundPlane,
    Cube,
};

struct MeshPlaceholder {
    MeshPrimitive primitive = MeshPrimitive::Cube;
    std::string debug_name;
};

struct RenderEntity {
    std::string name;
    Transform transform;
    MeshPlaceholder mesh;
};

struct DirectionalLight {
    Vec3 direction { -0.35, -1.0, -0.2 };
    Vec3 color { 1.0, 0.95, 0.82 };
    double intensity = 1.0;
};

class RenderScene {
public:
    RenderEntity& add_entity(RenderEntity entity);
    void set_directional_light(const DirectionalLight& light);

    [[nodiscard]] const std::vector<RenderEntity>& entities() const;
    [[nodiscard]] const DirectionalLight& directional_light() const;
    [[nodiscard]] std::size_t entity_count() const;

private:
    std::vector<RenderEntity> m_entities;
    DirectionalLight m_directional_light;
};

RenderScene create_basic_renderer_test_scene();

} // namespace Valley::Renderer
