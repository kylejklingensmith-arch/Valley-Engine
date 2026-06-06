#pragma once

#include "Valley/Renderer/Camera.h"
#include "Valley/Renderer/RenderGraph.h"
#include "Valley/Renderer/Scene.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace Valley::Renderer {

enum class RendererPath {
    TraditionalRaster,
    ExperimentalEquationSplat,
};

struct DebugOverlayData {
    double fps = 0.0;
    double frame_time_ms = 0.0;
    unsigned long long frame_index = 0;
    std::size_t entity_count = 0;
    std::size_t draw_calls = 0;
    std::size_t bbox_count = 0;
    std::size_t chunk_border_count = 0;
    std::size_t lod_transition_count = 0;
    double cpu_frame_ms = 0.0;
    double gpu_frame_ms = 0.0;
};

struct RenderFrame {
    const RenderScene* scene = nullptr;
    const Camera* camera = nullptr;
    const RenderGraph* graph = nullptr;
    DebugOverlayData debug_overlay;
};

class IRendererBackend {
public:
    virtual ~IRendererBackend() = default;

    [[nodiscard]] virtual std::string_view name() const = 0;
    [[nodiscard]] virtual RendererPath path() const = 0;

    virtual void begin_frame(const RenderFrame& frame) = 0;
    virtual void execute_pass(const RenderPass& pass, const RenderFrame& frame) = 0;
    virtual void end_frame(const RenderFrame& frame) = 0;

    [[nodiscard]] virtual const DebugOverlayData& last_overlay() const = 0;
    virtual bool write_debug_frame(const std::string& path) const = 0;
};

} // namespace Valley::Renderer
