#pragma once

#include "VulkanTypes.hpp"
#include "VulkanContext.hpp"
#include "VulkanShader.hpp"

namespace Renderer {

    class VulkanGraphicsPipeline
    {
    public:
        struct Config
        {
            std::vector<Ref<VulkanShader>> shaders;

            std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
            std::vector<VkPushConstantRange> pushConstantRanges;

            std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
            std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;

            VkPrimitiveTopology topology { VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };

            VkPolygonMode polygonMode { VK_POLYGON_MODE_FILL };
            VkCullModeFlags cullMode { VK_CULL_MODE_BACK_BIT };
            VkFrontFace frontFace { VK_FRONT_FACE_COUNTER_CLOCKWISE };
            f32 lineWidth { 1.0f };

            VkSampleCountFlagBits rasterSamples { VK_SAMPLE_COUNT_1_BIT };

            bool depthTestEnabled { true };
            bool depthWriteEnabled { true };
            VkCompareOp depthCompareOp { VK_COMPARE_OP_LESS };

            std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

            std::vector<VkFormat> colorAttachmentFormats;
            VkFormat depthAttachmentFormat { VK_FORMAT_UNDEFINED };
            VkFormat stencilAttachmentFormat { VK_FORMAT_UNDEFINED };
        };

    public:
        VulkanGraphicsPipeline(const Ref<VulkanContext>& context, const Config& cfg);
        ~VulkanGraphicsPipeline();

        inline const VkPipelineLayout& GetLayout() const { return m_Layout; }
        inline const VkPipeline& GetPipeline() const { return m_Pipeline; }

        void Bind(const VkCommandBuffer& cmd) const;
        void SetViewport(const VkCommandBuffer& cmd, const VkViewport& viewport);
        void SetScissor(const VkCommandBuffer& cmd, const VkRect2D& scissor);

    private:
        Ref<VulkanContext> m_Context;

        VkPipelineLayout m_Layout { VK_NULL_HANDLE };
        VkPipeline m_Pipeline { VK_NULL_HANDLE };
    };

}
