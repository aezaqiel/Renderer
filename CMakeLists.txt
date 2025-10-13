cmake_minimum_required(VERSION 3.30)

project(Renderer C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED COMPONENTS glslc)

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
    src/Renderer/Vulkan/VulkanSwapchain.hpp
    src/Renderer/Vulkan/VulkanSwapchain.cpp
    src/Renderer/Vulkan/VulkanShader.hpp
    src/Renderer/Vulkan/VulkanShader.cpp
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

find_program(glslc_exe NAMES glslc HINTS Vulkan::glslc)

set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(GLOB SHADERS
    ${SHADER_SRC_DIR}/*.vert
    ${SHADER_SRC_DIR}/*.frag
    ${SHADER_SRC_DIR}/*.comp
    ${SHADER_SRC_DIR}/*.geom
    ${SHADER_SRC_DIR}/*.tesc
    ${SHADER_SRC_DIR}/*.tese
    ${SHADER_SRC_DIR}/*.mesh
    ${SHADER_SRC_DIR}/*.task
    ${SHADER_SRC_DIR}/*.rgen
    ${SHADER_SRC_DIR}/*.rchit
    ${SHADER_SRC_DIR}/*.rmiss
)

add_custom_command(
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BIN_DIR}
    OUTPUT ${SHADER_BIN_DIR}
    COMMENT "Creating ${SHADER_BIN_DIR}"
)

foreach(source IN LISTS SHADERS)
    get_filename_component(FILENAME ${source} NAME)
    add_custom_command(
        COMMAND ${glslc_exe} --target-spv=spv1.4 -o ${SHADER_BIN_DIR}/${FILENAME}.spv ${source}
        OUTPUT ${SHADER_BIN_DIR}/${FILENAME}.spv
        DEPENDS ${source} ${SHADER_BIN_DIR}
        COMMENT "Compiling ${FILENAME}"
    )
    list(APPEND SPV_SHADERS ${SHADER_BIN_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SPV_SHADERS})
add_dependencies(${PROJECT_NAME} compile_shaders)
