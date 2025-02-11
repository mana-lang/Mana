#pragma once

#include "literals.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>

namespace mana {
enum class LogLevel : literals::u8 {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,  // Terminates execution
    Off
};

using SpdLogger = std::shared_ptr<spdlog::logger>;

class LoggerSink {
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> LogSink;

public:
    LoggerSink();

    auto CreateLogger(std::string_view name, LogLevel default_level = LogLevel::Debug) -> SpdLogger;
};

extern LoggerSink GlobalLoggerSink;

}  // namespace mana
