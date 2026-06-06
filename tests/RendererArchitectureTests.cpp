#include "Valley/Renderer/RenderGraph.h"
#include "Valley/Renderer/RendererModule.h"
#include "Valley/Renderer/Scene.h"
#include "Valley/Renderer/SoftwareRasterBackend.h"

#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error(std::string(message));
    }
}

void test_scene_submits_mesh_and_material_handles()
{
    Valley::Assets::AssetManager assets;
    const auto scene = Valley::Renderer::create_basic_renderer_test_scene(assets);

    require(scene.entity_count() == 2, "test scene has ground and cube entities");
    require(scene.entities()[0].mesh.valid(), "first entity submits a mesh handle");
    require(scene.entities()[1].mesh.valid(), "second entity submits a mesh handle");
    require(scene.entities()[0].material.valid(), "first entity submits a material handle");
    require(scene.entities()[1].material.valid(), "second entity submits a material handle");
    require(scene.mesh_resource(scene.entities()[0].mesh)->primitive_hint == Valley::Assets::MeshPrimitiveHint::GroundPlane, "first entity resolves to ground mesh resource");
    require(scene.mesh_resource(scene.entities()[1].mesh)->primitive_hint == Valley::Assets::MeshPrimitiveHint::Cube, "second entity resolves to cube mesh resource");
    require(scene.material_resource(scene.entities()[0].material) != nullptr, "material handle resolves through scene asset manager");
    require(scene.directional_light().intensity > 0.0, "directional light is configured");
}

void default_render_graph_has_expected_pass_order()
{
    const auto graph = Valley::Renderer::create_default_render_graph();

    require(graph.passes().size() == 3, "default graph has three passes");
    require(graph.passes()[0].kind == Valley::Renderer::RenderPassKind::Clear, "clear pass runs first");
    require(graph.passes()[1].kind == Valley::Renderer::RenderPassKind::SceneGeometry, "scene geometry pass runs second");
    require(graph.passes()[2].kind == Valley::Renderer::RenderPassKind::DebugOverlay, "debug overlay pass runs last");
    require(graph.contains("SceneGeometry"), "graph can look up a named pass");
}

void software_backend_renders_scene_submitted_through_handles()
{
    Valley::Assets::AssetManager assets;
    const auto scene = Valley::Renderer::create_basic_renderer_test_scene(assets);
    const auto graph = Valley::Renderer::create_default_render_graph();
    Valley::Renderer::Camera camera;
    camera.look_at({ 0.0, 4.0, 7.0 }, { 0.0, 0.5, 0.0 });

    Valley::Renderer::SoftwareRasterBackend backend(160, 90);
    const Valley::Renderer::RenderFrame frame {
        .scene = &scene,
        .camera = &camera,
        .graph = &graph,
        .debug_overlay = {
            .fps = 60.0,
            .frame_time_ms = 16.666,
            .frame_index = 0,
            .entity_count = scene.entity_count(),
        },
    };

    backend.begin_frame(frame);
    for (const auto& pass : graph.passes()) {
        backend.execute_pass(pass, frame);
    }
    backend.end_frame(frame);

    require(backend.path() == Valley::Renderer::RendererPath::TraditionalRaster, "software backend is the traditional raster path");
    require(backend.last_overlay().entity_count == scene.entity_count(), "overlay tracks entity count");
    require(backend.last_overlay().fps == 60.0, "overlay tracks fps");
}

} // namespace

int main()
{
    try {
        test_scene_submits_mesh_and_material_handles();
        default_render_graph_has_expected_pass_order();
        software_backend_renders_scene_submitted_through_handles();
    } catch (const std::exception& exception) {
        std::cerr << "RendererArchitectureTests failed: " << exception.what() << '\n';
        return 1;
    }

    std::cout << "RendererArchitectureTests passed.\n";
    return 0;
}
