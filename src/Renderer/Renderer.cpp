#include "Renderer.hpp"

// TEMPORARY
#include "Vulkan/VulkanShader.hpp"

namespace Renderer {

    Renderer::Renderer(const std::shared_ptr<Window>& window)
        : m_Window(window)
    {
        m_Context = std::make_shared<VulkanContext>(*m_Window);

        VulkanSwapchain::Config swapchainConfig {
            .extent = {
                .width = m_Window->Width(),
                .height = m_Window->Height()
            }
        };
        m_Swapchain = std::make_unique<VulkanSwapchain>(m_Context, swapchainConfig);

        VulkanShader vertexShader(m_Context, "shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        VulkanShader fragmentShader(m_Context, "shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    void Renderer::Resize(u32 width, u32 height)
    {
        m_Swapchain->Recreate(VkExtent2D { width, height });
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
