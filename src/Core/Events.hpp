#pragma once

#include <variant>
#include <functional>

#include "Types.hpp"
#include "KeyCodes.hpp"

namespace Renderer {

    struct BaseEvent { bool handled { false }; };

    struct WindowClosedEvent final : public BaseEvent { };
    struct WindowResizedEvent final : public BaseEvent { u32 width; u32 height; };
    struct WindowMovedEvent final : public BaseEvent { i32 x; i32 y; };
    struct WindowMinimizeEvent final : public BaseEvent { bool minimized; };
    struct WindowFocusEvent final : public BaseEvent { bool focused; };
    struct KeyPressedEvent final : public BaseEvent { KeyCode keycode; bool repeat; };
    struct KeyReleasedEvent final : public BaseEvent { KeyCode keycode; };
    struct KeyTypedEvent final : public BaseEvent { KeyCode keycode; };
    struct MouseButtonPressedEvent final : public BaseEvent { MouseButton button; };
    struct MouseButtonReleasedEvent final : public BaseEvent { MouseButton button; };
    struct MouseMovedEvent final : public BaseEvent { f32 x; f32 y; };
    struct MouseScrolledEvent final : public BaseEvent { f32 x; f32 y; };

    template <typename T>
    concept IsEvent = std::is_base_of_v<BaseEvent, T>;

    using Event = std::variant<
        WindowClosedEvent,
        WindowResizedEvent,
        WindowMovedEvent,
        WindowMinimizeEvent,
        WindowFocusEvent,
        KeyPressedEvent,
        KeyReleasedEvent,
        KeyTypedEvent,
        MouseButtonPressedEvent,
        MouseButtonReleasedEvent,
        MouseMovedEvent,
        MouseScrolledEvent
    >;

    template <typename T, typename Func>
    concept EventHandler  = IsEvent<T> && std::is_invocable_r_v<bool, Func, const T&>;

    class EventDispatcher
    {
    public:
        constexpr EventDispatcher(Event& event) noexcept
            : m_Event(event) {}
        
        template <IsEvent T, EventHandler<T> Func>
        inline void Dispatch(Func&& func) noexcept
        {
            if (T* event = std::get_if<T>(&m_Event)) {
                if (!event->handled) {
                    event->handled = std::invoke(std::forward<Func>(func), event);
                }
            }
        }

    private:
        Event& m_Event;
    };

}
