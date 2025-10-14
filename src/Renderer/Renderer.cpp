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
