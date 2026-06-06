#include "Valley/Renderer/Scene.h"

#include <utility>

namespace Valley::Renderer {

RenderScene::RenderScene(const Assets::AssetManager* assets)
    : m_assets(assets)
{
}

void RenderScene::set_asset_manager(const Assets::AssetManager* assets)
{
    m_assets = assets;
}

RenderEntity& RenderScene::add_entity(RenderEntity entity)
{
    m_entities.push_back(std::move(entity));
    return m_entities.back();
}

void RenderScene::set_directional_light(const DirectionalLight& light)
{
    m_directional_light = light;
}

const Assets::AssetManager* RenderScene::asset_manager() const
{
    return m_assets;
}

const Assets::MeshAsset* RenderScene::mesh_resource(const Assets::MeshHandle& handle) const
{
    return m_assets ? m_assets->find_mesh(handle) : nullptr;
}

const Assets::MaterialAsset* RenderScene::material_resource(const Assets::MaterialHandle& handle) const
{
    return m_assets ? m_assets->find_material(handle) : nullptr;
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

RenderScene create_basic_renderer_test_scene(Assets::AssetManager& assets)
{
    RenderScene scene(&assets);

    const auto ground_mesh = assets.register_mesh("engine/renderer/ground_plane.mesh", Assets::create_ground_plane_mesh_resource("Engine Ground Plane Mesh"));
    const auto cube_mesh = assets.register_mesh("engine/renderer/cube.mesh", Assets::create_cube_mesh_resource("Engine Cube Mesh"));
    const auto default_material = assets.register_material("engine/renderer/default_debug.material", Assets::create_placeholder_material("Engine Debug Material"));

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
        .mesh = ground_mesh,
        .material = default_material,
    });

    scene.add_entity({
        .name = "RendererTest_Cube",
        .transform = {
            .position = { 0.0, 1.0, 0.0 },
            .rotation = { 0.0, 0.0, 0.0 },
            .scale = { 1.5, 1.5, 1.5 },
        },
        .mesh = cube_mesh,
        .material = default_material,
    });

    return scene;
}

} // namespace Valley::Renderer
