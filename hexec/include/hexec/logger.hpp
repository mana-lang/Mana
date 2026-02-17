#pragma once

#include <mana/logger.hpp>

#ifndef HEXEC_LOG_NAME
#    define HEXEC_LOG_NAME "Hexe"
#endif

#ifndef HEXEC_LOG_LEVEL
#    if defined(MANA_DEBUG)
#        define HEXEC_LOG_LEVEL mana::LogLevel::Debug
#    elif defined(MANA_RELEASE)
#        define HEXEC_LOG_LEVEL mana::LogLevel::Debug
#    else
#        define HEXEC_LOG_LEVEL mana::LogLevel::Off
#    endif
#endif

namespace hexec {
extern mana::SpdLogger Log;
}
