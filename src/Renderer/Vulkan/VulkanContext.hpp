#pragma once

#include <optional>
#include <set>

#include "Core/Types.hpp"
#include "Core/Window.hpp"
#include "VulkanTypes.hpp"

namespace Renderer {

    class VulkanContext
    {
    public:
        VulkanContext(Window& window);
        ~VulkanContext();

        inline const VkInstance& GetInstance() const
        {
            return m_Instance;
        }

        inline const VkSurfaceKHR& GetSurface() const
        {
            return m_Surface;
        }

        inline const VkPhysicalDevice& GetPhysicalDevice() const
        {
            return m_PhysicalDevice;
        }
        inline const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const
        {
            return m_PhysicalDeviceProperties;
        }

        inline const VkQueue& GetGraphicsQueue() const
        {
            return m_GraphicsQueue;
        }

        inline u32 GetGraphicsQueueIndex() const
        {
            if (m_QueueIndices.HasGraphics())
                return m_QueueIndices.Graphics();
            LOG_ERROR("Graphics queue index not set");
            return std::numeric_limits<u32>::max();
        }

        inline const VkQueue& GetComputeQueue()
        {
            return m_ComputeQueue;
        }

        inline u32 GetComputeQueue() const
        {
            if (m_QueueIndices.HasCompute())
                return m_QueueIndices.Compute();
            LOG_ERROR("Compute queue index not set");
            return std::numeric_limits<u32>::max();
        }

        inline const VkQueue& GetTransferQueue() const
        {
            return m_TransferQueue;
        }

        inline u32 GetTransferQueueIndex() const
        {
            if (m_QueueIndices.HasTransfer())
                return m_QueueIndices.Transfer();
            LOG_ERROR("Transfer queue index not set");
            return std::numeric_limits<u32>::max();
        }

        inline const VkQueue& GetPresentQueue() const
        {
            return m_PresentQueue;
        }

        inline u32 GetPresentQueueIndex() const
        {
            if (m_QueueIndices.HasPresent())
                return m_QueueIndices.Present();
            LOG_ERROR("Present queue index not set");
            return std::numeric_limits<u32>::max();
        }

        inline std::set<u32> GetUniqueQueueIndices() const
        {
            return m_QueueIndices.GetUniqueIndices();
        }

        inline const VkDevice& GetDevice() const
        {
            return m_Device;
        }

    private:
        struct QueueFamilyIndices
        {
            std::optional<u32> graphics;
            std::optional<u32> compute;
            std::optional<u32> transfer;
            std::optional<u32> present;

            inline u32 Graphics() const { return graphics.value(); }
            inline u32 Compute() const { return compute.value(); }
            inline u32 Transfer() const { return transfer.value(); }
            inline u32 Present() const { return present.value(); }

            inline bool HasGraphics() const { return graphics.has_value(); }
            inline bool HasCompute() const { return compute.has_value(); }
            inline bool HasTransfer() const { return transfer.has_value(); }
            inline bool HasPresent() const { return present.has_value(); }

            inline bool IsComplete() const
            {
                return HasGraphics() && HasCompute() && HasTransfer() && HasPresent();
            }

            inline void Reset()
            {
                graphics.reset();
                compute.reset();
                transfer.reset();
                present.reset();
            }

            inline std::set<u32> GetUniqueIndices() const
            {
                return std::set<u32> {
                    Graphics(),
                    Compute(),
                    Transfer(),
                    Present()
                };
            }
        };

    private:
        void CreateInstance();

        static QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
        void PickPhysicalDevice();

        void CreateDevice();

    private:
        inline static std::vector<const char*> s_InstanceLayers;
        inline static std::vector<const char*> s_InstanceExtensions;
        inline static std::vector<const char*> s_DeviceExtensions {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkInstance m_Instance { VK_NULL_HANDLE };
        VkSurfaceKHR m_Surface { VK_NULL_HANDLE };

        VkPhysicalDevice m_PhysicalDevice { VK_NULL_HANDLE };
        VkPhysicalDeviceProperties m_PhysicalDeviceProperties;

        QueueFamilyIndices m_QueueIndices;
        VkQueue m_GraphicsQueue { VK_NULL_HANDLE };
        VkQueue m_ComputeQueue { VK_NULL_HANDLE };
        VkQueue m_TransferQueue { VK_NULL_HANDLE };
        VkQueue m_PresentQueue { VK_NULL_HANDLE };

        VkDevice m_Device { VK_NULL_HANDLE };
    };

}
