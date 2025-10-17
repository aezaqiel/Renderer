#pragma once

#include <map>

#include "Core/Types.hpp"
#include "Vulkan/VulkanTypes.hpp"

namespace Renderer {

    using ResourceHandle = u32;
    using PassHandle = u32;

    enum class ResourceType
    {
        Image,
        Buffer
    };

    enum class AccessType
    {
        Read,
        Write,
        ReadWrite
    };

    struct ImageDesc
    {
        u32 width { 0 };
        u32 height { 0 };
        VkFormat format { VK_FORMAT_UNDEFINED };
        VkImageUsageFlags usage { 0 };
        VkSampleCountFlagBits samples { VK_SAMPLE_COUNT_1_BIT };
        bool transient { false };
    };

    struct AccessInfo
    {
        ResourceHandle resource;
        AccessType type;
        VkImageLayout layout;
        VkPipelineStageFlags stage;
        VkAccessFlags accessMask;
    };

    struct Resource
    {
        ResourceType type;
        std::string name;
        ImageDesc imageDesc;
        bool imported { false };
        i32 firstUse { -1 };
        i32 lastUse { -1 };
    };

    struct Pass
    {
        std::string name;
        std::vector<AccessInfo> accesses;
        std::function<void(VkCommandBuffer, const std::unordered_map<ResourceHandle, VkImageView>&)> record;
    };

    struct Barrier
    {
        PassHandle srcPass;
        PassHandle dstPass;
        ResourceHandle resource;
        VkImageLayout oldLayout;
        VkImageLayout newLayout;
        VkPipelineStageFlags srcStageMask;
        VkPipelineStageFlags dstStageMask;
        VkAccessFlags srcAccessMask;
        VkAccessFlags dstAccessMask;
        u32 srcQueueFamily { VK_QUEUE_FAMILY_IGNORED };
        u32 dstQueueFamily { VK_QUEUE_FAMILY_IGNORED };
    };

    struct ExecutionPass
    {
        PassHandle pass;
        std::string name;
    };

    struct ExecutionPlan
    {
        std::vector<ExecutionPass> orderedPasses;
        std::vector<Resource> resources;
        std::vector<Barrier> barriers;
        std::vector<i32> allocationIdPerResource;
    };

    class RenderGraph
    {
    public:
        class PassBuilder
        {
        public:
            PassBuilder(Pass& outPass)
                : m_Pass(outPass)
            {
            }

            ~PassBuilder()
            {
                for (auto const& [_, val] : m_Accesses) {
                    m_Pass.accesses.push_back(val);
                }
            }

            void Reads(
                ResourceHandle resource,
                VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VkPipelineStageFlags stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VkAccessFlags accessMask = VK_ACCESS_SHADER_READ_BIT
            )
            {
                AddAccess({
                    .resource = resource,
                    .type = AccessType::Read,
                    .layout = layout,
                    .stage = stage,
                    .accessMask = accessMask
                });
            }

            void Writes(
                ResourceHandle resource,
                VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VkAccessFlags accessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
            )
            {
                AddAccess({
                    .resource = resource,
                    .type = AccessType::Write,
                    .layout = layout,
                    .stage = stage,
                    .accessMask = accessMask
                });
            }

        private:
            void AddAccess(AccessInfo ai)
            {
                if (m_Accesses.count(ai.resource)) {
                    AccessInfo& existing = m_Accesses[ai.resource];

                    if (existing.type == AccessType::Read && ai.type == AccessType::Write)
                        existing.type = AccessType::ReadWrite;

                    if (existing.type == AccessType::Write && ai.type == AccessType::Read)
                        existing.type = AccessType::ReadWrite;

                    existing.layout = ai.layout;
                    existing.stage |= ai.stage;
                    existing.accessMask |= ai.accessMask;
                } else {
                    m_Accesses[ai.resource] = ai;
                }
            }

        private:
            Pass& m_Pass;
            std::map<ResourceHandle, AccessInfo> m_Accesses;
        };

    public:
        RenderGraph() = default;

        const Resource& GetResource(ResourceHandle handle) const
        {
            return m_Resources.at(handle);
        }

        const Pass& GetPass(PassHandle handle) const
        {
            return m_Passes.at(handle);
        }

        ResourceHandle CreateImage(const std::string& name, ImageDesc desc, bool imported = false);
        PassHandle AddPass(const std::string& name, std::function<void(class RenderGraph::PassBuilder&)> setup, std::function<void(VkCommandBuffer, const std::unordered_map<ResourceHandle, VkImageView>&)> record = {});
        ExecutionPlan Compile();

    private:
        std::vector<Resource> m_Resources;
        std::vector<Pass> m_Passes;
    };

}
