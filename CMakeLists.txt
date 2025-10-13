cmake_minimum_required(VERSION 3.30)

project(Renderer C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/volk)

add_executable(${PROJECT_NAME}
    src/Main.cpp

    src/Core/Types.hpp
    src/Core/Logger.hpp
    src/Core/Logger.cpp
    src/Core/Application.hpp
    src/Core/Application.cpp
    src/Core/KeyCodes.hpp
    src/Core/Events.hpp
    src/Core/Window.hpp
    src/Core/Window.cpp

    src/Renderer/Renderer.hpp
    src/Renderer/Renderer.cpp

    src/Renderer/Vulkan/VulkanTypes.hpp
    src/Renderer/Vulkan/VulkanContext.hpp
    src/Renderer/Vulkan/VulkanContext.cpp
)

target_include_directories(${PROJECT_NAME}
PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    spdlog
    glfw
    volk
)

target_compile_definitions(${PROJECT_NAME}
PRIVATE
    NOMINMAX
    GLFW_INCLUDE_NONE
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        GLFW_EXPOSE_NATIVE_WIN32
        VK_USE_PLATFORM_WIN32_KHR
    )
endif()
