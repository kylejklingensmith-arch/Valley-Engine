#include "Valley/Renderer/RendererModule.h"

#include "Valley/Core/Logger.h"
#include "Valley/Renderer/SoftwareRasterBackend.h"

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

std::string_view RendererModule::name() const
{
    return "Renderer";
}

void RendererModule::on_attach()
{
    m_backend = create_backend(m_desc);

    std::ostringstream message;
    message << "Renderer module attached with " << m_backend->name() << " backend, "
            << m_graph.passes().size() << " render passes, and " << m_scene.entity_count()
            << " test-scene entities.";
    Core::Log::info("Renderer", message.str());
}

void RendererModule::on_update(const Core::TimeStep& step)
{
    if (!m_backend) {
        return;
    }

    m_camera.orbit_origin(step.elapsed_seconds, 7.0, 4.0);

    const double fps = step.real_delta_seconds > 0.0 ? 1.0 / step.real_delta_seconds : 0.0;
    const RenderFrame frame {
        .scene = &m_scene,
        .camera = &m_camera,
        .graph = &m_graph,
        .debug_overlay = {
            .fps = fps,
            .frame_time_ms = step.real_delta_seconds * 1000.0,
            .frame_index = step.frame_index,
            .entity_count = m_scene.entity_count(),
        },
    };

    m_backend->begin_frame(frame);
    for (const auto& pass : m_graph.passes()) {
        m_backend->execute_pass(pass, frame);
    }
    m_backend->end_frame(frame);

    if (!m_desc.debug_frame_path.empty() && !m_wrote_debug_frame) {
        if (m_backend->write_debug_frame(m_desc.debug_frame_path)) {
            Core::Log::info("Renderer", std::string("Wrote debug render frame: ").append(m_desc.debug_frame_path));
        } else {
            Core::Log::warn("Renderer", std::string("Failed to write debug render frame: ").append(m_desc.debug_frame_path));
        }
        m_wrote_debug_frame = true;
    }
}

void RendererModule::on_detach()
{
    Core::Log::info("Renderer", "Renderer module detached.");
}

const RenderScene& RendererModule::scene() const
{
    return m_scene;
}

const RenderGraph& RendererModule::graph() const
{
    return m_graph;
}

const Camera& RendererModule::camera() const
{
    return m_camera;
}

const IRendererBackend& RendererModule::backend() const
{
    if (!m_backend) {
        throw std::logic_error("Renderer backend is not attached.");
    }

    return *m_backend;
}

} // namespace Valley::Renderer
