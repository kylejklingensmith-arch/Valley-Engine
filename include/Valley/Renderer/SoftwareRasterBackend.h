#pragma once

#include "Valley/Renderer/IRendererBackend.h"

#include <cstdint>
#include <vector>

namespace Valley::Renderer {

struct ColorRgb {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

class SoftwareRasterBackend final : public IRendererBackend {
public:
    SoftwareRasterBackend(int width, int height);

    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] RendererPath path() const override;

    void begin_frame(const RenderFrame& frame) override;
    void execute_pass(const RenderPass& pass, const RenderFrame& frame) override;
    void end_frame(const RenderFrame& frame) override;

    [[nodiscard]] const DebugOverlayData& last_overlay() const override;
    bool write_debug_frame(const std::string& path) const override;

private:
    void clear(const ColorRgb& color);
    void render_scene(const RenderFrame& frame);
    void render_overlay(const RenderFrame& frame);
    void draw_entity(const RenderEntity& entity, const Camera& camera);
    void draw_rect(int min_x, int min_y, int max_x, int max_y, const ColorRgb& color);
    void draw_line(int x0, int y0, int x1, int y1, const ColorRgb& color);
    void draw_text_blocks(int x, int y, std::string_view text, const ColorRgb& color);
    void set_pixel(int x, int y, const ColorRgb& color);

    int m_width = 0;
    int m_height = 0;
    std::vector<ColorRgb> m_pixels;
    DebugOverlayData m_last_overlay;
};

} // namespace Valley::Renderer
