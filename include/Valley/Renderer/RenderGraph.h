#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace Valley::Renderer {

enum class RenderPassKind {
    Clear,
    SceneGeometry,
    DebugOverlay,
};

struct RenderPass {
    std::string name;
    RenderPassKind kind = RenderPassKind::SceneGeometry;
};

class RenderGraph {
public:
    void add_pass(std::string name, RenderPassKind kind);
    void clear();

    [[nodiscard]] const std::vector<RenderPass>& passes() const;
    [[nodiscard]] bool contains(std::string_view name) const;

private:
    std::vector<RenderPass> m_passes;
};

RenderGraph create_default_render_graph();

} // namespace Valley::Renderer
