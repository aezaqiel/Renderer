#include "VulkanSwapchain.hpp"

#include <algorithm>

namespace Renderer {

    VulkanSwapchain::VulkanSwapchain(const std::shared_ptr<VulkanContext>& context, const Config& config)
        : m_Context(context), m_Config(config)
    {
        CreateSwapchain();
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        CleanupSwapchainResources();

        if (m_Swapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(m_Context->GetDevice(), m_Swapchain, nullptr);
    }

    bool VulkanSwapchain::AcquireNextImage(VkSemaphore& signalSemaphore, u64 timeout)
    {
        if (m_Swapchain == VK_NULL_HANDLE) {
            LOG_ERROR("Swapchain not created");
            return false;
        }

        VkResult result = vkAcquireNextImageKHR(m_Context->GetDevice(), m_Swapchain, timeout, signalSemaphore, VK_NULL_HANDLE, &m_CurrentImageIndex);
        if (result != VK_SUCCESS) {
            LOG_WARN("Failed to acquire swapchain image");
            return false;
        }

        return true;
    }

    bool VulkanSwapchain::Present(const VkQueue& presentQueue, const VkSemaphore& waitSemaphore, const VkFence& signalFence) const
    {
        if (m_Swapchain == VK_NULL_HANDLE) {
            LOG_ERROR("Swapchain not created");
            return false;
        }

        VkSwapchainPresentFenceInfoKHR presentFenceInfo {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR,
            .pNext = nullptr,
            .swapchainCount = 1,
            .pFences = &signalFence
        };

        VkPresentInfoKHR presentInfo {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = &presentFenceInfo,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &waitSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &m_Swapchain,
            .pImageIndices = &m_CurrentImageIndex,
            .pResults = nullptr
        };

        VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
        if (result != VK_SUCCESS) {
            LOG_WARN("Failed to present swapchain");
            return false;
        }

        return true;
    }

    void VulkanSwapchain::Recreate(VkExtent2D extent)
    {
        m_Config.extent = extent;

        VkSwapchainKHR oldSwapchain = m_Swapchain;
        CreateSwapchain(oldSwapchain);
    }

    VulkanSwapchain::SupportDetails VulkanSwapchain::QueueSwapchainSupport(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface) const
    {
        SupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.surfaceCapabilities);

        u32 formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        if (formatCount > 0) {
            details.surfaceFormats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.surfaceFormats.data());
        }

        u32 presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        if (presentModeCount > 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR VulkanSwapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available) const
    {
        if (available.empty()) {
            LOG_WARN("No surface formats available");
            return {};
        }

        VkSurfaceFormatKHR format;

        if (available.size() == 1 && available.at(0).format == VK_FORMAT_UNDEFINED) {
            format.format = m_Config.preferredFormat;
            format.colorSpace = m_Config.preferredColorSpace;
            return format;
        }

        for (const auto& f : available) {
            if (f.format == m_Config.preferredFormat && f.colorSpace == m_Config.preferredColorSpace)
                return f;
        }

        LOG_WARN("Requested swapchain surface format not available");
        return available.at(0);
    }

    VkPresentModeKHR VulkanSwapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& available) const
    {
        for (const auto& pm : available) {
            if (pm == m_Config.preferredPresentMode)
                return pm;
        }

        for (const auto& pm : available) {
            if (pm == VK_PRESENT_MODE_IMMEDIATE_KHR)
                return pm;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanSwapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR& caps, const VkExtent2D& desired) const
    {
        if (caps.currentExtent.width != std::numeric_limits<u32>::max()) {
            return caps.currentExtent;
        }

        VkExtent2D extent;
        extent.width = std::clamp(desired.width, caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = std::clamp(desired.height, caps.minImageExtent.height, caps.maxImageExtent.height);

        return extent;
    }

    void VulkanSwapchain::CreateSwapchain(const VkSwapchainKHR& oldSwapchain)
    {
        SupportDetails support = QueueSwapchainSupport(m_Context->GetPhysicalDevice(), m_Context->GetSurface());

        m_Format = ChooseSurfaceFormat(support.surfaceFormats).format;
        m_ColorSpace = ChooseSurfaceFormat(support.surfaceFormats).colorSpace;
        m_PresentMode = ChoosePresentMode(support.presentModes);
        m_Extent = ChooseExtent(support.surfaceCapabilities, (m_Config.extent.width == 0 || m_Config.extent.height == 0) ? support.surfaceCapabilities.currentExtent : m_Config.extent);

        m_ImageCount = support.surfaceCapabilities.minImageCount + 1;
        if (support.surfaceCapabilities.maxImageCount > 0)
            m_ImageCount = std::min(m_ImageCount, support.surfaceCapabilities.maxImageCount);

        VkSwapchainCreateInfoKHR createInfo {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = m_Context->GetSurface(),
            .minImageCount = m_ImageCount,
            .imageFormat = m_Format,
            .imageColorSpace = m_ColorSpace,
            .imageExtent = m_Extent,
            .imageArrayLayers = 1,
            .imageUsage = m_Config.imageUsage,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = support.surfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = m_PresentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = oldSwapchain
        };

        std::vector<u32> queueIndices;
        for (const auto& index : m_Context->GetUniqueQueueIndices())
            queueIndices.push_back(index);

        if (queueIndices.size() > 1) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = static_cast<u32>(queueIndices.size());
            createInfo.pQueueFamilyIndices = queueIndices.data();
        }

        VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
        VK_CHECK(vkCreateSwapchainKHR(m_Context->GetDevice(), &createInfo, nullptr, &newSwapchain));

        if (oldSwapchain != VK_NULL_HANDLE)
            CleanupSwapchainResources();

        vkGetSwapchainImagesKHR(m_Context->GetDevice(), newSwapchain, &m_ImageCount, nullptr);

        m_Images.resize(m_ImageCount);
        vkGetSwapchainImagesKHR(m_Context->GetDevice(), newSwapchain, &m_ImageCount, m_Images.data());

        m_ImageViews.resize(m_ImageCount);
        for (usize i = 0; i < m_ImageCount; ++i) {
            VkImageViewCreateInfo viewInfo {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = m_Images.at(i),
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_Format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            VK_CHECK(vkCreateImageView(m_Context->GetDevice(), &viewInfo, nullptr, &m_ImageViews.at(i)));
        }

        if (oldSwapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(m_Context->GetDevice(), oldSwapchain, nullptr);

        m_Swapchain = std::move(newSwapchain);
    }

    void VulkanSwapchain::CleanupSwapchainResources()
    {
        for (auto& view : m_ImageViews) {
            if (view != VK_NULL_HANDLE)
                vkDestroyImageView(m_Context->GetDevice(), view, nullptr);
        }
    }

}
