#include "Valley/Platform/Window.h"

#include "Valley/Core/Logger.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <memory>
#include <stdexcept>
#include <utility>

namespace Valley::Platform {
namespace {
KeyCode keycode_from_keysym(KeySym symbol)
{
    switch (symbol) {
    case XK_space: return KeyCode::Space;
    case XK_Return: return KeyCode::Enter;
    case XK_Escape: return KeyCode::Escape;
    case XK_Tab: return KeyCode::Tab;
    case XK_grave: return KeyCode::Backquote;
    case XK_w: case XK_W: return KeyCode::W;
    case XK_a: case XK_A: return KeyCode::A;
    case XK_s: case XK_S: return KeyCode::S;
    case XK_d: case XK_D: return KeyCode::D;
    case XK_q: case XK_Q: return KeyCode::Q;
    case XK_e: case XK_E: return KeyCode::E;
    case XK_p: case XK_P: return KeyCode::P;
    case XK_f: case XK_F: return KeyCode::F;
    case XK_Left: return KeyCode::Left;
    case XK_Right: return KeyCode::Right;
    case XK_Up: return KeyCode::Up;
    case XK_Down: return KeyCode::Down;
    case XK_Shift_L: return KeyCode::LeftShift;
    case XK_Shift_R: return KeyCode::RightShift;
    case XK_equal: case XK_plus: return KeyCode::Equal;
    case XK_minus: case XK_underscore: return KeyCode::Minus;
    case XK_period: case XK_greater: return KeyCode::Period;
    case XK_comma: case XK_less: return KeyCode::Comma;
    default: return KeyCode::Unknown;
    }
}

MouseButton mouse_button_from_x_button(unsigned int button)
{
    switch (button) {
    case Button1: return MouseButton::Left;
    case Button2: return MouseButton::Middle;
    case Button3: return MouseButton::Right;
    default: return MouseButton::Count;
    }
}

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
        XSelectInput(m_display, m_window, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);

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
        m_input.begin_frame();

        while (XPending(m_display) > 0) {
            XEvent event;
            XNextEvent(m_display, &event);

            if (event.type == ClientMessage && static_cast<Atom>(event.xclient.data.l[0]) == m_delete_message) {
                request_close();
            }

            if (event.type == DestroyNotify) {
                request_close();
            }

            if (event.type == KeyPress || event.type == KeyRelease) {
                const KeySym symbol = XLookupKeysym(&event.xkey, 0);
                m_input.set_key_down(keycode_from_keysym(symbol), event.type == KeyPress);
            }

            if (event.type == ButtonPress || event.type == ButtonRelease) {
                const MouseButton button = mouse_button_from_x_button(event.xbutton.button);
                if (button != MouseButton::Count) {
                    m_input.set_mouse_button_down(button, event.type == ButtonPress);
                }
                m_input.set_mouse_position(static_cast<double>(event.xbutton.x), static_cast<double>(event.xbutton.y));
            }

            if (event.type == MotionNotify) {
                m_input.set_mouse_position(static_cast<double>(event.xmotion.x), static_cast<double>(event.xmotion.y));
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
    InputSystem m_input;
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
