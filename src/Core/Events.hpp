#pragma once

#include <variant>
#include <functional>
#include <vector>
#include <mutex>

#include "Types.hpp"
#include "KeyCodes.hpp"

namespace Renderer {

    struct BaseEvent { bool handled { false }; };

    struct WindowClosedEvent final : public BaseEvent
    {
        constexpr WindowClosedEvent() noexcept = default;
    };

    struct WindowResizedEvent final : public BaseEvent
    {
        u32 width;
        u32 height;

        constexpr WindowResizedEvent(u32 width, u32 height) noexcept
            : height(height), width(width) {}
    };

    struct WindowMovedEvent final : public BaseEvent
    {
        i32 x;
        i32 y;

        constexpr WindowMovedEvent(i32 x, i32 y) noexcept
            : x(x), y(y) {}
    };

    struct WindowMinimizeEvent final : public BaseEvent
    {
        bool minimized;

        constexpr WindowMinimizeEvent(bool minimized) noexcept
            : minimized(minimized) {}
    };

    struct WindowFocusEvent final : public BaseEvent
    {
        bool focused;

        constexpr WindowFocusEvent(bool focused) noexcept
            : focused(focused) {}
    };

    struct KeyPressedEvent final : public BaseEvent
    {
        KeyCode keycode;
        bool repeat;

        constexpr KeyPressedEvent(KeyCode code, bool repeat) noexcept
            : keycode(code), repeat(repeat) {}
    };

    struct KeyReleasedEvent final : public BaseEvent
    {
        KeyCode keycode;

        constexpr KeyReleasedEvent(KeyCode code) noexcept
            : keycode(code) {}
    };

    struct KeyTypedEvent final : public BaseEvent
    {
        KeyCode keycode;

        constexpr KeyTypedEvent(KeyCode code) noexcept
            : keycode(code) {}
    };

    struct MouseButtonPressedEvent final : public BaseEvent
    {
        MouseButton button;

        constexpr MouseButtonPressedEvent(MouseButton button) noexcept
            : button(button) {}
    };

    struct MouseButtonReleasedEvent final : public BaseEvent
    {
        MouseButton button;

        constexpr MouseButtonReleasedEvent(MouseButton button) noexcept
            : button(button) {}
    };

    struct MouseMovedEvent final : public BaseEvent
    {
        f32 x;
        f32 y;

        constexpr MouseMovedEvent(f32 x, f32 y) noexcept
            : x(x), y(y) {}
    };

    struct MouseScrolledEvent final : public BaseEvent
    {
        f32 x;
        f32 y;

        constexpr MouseScrolledEvent(f32 x, f32 y) noexcept
            : x(x), y(y) {}
    };

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
    concept EventHandler  = std::is_invocable_r_v<bool, Func, const T&>;

    class EventDispatcher
    {
    public:
        constexpr EventDispatcher(Event& event) noexcept
            : m_Event(event) {}
        
        template <IsEvent T, typename Func> requires EventHandler<T, Func>
        inline void Dispatch(Func&& func) noexcept
        {
            if (T* event = std::get_if<T>(&m_Event)) {
                if (!event->handled) {
                    event->handled = std::invoke(std::forward<Func>(func), *event);
                }
            }
        }

    private:
        Event& m_Event;
    };

    class EventQueue
    {
    public:
        EventQueue() = default;
        ~EventQueue() = default;

        EventQueue(EventQueue& other) = delete;
        EventQueue(const EventQueue& other) = delete;

        template <IsEvent T>
        inline void Push(T&& event)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Queue.push_back(event);
        }

        inline std::vector<Event> Poll()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);

            std::vector<Event> polled;
            polled.swap(m_Queue);

            return std::move(polled);
        }

    private:
        std::vector<Event> m_Queue;
        std::mutex m_Mutex;
    };

}
