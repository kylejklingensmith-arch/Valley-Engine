#pragma once

#include <memory>
#include <string>

namespace Valley::Platform {

struct WindowDesc {
    std::string title = "Valley Engine";
    int width = 1280;
    int height = 720;
    bool headless = false;
};

class Window {
public:
    virtual ~Window() = default;

    virtual void poll_events() = 0;
    [[nodiscard]] virtual bool should_close() const = 0;
    virtual void request_close() = 0;
    [[nodiscard]] virtual bool is_headless() const = 0;
    [[nodiscard]] virtual const WindowDesc& desc() const = 0;
};

std::unique_ptr<Window> create_window(const WindowDesc& desc);

} // namespace Valley::Platform
