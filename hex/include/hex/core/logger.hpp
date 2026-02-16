#pragma once

#include <mana/logger.hpp>

#ifndef HEX_LOG_NAME
#    define HEX_LOG_NAME "Hex"
#endif

#ifndef HEX_LOG_LEVEL
#    if defined(HEX_DEBUG)
#        define HEX_LOG_LEVEL mana::LogLevel::Debug
#    elif defined(HEX_RELEASE)
#        define HEX_LOG_LEVEL mana::LogLevel::Info
#    else
#        define HEX_LOG_LEVEL mana::LogLevel::Off
#    endif
#endif

namespace hex {
extern mana::SpdLogger Log;
}
