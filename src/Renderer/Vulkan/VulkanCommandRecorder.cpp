#include "VulkanCommandRecorder.hpp"

namespace Renderer {

    VulkanCommandRecorder::VulkanCommandRecorder(const std::shared_ptr<VulkanContext>& context, const DeviceQueue& queueFamily)
        : m_Context(context), m_QueueFamily(queueFamily)
    {
        VkCommandPoolCreateInfo poolInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_QueueFamily.index
        };

        VK_CHECK(vkCreateCommandPool(m_Context->GetDevice(), &poolInfo, nullptr, &m_CommandPool));

        VkCommandBufferAllocateInfo allocateInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        vkAllocateCommandBuffers(m_Context->GetDevice(), &allocateInfo, &m_CommandBuffer);

        static constexpr VkSemaphoreCreateInfo semaphoreInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };

        VK_CHECK(vkCreateSemaphore(m_Context->GetDevice(), &semaphoreInfo, nullptr, &m_InFlightSemaphore));

        static constexpr VkFenceCreateInfo fenceInfo {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        VK_CHECK(vkCreateFence(m_Context->GetDevice(), &fenceInfo, nullptr, &m_InFlightFence));
    }

    VulkanCommandRecorder::~VulkanCommandRecorder()
    {
        vkQueueWaitIdle(m_QueueFamily.queue);

        if (m_InFlightFence != VK_NULL_HANDLE)
            vkDestroyFence(m_Context->GetDevice(), m_InFlightFence, nullptr);

        if (m_InFlightSemaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(m_Context->GetDevice(), m_InFlightSemaphore, nullptr);

        if (m_CommandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(m_Context->GetDevice(), m_CommandPool, nullptr);
    }

    void VulkanCommandRecorder::Record(const std::function<void(const VkCommandBuffer&)>& task)
    {
        static constexpr VkCommandBufferBeginInfo beginInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr
        };

        VK_CHECK(vkWaitForFences(m_Context->GetDevice(), 1, &m_InFlightFence, VK_TRUE, std::numeric_limits<u64>::max()));
        vkResetFences(m_Context->GetDevice(), 1, &m_InFlightFence);

        VK_CHECK(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
        task(m_CommandBuffer);
        VK_CHECK(vkEndCommandBuffer(m_CommandBuffer));
    }

    const VkSemaphore& VulkanCommandRecorder::Submit(const std::vector<VkSemaphore>& waitSemaphore, const std::vector<VkPipelineStageFlags>& waitStages)
    {
        VkSubmitInfo submitInfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<u32>(waitSemaphore.size()),
            .pWaitSemaphores = waitSemaphore.data(),
            .pWaitDstStageMask = waitStages.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &m_CommandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_InFlightSemaphore
        };

        VK_CHECK(vkQueueSubmit(m_QueueFamily.queue, 1, &submitInfo, m_InFlightFence));

        return m_InFlightSemaphore;
    }

}
