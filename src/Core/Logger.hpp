#pragma once

#include <memory>
#include <spdlog/logger.h>

namespace Renderer {

    class Logger
    {
    public:
        static void Init();
        static void Shutdown();

        inline static const std::unique_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

    private:
        inline static std::unique_ptr<spdlog::logger> s_Logger;
    };

}

#ifndef NDEBUG
    #define LOG_TRACE(...) ::Renderer::Logger::GetLogger()->trace(__VA_ARGS__);
    #define LOG_DEBUG(...) ::Renderer::Logger::GetLogger()->debug(__VA_ARGS__);
    #define LOG_INFO(...)  ::Renderer::Logger::GetLogger()->info(__VA_ARGS__);
    #define LOG_WARN(...)  ::Renderer::Logger::GetLogger()->warn(__VA_ARGS__);
    #define LOG_ERROR(...) ::Renderer::Logger::GetLogger()->error(__VA_ARGS__);
    #define LOG_FATAL(...) ::Renderer::Logger::GetLogger()->critical(__VA_ARGS__);
#else
    #define LOG_TRACE(...)
    #define LOG_DEBUG(...)
    #define LOG_INFO(...)
    #define LOG_WARN(...)
    #define LOG_ERROR(...)
    #define LOG_FATAL(...)
#endif
