#include "Renderer.hpp"

namespace Renderer {

    Renderer::Renderer(const std::shared_ptr<Window>& window)
        : m_Window(window)
    {
        m_Context = std::make_shared<VulkanContext>(*m_Window);
    }

    void Renderer::Resize()
    {
    }

    void Renderer::BeginFrame()
    {
    }

    void Renderer::EndFrame()
    {
    }

    void Renderer::Draw()
    {
    }

}
