#include "Valley/Assets/AssetManager.h"
#include "Valley/Assets/AssetRegistry.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error(std::string(message));
    }
}

void stable_asset_ids_are_deterministic()
{
    Valley::Assets::AssetRegistry first;
    Valley::Assets::AssetRegistry second;

    const auto first_mesh = first.register_asset(Valley::Assets::AssetType::Mesh, "engine/test/cube.mesh");
    const auto duplicate_mesh = first.register_asset(Valley::Assets::AssetType::Mesh, "engine/test/cube.mesh");
    const auto second_mesh = second.register_asset(Valley::Assets::AssetType::Mesh, "engine/test/cube.mesh");
    const auto material = first.register_asset(Valley::Assets::AssetType::Material, "engine/test/cube.material");

    require(first_mesh == duplicate_mesh, "re-registering an asset returns the same stable handle");
    require(first_mesh.id == second_mesh.id, "stable asset IDs are reproducible across registries");
    require(first.find("engine/test/cube.mesh") == first_mesh, "registry can find by virtual path");
    require(first.find_by_id(first_mesh.id)->virtual_path == "engine/test/cube.mesh", "registry can find by stable asset ID");
    require(first_mesh.id != material.id, "different resource virtual paths/types receive distinct IDs");
}

void mesh_material_and_texture_resources_can_be_registered_and_looked_up()
{
    Valley::Assets::AssetManager assets;
    const auto mesh = assets.register_mesh("engine/test/ground.mesh", Valley::Assets::create_ground_plane_mesh_resource("Ground Resource"));
    const auto texture = assets.register_texture("engine/test/albedo.texture", { .debug_name = "Albedo Resource", .width = 2, .height = 2 });
    const auto material = assets.register_material("engine/test/material.material", { .debug_name = "Material Resource", .albedo_texture = texture, .uses_texture = true });

    const auto* mesh_resource = assets.find_mesh(mesh);
    const auto* material_resource = assets.find_material(material);
    const auto* texture_resource = assets.find_texture(texture);

    require(mesh_resource != nullptr, "mesh lookup returns registered CPU resource");
    require(mesh_resource->primitive_hint == Valley::Assets::MeshPrimitiveHint::GroundPlane, "mesh stores primitive hint for renderer submission");
    require(mesh_resource->vertices.size() == 4, "mesh stores CPU-side vertices");
    require(mesh_resource->indices.size() == 6, "mesh stores CPU-side indices");
    require(material_resource != nullptr && material_resource->uses_texture, "material placeholder lookup succeeds");
    require(texture_resource != nullptr && texture_resource->width == 2 && texture_resource->height == 2, "texture lookup succeeds");

    require(assets.acquire_mesh(mesh)->vertex_count == 4, "mesh acquire returns CPU resource cache");
    require(assets.acquire_material(material)->uses_texture, "material acquire returns placeholder cache");
    require(assets.acquire_texture(texture)->width == 2, "texture acquire returns resource cache");
}

void simple_import_stubs_prepare_future_gltf_and_png_paths()
{
    Valley::Assets::AssetManager assets;
    const auto mesh = assets.import_gltf("engine/terrain_chunk", std::string(VALLEY_SOURCE_ROOT) + "/assets/test_scene.gltf");
    const auto texture = assets.import_png("engine/materials/albedo", std::string(VALLEY_SOURCE_ROOT) + "/assets/test_albedo.png.fixture");
    const auto loaded_mesh = assets.acquire_mesh(mesh);
    const auto loaded_texture = assets.acquire_texture(texture);

    require(loaded_mesh && loaded_mesh->vertex_count != 0, "gltf import stub registers a mesh resource");
    require(loaded_texture && loaded_texture->width != 0 && loaded_texture->height != 0, "png import stub registers a texture resource");
    require(assets.hot_reload(texture), "texture hot reload works through typed handles");
}

} // namespace

int main()
{
    try {
        stable_asset_ids_are_deterministic();
        mesh_material_and_texture_resources_can_be_registered_and_looked_up();
        simple_import_stubs_prepare_future_gltf_and_png_paths();
    } catch (const std::exception& exception) {
        std::cerr << "AssetPipelineTests failed: " << exception.what() << '\n';
        return 1;
    }
    std::cout << "AssetPipelineTests passed.\n";
    return 0;
}
