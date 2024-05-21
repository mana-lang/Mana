#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <Salem/Core/TypeAliases.hpp>
#include <Salem/Core/ExitCodes.hpp>

#include <memory>

namespace salem {

enum class LogLevel : u8 {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,  // Terminates execution
    Off
};

#ifndef SALEM_LOG_LEVEL
#    if defined(SALEM_VERBOSE)
#        define SALEM_LOG_LEVEL LogLevel::Trace
#    elif defined(SALEM_DEBUG)
#        define SALEM_LOG_LEVEL LogLevel::Debug
#    elif defined(SALEM_RELEASE)
#        define SALEM_LOG_LEVEL LogLevel::Info
#    else
#        define SALEM_LOG_LEVEL LogLevel::Off
#    endif
#endif

template<typename... Args>
void log(LogLevel log_level, const char* msg, Args&&... args);

i64 log_counter(LogLevel log_level);

class Internal_Log_ {
    using SpdLogger = std::shared_ptr<spdlog::logger>;

    template<typename... Args>
    friend void log(LogLevel log_level, const char* msg, Args&&... args);
    friend i64 log_counter(LogLevel log_level);

    Internal_Log_() = default;

    static void init() {
        if (!was_initialized_) {
            was_initialized_ = true;

            logger_->set_pattern("%^%v%$");
            logger_->set_level(static_cast<spdlog::level::level_enum>(SALEM_LOG_LEVEL));
            //logger->log(static_cast<spdlog::level::level_enum>(LogLevel::Info), "--- Salem v{}\n", SALEM_VERSION_STR);

            logger_->set_pattern("%^<%n>%$ %v");

            counters_ = {};
        }
    }

    inline static bool      was_initialized_ {false};
    inline static SpdLogger logger_ {spdlog::stdout_color_st("Salem")};

    inline static struct Counters {
        i64 trace;
        i64 debug;
        i64 info;
        i64 warnings;
        i64 errors;
    } counters_;
};

template<typename... Args>
void log(const LogLevel log_level, const char* msg, Args&&... args) {
    if (!Internal_Log_::was_initialized_) {
        Internal_Log_::init();
    }

    const auto runtime_msg = fmt::runtime(msg);

    switch (log_level) {
        using enum LogLevel;
        
    case Trace:
        Internal_Log_::logger_->trace(runtime_msg, std::forward<Args>(args)...);
        ++Internal_Log_::counters_.trace;
        break;
    case Debug:
        Internal_Log_::logger_->debug(runtime_msg, std::forward<Args>(args)...);
        ++Internal_Log_::counters_.debug;
        break;
    case Info:
        Internal_Log_::logger_->info(runtime_msg, std::forward<Args>(args)...);
        ++Internal_Log_::counters_.info;
        break;
    case Warn:
        Internal_Log_::logger_->warn(runtime_msg, std::forward<Args>(args)...);
        ++Internal_Log_::counters_.warnings;
        break;
    case Error:
        Internal_Log_::logger_->error(runtime_msg, std::forward<Args>(args)...);
        ++Internal_Log_::counters_.errors;
        break;
    case Critical:
        Internal_Log_::logger_->critical(runtime_msg, std::forward<Args>(args)...);
        Internal_Log_::logger_->critical("Shutting down.");
        throw std::runtime_error("Critical error");
    case Off:
        break;
    // default:
    //     Internal_Log_::logger_->error("You really shouldn't have come here.");
    }
}

template<typename... Args>
void log(const char* msg, Args&&... args) {
    log(LogLevel::Info, msg, std::forward<Args>(args)...);
}

inline i64 log_counter(const LogLevel log_level) {
    const auto& [trace, debug, info, warnings, errors] = Internal_Log_::counters_;

    switch (log_level) {
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
        log(Error, "Critical errors always throw, so this path should never happen.");
        return EXIT::LOG_COUNTER_CRITICAL_CASE;
    case Off:
        return 0;
    }

    return EXIT::LOG_COUNTER_PAST_SWITCH;
}

}  // namespace salem