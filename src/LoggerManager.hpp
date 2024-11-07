#pragma once

#include <memory>
#include <string>
#include <spdlog/spdlog.h>

// Logging levels in ascending order of importance
// 
// TRACE
// DEBUG
// INFO
// WARN
// ERROR
// FATAL(CRITICAL in spdlog)

class LoggerManager {

public:
    static void Init();
    static void Shutdown();

    // Log methods with variadic template and fmt::format_string for formatting
    template<typename... Args>
    static void LogTrace(fmt::format_string<Args...> format, Args&&... args) {
        spdlog::trace(format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void LogDebug(fmt::format_string<Args...> format, Args&&... args) {
        spdlog::debug(format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void LogInfo(fmt::format_string<Args...> format, Args&&... args) {
        spdlog::info(format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void LogWarn(fmt::format_string<Args...> format, Args&&... args) {
        spdlog::warn(format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void LogError(fmt::format_string<Args...> format, Args&&... args) {
        spdlog::error(format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void LogFatal(fmt::format_string<Args...> format, Args&&... args) {
        spdlog::critical(format, std::forward<Args>(args)...);
    }

private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};

