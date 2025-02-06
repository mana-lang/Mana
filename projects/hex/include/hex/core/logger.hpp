#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <mana/exit-codes.hpp>
#include <mana/literals.hpp>

#include <memory>

namespace hex {
using namespace mana::literals;
enum class LogLevel : u8 {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,  // Terminates execution
    Off
};

#ifndef HEX_LOGGER_NAME
#    define HEX_LOGGER_NAME "Hex"
#endif

#ifndef HEX_LOG_LEVEL
#    if defined(HEX_VERBOSE)
#        define HEX_LOG_LEVEL LogLevel::Trace
#    elif defined(HEX_DEBUG)
#        define HEX_LOG_LEVEL LogLevel::Debug
#    elif defined(HEX_RELEASE)
#        define HEX_LOG_LEVEL LogLevel::Info
#    else
#        define HEX_LOG_LEVEL LogLevel::Off
#    endif
#endif

template <typename... Args>
void Log(LogLevel level, const char* msg, Args&&... args);

i64 LogCounter(LogLevel level);

class Internal_Log_ {
    using Logger = std::shared_ptr<spdlog::logger>;

    template <typename... Args>
    friend void Log(LogLevel level, const char* msg, Args&&... args);
    friend i64  LogCounter(LogLevel level);

    Internal_Log_() = default;

    static void init() {
        if (!was_initialized) {
            was_initialized = true;

            logger->set_level(static_cast<spdlog::level::level_enum>(HEX_LOG_LEVEL));
            logger->set_pattern("%^<%n>%$ %v");

            counters = {};
        }
    }

    inline static bool        was_initialized {false};
    inline static std::string logger_name {HEX_LOGGER_NAME};
    inline static Logger      logger {spdlog::stdout_color_mt(logger_name)};

    inline static struct {
        i64 trace;
        i64 debug;
        i64 info;
        i64 warnings;
        i64 errors;
    } counters;
};

template <typename... Args>
void Log(const LogLevel level, const char* msg, Args&&... args) {
    if (!Internal_Log_::was_initialized) {
        Internal_Log_::init();
    }

    const auto runtime_msg = fmt::runtime(msg);

    switch (level) {
        using enum LogLevel;

    case Trace:
        Internal_Log_::logger->trace(runtime_msg, std::forward<Args>(args)...);
        ++Internal_Log_::counters.trace;
        break;
    case Debug:
        Internal_Log_::logger->debug(runtime_msg, std::forward<Args>(args)...);
        ++Internal_Log_::counters.debug;
        break;
    case Info:
        Internal_Log_::logger->info(runtime_msg, std::forward<Args>(args)...);
        ++Internal_Log_::counters.info;
        break;
    case Warn:
        Internal_Log_::logger->warn(runtime_msg, std::forward<Args>(args)...);
        ++Internal_Log_::counters.warnings;
        break;
    case Error:
        Internal_Log_::logger->error(runtime_msg, std::forward<Args>(args)...);
        ++Internal_Log_::counters.errors;
        break;
    case Critical:
        Internal_Log_::logger->critical(runtime_msg, std::forward<Args>(args)...);
        Internal_Log_::logger->critical("Shutting down.");
        throw std::runtime_error("Critical error");  // not a fan of this
    case Off:
        break;
    }
}

template <typename... Args>
void Log(const char* msg, Args&&... args) {
    Log(LogLevel::Info, msg, std::forward<Args>(args)...);
}

template <typename... Args>
void LogErr(const char* msg, Args&&... args) {
    Log(LogLevel::Error, msg, std::forward<Args>(args)...);
}

inline i64 LogCounter(const LogLevel level) {
    const auto& [trace, debug, info, warnings, errors] = Internal_Log_::counters;

    switch (level) {
        using enum LogLevel;

    case Trace:
        return trace;
    case Debug:
        return debug;
    case Info:
        return info;
    case Warn:
        return warnings;
    case Error:
        return errors;
    case Critical:
        Log(Error, "Critical errors always throw, so this path should never happen.");
        return exit_code(mana::Exit::LOG_LogCounterIllegalPath);
    case Off:
        return 0;
    }

    return exit_code(mana::Exit::LOG_LogCounterExhausted);
}
}  // namespace hex
