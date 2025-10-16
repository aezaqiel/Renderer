#include "Application.hpp"

namespace Renderer {

    Application::Application()
    {
        s_Instance = this;

        m_EventQueue = CreateScope<EventQueue>();

        m_Window = CreateRef<Window>(Window::Config{
            .width = 1280,
            .height = 720,
            .title = "Renderer"
        });
        m_Window->BindEventQueue(m_EventQueue.get());

        m_Renderer = CreateScope<Renderer>(m_Window);
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
                std::vector<Renderer::RenderPacket> renderPackets(1, {});
                m_Renderer->Submit(renderPackets);
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

            dispatcher.Dispatch<WindowResizedEvent>([&](const WindowResizedEvent& e) -> bool {
                m_Renderer->RequestResize(e.width, e.height);
                return false;
            });

            // TODO: layers OnEvent
        }
    }

}
