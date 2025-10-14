#pragma once

#include "Core/Window.hpp"
#include "Vulkan/VulkanContext.hpp"
#include "Vulkan/VulkanSwapchain.hpp"
#include "Vulkan/VulkanGraphicsPipeline.hpp"

namespace Renderer {

    class Renderer
    {
    public:
        Renderer(const std::shared_ptr<Window>& window);
        ~Renderer() = default;

        void Resize(u32 width, u32 height);

        void BeginFrame();
        void EndFrame();

        void Draw();

    private:
        std::shared_ptr<Window> m_Window;

        std::shared_ptr<VulkanContext> m_Context;
        std::unique_ptr<VulkanSwapchain> m_Swapchain;
        std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipeline;
    };

}
