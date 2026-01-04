#include <mana/logger.hpp>

namespace mana {
LoggerSink::LoggerSink() {
    LogSink        = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    DefaultPattern = "%^<%n>%$ %v";
}

auto LoggerSink::CreateLogger(const std::string_view name, LogLevel default_level) -> SpdLogger {
    auto ret = std::make_shared<spdlog::logger>(std::string(name), LogSink);
    spdlog::initialize_logger(ret);

    ret->set_level(static_cast<spdlog::level::level_enum>(default_level));
    ret->set_pattern(DefaultPattern);

    return ret;
}

void LoggerSink::AppendFileLogger(const std::string_view file_name, const SpdLogger& logger) const {
    logger->sinks().push_back(
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string(file_name), true)
    );

    logger->set_pattern(DefaultPattern);
}
} // namespace mana
