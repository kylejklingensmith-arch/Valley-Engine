#pragma once

#include "Valley/Core/EngineClock.h"
#include "Valley/Core/TimeStep.h"
#include "Valley/Platform/Input.h"
#include "Valley/Renderer/Camera.h"

namespace Valley::ToolsDebug {

struct DebugControlDesc {
    double free_camera_speed = 6.0;
    double fast_camera_multiplier = 4.0;
    double time_scale_step = 0.25;
    double min_time_scale = 0.0;
    double max_time_scale = 8.0;
};

struct DebugControlState {
    bool debug_overlays_enabled = true;
    bool free_camera_active = false;
};

class DebugControls {
public:
    explicit DebugControls(DebugControlDesc desc = {});

    void apply(const Platform::InputSystem& input, Core::EngineClock& clock, const Core::TimeStep& step);
    void move_free_camera(const Platform::InputSystem& input, Renderer::Camera& camera, const Core::TimeStep& step) const;

    [[nodiscard]] const DebugControlState& state() const;
    [[nodiscard]] bool debug_overlays_enabled() const;

private:
    DebugControlDesc m_desc;
    DebugControlState m_state;
};

} // namespace Valley::ToolsDebug
