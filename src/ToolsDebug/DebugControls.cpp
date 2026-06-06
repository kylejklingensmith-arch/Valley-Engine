#include "Valley/ToolsDebug/DebugControls.h"

#include <algorithm>

namespace Valley::ToolsDebug {
namespace {
Renderer::Vec3 cross(const Renderer::Vec3& lhs, const Renderer::Vec3& rhs)
{
    return { lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x };
}
} // namespace

DebugControls::DebugControls(DebugControlDesc desc)
    : m_desc(desc)
{
}

void DebugControls::apply(const Platform::InputSystem& input, Core::EngineClock& clock, const Core::TimeStep&)
{
    if (input.action_pressed("debug.pause_toggle")) {
        clock.set_paused(!clock.is_paused());
    }

    if (input.action_pressed("debug.step_fixed_tick")) {
        clock.request_debug_step();
    }

    if (input.action_pressed("debug.time_scale_up")) {
        clock.set_time_scale(std::min(m_desc.max_time_scale, clock.time_scale() + m_desc.time_scale_step));
    }

    if (input.action_pressed("debug.time_scale_down")) {
        clock.set_time_scale(std::max(m_desc.min_time_scale, clock.time_scale() - m_desc.time_scale_step));
    }

    if (input.action_pressed("debug.overlays_toggle")) {
        m_state.debug_overlays_enabled = !m_state.debug_overlays_enabled;
    }

    m_state.free_camera_active = input.action_held("debug.camera.forward") || input.action_held("debug.camera.backward") || input.action_held("debug.camera.left") || input.action_held("debug.camera.right") || input.action_held("debug.camera.up") || input.action_held("debug.camera.down");
}

void DebugControls::move_free_camera(const Platform::InputSystem& input, Renderer::Camera& camera, const Core::TimeStep& step) const
{
    Renderer::Vec3 movement;
    const Renderer::Vec3 forward = Renderer::normalized({ camera.forward().x, 0.0, camera.forward().z });
    const Renderer::Vec3 right = Renderer::normalized(cross(forward, { 0.0, 1.0, 0.0 }));
    const Renderer::Vec3 up { 0.0, 1.0, 0.0 };

    if (input.action_held("debug.camera.forward")) {
        movement = movement + forward;
    }
    if (input.action_held("debug.camera.backward")) {
        movement = movement - forward;
    }
    if (input.action_held("debug.camera.right")) {
        movement = movement + right;
    }
    if (input.action_held("debug.camera.left")) {
        movement = movement - right;
    }
    if (input.action_held("debug.camera.up")) {
        movement = movement + up;
    }
    if (input.action_held("debug.camera.down")) {
        movement = movement - up;
    }

    const double movement_length = Renderer::length(movement);
    if (movement_length <= 0.000001) {
        return;
    }

    const double multiplier = input.action_held("debug.camera.fast") ? m_desc.fast_camera_multiplier : 1.0;
    const double distance = m_desc.free_camera_speed * multiplier * step.real_delta_seconds;
    const Renderer::Vec3 new_position = camera.position() + Renderer::normalized(movement) * distance;
    camera.look_at(new_position, new_position + camera.forward());
}

const DebugControlState& DebugControls::state() const { return m_state; }
bool DebugControls::debug_overlays_enabled() const { return m_state.debug_overlays_enabled; }

} // namespace Valley::ToolsDebug
