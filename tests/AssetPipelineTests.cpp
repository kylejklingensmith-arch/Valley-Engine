#include "Valley/Assets/AssetManager.h"

#include <iostream>
#include <stdexcept>

int main()
{
    try {
        Valley::Assets::AssetManager assets;
        const auto mesh = assets.import_gltf("engine/terrain_chunk", std::string(VALLEY_SOURCE_ROOT) + "/assets/test_scene.gltf");
        const auto tex = assets.import_png("engine/materials/albedo", std::string(VALLEY_SOURCE_ROOT) + "/assets/test_albedo.png.fixture");
        const auto loaded_mesh = assets.acquire_mesh(mesh);
        const auto loaded_tex = assets.acquire_texture(tex);
        if (!loaded_mesh || loaded_mesh->vertex_count == 0) {
            throw std::runtime_error("mesh import failed");
        }
        if (!loaded_tex || loaded_tex->width == 0 || loaded_tex->height == 0) {
            throw std::runtime_error("texture import failed");
        }
        if (!assets.hot_reload(tex)) {
            throw std::runtime_error("hot reload failed");
        }
    } catch (const std::exception& e) {
        std::cerr << "AssetPipelineTests failed: " << e.what() << '\n';
        return 1;
    }
    std::cout << "AssetPipelineTests passed.\n";
    return 0;
}
