#pragma once

#include <optional>
#include <set>

#include "VulkanTypes.hpp"
#include "Core/Window.hpp"

namespace Renderer {

    struct DeviceQueue
    {
        u32 index { std::numeric_limits<u32>::max() };
        VkQueue queue { VK_NULL_HANDLE };
    };

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

        inline const DeviceQueue& GetGraphicsDeviceQueue() const
        {
            return m_GraphicsQueue;
        }

        inline const VkQueue& GetGraphicsQueue() const
        {
            return m_GraphicsQueue.queue;
        }

        inline u32 GetGraphicsQueueIndex() const
        {
            u32 index = m_GraphicsQueue.index;
            if (index == std::numeric_limits<u32>::max()) {
                LOG_ERROR("Graphics queue index not set")
            }
            return index;
        }

        inline const DeviceQueue& GetComputeDeviceQueue() const
        {
            return m_ComputeQueue;
        }

        inline const VkQueue& GetComputeQueue()
        {
            return m_ComputeQueue.queue;
        }

        inline u32 GetComputeQueueIndex() const
        {
            u32 index = m_ComputeQueue.index;
            if (index == std::numeric_limits<u32>::max()) {
                LOG_ERROR("Compute queue index not set")
            }
            return index;
        }

        inline const DeviceQueue& GetTransferDeviceQueue() const
        {
            return m_TransferQueue;
        }

        inline const VkQueue& GetTransferQueue() const
        {
            return m_TransferQueue.queue;
        }

        inline u32 GetTransferQueueIndex() const
        {
            u32 index = m_TransferQueue.index;
            if (index == std::numeric_limits<u32>::max()) {
                LOG_ERROR("Transfer queue index not set")
            }
            return index;
        }

        inline const DeviceQueue& GetPresentDeviceQueue() const
        {
            return m_PresentQueue;
        }

        inline const VkQueue& GetPresentQueue() const
        {
            return m_PresentQueue.queue;
        }

        inline u32 GetPresentQueueIndex() const
        {
            u32 index = m_PresentQueue.index;
            if (index == std::numeric_limits<u32>::max()) {
                LOG_ERROR("Present queue index not set")
            }
            return index;
        }

        inline std::set<u32> GetUniqueQueueIndices() const
        {
            return std::set<u32> {
                m_GraphicsQueue.index,
                m_ComputeQueue.index,
                m_TransferQueue.index,
                m_PresentQueue.index
            };
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

        DeviceQueue m_GraphicsQueue;
        DeviceQueue m_ComputeQueue;
        DeviceQueue m_TransferQueue;
        DeviceQueue m_PresentQueue;

        VkDevice m_Device { VK_NULL_HANDLE };
    };

}
