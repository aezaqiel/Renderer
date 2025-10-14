#pragma once

#include "VulkanTypes.hpp"
#include "VulkanContext.hpp"

namespace Renderer {

    class VulkanCommandRecorder
    {
    public:
        VulkanCommandRecorder(const std::shared_ptr<VulkanContext>& context, const DeviceQueue& queueFamily);
        ~VulkanCommandRecorder();

        void Record(const std::function<void(const VkCommandBuffer&)>& task);
        const VkSemaphore& Submit(const std::vector<VkSemaphore>& waitSemaphore, const std::vector<VkPipelineStageFlags>& waitStages);

    private:
        std::shared_ptr<VulkanContext> m_Context;
        const DeviceQueue& m_QueueFamily;

        VkCommandPool m_CommandPool { VK_NULL_HANDLE };
        VkCommandBuffer m_CommandBuffer { VK_NULL_HANDLE };

        VkSemaphore m_InFlightSemaphore { VK_NULL_HANDLE };
        VkFence m_InFlightFence { VK_NULL_HANDLE };
    };

}
