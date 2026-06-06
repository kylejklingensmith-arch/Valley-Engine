#include "Valley/Renderer/Scene.h"

#include <utility>

namespace Valley::Renderer {

RenderEntity& RenderScene::add_entity(RenderEntity entity)
{
    m_entities.push_back(std::move(entity));
    return m_entities.back();
}

void RenderScene::set_directional_light(const DirectionalLight& light)
{
    m_directional_light = light;
}

const std::vector<RenderEntity>& RenderScene::entities() const
{
    return m_entities;
}

const DirectionalLight& RenderScene::directional_light() const
{
    return m_directional_light;
}

std::size_t RenderScene::entity_count() const
{
    return m_entities.size();
}

RenderScene create_basic_renderer_test_scene()
{
    RenderScene scene;

    scene.set_directional_light({
        .direction = normalized({ -0.4, -1.0, -0.25 }),
        .color = { 1.0, 0.94, 0.78 },
        .intensity = 1.0,
    });

    scene.add_entity({
        .name = "RendererTest_GroundPlane",
        .transform = {
            .position = { 0.0, 0.0, 0.0 },
            .rotation = { 0.0, 0.0, 0.0 },
            .scale = { 12.0, 1.0, 12.0 },
        },
        .mesh = {
            .primitive = MeshPrimitive::GroundPlane,
            .debug_name = "Engine Ground Plane Placeholder",
        },
        .material = {},
    });

    scene.add_entity({
        .name = "RendererTest_Cube",
        .transform = {
            .position = { 0.0, 1.0, 0.0 },
            .rotation = { 0.0, 0.0, 0.0 },
            .scale = { 1.5, 1.5, 1.5 },
        },
        .mesh = {
            .primitive = MeshPrimitive::Cube,
            .debug_name = "Engine Cube Placeholder",
        },
        .material = {},
    });

    return scene;
}

} // namespace Valley::Renderer
