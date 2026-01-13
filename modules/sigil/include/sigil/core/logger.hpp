#pragma once

#include <mana/logger.hpp>

#ifndef SIGIL_LOG_NAME
#    define SIGIL_LOG_NAME "Sigil"
#endif

#ifndef SIGIL_LOG_LEVEL
#    if defined(SIGIL_DEBUG)
#        define SIGIL_LOG_LEVEL mana::LogLevel::Debug
#    elif defined(SIGIL_RELEASE)
#        define SIGIL_LOG_LEVEL mana::LogLevel::Info
#    else
#        define SIGIL_LOG_LEVEL mana::LogLevel::Off
#    endif
#endif

namespace sigil {
extern mana::SpdLogger Log;
extern mana::SpdLogger FileLog;
}
