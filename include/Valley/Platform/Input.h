#pragma once

#include <array>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Valley::Platform {

enum class KeyCode {
    Unknown = 0,
    Space,
    Enter,
    Escape,
    Tab,
    Backquote,
    W,
    A,
    S,
    D,
    Q,
    E,
    P,
    F,
    Digit0,
    Left,
    Right,
    Up,
    Down,
    LeftShift,
    RightShift,
    Equal,
    Minus,
    Period,
    Comma,
    Count
};

enum class MouseButton {
    Left = 0,
    Right,
    Middle,
    Count
};

struct ButtonState {
    bool pressed = false;
    bool held = false;
    bool released = false;
};

enum class InputBindingDevice {
    Key,
    MouseButton
};

struct InputBinding {
    InputBindingDevice device = InputBindingDevice::Key;
    KeyCode key = KeyCode::Unknown;
    MouseButton mouse_button = MouseButton::Left;
};

class ActionMap {
public:
    void bind_key(std::string action_name, KeyCode key);
    void bind_mouse_button(std::string action_name, MouseButton button);
    void clear_action(std::string_view action_name);

    [[nodiscard]] const std::vector<InputBinding>* bindings_for(std::string_view action_name) const;
    [[nodiscard]] bool has_action(std::string_view action_name) const;

private:
    std::unordered_map<std::string, std::vector<InputBinding>> m_bindings;
};

class InputSystem {
public:
    void begin_frame();
    void set_key_down(KeyCode key, bool down);
    void set_mouse_button_down(MouseButton button, bool down);
    void set_mouse_position(double x, double y);
    void clear_all();

    [[nodiscard]] ButtonState key_state(KeyCode key) const;
    [[nodiscard]] ButtonState mouse_button_state(MouseButton button) const;
    [[nodiscard]] ButtonState action_state(std::string_view action_name) const;
    [[nodiscard]] bool action_pressed(std::string_view action_name) const;
    [[nodiscard]] bool action_held(std::string_view action_name) const;
    [[nodiscard]] bool action_released(std::string_view action_name) const;
    [[nodiscard]] double mouse_x() const;
    [[nodiscard]] double mouse_y() const;

    [[nodiscard]] ActionMap& actions();
    [[nodiscard]] const ActionMap& actions() const;

private:
    struct DigitalButton {
        bool down = false;
        bool pressed = false;
        bool released = false;
    };

    static constexpr std::size_t key_count = static_cast<std::size_t>(KeyCode::Count);
    static constexpr std::size_t mouse_button_count = static_cast<std::size_t>(MouseButton::Count);

    template <typename Index>
    static constexpr std::size_t to_index(Index value)
    {
        return static_cast<std::size_t>(value);
    }

    std::array<DigitalButton, key_count> m_keys {};
    std::array<DigitalButton, mouse_button_count> m_mouse_buttons {};
    double m_mouse_x = 0.0;
    double m_mouse_y = 0.0;
    ActionMap m_actions;
};

[[nodiscard]] ActionMap create_default_debug_action_map();

} // namespace Valley::Platform
