#pragma once

#include "Valley/Core/EngineClock.h"
#include "Valley/Core/Module.h"
#include "Valley/Platform/Window.h"

#include <memory>
#include <vector>

namespace Valley::Core {

struct ApplicationDesc {
    Platform::WindowDesc window;
    EngineClockDesc clock;
    unsigned long long max_frames = 0;
};

class Application {
public:
    explicit Application(ApplicationDesc desc);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void add_module(std::unique_ptr<Module> module);
    [[nodiscard]] EngineClock& clock();
    [[nodiscard]] const EngineClock& clock() const;
    int run();

private:
    ApplicationDesc m_desc;
    EngineClock m_clock;
    std::unique_ptr<Platform::Window> m_window;
    std::vector<std::unique_ptr<Module>> m_modules;
    bool m_started = false;
};

} // namespace Valley::Core
