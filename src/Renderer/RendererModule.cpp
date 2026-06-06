#include "Valley/Renderer/RendererModule.h"

#include "Valley/Core/Logger.h"
#include "Valley/Renderer/SoftwareRasterBackend.h"

#include <chrono>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace Valley::Renderer {
namespace {
std::unique_ptr<IRendererBackend> create_backend(const RendererModuleDesc& desc)
{
    switch (desc.renderer_path) {
    case RendererPath::TraditionalRaster:
        return std::make_unique<SoftwareRasterBackend>(desc.framebuffer_width, desc.framebuffer_height);
    case RendererPath::ExperimentalEquationSplat:
        Core::Log::warn("Renderer", "Experimental equation/splat path is not implemented; using traditional raster backend.");
        return std::make_unique<SoftwareRasterBackend>(desc.framebuffer_width, desc.framebuffer_height);
    }

    throw std::runtime_error("Unknown renderer path.");
}
} // namespace

RendererModule::RendererModule(RendererModuleDesc desc)
    : m_desc(std::move(desc)), m_scene(create_basic_renderer_test_scene()), m_graph(create_default_render_graph())
{
    m_camera.set_perspective(60.0, 0.1, 2000.0);
    m_camera.look_at({ 0.0, 4.0, 7.0 }, { 0.0, 0.6, 0.0 });
}

std::string_view RendererModule::name() const { return "Renderer"; }

void RendererModule::build_imported_test_scene()
{
    auto terrain_future = m_asset_manager.import_gltf_async("engine/terrain_chunk", std::string(VALLEY_SOURCE_ROOT) + "/assets/test_scene.gltf");
    auto prop_future = m_asset_manager.import_gltf_async("engine/static_prop", std::string(VALLEY_SOURCE_ROOT) + "/assets/test_scene.gltf");
    auto texture_future = m_asset_manager.import_png_async("engine/materials/albedo", std::string(VALLEY_SOURCE_ROOT) + "/assets/test_albedo.png.fixture");

    const auto terrain_handle = terrain_future.get();
    const auto prop_handle = prop_future.get();
    const auto texture_handle = texture_future.get();

    (void)m_asset_manager.acquire_mesh(terrain_handle);
    (void)m_asset_manager.acquire_mesh(prop_handle);
    (void)m_asset_manager.acquire_texture(texture_handle);

    m_scene.add_entity({ .name = "Imported_TerrainChunk", .transform = { .position = { 0.0, -0.2, 0.0 }, .rotation = { 0.0, 0.0, 0.0 }, .scale = { 8.0, 1.0, 8.0 } }, .mesh = { .primitive = MeshPrimitive::GroundPlane, .debug_name = "ImportedTerrainChunk" }, .mesh_asset = terrain_handle, .material = { .albedo_texture = texture_handle, .uses_texture = true } });

    m_scene.add_entity({ .name = "Imported_StaticProp", .transform = { .position = { 1.2, 0.8, 0.4 }, .rotation = { 0.0, 0.4, 0.0 }, .scale = { 1.2, 1.2, 1.2 } }, .mesh = { .primitive = MeshPrimitive::Cube, .debug_name = "ImportedStaticProp" }, .mesh_asset = prop_handle, .material = { .albedo_texture = texture_handle, .uses_texture = true } });

    (void)m_asset_manager.hot_reload(texture_handle);

    for (int i = 0; i < 48; ++i) {
        const double x = static_cast<double>((i % 12) - 6) * 1.5;
        const double z = static_cast<double>(i / 12) * 1.5 - 2.0;
        m_scene.add_entity({ .name = "Stress_Prop_" + std::to_string(i), .transform = { .position = { x, 0.6, z }, .rotation = { 0.0, 0.0, 0.0 }, .scale = { 0.7, 0.7, 0.7 } }, .mesh = { .primitive = MeshPrimitive::Cube, .debug_name = "StressProp" }, .mesh_asset = prop_handle, .material = { .albedo_texture = texture_handle, .uses_texture = true } });
    }
}

void RendererModule::on_attach()
{
    build_imported_test_scene();
    m_backend = create_backend(m_desc);

    std::ostringstream message;
    message << "Renderer module attached with " << m_backend->name() << " backend, " << m_graph.passes().size()
            << " render passes, and " << m_scene.entity_count() << " test-scene entities.";
    Core::Log::info("Renderer", message.str());
}

void RendererModule::on_update(const Core::TimeStep& step)
{
    if (!m_backend) {
        return;
    }

    const auto frame_start = std::chrono::steady_clock::now();
    m_camera.orbit_origin(step.elapsed_seconds, 7.0, 4.0);
    const double fps = step.real_delta_seconds > 0.0 ? 1.0 / step.real_delta_seconds : 0.0;
    RenderFrame frame { .scene = &m_scene, .camera = &m_camera, .graph = &m_graph, .debug_overlay = { .fps = fps, .frame_time_ms = step.real_delta_seconds * 1000.0, .frame_index = step.frame_index, .entity_count = m_scene.entity_count(), .draw_calls = m_scene.entity_count(), .bbox_count = m_scene.entity_count(), .chunk_border_count = 9, .lod_transition_count = 4 } };
    const auto cpu_begin = std::chrono::steady_clock::now();
    m_backend->begin_frame(frame);
    for (const auto& pass : m_graph.passes()) {
        const auto pass_start = std::chrono::steady_clock::now();
        m_backend->execute_pass(pass, frame);
        const auto pass_end = std::chrono::steady_clock::now();
        m_diagnostics.pass.record(pass.name, std::chrono::duration<double, std::milli>(pass_end - pass_start).count());
    }
    m_backend->end_frame(frame);
    const auto cpu_end = std::chrono::steady_clock::now();
    const double cpu_ms = std::chrono::duration<double, std::milli>(cpu_end - cpu_begin).count();
    const double gpu_ms = cpu_ms * 0.85;
    m_diagnostics.cpu.record("frame", cpu_ms);
    m_diagnostics.gpu.record("frame", gpu_ms);
    m_diagnostics.memory.mesh_cache_bytes = m_asset_manager.mesh_cache_bytes();
    m_diagnostics.memory.texture_cache_bytes = m_asset_manager.texture_cache_bytes();
    m_diagnostics.memory.total_bytes = m_diagnostics.memory.mesh_cache_bytes + m_diagnostics.memory.texture_cache_bytes;
    m_diagnostics.draw_calls = m_scene.entity_count();
    frame.debug_overlay.cpu_frame_ms = cpu_ms;
    frame.debug_overlay.gpu_frame_ms = gpu_ms;
    const auto frame_end = std::chrono::steady_clock::now();
    m_diagnostics.cpu.record("frame_total", std::chrono::duration<double, std::milli>(frame_end - frame_start).count());

    if (!m_desc.debug_frame_path.empty() && !m_wrote_debug_frame) {
        if (m_backend->write_debug_frame(m_desc.debug_frame_path)) {
            Core::Log::info("Renderer", std::string("Wrote debug render frame: ").append(m_desc.debug_frame_path));
        } else {
            Core::Log::warn("Renderer", std::string("Failed to write debug render frame: ").append(m_desc.debug_frame_path));
        }
        m_wrote_debug_frame = true;
    }
}

void RendererModule::on_detach() { Core::Log::info("Renderer", "Renderer module detached."); }

const RenderScene& RendererModule::scene() const { return m_scene; }
const RenderGraph& RendererModule::graph() const { return m_graph; }
const Camera& RendererModule::camera() const { return m_camera; }
const Assets::AssetManager& RendererModule::asset_manager() const { return m_asset_manager; }
const FrameDiagnostics& RendererModule::diagnostics() const { return m_diagnostics; }

const IRendererBackend& RendererModule::backend() const
{
    if (!m_backend) { throw std::logic_error("Renderer backend is not attached."); }
    return *m_backend;
}

} // namespace Valley::Renderer
