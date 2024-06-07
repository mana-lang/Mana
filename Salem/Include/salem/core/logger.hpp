#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <salem/core/type_aliases.hpp>
#include <salem/core/exit_codes.hpp>

#include <memory>

namespace salem {
enum class log_level : u8 {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical, // Terminates execution
    Off
};

#ifndef SALEM_LOG_LEVEL
#    if defined(SALEM_VERBOSE)
#        define SALEM_LOG_LEVEL log_level::Trace
#    elif defined(SALEM_DEBUG)
#        define SALEM_LOG_LEVEL log_level::Debug
#    elif defined(SALEM_RELEASE)
#        define SALEM_LOG_LEVEL log_level::Info
#    else
#        define SALEM_LOG_LEVEL log_level::Off
#    endif
#endif

template <typename... Args>
void log(log_level level, const char* msg, Args&&... args);

i64 log_counter(log_level level);

class internal_log_ {
    using logger = std::shared_ptr<spdlog::logger>;

    template <typename... Args>
    friend void log(log_level level, const char* msg, Args&&... args);
    friend i64  log_counter(log_level level);

    internal_log_() = default;

    static void init() {
        if (!was_initialized_) {
            was_initialized_ = true;

            logger_->set_pattern("%^%v%$");
            logger_->set_level(
                               static_cast<spdlog::level::level_enum>(
                                   SALEM_LOG_LEVEL)
                              );
            //logger->log(static_cast<spdlog::level::level_enum>(LogLevel::Info), "--- Salem v{}\n", SALEM_VERSION_STR);

            logger_->set_pattern("%^<%n>%$ %v");

            counters_ = {};
        }
    }

    inline static bool   was_initialized_{false};
    inline static logger logger_{spdlog::stdout_color_st("Salem")};

    inline static struct counters {
        i64 trace;
        i64 debug;
        i64 info;
        i64 warnings;
        i64 errors;
    } counters_;
};

template <typename... Args>
void log(const log_level level, const char* msg, Args&&... args) {
    if (!internal_log_::was_initialized_) {
        internal_log_::init();
    }

    const auto runtime_msg = fmt::runtime(msg);

    switch (level) {
        using enum log_level;

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
        internal_log_::logger_->critical(
                                         runtime_msg,
                                         std::forward<Args>(args)...
                                        );
        internal_log_::logger_->critical("Shutting down.");
        throw std::runtime_error("Critical error");
    case Off:
        break;
    // default:
    //     internal_log_::logger_->error("You really shouldn't have come here.");
    }
}

template <typename... Args>
void log(const char* msg, Args&&... args) {
    log(log_level::Info, msg, std::forward<Args>(args)...);
}

inline i64 log_counter(const log_level level) {
    const auto& [trace, debug, info, warnings, errors] =
        internal_log_::counters_;

    switch (level) {
        using enum log_level;

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
        log(
            Error,
            "Critical errors always throw, so this path should never happen."
           );
        return exit_code(exit::LOG_LogCounterIllegalPath);
    case Off:
        return 0;
    }

    return exit_code(exit::LOG_LogCounterExhausted);
}
} // namespace salem
