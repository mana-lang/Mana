#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <hex/core/exit_codes.hpp>
#include <hex/core/type_aliases.hpp>

#include <memory>

namespace hex {
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
void log(LogLevel level, const char* msg, Args&&... args);

i64 log_counter(LogLevel level);

class internal_log_ {
    using logger = std::shared_ptr<spdlog::logger>;

    template <typename... Args>
    friend void log(LogLevel level, const char* msg, Args&&... args);
    friend i64  log_counter(LogLevel level);

    internal_log_() = default;

    static void init() {
        if (!was_initialized_) {
            was_initialized_ = true;

            logger_->set_level(static_cast<spdlog::level::level_enum>(HEX_LOG_LEVEL));
            logger_->set_pattern("%^<%n>%$ %v");

            counters_ = {};
        }
    }

    inline static bool        was_initialized_ {false};
    inline static std::string logger_name_ {HEX_LOGGER_NAME};
    inline static logger      logger_ {spdlog::stdout_color_mt(logger_name_)};

    inline static struct counters {
        i64 trace;
        i64 debug;
        i64 info;
        i64 warnings;
        i64 errors;
    } counters_;
};

template <typename... Args>
void log(const LogLevel level, const char* msg, Args&&... args) {
    if (!internal_log_::was_initialized_) {
        internal_log_::init();
    }

    const auto runtime_msg = fmt::runtime(msg);

    switch (level) {
        using enum LogLevel;

    case Trace:
        internal_log_::logger_->trace(runtime_msg, std::forward<Args>(args)...);
        ++internal_log_::counters_.trace;
        break;
    case Debug:
        internal_log_::logger_->debug(runtime_msg, std::forward<Args>(args)...);
        ++internal_log_::counters_.debug;
        break;
    case Info:
        internal_log_::logger_->info(runtime_msg, std::forward<Args>(args)...);
        ++internal_log_::counters_.info;
        break;
    case Warn:
        internal_log_::logger_->warn(runtime_msg, std::forward<Args>(args)...);
        ++internal_log_::counters_.warnings;
        break;
    case Error:
        internal_log_::logger_->error(runtime_msg, std::forward<Args>(args)...);
        ++internal_log_::counters_.errors;
        break;
    case Critical:
        internal_log_::logger_->critical(runtime_msg, std::forward<Args>(args)...);
        internal_log_::logger_->critical("Shutting down.");
        throw std::runtime_error("Critical error");
    case Off:
        break;
    }
}

template <typename... Args>
void log(const char* msg, Args&&... args) {
    log(LogLevel::Info, msg, std::forward<Args>(args)...);
}

inline i64 log_counter(const LogLevel level) {
    const auto& [trace, debug, info, warnings, errors] = internal_log_::counters_;

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
        log(Error, "Critical errors always throw, so this path should never happen.");
        return exit_code(exit::LOG_LogCounterIllegalPath);
    case Off:
        return 0;
    }

    return exit_code(exit::LOG_LogCounterExhausted);
}
}  // namespace hex
