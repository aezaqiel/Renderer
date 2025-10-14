#include "VulkanGraphicsPipeline.hpp"

namespace Renderer {

    VulkanGraphicsPipeline::VulkanGraphicsPipeline(const std::shared_ptr<VulkanContext>& context, const Config& cfg)
        : m_Context(context)
    {
        VkPipelineLayoutCreateInfo layoutInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = static_cast<u32>(cfg.descriptorSetLayouts.size()),
            .pSetLayouts = cfg.descriptorSetLayouts.data(),
            .pushConstantRangeCount = static_cast<u32>(cfg.pushConstantRanges.size()),
            .pPushConstantRanges = cfg.pushConstantRanges.data()
        };

        VK_CHECK(vkCreatePipelineLayout(m_Context->GetDevice(), &layoutInfo, nullptr, &m_Layout));

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;;
        shaderStages.reserve(cfg.shaders.size());
        for (const auto& shader : cfg.shaders) {
            shaderStages.push_back(VkPipelineShaderStageCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = shader->GetStage(),
                .module = shader->GetModule(),
                .pName = "main",
                .pSpecializationInfo = nullptr
            });
        }

        VkPipelineVertexInputStateCreateInfo vertexInputState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = static_cast<u32>(cfg.vertexBindingDescriptions.size()),
            .pVertexBindingDescriptions = cfg.vertexBindingDescriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<u32>(cfg.vertexAttributeDescriptions.size()),
            .pVertexAttributeDescriptions = cfg.vertexAttributeDescriptions.data()
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = cfg.topology,
            .primitiveRestartEnable = VK_FALSE
        };

        VkPipelineViewportStateCreateInfo viewportState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr
        };

        VkPipelineRasterizationStateCreateInfo rasterizationState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = cfg.polygonMode,
            .cullMode = cfg.cullMode,
            .frontFace = cfg.frontFace,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = cfg.lineWidth
        };

        VkPipelineMultisampleStateCreateInfo multisampleState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = cfg.rasterSamples,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        VkPipelineDepthStencilStateCreateInfo depthStencilState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = static_cast<VkBool32>(cfg.depthTestEnabled),
            .depthWriteEnable = static_cast<VkBool32>(cfg.depthWriteEnabled),
            .depthCompareOp = cfg.depthCompareOp,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE
        };

        VkPipelineColorBlendStateCreateInfo colorBlendState {
            colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            colorBlendState.pNext = nullptr,
            colorBlendState.flags = 0,
            colorBlendState.logicOpEnable = VK_FALSE,
            colorBlendState.logicOp = VK_LOGIC_OP_COPY,
            colorBlendState.attachmentCount = static_cast<u32>(cfg.colorBlendAttachments.size()),
            colorBlendState.pAttachments = cfg.colorBlendAttachments.data(),
            colorBlendState.blendConstants[0] = 0.0f,
            colorBlendState.blendConstants[1] = 0.0f,
            colorBlendState.blendConstants[2] = 0.0f,
            colorBlendState.blendConstants[3] = 0.0f
        };

        std::vector<VkDynamicState> dynamicStates {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = static_cast<u32>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        VkPipelineRenderingCreateInfo renderingInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .pNext = nullptr,
            .colorAttachmentCount = static_cast<u32>(cfg.colorAttachmentFormats.size()),
            .pColorAttachmentFormats = cfg.colorAttachmentFormats.data(),
            .depthAttachmentFormat = cfg.depthAttachmentFormat,
            .stencilAttachmentFormat = cfg.stencilAttachmentFormat
        };

        VkGraphicsPipelineCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderingInfo,
            .flags = 0,
            .stageCount = static_cast<u32>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputState,
            .pInputAssemblyState = &inputAssemblyState,
            .pTessellationState = nullptr,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizationState,
            .pMultisampleState = &multisampleState,
            .pDepthStencilState = &depthStencilState,
            .pColorBlendState = &colorBlendState,
            .pDynamicState = &dynamicState,
            .layout = m_Layout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        VK_CHECK(vkCreateGraphicsPipelines(m_Context->GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_Pipeline));
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
        if (m_Pipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(m_Context->GetDevice(), m_Pipeline, nullptr);

        if (m_Layout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(m_Context->GetDevice(), m_Layout, nullptr);
    }

    void VulkanGraphicsPipeline::Bind(const VkCommandBuffer& cmd) const
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    }

    void VulkanGraphicsPipeline::SetViewport(const VkCommandBuffer& cmd, const VkViewport& viewport)
    {
        vkCmdSetViewport(cmd, 0, 1, &viewport);
    }

    void VulkanGraphicsPipeline::SetScissor(const VkCommandBuffer& cmd, const VkRect2D& scissor)
    {
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

}
