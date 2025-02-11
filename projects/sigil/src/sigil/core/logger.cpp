#include <sigil/core/logger.hpp>

namespace sigil {
mana::SpdLogger Log = mana::GlobalLoggerSink.CreateLogger(SIGIL_LOG_NAME, SIGIL_LOG_LEVEL);
}