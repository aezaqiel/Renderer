#include "Logger.hpp"

#include <string>
#include <filesystem>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Renderer {

    void Logger::Init()
    {
        std::string logDir = "logs";
        if (!std::filesystem::exists(logDir))
            std::filesystem::create_directory(logDir);

        std::vector<spdlog::sink_ptr> sinks = {
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
            std::make_shared<spdlog::sinks::basic_file_sink_mt>(logDir + "/Renderer.log", true)
        };

        for (auto& sink : sinks)
            sink->set_pattern("[%H:%M:%S %z] [%n] [%^%l%$] [thread %t] %v");

        s_Logger = std::make_unique<spdlog::logger>("RENDERER", sinks.begin(), sinks.end());
        s_Logger->set_level(spdlog::level::trace);
    }

    void Logger::Shutdown()
    {
        s_Logger.reset();
        spdlog::drop_all();
    }

}
