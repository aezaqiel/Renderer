cmake_minimum_required(VERSION 3.30)

project(Renderer
    LANGUAGES C CXX
    VERSION 0.1.0
    DESCRIPTION "Vulkan Renderer"
)

cmake_policy(SET CMP0079 NEW)


set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(GLSLC_EXE NAMES glslc HINTS Vulkan::glslc)

if(WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/volk)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PREFIX "Headers"
    FILES
        src/Core/Types.hpp
        src/Core/Logger.hpp
        src/Core/Application.hpp
        src/Core/KeyCodes.hpp
        src/Core/Events.hpp
        src/Core/Window.hpp
        src/Renderer/Renderer.hpp
        src/Renderer/RenderGraph.hpp
        src/Renderer/Vulkan/VulkanTypes.hpp
        src/Renderer/Vulkan/VulkanContext.hpp
        src/Renderer/Vulkan/VulkanSwapchain.hpp
        src/Renderer/Vulkan/VulkanShader.hpp
        src/Renderer/Vulkan/VulkanGraphicsPipeline.hpp
        src/Renderer/Vulkan/VulkanCommandRecorder.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PREFIX "Sources"
    FILES
        src/Main.cpp
        src/Core/Logger.cpp
        src/Core/Application.cpp
        src/Core/Window.cpp
        src/Renderer/Renderer.cpp
        src/Renderer/RenderGraph.cpp
        src/Renderer/Vulkan/VulkanContext.cpp
        src/Renderer/Vulkan/VulkanSwapchain.cpp
        src/Renderer/Vulkan/VulkanShader.cpp
        src/Renderer/Vulkan/VulkanGraphicsPipeline.cpp
        src/Renderer/Vulkan/VulkanCommandRecorder.cpp
)

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
    src/Renderer/RenderGraph.hpp
    src/Renderer/RenderGraph.cpp

    src/Renderer/Vulkan/VulkanTypes.hpp
    src/Renderer/Vulkan/VulkanContext.hpp
    src/Renderer/Vulkan/VulkanContext.cpp
    src/Renderer/Vulkan/VulkanSwapchain.hpp
    src/Renderer/Vulkan/VulkanSwapchain.cpp
    src/Renderer/Vulkan/VulkanShader.hpp
    src/Renderer/Vulkan/VulkanShader.cpp
    src/Renderer/Vulkan/VulkanGraphicsPipeline.hpp
    src/Renderer/Vulkan/VulkanGraphicsPipeline.cpp
    src/Renderer/Vulkan/VulkanCommandRecorder.hpp
    src/Renderer/Vulkan/VulkanCommandRecorder.cpp
)

target_include_directories(${PROJECT_NAME}
PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    spdlog::spdlog
    glfw
    volk
)

target_compile_definitions(${PROJECT_NAME}
PRIVATE
    NOMINMAX
    GLFW_INCLUDE_NONE
    $<$<BOOL:${VOLK_STATIC_DEFINES}>:${VOLK_STATIC_DEFINES}>
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        GLFW_EXPOSE_NATIVE_WIN32
    )
endif()

set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

function(compile_shaders_target target_name)
    file(GLOB_RECURSE SHADER_SOURCES
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

    if(NOT SHADER_SOURCES)
        message(WARNING "No shader sources found in ${SHADER_SRC_DIR}")
        return()
    endif()

    add_custom_command(
        OUTPUT ${SHADER_BIN_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BIN_DIR}
        COMMENT "Creating shader output directory: ${SHADER_BIN_DIR}"
    )

    set(SPV_SHADERS "")

    foreach(shader_source ${SHADER_SOURCES})
        get_filename_component(shader_name ${shader_source} NAME)
        set(spv_output ${SHADER_BIN_DIR}/${shader_name}.spv)

        add_custom_command(
            OUTPUT ${spv_output}
            COMMAND ${GLSLC_EXE}
                --target-spv=spv1.4
                -o ${spv_output}
                ${shader_source}
            DEPENDS ${shader_source} ${SHADER_BIN_DIR}
            COMMENT "Compiling shader: ${shader_name}"
            VERBATIM
        )

        list(APPEND SPV_SHADERS ${spv_output})
    endforeach()

    add_custom_target(${target_name}
        DEPENDS ${SPV_SHADERS}
        COMMENT "Building all shaders"
    )

    add_dependencies(${PROJECT_NAME} ${target_name})
    
    target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${SHADER_BIN_DIR}
    )
endfunction()

compile_shaders_target(compile_shaders)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

message(STATUS "Renderer Configuration:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Shader Source Dir: ${SHADER_SRC_DIR}")
message(STATUS "  Shader Binary Dir: ${SHADER_BIN_DIR}")
message(STATUS "  glslc Found: ${GLSLC_EXECUTABLE}")
