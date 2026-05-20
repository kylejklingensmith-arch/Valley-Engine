#include "Valley/Renderer/SoftwareRasterBackend.h"

#include "Valley/Core/Logger.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace Valley::Renderer {
namespace {
ColorRgb scaled_color(const Vec3& color, double intensity)
{
    const auto scale_channel = [intensity](double value) {
        return static_cast<std::uint8_t>(std::clamp(value * intensity, 0.0, 1.0) * 255.0);
    };

    return { scale_channel(color.x), scale_channel(color.y), scale_channel(color.z) };
}
} // namespace

SoftwareRasterBackend::SoftwareRasterBackend(int width, int height)
    : m_width(width), m_height(height), m_pixels(static_cast<std::size_t>(width * height))
{
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("SoftwareRasterBackend requires a positive framebuffer size.");
    }
}

std::string_view SoftwareRasterBackend::name() const
{
    return "SoftwareRaster";
}

RendererPath SoftwareRasterBackend::path() const
{
    return RendererPath::TraditionalRaster;
}

void SoftwareRasterBackend::begin_frame(const RenderFrame&)
{
    clear({ 9, 12, 18 });
}

void SoftwareRasterBackend::execute_pass(const RenderPass& pass, const RenderFrame& frame)
{
    switch (pass.kind) {
    case RenderPassKind::Clear:
        clear({ 9, 12, 18 });
        break;
    case RenderPassKind::SceneGeometry:
        render_scene(frame);
        break;
    case RenderPassKind::DebugOverlay:
        render_overlay(frame);
        break;
    }
}

void SoftwareRasterBackend::end_frame(const RenderFrame& frame)
{
    m_last_overlay = frame.debug_overlay;

    if (frame.debug_overlay.frame_index == 0) {
        std::ostringstream message;
        message << "Rendered test frame with " << frame.debug_overlay.entity_count
                << " entities at " << frame.debug_overlay.frame_time_ms << " ms.";
        Core::Log::info("Renderer", message.str());
    }
}

const DebugOverlayData& SoftwareRasterBackend::last_overlay() const
{
    return m_last_overlay;
}

bool SoftwareRasterBackend::write_debug_frame(const std::string& path) const
{
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        return false;
    }

    output << "P6\n" << m_width << ' ' << m_height << "\n255\n";
    for (const auto& pixel : m_pixels) {
        output.write(reinterpret_cast<const char*>(&pixel.r), sizeof(pixel.r));
        output.write(reinterpret_cast<const char*>(&pixel.g), sizeof(pixel.g));
        output.write(reinterpret_cast<const char*>(&pixel.b), sizeof(pixel.b));
    }

    return output.good();
}

void SoftwareRasterBackend::clear(const ColorRgb& color)
{
    std::fill(m_pixels.begin(), m_pixels.end(), color);
}

void SoftwareRasterBackend::render_scene(const RenderFrame& frame)
{
    if (frame.scene == nullptr || frame.camera == nullptr) {
        return;
    }

    const DirectionalLight& light = frame.scene->directional_light();
    const ColorRgb horizon = scaled_color(light.color, 0.16 + light.intensity * 0.08);
    draw_rect(0, m_height / 2, m_width - 1, m_height - 1, horizon);

    for (const auto& entity : frame.scene->entities()) {
        draw_entity(entity, *frame.camera);
    }
}

void SoftwareRasterBackend::render_overlay(const RenderFrame& frame)
{
    const auto& overlay = frame.debug_overlay;
    draw_rect(8, 8, 210, 54, { 18, 24, 32 });

    std::ostringstream first_line;
    first_line << "FPS " << static_cast<int>(overlay.fps);
    draw_text_blocks(14, 14, first_line.str(), { 96, 220, 255 });

    std::ostringstream second_line;
    second_line << "MS " << static_cast<int>(overlay.frame_time_ms * 100.0) / 100.0;
    draw_text_blocks(14, 28, second_line.str(), { 180, 235, 140 });

    std::ostringstream third_line;
    third_line << "ENT " << overlay.entity_count;
    draw_text_blocks(14, 42, third_line.str(), { 255, 220, 120 });
}

void SoftwareRasterBackend::draw_entity(const RenderEntity& entity, const Camera& camera)
{
    const double pixels_per_meter = 24.0;
    const int center_x = static_cast<int>(m_width * 0.5 + (entity.transform.position.x - camera.position().x * 0.15) * pixels_per_meter);
    const int center_y = static_cast<int>(m_height * 0.62 - (entity.transform.position.z - camera.position().z * 0.15) * pixels_per_meter - entity.transform.position.y * 18.0);

    if (entity.mesh.primitive == MeshPrimitive::GroundPlane) {
        const int half_width = static_cast<int>(entity.transform.scale.x * pixels_per_meter * 0.5);
        const int half_depth = static_cast<int>(entity.transform.scale.z * pixels_per_meter * 0.25);
        draw_rect(center_x - half_width, center_y - half_depth, center_x + half_width, center_y + half_depth, { 44, 82, 48 });
        for (int line = -4; line <= 4; ++line) {
            const int y = center_y + line * half_depth / 4;
            draw_line(center_x - half_width, y, center_x + half_width, y, { 55, 100, 59 });
        }
        return;
    }

    const int size = static_cast<int>(entity.transform.scale.x * pixels_per_meter);
    const int offset = std::max(4, size / 3);
    draw_rect(center_x - size / 2, center_y - size, center_x + size / 2, center_y, { 126, 93, 58 });
    draw_line(center_x - size / 2, center_y - size, center_x - size / 2 + offset, center_y - size - offset, { 180, 138, 86 });
    draw_line(center_x + size / 2, center_y - size, center_x + size / 2 + offset, center_y - size - offset, { 180, 138, 86 });
    draw_line(center_x + size / 2, center_y, center_x + size / 2 + offset, center_y - offset, { 180, 138, 86 });
    draw_line(center_x - size / 2 + offset, center_y - size - offset, center_x + size / 2 + offset, center_y - size - offset, { 180, 138, 86 });
    draw_line(center_x + size / 2 + offset, center_y - size - offset, center_x + size / 2 + offset, center_y - offset, { 180, 138, 86 });
    draw_line(center_x + size / 2 + offset, center_y - offset, center_x + size / 2, center_y, { 180, 138, 86 });
}

void SoftwareRasterBackend::draw_rect(int min_x, int min_y, int max_x, int max_y, const ColorRgb& color)
{
    const int clamped_min_x = std::clamp(std::min(min_x, max_x), 0, m_width - 1);
    const int clamped_max_x = std::clamp(std::max(min_x, max_x), 0, m_width - 1);
    const int clamped_min_y = std::clamp(std::min(min_y, max_y), 0, m_height - 1);
    const int clamped_max_y = std::clamp(std::max(min_y, max_y), 0, m_height - 1);

    for (int y = clamped_min_y; y <= clamped_max_y; ++y) {
        for (int x = clamped_min_x; x <= clamped_max_x; ++x) {
            set_pixel(x, y, color);
        }
    }
}

void SoftwareRasterBackend::draw_line(int x0, int y0, int x1, int y1, const ColorRgb& color)
{
    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    while (true) {
        set_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) {
            break;
        }

        const int doubled_error = 2 * error;
        if (doubled_error >= dy) {
            error += dy;
            x0 += sx;
        }
        if (doubled_error <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

void SoftwareRasterBackend::draw_text_blocks(int x, int y, std::string_view text, const ColorRgb& color)
{
    constexpr int glyph_width = 4;
    constexpr int glyph_height = 7;
    constexpr int glyph_spacing = 2;

    int cursor = x;
    for (const char character : text) {
        if (character != ' ') {
            draw_rect(cursor, y, cursor + glyph_width - 1, y + glyph_height - 1, color);
        }
        cursor += glyph_width + glyph_spacing;
    }
}

void SoftwareRasterBackend::set_pixel(int x, int y, const ColorRgb& color)
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return;
    }

    m_pixels[static_cast<std::size_t>(y * m_width + x)] = color;
}

} // namespace Valley::Renderer
