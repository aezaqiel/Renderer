#include "Renderer.hpp"

// TEMPORARY
#include "Vulkan/VulkanShader.hpp"
#include "RenderGraph.hpp"

namespace Renderer {

    Renderer::Renderer(const Ref<Window>& window)
        : m_Window(window)
    {
        m_RenderThread = std::thread(&Renderer::RenderThreadLoop, this);
    }

    Renderer::~Renderer()
    {
        m_Running = false;
        m_QueueCondition.notify_one();

        if (m_RenderThread.joinable())
            m_RenderThread.join();
    }

    void Renderer::RequestResize(u32 width, u32 height)
    {
        {
            std::lock_guard<std::mutex> lock(m_RenderMutex);
            m_ResizeRequest = { true, width, height };
        }

        m_QueueCondition.notify_one();
    }

    void Renderer::Submit(std::vector<RenderPacket>& packets)
    {
        {
            std::lock_guard<std::mutex> lock(m_RenderMutex);
            m_StagingQueue.swap(packets);
        }

        m_QueueCondition.notify_one();
    }

    void Renderer::RenderThreadLoop()
    {
        LOG_INFO("Render thread running")

        CreateResources();

        while (m_Running) {
            std::unique_lock<std::mutex> lock(m_RenderMutex);

            m_QueueCondition.wait(lock, [this]{
                return !m_StagingQueue.empty() || !m_Running || m_ResizeRequest.pending;
            });

            if (m_ResizeRequest.pending) {
                lock.unlock();
                HandleResize();
                continue;
            }

            if (!m_Running && m_StagingQueue.empty())
                break;

            m_RenderQueue.swap(m_StagingQueue);
            lock.unlock();

            ProcessFrame();
            m_RenderQueue.clear();
        }

        vkDeviceWaitIdle(m_Context->GetDevice());
        DestroyResources();
    }

    void Renderer::ProcessFrame()
    {
        VK_CHECK(vkWaitForFences(m_Context->GetDevice(), 1, &m_Sync.at(m_FrameIndex).inFlight, VK_TRUE, std::numeric_limits<u64>::max()));

        if (!m_Swapchain->AcquireNextImage(m_Sync.at(m_FrameIndex).imageAvailable)) {
            return;
        }

        vkResetFences(m_Context->GetDevice(), 1, &m_Sync.at(m_FrameIndex).inFlight);

        RenderGraph rg;

        ImageDesc swapchainDesc {
            .width = m_Swapchain->GetWidth(),
            .height = m_Swapchain->GetHeight(),
            .format = m_Swapchain->GetFormat(),
        };

        ResourceHandle swapchainHandle = rg.CreateImage("Swapchain", swapchainDesc);

        Scope<VulkanGraphicsPipeline> pipeline = CreateScope<VulkanGraphicsPipeline>(m_Context, m_PipelineConfig);

        rg.AddPass("DrawTriangle",
            [&](RenderGraph::PassBuilder& builder) {
                builder.Writes(swapchainHandle, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            },
            [&](VkCommandBuffer cmd, const std::unordered_map<ResourceHandle, VkImageView>& imageViews) {
                static constexpr VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};

                VkRenderingAttachmentInfo colorAttachmentInfo {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                    .pNext = nullptr,
                    .imageView = imageViews.at(swapchainHandle),
                    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .resolveMode = VK_RESOLVE_MODE_NONE,
                    .resolveImageView = VK_NULL_HANDLE,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = clearColor
                };

                VkRenderingInfo renderingInfo {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .renderArea = {
                        .offset = { 0, 0 },
                        .extent = m_Swapchain->GetExtent()
                    },
                    .layerCount = 1,
                    .viewMask = 0,
                    .colorAttachmentCount = 1,
                    .pColorAttachments = &colorAttachmentInfo,
                    .pDepthAttachment = nullptr,
                    .pStencilAttachment = nullptr
                };

                vkCmdBeginRendering(cmd, &renderingInfo);
                pipeline->Bind(cmd);

                VkViewport viewport {
                    0.0f, 0.0f,
                    static_cast<f32>(m_Swapchain->GetWidth()), static_cast<f32>(m_Swapchain->GetHeight()),
                    0.0f, 1.0f,
                };

                VkRect2D scissor {
                    { 0, 0 },
                    m_Swapchain->GetExtent()
                };

                vkCmdSetViewport(cmd, 0, 1, &viewport);
                vkCmdSetScissor(cmd, 0, 1, &scissor);

                vkCmdDraw(cmd, 3, 1, 0, 0);

                vkCmdEndRendering(cmd);
            }
        );

        rg.AddPass("Present",
            [&](RenderGraph::PassBuilder& builder) {
                builder.Reads(swapchainHandle, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0);
            },
            nullptr
        );

        ExecutionPlan plan = rg.Compile();

        std::unordered_map<ResourceHandle, VkImage> images;
        std::unordered_map<ResourceHandle, VkImageView> imageViews;

        images[swapchainHandle] = m_Swapchain->GetCurrentImage();
        imageViews[swapchainHandle] = m_Swapchain->GetCurrentImageView();

        m_Commands.at(m_FrameIndex)->Record([&](const VkCommandBuffer& cmd) {
            std::unordered_map<ResourceHandle, VkImageLayout> currentLayouts;
            currentLayouts[swapchainHandle] = VK_IMAGE_LAYOUT_UNDEFINED;

            for (const auto& execPass : plan.orderedPasses) {
                std::vector<VkImageMemoryBarrier> imageBarriers;
                VkPipelineStageFlags srcStageMask = 0;
                VkPipelineStageFlags dstStageMask = 0;

                for (const auto& barrier : plan.barriers) {
                    if (barrier.dstPass == execPass.pass) {
                        VkImageLayout oldLayout = currentLayouts.at(barrier.resource);

                        imageBarriers.push_back(VkImageMemoryBarrier {
                            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                            .pNext = nullptr,
                            .srcAccessMask = barrier.srcAccessMask,
                            .dstAccessMask = barrier.dstAccessMask,
                            .oldLayout = oldLayout,
                            .newLayout = barrier.newLayout,
                            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            .image = images.at(barrier.resource),
                            .subresourceRange = {
                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                .baseMipLevel = 0,
                                .levelCount = 1,
                                .baseArrayLayer = 0,
                                .layerCount = 1
                            }
                        });
                        srcStageMask |= barrier.srcStageMask;
                        dstStageMask |= barrier.dstStageMask;
                    }
                }

                if (!imageBarriers.empty()) {
                    if (srcStageMask == 0)
                        srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

                    if (dstStageMask == 0)
                        dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

                    vkCmdPipelineBarrier(cmd,
                        srcStageMask,
                        dstStageMask,
                        0,
                        0, nullptr,
                        0, nullptr,
                        static_cast<u32>(imageBarriers.size()),
                        imageBarriers.data()
                    );

                    for (const auto& b : plan.barriers) {
                        if (b.dstPass == execPass.pass)
                            currentLayouts[b.resource] = b.newLayout;
                    }
                }

                const auto& passInfo = rg.GetPass(execPass.pass);
                if (passInfo.record)
                    passInfo.record(cmd, imageViews);
            }
        });

        vkWaitForFences(m_Context->GetDevice(), 1, &m_Sync.at(m_FrameIndex).inPresent, VK_TRUE, std::numeric_limits<u64>::max());
        m_Commands.at(m_FrameIndex)->Submit(
            { m_Sync.at(m_FrameIndex).imageAvailable },
            { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
            { m_Sync.at(m_FrameIndex).renderFinished },
            m_Sync.at(m_FrameIndex).inFlight
        );

        vkResetFences(m_Context->GetDevice(), 1, &m_Sync.at(m_FrameIndex).inPresent);
        m_Swapchain->Present(m_Context->GetPresentQueue(), m_Sync.at(m_FrameIndex).renderFinished, m_Sync.at(m_FrameIndex).inPresent);

        vkWaitForFences(m_Context->GetDevice(), 1, &m_Sync.at(m_FrameIndex).inFlight, VK_TRUE, std::numeric_limits<u64>::max());
        vkWaitForFences(m_Context->GetDevice(), 1, &m_Sync.at(m_FrameIndex).inPresent, VK_TRUE, std::numeric_limits<u64>::max());

        m_FrameIndex = (m_FrameIndex + 1) % s_FrameInFlight;
    }

    void Renderer::CreateResources()
    {
        m_Context = CreateRef<VulkanContext>(*m_Window);

        VulkanSwapchain::Config swapchainConfig {
            .extent = {
                .width = m_Window->Width(),
                .height = m_Window->Height()
            }
        };
        m_Swapchain = CreateScope<VulkanSwapchain>(m_Context, swapchainConfig);

        for (usize i = 0; i < s_FrameInFlight; ++i)
            m_Commands.at(i) = CreateScope<VulkanCommandRecorder>(m_Context, m_Context->GetGraphicsDeviceQueue());

        m_PipelineConfig.shaders.push_back(CreateRef<VulkanShader>(m_Context, "../shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
        m_PipelineConfig.shaders.push_back(CreateRef<VulkanShader>(m_Context, "../shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));
        m_PipelineConfig.frontFace = VK_FRONT_FACE_CLOCKWISE;
        m_PipelineConfig.depthTestEnabled = false;
        m_PipelineConfig.depthWriteEnabled = false;
        m_PipelineConfig.colorBlendAttachments.push_back(VkPipelineColorBlendAttachmentState {
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        });
        m_PipelineConfig.colorAttachmentFormats.push_back(m_Swapchain->GetFormat());

        static constexpr VkSemaphoreCreateInfo semaphoreInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };

        static constexpr VkFenceCreateInfo fenceInfo {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        for (usize i = 0; i < s_FrameInFlight; ++i) {
            vkCreateSemaphore(m_Context->GetDevice(), &semaphoreInfo, nullptr, &m_Sync.at(i).imageAvailable);
            vkCreateSemaphore(m_Context->GetDevice(), &semaphoreInfo, nullptr, &m_Sync.at(i).renderFinished);
            vkCreateFence(m_Context->GetDevice(), &fenceInfo, nullptr, &m_Sync.at(i).inFlight);
            vkCreateFence(m_Context->GetDevice(), &fenceInfo, nullptr, &m_Sync.at(i).inPresent);
        }

    }

    void Renderer::DestroyResources()
    {
        for (usize i = 0; i < s_FrameInFlight; ++i) {
            vkDestroyFence(m_Context->GetDevice(), m_Sync.at(i).inPresent, nullptr);
            vkDestroyFence(m_Context->GetDevice(), m_Sync.at(i).inFlight, nullptr);
            vkDestroySemaphore(m_Context->GetDevice(), m_Sync.at(i).renderFinished, nullptr);
            vkDestroySemaphore(m_Context->GetDevice(), m_Sync.at(i).imageAvailable, nullptr);
        }
        for (auto& command : m_Commands)
            command.reset();
        m_Swapchain.reset();
        m_Context.reset();
    }

    void Renderer::HandleResize()
    {
        ResizeRequest resize;
        {
            std::lock_guard<std::mutex> lock(m_RenderMutex);
            if (!m_ResizeRequest.pending)
                return;
            resize = m_ResizeRequest;
            m_ResizeRequest.pending = false;
        }

        if (resize.width == 0 || resize.height == 0)
            return;

        for (usize i = 0; i < s_FrameInFlight; ++i) {
            vkWaitForFences(m_Context->GetDevice(), 1, &m_Sync.at(i).inFlight, VK_TRUE, std::numeric_limits<u64>::max());
            vkWaitForFences(m_Context->GetDevice(), 1, &m_Sync.at(i).inPresent, VK_TRUE, std::numeric_limits<u64>::max());
        }

        m_Swapchain->Recreate(VkExtent2D{ resize.width, resize.height });
    }

}
