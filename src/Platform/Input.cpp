#include "Valley/Platform/Input.h"

#include <utility>

namespace Valley::Platform {
namespace {
template <typename Index, typename Container>
bool valid_index(Index value, const Container& container)
{
    const auto index = static_cast<std::size_t>(value);
    return index < container.size();
}

} // namespace

void ActionMap::bind_key(std::string action_name, KeyCode key)
{
    m_bindings[std::move(action_name)].push_back({ .device = InputBindingDevice::Key, .key = key });
}

void ActionMap::bind_mouse_button(std::string action_name, MouseButton button)
{
    m_bindings[std::move(action_name)].push_back({ .device = InputBindingDevice::MouseButton, .mouse_button = button });
}

void ActionMap::clear_action(std::string_view action_name)
{
    m_bindings.erase(std::string(action_name));
}

const std::vector<InputBinding>* ActionMap::bindings_for(std::string_view action_name) const
{
    const auto found = m_bindings.find(std::string(action_name));
    if (found == m_bindings.end()) {
        return nullptr;
    }

    return &found->second;
}

bool ActionMap::has_action(std::string_view action_name) const
{
    return bindings_for(action_name) != nullptr;
}

void InputSystem::begin_frame()
{
    for (auto& key : m_keys) {
        key.pressed = false;
        key.released = false;
    }

    for (auto& button : m_mouse_buttons) {
        button.pressed = false;
        button.released = false;
    }
}

void InputSystem::set_key_down(KeyCode key, bool down)
{
    if (!valid_index(key, m_keys) || key == KeyCode::Unknown) {
        return;
    }

    auto& state = m_keys[to_index(key)];
    if (state.down == down) {
        return;
    }

    state.down = down;
    state.pressed = down;
    state.released = !down;
}

void InputSystem::set_mouse_button_down(MouseButton button, bool down)
{
    if (!valid_index(button, m_mouse_buttons)) {
        return;
    }

    auto& state = m_mouse_buttons[to_index(button)];
    if (state.down == down) {
        return;
    }

    state.down = down;
    state.pressed = down;
    state.released = !down;
}

void InputSystem::set_mouse_position(double x, double y)
{
    m_mouse_x = x;
    m_mouse_y = y;
}

void InputSystem::clear_all()
{
    for (auto& key : m_keys) {
        key = {};
    }

    for (auto& button : m_mouse_buttons) {
        button = {};
    }
}

ButtonState InputSystem::key_state(KeyCode key) const
{
    if (!valid_index(key, m_keys) || key == KeyCode::Unknown) {
        return {};
    }

    const auto& button = m_keys[to_index(key)];
    return { .pressed = button.pressed, .held = button.down, .released = button.released };
}

ButtonState InputSystem::mouse_button_state(MouseButton button) const
{
    if (!valid_index(button, m_mouse_buttons)) {
        return {};
    }

    const auto& state = m_mouse_buttons[to_index(button)];
    return { .pressed = state.pressed, .held = state.down, .released = state.released };
}

ButtonState InputSystem::action_state(std::string_view action_name) const
{
    ButtonState combined;
    const auto* bindings = m_actions.bindings_for(action_name);
    if (!bindings) {
        return combined;
    }

    for (const auto& binding : *bindings) {
        const ButtonState state = binding.device == InputBindingDevice::Key ? key_state(binding.key) : mouse_button_state(binding.mouse_button);
        combined.pressed = combined.pressed || state.pressed;
        combined.held = combined.held || state.held;
        combined.released = combined.released || state.released;
    }

    return combined;
}

bool InputSystem::action_pressed(std::string_view action_name) const { return action_state(action_name).pressed; }
bool InputSystem::action_held(std::string_view action_name) const { return action_state(action_name).held; }
bool InputSystem::action_released(std::string_view action_name) const { return action_state(action_name).released; }
double InputSystem::mouse_x() const { return m_mouse_x; }
double InputSystem::mouse_y() const { return m_mouse_y; }
ActionMap& InputSystem::actions() { return m_actions; }
const ActionMap& InputSystem::actions() const { return m_actions; }

ActionMap create_default_debug_action_map()
{
    ActionMap actions;
    actions.bind_key("debug.pause_toggle", KeyCode::P);
    actions.bind_key("debug.step_fixed_tick", KeyCode::Period);
    actions.bind_key("debug.time_scale_up", KeyCode::Equal);
    actions.bind_key("debug.time_scale_down", KeyCode::Minus);
    actions.bind_key("debug.time_scale_reset", KeyCode::Digit0);
    actions.bind_key("debug.overlays_toggle", KeyCode::Backquote);
    actions.bind_key("debug.camera.forward", KeyCode::W);
    actions.bind_key("debug.camera.backward", KeyCode::S);
    actions.bind_key("debug.camera.left", KeyCode::A);
    actions.bind_key("debug.camera.right", KeyCode::D);
    actions.bind_key("debug.camera.up", KeyCode::E);
    actions.bind_key("debug.camera.down", KeyCode::Q);
    actions.bind_key("debug.camera.fast", KeyCode::LeftShift);
    actions.bind_key("debug.camera.fast", KeyCode::RightShift);
    return actions;
}

} // namespace Valley::Platform
