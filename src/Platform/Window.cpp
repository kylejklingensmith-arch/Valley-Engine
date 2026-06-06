#include "Valley/Platform/Window.h"

#include "Valley/Core/Logger.h"

#include <cstdlib>
#include <memory>
#include <utility>

namespace Valley::Platform {
namespace {
class HeadlessWindow final : public Window {
public:
    explicit HeadlessWindow(WindowDesc desc)
        : m_desc(std::move(desc))
    {
        m_desc.headless = true;
        Core::Log::info("Platform", "Using headless window backend.");
    }

    void poll_events() override
    {
        m_input.begin_frame();
    }

    [[nodiscard]] bool should_close() const override
    {
        return m_should_close;
    }

    void request_close() override
    {
        m_should_close = true;
    }

    [[nodiscard]] InputSystem& input() override
    {
        return m_input;
    }

    [[nodiscard]] const InputSystem& input() const override
    {
        return m_input;
    }

    [[nodiscard]] bool is_headless() const override
    {
        return true;
    }

    [[nodiscard]] const WindowDesc& desc() const override
    {
        return m_desc;
    }

private:
    WindowDesc m_desc;
    InputSystem m_input;
    bool m_should_close = false;
};

bool has_display_environment()
{
    const char* display = std::getenv("DISPLAY");
    return display != nullptr && display[0] != '\0';
}
} // namespace

#ifdef VALLEY_PLATFORM_X11
std::unique_ptr<Window> create_x11_window(const WindowDesc& desc);
#endif

std::unique_ptr<Window> create_window(const WindowDesc& desc)
{
    if (desc.headless) {
        return std::make_unique<HeadlessWindow>(desc);
    }

#ifdef VALLEY_PLATFORM_X11
    if (has_display_environment()) {
        if (auto window = create_x11_window(desc)) {
            return window;
        }

        Core::Log::warn("Platform", "X11 window creation failed; falling back to headless mode.");
    } else {
        Core::Log::warn("Platform", "DISPLAY is not set; falling back to headless mode.");
    }
#else
    Core::Log::warn("Platform", "No native window backend was built; falling back to headless mode.");
#endif

    return std::make_unique<HeadlessWindow>(desc);
}

} // namespace Valley::Platform
