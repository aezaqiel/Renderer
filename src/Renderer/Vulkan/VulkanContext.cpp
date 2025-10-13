#include "VulkanContext.hpp"

#include <vector>

namespace Renderer {

    VulkanContext::VulkanContext(Window& window)
    {
#ifndef NDEBUG
        s_InstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif
        s_InstanceExtensions = Window::GetRequiredVulkanExtensions();

        VK_CHECK(volkInitialize());
        CreateInstance();

        m_Surface = window.CreateVulkanSurface(m_Instance);

        PickPhysicalDevice();
        LOG_INFO("Physical device: {}", m_PhysicalDeviceProperties.deviceName);

        m_QueueIndices = FindQueueFamilies(m_PhysicalDevice, m_Surface);
        LOG_INFO("Graphics queue family index: {}", m_QueueIndices.Graphics());
        LOG_INFO("Compute queue family index: {}", m_QueueIndices.Compute());
        LOG_INFO("Transfer queue family index: {}", m_QueueIndices.Transfer());
        LOG_INFO("Present queue family index: {}", m_QueueIndices.Present());

        CreateDevice();
    }

    VulkanContext::~VulkanContext()
    {
        vkDestroyDevice(m_Device, nullptr);
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);
    }

    void VulkanContext::CreateInstance()
    {
#ifndef NDEBUG
        { // Check layers support
            u32 layerCount = 0;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            auto it = std::remove_if(s_InstanceLayers.begin(), s_InstanceLayers.end(), [&availableLayers](const char* layer) {
                for (const auto& available : availableLayers) {
                    if (std::strcmp(layer, available.layerName) == 0)
                        return false;
                }

                LOG_ERROR("Requested instance layer {} not available", layer);
                return true;
            });

            s_InstanceLayers.erase(it, s_InstanceLayers.end());
        }

        { // Check extensions support
            u32 extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

            auto it = std::remove_if(s_InstanceExtensions.begin(), s_InstanceExtensions.end(), [&availableExtensions](const char* extension) {
                for (const auto& available : availableExtensions) {
                    if (std::strcmp(extension, available.extensionName) == 0)
                        return false;
                }

                LOG_ERROR("Requested instance extension {} not available", extension);
                return true;
            });

            s_InstanceExtensions.erase(it, s_InstanceExtensions.end());
        }
#endif

        u32 apiVersion = 0;
        vkEnumerateInstanceVersion(&apiVersion);
        LOG_INFO("Vulkan API variant {} version {}.{}.{}",
            VK_API_VERSION_VARIANT(apiVersion),
            VK_API_VERSION_MAJOR(apiVersion),
            VK_API_VERSION_MINOR(apiVersion),
            VK_API_VERSION_PATCH(apiVersion)
        );

        VkApplicationInfo info {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Renderer",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = apiVersion
        };

        VkInstanceCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &info,
            .enabledLayerCount = static_cast<u32>(s_InstanceLayers.size()),
            .ppEnabledLayerNames = s_InstanceLayers.data(),
            .enabledExtensionCount = static_cast<u32>(s_InstanceExtensions.size()),
            .ppEnabledExtensionNames = s_InstanceExtensions.data()
        };

        VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
        volkLoadInstance(m_Instance);

        LOG_INFO("Instance layers:");
        for (const auto& layer : s_InstanceLayers)
            LOG_INFO(" - {}", layer);

        LOG_INFO("Instance extensions:");
        for (const auto& extension : s_InstanceExtensions)
            LOG_INFO(" - {}", extension);
    }

    VulkanContext::QueueFamilyIndices VulkanContext::FindQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface)
    {
        QueueFamilyIndices indices {};
        indices.Reset();

        u32 familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);

        if (familyCount == 0)
            return indices;

        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

        u32 index = 0;
        for (const auto& queue : families) {
            bool graphicsSupport = queue.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool computeSupport = queue.queueFlags & VK_QUEUE_COMPUTE_BIT;
            bool transferSupport = queue.queueFlags & VK_QUEUE_TRANSFER_BIT;

            if (graphicsSupport && !indices.HasGraphics())
                indices.graphics = index;

            if (graphicsSupport && !indices.HasPresent()) {
                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentSupport);

                if (presentSupport == VK_TRUE) {
                    indices.graphics = index;
                    indices.present = index;

                    index += 1;
                    continue;
                }
            }

            if (computeSupport && !graphicsSupport && !indices.HasCompute())
                indices.compute = index;

            if (transferSupport && !graphicsSupport && !computeSupport && !indices.HasTransfer())
                indices.transfer = index;

            index += 1;
        }

        if (!indices.HasPresent()) {
            index = 0;
            for (const auto& queue : families) {
                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentSupport);

                if (presentSupport == VK_TRUE) {
                    indices.present = index;
                    break;
                }

                index += 1;
            }
        }

        if (indices.HasGraphics()) {
            const auto& queue = families[indices.Graphics()];

            if (!indices.HasCompute() && (queue.queueFlags & VK_QUEUE_COMPUTE_BIT))
                indices.compute = indices.Graphics();

            if (!indices.HasTransfer() && (queue.queueFlags & VK_QUEUE_TRANSFER_BIT))
                indices.transfer = indices.Transfer();
        }

        if (!indices.HasCompute() || !indices.HasTransfer()) {
            index = 0;
            for (const auto& queue : families) {
                bool computeSupport = queue.queueFlags & VK_QUEUE_COMPUTE_BIT;
                bool transferSupport = queue.queueFlags & VK_QUEUE_TRANSFER_BIT;

                if (!indices.HasCompute() && computeSupport)
                    indices.compute = index;

                if (!indices.HasTransfer() && transferSupport)
                    indices.transfer = index;

                if (indices.HasCompute() && indices.HasTransfer())
                    break;

                index += 1;
            }
        }

        return indices;
    }

    void VulkanContext::PickPhysicalDevice()
    {
        u32 deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            LOG_FATAL("No supported physical device found");
            return;
        }

        std::vector<VkPhysicalDevice> availableDevices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, availableDevices.data());

        for (const auto& device : availableDevices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);

            if (props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                continue;

            QueueFamilyIndices indices = FindQueueFamilies(device, m_Surface);
            if (indices.IsComplete()) {
                m_PhysicalDevice = device;
                m_PhysicalDeviceProperties = props;
                break;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE) {
            LOG_WARN("Optimal physical device not found. Using fallback selection");

            m_PhysicalDevice = availableDevices.at(0);

            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties);
        }
    }

    void VulkanContext::CreateDevice()
    {
        {
            u32 extensionCount = 0;
            vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

            auto it = std::remove_if(s_DeviceExtensions.begin(), s_DeviceExtensions.end(), [&availableExtensions](const char* extension) {
                for (const auto& available : availableExtensions) {
                    if (std::strcmp(extension, available.extensionName) == 0)
                        return false;
                }

                LOG_ERROR("Requested device extension {} not available", extension);
                return true;
            });

            s_DeviceExtensions.erase(it, s_DeviceExtensions.end());
        }

        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        queueInfos.reserve(m_QueueIndices.GetUniqueIndices().size());

        for (u32 i : m_QueueIndices.GetUniqueIndices()) {
            static f32 priority[] = { 1.0f };

            queueInfos.emplace_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = i,
                .queueCount = 1,
                .pQueuePriorities = priority
            });
        }

        VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR swapchainMaintenance1 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR,
            .pNext = nullptr,
            .swapchainMaintenance1 = VK_TRUE
        };

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
            .pNext = &swapchainMaintenance1,
            .dynamicRendering = VK_TRUE
        };

        VkPhysicalDeviceFeatures2 features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &dynamicRendering,
            .features = VkPhysicalDeviceFeatures {}
        };

        VkDeviceCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features,
            .flags = 0,
            .queueCreateInfoCount = static_cast<u32>(queueInfos.size()),
            .pQueueCreateInfos = queueInfos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<u32>(s_DeviceExtensions.size()),
            .ppEnabledExtensionNames = s_DeviceExtensions.data(),
            .pEnabledFeatures = nullptr
        };

        VK_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));
        volkLoadDevice(m_Device);

        vkGetDeviceQueue(m_Device, m_QueueIndices.Graphics(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, m_QueueIndices.Compute(), 0, &m_ComputeQueue);
        vkGetDeviceQueue(m_Device, m_QueueIndices.Transfer(), 0, &m_TransferQueue);
        vkGetDeviceQueue(m_Device, m_QueueIndices.Present(), 0, &m_PresentQueue);

        LOG_INFO("Device extensions:");
        for (const auto& extension : s_DeviceExtensions)
            LOG_INFO(" - {}", extension);
    }

}
