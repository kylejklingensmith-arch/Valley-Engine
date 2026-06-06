#include "Valley/Platform/Input.h"
#include "Valley/Core/EngineClock.h"
#include "Valley/ToolsDebug/DebugControls.h"

#include <cassert>
#include <iostream>

using namespace Valley;

namespace {

void key_state_transitions_report_pressed_held_and_released()
{
    Platform::InputSystem input;

    input.set_key_down(Platform::KeyCode::W, true);
    auto key = input.key_state(Platform::KeyCode::W);
    assert(key.pressed);
    assert(key.held);
    assert(!key.released);

    input.begin_frame();
    key = input.key_state(Platform::KeyCode::W);
    assert(!key.pressed);
    assert(key.held);
    assert(!key.released);

    input.set_key_down(Platform::KeyCode::W, false);
    key = input.key_state(Platform::KeyCode::W);
    assert(!key.pressed);
    assert(!key.held);
    assert(key.released);

    input.begin_frame();
    key = input.key_state(Platform::KeyCode::W);
    assert(!key.pressed);
    assert(!key.held);
    assert(!key.released);
}

void mouse_state_tracks_buttons_and_position()
{
    Platform::InputSystem input;
    input.set_mouse_position(42.0, 24.0);
    input.set_mouse_button_down(Platform::MouseButton::Left, true);

    const auto button = input.mouse_button_state(Platform::MouseButton::Left);
    assert(button.pressed);
    assert(button.held);
    assert(input.mouse_x() == 42.0);
    assert(input.mouse_y() == 24.0);
}

void action_map_combines_key_and_mouse_bindings()
{
    Platform::InputSystem input;
    input.actions().bind_key("select", Platform::KeyCode::Space);
    input.actions().bind_mouse_button("select", Platform::MouseButton::Left);

    input.set_mouse_button_down(Platform::MouseButton::Left, true);
    assert(input.action_pressed("select"));
    assert(input.action_held("select"));

    input.begin_frame();
    input.set_mouse_button_down(Platform::MouseButton::Left, false);
    input.set_key_down(Platform::KeyCode::Space, true);
    const auto action = input.action_state("select");
    assert(action.pressed);
    assert(action.held);
    assert(action.released);
}

void debug_actions_drive_engine_clock_and_overlay_state()
{
    Platform::InputSystem input;
    input.actions() = Platform::create_default_debug_action_map();
    Core::EngineClock clock;
    ToolsDebug::DebugControls controls({ .time_scale_step = 0.5 });

    input.set_key_down(Platform::KeyCode::P, true);
    controls.apply(input, clock, {});
    assert(clock.is_paused());

    input.begin_frame();
    input.set_key_down(Platform::KeyCode::Period, true);
    controls.apply(input, clock, {});
    assert(clock.queued_debug_steps() == 1);

    input.begin_frame();
    input.set_key_down(Platform::KeyCode::Equal, true);
    controls.apply(input, clock, {});
    assert(clock.time_scale() == 1.5);

    input.begin_frame();
    input.set_key_down(Platform::KeyCode::Backquote, true);
    controls.apply(input, clock, {});
    assert(!controls.debug_overlays_enabled());
}

} // namespace

int main()
{
    key_state_transitions_report_pressed_held_and_released();
    mouse_state_tracks_buttons_and_position();
    action_map_combines_key_and_mouse_bindings();
    debug_actions_drive_engine_clock_and_overlay_state();
    std::cout << "Input tests passed.\n";
    return 0;
}
