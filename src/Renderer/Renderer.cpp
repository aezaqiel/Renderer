#include "Renderer.hpp"

// TEMPORARY
#include "Vulkan/VulkanShader.hpp"

namespace Renderer {

    Renderer::Renderer(const std::shared_ptr<Window>& window)
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

    void Renderer::Resize(u32 width, u32 height)
    {
        m_Swapchain->Recreate(VkExtent2D { width, height });
    }

    void Renderer::Submit(std::vector<RenderPacket>& packets)
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_StagingQueue.swap(packets);
        lock.unlock();

        m_QueueCondition.notify_one();
    }

    void Renderer::RenderThreadLoop()
    {
        LOG_DEBUG("Render thread running");

        m_Context = std::make_shared<VulkanContext>(*m_Window);

        VulkanSwapchain::Config swapchainConfig {
            .extent = {
                .width = m_Window->Width(),
                .height = m_Window->Height()
            }
        };
        m_Swapchain = std::make_unique<VulkanSwapchain>(m_Context, swapchainConfig);

        for (usize i = 0; i < s_FrameInFlight; ++i)
            m_Commands.at(i) = std::make_unique<VulkanCommandRecorder>(m_Context, m_Context->GetGraphicsDeviceQueue());

        VulkanGraphicsPipeline::Config graphicsPipelineConfig;
        graphicsPipelineConfig.shaders.push_back(std::make_shared<VulkanShader>(m_Context, "shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
        graphicsPipelineConfig.shaders.push_back(std::make_shared<VulkanShader>(m_Context, "shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));
        graphicsPipelineConfig.frontFace = VK_FRONT_FACE_CLOCKWISE;
        graphicsPipelineConfig.depthTestEnabled = false;
        graphicsPipelineConfig.depthWriteEnabled = false;
        graphicsPipelineConfig.colorBlendAttachments.push_back(VkPipelineColorBlendAttachmentState {
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        });
        graphicsPipelineConfig.colorAttachmentFormats.push_back(m_Swapchain->GetFormat());

        m_GraphicsPipeline = std::make_unique<VulkanGraphicsPipeline>(m_Context, graphicsPipelineConfig);

        CreateSyncObjects();

        while (m_Running) {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_QueueCondition.wait(lock, [this]{
                return !m_StagingQueue.empty() || !m_Running;
            });

            if (!m_Running && m_StagingQueue.empty())
                break;

            m_RenderQueue.swap(m_StagingQueue);
            lock.unlock();

            ProcessFrame();

            m_RenderQueue.clear();
        }

        vkDeviceWaitIdle(m_Context->GetDevice());

        DestroySyncObjects();
        m_GraphicsPipeline.reset();
        for (auto& command : m_Commands)
            command.reset();
        m_Swapchain.reset();
        m_Context.reset();
    }

    void Renderer::ProcessFrame()
    {
        VK_CHECK(vkWaitForFences(m_Context->GetDevice(), 1, &m_Sync.at(m_FrameIndex).inFlight, VK_TRUE, std::numeric_limits<u64>::max()));

        if (m_Swapchain->AcquireNextImage(m_Sync.at(m_FrameIndex).imageAvailable)) {
            vkResetFences(m_Context->GetDevice(), 1, &m_Sync.at(m_FrameIndex).inFlight);

            m_Commands.at(m_FrameIndex)->Record([&](const VkCommandBuffer& cmd) {
                VkImageMemoryBarrier imageBarrier {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = 0,
                    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = m_Swapchain->GetCurrentImage(),
                    .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                    }
                };

                vkCmdPipelineBarrier(cmd,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageBarrier
                );

                static constexpr VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};

                VkRenderingAttachmentInfo colorAttachmentInfo {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext = nullptr,
                    .imageView = m_Swapchain->GetCurrentImageView(),
                    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
                    .colorAttachmentCount = 1,
                    .pColorAttachments = &colorAttachmentInfo,
                    .pDepthAttachment = nullptr,
                    .pStencilAttachment = nullptr
                };

                vkCmdBeginRendering(cmd, &renderingInfo);

                m_GraphicsPipeline->Bind(cmd);

                VkViewport viewport {
                    .x = 0.0f, .y = 0.0f,
                    .width = static_cast<f32>(m_Swapchain->GeWidth()),
                    .height = static_cast<f32>(m_Swapchain->GeHeight()),
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f
                };
                m_GraphicsPipeline->SetViewport(cmd, viewport);

                VkRect2D scissor {
                    .offset = { 0, 0 },
                    .extent = m_Swapchain->GetExtent()
                };
                m_GraphicsPipeline->SetScissor(cmd, scissor);

                vkCmdDraw(cmd, 3, 1, 0, 0);

                vkCmdEndRendering(cmd);

                imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                imageBarrier.dstAccessMask = 0;
                imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                vkCmdPipelineBarrier(cmd,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageBarrier
                );
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

            m_FrameIndex = (m_FrameIndex + 1) % s_FrameInFlight;
        }
    }

    void Renderer::CreateSyncObjects()
    {
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

    void Renderer::DestroySyncObjects()
    {
        for (usize i = 0; i < s_FrameInFlight; ++i) {
            vkDestroyFence(m_Context->GetDevice(), m_Sync.at(i).inPresent, nullptr);
            vkDestroyFence(m_Context->GetDevice(), m_Sync.at(i).inFlight, nullptr);
            vkDestroySemaphore(m_Context->GetDevice(), m_Sync.at(i).renderFinished, nullptr);
            vkDestroySemaphore(m_Context->GetDevice(), m_Sync.at(i).imageAvailable, nullptr);
        }
    }

}
