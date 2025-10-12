#pragma once

#include "Events.hpp"
#include "Window.hpp"

namespace Renderer {

    class Application
    {
    public:
        Application();
        ~Application();

        static const Application& Get() { return *s_Instance; }
        inline const Window& GetWindow() const { return *m_Window; }

        void Run();

    private:
        void ProcessEvents();

    private:
        inline static Application* s_Instance { nullptr };

        bool m_Running { true };
        bool m_Minimized { false };

        std::unique_ptr<EventQueue> m_EventQueue;
        std::unique_ptr<Window> m_Window;
    };

}
