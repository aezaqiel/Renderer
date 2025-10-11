cmake_minimum_required(VERSION 3.30)

project(Renderer C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(thirdparty/spdlog)

add_executable(${PROJECT_NAME}
    src/Main.cpp

    src/Core/Types.hpp
    src/Core/Logger.hpp
    src/Core/Logger.cpp
    src/Core/Application.hpp
    src/Core/Application.cpp
    src/Core/KeyCodes.hpp
    src/Core/Events.hpp
)

target_include_directories(${PROJECT_NAME}
PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    spdlog
)
