#include <mana/logger.hpp>

namespace mana {
LoggerSink GlobalLoggerSink = {};

LoggerSink::LoggerSink() {
    LogSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
}

auto LoggerSink::CreateLogger(const std::string_view name, LogLevel default_level) -> SpdLogger {
    auto ret = std::make_shared<spdlog::logger>(std::string(name), LogSink);
    initialize_logger(ret);

    ret->set_level(static_cast<spdlog::level::level_enum>(default_level));
    ret->set_pattern("%^<%n>%$ %v");

    return ret;
}
}  // namespace mana
