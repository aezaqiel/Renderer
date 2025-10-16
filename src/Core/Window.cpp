#include "Window.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Logger.hpp"
#include "Renderer/Vulkan/VulkanTypes.hpp"

namespace Renderer {

    Window::Window(const Config& config)
    {
        glfwSetErrorCallback([](i32 code, const char* desc) {
            LOG_ERROR("GLFW Error {}: {}", code, desc)
#ifdef NDEBUG
            (void)code;
            (void)desc;
#endif
        });

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow(
            static_cast<u32>(config.width), static_cast<u32>(config.height),
            config.title.c_str(),
            nullptr, nullptr
        );

        {
            i32 width, height;
            glfwGetFramebufferSize(m_Window, &width, &height);
            m_Data.width = static_cast<u32>(width);
            m_Data.height = static_cast<u32>(height);
        }

        glfwSetWindowUserPointer(m_Window, &m_Data);

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.eventQueue->Push(WindowClosedEvent());
        });

        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, i32 width, i32 height) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.width = static_cast<u32>(width);
            data.height = static_cast<u32>(height);

            data.eventQueue->Push(WindowResizedEvent(data.width, data.height));
        });

        glfwSetWindowPosCallback(m_Window, [](GLFWwindow* window, i32 x, i32 y) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.eventQueue->Push(WindowMovedEvent(x, y));
        });

        glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, i32 iconified) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.eventQueue->Push(WindowMinimizeEvent(static_cast<bool>(iconified)));
        });

        glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, i32 focused) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.eventQueue->Push(WindowFocusEvent(static_cast<bool>(focused)));
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, i32 key, i32, i32 action, i32) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action) {
                case GLFW_PRESS: {
                    data.eventQueue->Push(KeyPressedEvent(static_cast<KeyCode>(key), false));
                } break;
                case GLFW_RELEASE: {
                    data.eventQueue->Push(KeyReleasedEvent(static_cast<KeyCode>(key)));
                } break;
                case GLFW_REPEAT: {
                    data.eventQueue->Push(KeyPressedEvent(static_cast<KeyCode>(key), true));
                } break;
                default: {
                    LOG_WARN("Unknown key action {}", action)
                }
            }
        });

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, u32 code) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.eventQueue->Push(KeyTypedEvent(static_cast<KeyCode>(code)));
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, i32 button, i32 action, i32) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action) {
                case GLFW_PRESS: {
                    data.eventQueue->Push(MouseButtonPressedEvent(static_cast<MouseButton>(button)));
                } break;
                case GLFW_RELEASE: {
                    data.eventQueue->Push(MouseButtonReleasedEvent(static_cast<MouseButton>(button)));
                } break;
                default: {
                    LOG_WARN("Unknown mouse button action {}", action)
                }
            }
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, f64 x, f64 y) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.eventQueue->Push(MouseMovedEvent(static_cast<f32>(x), static_cast<f32>(y)));
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, f64 x, f64 y) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.eventQueue->Push(MouseScrolledEvent(static_cast<f32>(x), static_cast<f32>(y)));
        });

        LOG_INFO("Created window [{}] ({}, {})", glfwGetWindowTitle(m_Window), m_Data.width, m_Data.height)
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
        glfwTerminate();
    }

    void Window::PollEvents()
    {
        glfwPollEvents();
    }

    std::vector<const char*> Window::GetRequiredVulkanExtensions()
    {
        u32 count = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&count);

        return std::vector<const char*>(extensions, extensions + count);
    }

    VkSurfaceKHR Window::CreateVulkanSurface(const VkInstance& instance) const
    {
        VkSurfaceKHR surface { VK_NULL_HANDLE };
        VK_CHECK(glfwCreateWindowSurface(instance, m_Window, nullptr, &surface));

        return surface;
    }

}
