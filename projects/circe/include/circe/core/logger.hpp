#pragma once

#include <mana/logger.hpp>

#ifndef CIRCE_LOG_NAME
#    define CIRCE_LOG_NAME "Circe"
#endif

#ifndef CIRCE_LOG_LEVEL
#    if defined(CIRCE_DEBUG)
#        define CIRCE_LOG_LEVEL mana::LogLevel::Debug
#    elif defined(CIRCE_RELEASE)
#        define CIRCE_LOG_LEVEL mana::LogLevel::Info
#    else
#        define CIRCE_LOG_LEVEL mana::LogLevel::Off
#    endif
#endif

namespace circe {
// extern mana::SpdLogger Log;

inline mana::SpdLogger& Log() {
    static mana::SpdLogger Log = mana::GlobalLoggerSink().CreateLogger(CIRCE_LOG_NAME, CIRCE_LOG_LEVEL);
    return Log;
}
}
