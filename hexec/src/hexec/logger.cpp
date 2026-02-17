#include <hexec/logger.hpp>

namespace hexec {
mana::SpdLogger Log = mana::GlobalLoggerSink().CreateLogger(HEXEC_LOG_NAME, HEXEC_LOG_LEVEL);
}
