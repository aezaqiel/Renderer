#pragma once

#include <string>

#include "VulkanTypes.hpp"
#include "VulkanContext.hpp"

namespace Renderer {

    class VulkanShader
    {
    public:
        VulkanShader(const Ref<VulkanContext>& context, const std::string& filepath, VkShaderStageFlagBits stage);
        ~VulkanShader();

        inline const VkShaderModule& GetModule() const { return m_Module; }
        inline const VkShaderStageFlagBits& GetStage() const { return m_Stage; }

    private:
        static std::vector<u8> ReadFile(const std::string& filepath);

    private:
        Ref<VulkanContext> m_Context;

        VkShaderModule m_Module { VK_NULL_HANDLE };
        VkShaderStageFlagBits m_Stage { VK_SHADER_STAGE_ALL };
    };

}
