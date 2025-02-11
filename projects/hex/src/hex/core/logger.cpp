#include <hex/core/logger.hpp>

namespace hex {
mana::SpdLogger Log = mana::GlobalLoggerSink.CreateLogger(HEX_LOG_NAME, HEX_LOG_LEVEL);
}