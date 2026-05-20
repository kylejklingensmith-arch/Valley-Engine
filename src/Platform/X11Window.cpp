#include "Valley/Platform/Window.h"

#include "Valley/Core/Logger.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <memory>
#include <stdexcept>
#include <utility>

namespace Valley::Platform {
namespace {
class X11Window final : public Window {
public:
    explicit X11Window(WindowDesc desc)
        : m_desc(std::move(desc))
    {
        m_display = XOpenDisplay(nullptr);
        if (m_display == nullptr) {
            throw std::runtime_error("XOpenDisplay returned null.");
        }

        const int screen = DefaultScreen(m_display);
        const auto root = RootWindow(m_display, screen);
        const auto background = BlackPixel(m_display, screen);

        m_window = XCreateSimpleWindow(
            m_display,
            root,
            0,
            0,
            static_cast<unsigned int>(m_desc.width),
            static_cast<unsigned int>(m_desc.height),
            0,
            background,
            background);

        XStoreName(m_display, m_window, m_desc.title.c_str());
        XSelectInput(m_display, m_window, ExposureMask | KeyPressMask | StructureNotifyMask);

        m_delete_message = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(m_display, m_window, &m_delete_message, 1);

        XMapWindow(m_display, m_window);
        XFlush(m_display);

        Core::Log::info("Platform", "Created X11 window backend.");
    }

    ~X11Window() override
    {
        if (m_display != nullptr && m_window != 0) {
            XDestroyWindow(m_display, m_window);
        }

        if (m_display != nullptr) {
            XCloseDisplay(m_display);
        }
    }

    void poll_events() override
    {
        while (XPending(m_display) > 0) {
            XEvent event;
            XNextEvent(m_display, &event);

            if (event.type == ClientMessage && static_cast<Atom>(event.xclient.data.l[0]) == m_delete_message) {
                request_close();
            }

            if (event.type == DestroyNotify) {
                request_close();
            }
        }
    }

    [[nodiscard]] bool should_close() const override
    {
        return m_should_close;
    }

    void request_close() override
    {
        m_should_close = true;
    }

    [[nodiscard]] bool is_headless() const override
    {
        return false;
    }

    [[nodiscard]] const WindowDesc& desc() const override
    {
        return m_desc;
    }

private:
    WindowDesc m_desc;
    Display* m_display = nullptr;
    ::Window m_window = 0;
    Atom m_delete_message = 0;
    bool m_should_close = false;
};
} // namespace

std::unique_ptr<Window> create_x11_window(const WindowDesc& desc)
{
    try {
        return std::make_unique<X11Window>(desc);
    } catch (const std::exception& exception) {
        Core::Log::warn("Platform", exception.what());
        return nullptr;
    }
}

} // namespace Valley::Platform
