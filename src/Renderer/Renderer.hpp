#pragma once

#include "Core/Window.hpp"
#include "Vulkan/VulkanContext.hpp"
#include "Vulkan/VulkanSwapchain.hpp"
#include "Vulkan/VulkanCommandRecorder.hpp"
#include "Vulkan/VulkanGraphicsPipeline.hpp"

namespace Renderer {

    class Renderer
    {
    public:
        Renderer(const std::shared_ptr<Window>& window);
        ~Renderer();

        void Resize(u32 width, u32 height);

        void Draw();

    private:
        std::shared_ptr<Window> m_Window;

        std::shared_ptr<VulkanContext> m_Context;
        std::unique_ptr<VulkanSwapchain> m_Swapchain;
        std::unique_ptr<VulkanCommandRecorder> m_Command;

        std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipeline;
    };

}
