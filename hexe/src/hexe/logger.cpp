#include <hexe/logger.hpp>

namespace hexe {
mana::SpdLogger Log = mana::GlobalLoggerSink().CreateLogger(HEXE_LOG_NAME, HEXE_LOG_LEVEL);
}
