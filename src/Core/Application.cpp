#include "Valley/Core/Application.h"

#include "Valley/Core/Logger.h"
#include "Valley/Platform/Input.h"

#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>

namespace Valley::Core {

Application::Application(ApplicationDesc desc)
    : m_desc(std::move(desc)), m_clock(m_desc.clock), m_window(Platform::create_window(m_desc.window))
{
    if (!m_window) {
        throw std::runtime_error("Failed to create a platform window backend.");
    }

    m_window->input().actions() = Platform::create_default_debug_action_map();
}

Application::~Application()
{
    if (!m_started) {
        return;
    }

    for (auto module = m_modules.rbegin(); module != m_modules.rend(); ++module) {
        (*module)->on_detach();
    }
}

void Application::add_module(std::unique_ptr<Module> module)
{
    if (!module) {
        throw std::invalid_argument("Cannot add a null engine module.");
    }

    if (m_started) {
        throw std::logic_error("Modules must be registered before Application::run().");
    }

    m_modules.push_back(std::move(module));
}

EngineClock& Application::clock()
{
    return m_clock;
}

const EngineClock& Application::clock() const
{
    return m_clock;
}

Platform::InputSystem& Application::input()
{
    return m_window->input();
}

const Platform::InputSystem& Application::input() const
{
    return m_window->input();
}

ToolsDebug::DebugControls& Application::debug_controls()
{
    return m_debug_controls;
}

const ToolsDebug::DebugControls& Application::debug_controls() const
{
    return m_debug_controls;
}

int Application::run()
{
    Log::info("Core", "Starting Valley Engine.");

    for (auto& module : m_modules) {
        Log::info("Core", std::string("Attaching module: ").append(module->name()));
        module->on_attach();
    }

    m_started = true;

    using system_clock = std::chrono::steady_clock;
    auto previous_time = system_clock::now();

    while (!m_window->should_close()) {
        const auto now = system_clock::now();
        const std::chrono::duration<double> real_delta = now - previous_time;
        previous_time = now;

        m_window->poll_events();

        m_debug_controls.apply(m_window->input(), m_clock, { .real_delta_seconds = real_delta.count() });

        const TimeStep frame_step = m_clock.advance(real_delta.count());

        for (unsigned int fixed_update = 0; fixed_update < frame_step.fixed_update_count; ++fixed_update) {
            const auto fixed_frame_index = frame_step.fixed_frame_index + fixed_update;
            const TimeStep fixed_step {
                .delta_seconds = frame_step.fixed_delta_seconds,
                .elapsed_seconds = static_cast<double>(fixed_frame_index + 1) * frame_step.fixed_delta_seconds,
                .real_delta_seconds = frame_step.real_delta_seconds,
                .real_elapsed_seconds = frame_step.real_elapsed_seconds,
                .fixed_delta_seconds = frame_step.fixed_delta_seconds,
                .time_scale = frame_step.time_scale,
                .fixed_update_count = frame_step.fixed_update_count,
                .frame_index = frame_step.frame_index,
                .fixed_frame_index = fixed_frame_index,
                .paused = frame_step.paused,
                .debug_step = frame_step.debug_step,
            };

            for (auto& module : m_modules) {
                module->on_fixed_update(fixed_step);
            }
        }

        for (auto& module : m_modules) {
            module->on_update(frame_step);
        }

        if (m_desc.max_frames != 0 && m_clock.frame_index() >= m_desc.max_frames) {
            m_window->request_close();
        }

        if (m_window->is_headless()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    Log::info("Core", "Valley Engine stopped cleanly.");
    return 0;
}

} // namespace Valley::Core
