#pragma once

#include "Types.hpp"
#include "Events.hpp"
#include "Window.hpp"
#include "Renderer/Renderer.hpp"

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

        Scope<EventQueue> m_EventQueue;
        Ref<Window> m_Window;
        Scope<Renderer> m_Renderer;
    };

}
