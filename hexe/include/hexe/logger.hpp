#pragma once

#include <mana/logger.hpp>

#ifndef HEXE_LOG_NAME
#    define HEXE_LOG_NAME "Hexe"
#endif

#ifndef HEXE_LOG_LEVEL
#    if defined(HEXE_DEBUG)
#        define HEXE_LOG_LEVEL mana::LogLevel::Debug
#    elif defined(HEXE_RELEASE)
#        define HEXE_LOG_LEVEL mana::LogLevel::Debug
#    else
#        define HEXE_LOG_LEVEL mana::LogLevel::Off
#    endif
#endif

namespace hexe {
extern mana::SpdLogger Log;
}
