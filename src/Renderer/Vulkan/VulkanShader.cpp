#include "VulkanShader.hpp"

#include <fstream>

namespace Renderer {

    VulkanShader::VulkanShader(const Ref<VulkanContext>& context, const std::string& filepath, VkShaderStageFlagBits stage)
        : m_Context(context), m_Stage(stage)
    {
        std::vector<u8> code = ReadFile(filepath);

        VkShaderModuleCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = static_cast<usize>(code.size()),
            .pCode = reinterpret_cast<const u32*>(code.data())
        };

        VK_CHECK(vkCreateShaderModule(m_Context->GetDevice(), &createInfo, nullptr, &m_Module));
        if (m_Module != VK_NULL_HANDLE)
            LOG_INFO("Loaded shader file {}", filepath);
    }

    VulkanShader::~VulkanShader()
    {
        if (m_Module != VK_NULL_HANDLE)
            vkDestroyShaderModule(m_Context->GetDevice(), m_Module, nullptr);
    }

    std::vector<u8> VulkanShader::ReadFile(const std::string& filepath)
    {
        std::vector<u8> data;

        std::ifstream file(filepath, std::ios::binary | std::ios::ate);

        if (!file.is_open()) {
            LOG_ERROR("Failed to open file {}", filepath);
            return data;
        }

        data.resize(static_cast<usize>(file.tellg()));
        file.seekg(0, std::ios::beg);

        file.read(reinterpret_cast<char*>(data.data()), data.size());

        file.close();

        return data;
    }

}
