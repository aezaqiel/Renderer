#pragma once

#include <expected>

#include "VulkanTypes.hpp"
#include "VulkanContext.hpp"

namespace Renderer {

    class VulkanSwapchain
    {
    public:
        struct Config
        {
            VkExtent2D extent { 0, 0 };
            VkFormat preferredFormat { VK_FORMAT_B8G8R8A8_SRGB };
            VkColorSpaceKHR preferredColorSpace { VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
            VkPresentModeKHR preferredPresentMode { VK_PRESENT_MODE_MAILBOX_KHR };
            VkImageUsageFlags imageUsage { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT };
        };

    public:
        VulkanSwapchain(const std::shared_ptr<VulkanContext>& context, const Config& config);
        ~VulkanSwapchain();

        inline const VkSwapchainKHR& GetSwapchain() const { return m_Swapchain; }
        inline const std::vector<VkImage>& GetImages() const { return m_Images; }
        inline const std::vector<VkImageView>& GetImageViews() const { return m_ImageViews; }
        inline u32 GetImageCount() const { return m_ImageCount; }

        inline const VkFormat& GetFormat() const { return m_Format; }
        inline const VkPresentModeKHR GetPresentMode() const { return m_PresentMode; }

        inline const VkExtent2D GetExtent() const { return m_Extent; }
        inline u32 GetWidth() const { return m_Extent.width; }
        inline u32 GetHeight() const { return m_Extent.height; }

        inline u32 GetCurrentImageIndex() const { return m_CurrentImageIndex; }
        inline const VkImage& GetCurrentImage() const { return m_Images[m_CurrentImageIndex]; }
        inline const VkImageView& GetCurrentImageView() const { return m_ImageViews[m_CurrentImageIndex]; }

        bool AcquireNextImage(VkSemaphore& signalSemaphore, u64 timeout = std::numeric_limits<u64>::max());
        bool Present(const VkQueue& presentQueue, const VkSemaphore& waitSemaphore, const VkFence& signalFence) const;

        void Recreate(VkExtent2D extent);

    private:
        struct SupportDetails
        {
            VkSurfaceCapabilitiesKHR surfaceCapabilities {};
            std::vector<VkSurfaceFormatKHR> surfaceFormats;
            std::vector<VkPresentModeKHR> presentModes;
        };

    private:
        SupportDetails QueueSwapchainSupport(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface) const;
        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available) const;
        VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& available) const;
        VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& caps, const VkExtent2D& desired) const;

        void CreateSwapchain(const VkSwapchainKHR& oldSwapchain = VK_NULL_HANDLE);
        void CleanupSwapchainResources();

    private:
        std::shared_ptr<VulkanContext> m_Context;
        Config m_Config;

        VkSwapchainKHR m_Swapchain { VK_NULL_HANDLE };

        u32 m_CurrentImageIndex { 0 };

        u32 m_ImageCount { 0 };
        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;

        VkFormat m_Format;
        VkColorSpaceKHR m_ColorSpace;
        VkPresentModeKHR m_PresentMode { VK_PRESENT_MODE_FIFO_KHR };
        VkExtent2D m_Extent { 0, 0 };
    };

}
