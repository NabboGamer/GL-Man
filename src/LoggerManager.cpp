#pragma once

#include "LoggerManager.hpp"
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Static logger definition
std::shared_ptr<spdlog::logger> LoggerManager::s_Logger = nullptr;

void LoggerManager::Init() {
    spdlog::init_thread_pool(8192, 1); // Pool for asynchronous logging

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%d-%m-%Y %H:%M:%S.%e] [%^%l%$] %v");

    // Create an asynchronous logger and set it as the default logger
    s_Logger = std::make_shared<spdlog::async_logger>(
        "async_logger",
        console_sink,
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block
    );

    spdlog::set_default_logger(s_Logger);

    // Set log level based on build mode
#ifdef _DEBUG
    s_Logger->set_level(spdlog::level::debug);
    spdlog::info("Logger initialized in Debug mode with DEBUG level");
#else
    s_Logger->set_level(spdlog::level::info);
    spdlog::info("Logger initialized in Release mode with INFO level");
#endif
}

void LoggerManager::Shutdown() {
    spdlog::shutdown();
}