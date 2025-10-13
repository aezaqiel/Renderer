#pragma once

#include <string>

#include "Types.hpp"
#include "Events.hpp"

#include "Renderer/Vulkan/VulkanTypes.hpp"

struct GLFWwindow;

namespace Renderer {

    class Window
    {
        friend class Application;
    public:
        struct Config
        {
            u32 width { 1280 };
            u32 height { 720 };
            std::string title { "Window" };
        };

    public:
        Window(const Config& config);
        ~Window();

        inline u32 Width() const { return m_Data.width; };
        inline u32 Height() const { return m_Data.height; };
        inline GLFWwindow* Native() const { return m_Window; }

        static std::vector<const char*> GetRequiredVulkanExtensions();
        VkSurfaceKHR CreateVulkanSurface(const VkInstance& instance) const;
    
    protected:
        static void PollEvents();

        inline void BindEventQueue(EventQueue* queue)
        {
            m_Data.eventQueue = queue;
        }

    private:
        struct WindowData
        {
            u32 width { 0 };
            u32 height { 0 };

            EventQueue* eventQueue { nullptr };
        };

        GLFWwindow* m_Window { nullptr };
        WindowData m_Data;
    };

}
