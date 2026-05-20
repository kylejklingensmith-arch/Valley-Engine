#pragma once

#include "Valley/Core/Module.h"
#include "Valley/Renderer/Camera.h"
#include "Valley/Renderer/IRendererBackend.h"
#include "Valley/Renderer/RenderGraph.h"
#include "Valley/Renderer/Scene.h"

#include <memory>
#include <string>

namespace Valley::Renderer {

struct RendererModuleDesc {
    int framebuffer_width = 320;
    int framebuffer_height = 180;
    RendererPath renderer_path = RendererPath::TraditionalRaster;
    std::string debug_frame_path;
};

class RendererModule final : public Core::Module {
public:
    explicit RendererModule(RendererModuleDesc desc = {});

    [[nodiscard]] std::string_view name() const override;
    void on_attach() override;
    void on_update(const Core::TimeStep& step) override;
    void on_detach() override;

    [[nodiscard]] const RenderScene& scene() const;
    [[nodiscard]] const RenderGraph& graph() const;
    [[nodiscard]] const Camera& camera() const;
    [[nodiscard]] const IRendererBackend& backend() const;

private:
    RendererModuleDesc m_desc;
    RenderScene m_scene;
    RenderGraph m_graph;
    Camera m_camera;
    std::unique_ptr<IRendererBackend> m_backend;
    bool m_wrote_debug_frame = false;
};

} // namespace Valley::Renderer
