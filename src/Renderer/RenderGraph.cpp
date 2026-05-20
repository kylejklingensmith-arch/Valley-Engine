#include "Valley/Renderer/RenderGraph.h"

#include <utility>

namespace Valley::Renderer {

void RenderGraph::add_pass(std::string name, RenderPassKind kind)
{
    m_passes.push_back({ .name = std::move(name), .kind = kind });
}

void RenderGraph::clear()
{
    m_passes.clear();
}

const std::vector<RenderPass>& RenderGraph::passes() const
{
    return m_passes;
}

bool RenderGraph::contains(std::string_view name) const
{
    for (const auto& pass : m_passes) {
        if (pass.name == name) {
            return true;
        }
    }

    return false;
}

RenderGraph create_default_render_graph()
{
    RenderGraph graph;
    graph.add_pass("Clear", RenderPassKind::Clear);
    graph.add_pass("SceneGeometry", RenderPassKind::SceneGeometry);
    graph.add_pass("DebugOverlay", RenderPassKind::DebugOverlay);
    return graph;
}

} // namespace Valley::Renderer
