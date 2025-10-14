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

        m_Command = std::make_unique<VulkanCommandRecorder>(m_Context, m_Context->GetGraphicsDeviceQueue());

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
    }

    Renderer::~Renderer()
    {
        vkDeviceWaitIdle(m_Context->GetDevice());
    }

    void Renderer::Resize(u32 width, u32 height)
    {
        m_Swapchain->Recreate(VkExtent2D { width, height });
    }

    void Renderer::Draw()
    {
        auto imageAvailable = m_Swapchain->AcquireNextImage();
        if (imageAvailable) {
            m_Command->Record([&](const VkCommandBuffer& cmd) {
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

            VkSemaphore renderFinished = m_Command->Submit({ *imageAvailable }, { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT });

            m_Swapchain->Present(m_Context->GetPresentQueue(), renderFinished);
        }
    }

}
