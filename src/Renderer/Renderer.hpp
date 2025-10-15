#pragma once

#include <array>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "Core/Window.hpp"
#include "Vulkan/VulkanContext.hpp"
#include "Vulkan/VulkanSwapchain.hpp"
#include "Vulkan/VulkanCommandRecorder.hpp"
#include "Vulkan/VulkanGraphicsPipeline.hpp"

namespace Renderer {

    class Renderer
    {
    public:
        struct RenderPacket
        {
        };

    public:
        Renderer(const std::shared_ptr<Window>& window);
        ~Renderer();

        void Resize(u32 width, u32 height);
        void Submit(std::vector<RenderPacket>& packets);

    private:
        struct SyncData
        {
            VkSemaphore imageAvailable { VK_NULL_HANDLE };
            VkSemaphore renderFinished { VK_NULL_HANDLE };
            VkFence inFlight { VK_NULL_HANDLE };
            VkFence inPresent { VK_NULL_HANDLE };
        };

    private:
        void RenderThreadLoop();
        void ProcessFrame();

        void CreateSyncObjects();
        void DestroySyncObjects();

    private:
        std::thread m_RenderThread;
        std::atomic<bool> m_Running { true };

        std::mutex m_QueueMutex;
        std::condition_variable m_QueueCondition;
        std::vector<RenderPacket> m_StagingQueue;
        std::vector<RenderPacket> m_RenderQueue;

        std::shared_ptr<Window> m_Window;

        std::shared_ptr<VulkanContext> m_Context;
        std::unique_ptr<VulkanSwapchain> m_Swapchain;

        std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipeline;

        inline static constexpr usize s_FrameInFlight { 2 };

        usize m_FrameIndex { 0 };
        std::array<std::unique_ptr<VulkanCommandRecorder>, s_FrameInFlight> m_Commands;
        std::array<SyncData, s_FrameInFlight> m_Sync;
    };

}
