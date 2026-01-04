#pragma once

#include "literals.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>

#include "../../../../.build/release/_deps/spdlog-src/include/spdlog/sinks/basic_file_sink.h"

namespace mana {
enum class LogLevel : literals::u8 {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical, // Terminates execution
    Off
};

using SpdLogger = std::shared_ptr<spdlog::logger>;

class LoggerSink {
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> LogSink;

public:
    LoggerSink();

    auto CreateLogger(std::string_view name, LogLevel default_level = LogLevel::Debug) -> SpdLogger;
    void AppendFileLogger(std::string_view file_name, const SpdLogger& logger) const;

    std::string DefaultPattern;
};

inline LoggerSink& GlobalLoggerSink() {
    static LoggerSink logger_sink;
    return logger_sink;
}

} // namespace mana
