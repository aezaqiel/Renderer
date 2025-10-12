#include "Application.hpp"

namespace Renderer {

    Application::Application()
    {
        s_Instance = this;

        m_EventQueue = std::make_unique<EventQueue>();

        m_Window = std::make_unique<Window>(Window::Config{
            .width = 1280,
            .height = 720,
            .title = "Renderer"
        });
        m_Window->BindEventQueue(m_EventQueue.get());
    }

    Application::~Application()
    {
        s_Instance = nullptr;
    }

    void Application::Run()
    {
        while (m_Running) {
            Window::PollEvents();
            ProcessEvents();

            if (!m_Minimized) {
            }
        }
    }

    void Application::ProcessEvents()
    {
        for (auto& event : m_EventQueue->Poll()) {
            EventDispatcher dispatcher(event);

            dispatcher.Dispatch<WindowClosedEvent>([&](const WindowClosedEvent& e) -> bool {
                m_Running = false;
                return true;
            });

            dispatcher.Dispatch<WindowMinimizeEvent>([&](const WindowMinimizeEvent& e) -> bool {
                m_Minimized = e.minimized;
                return false;
            });

            // TODO: layers OnEvent
        }
    }

}
