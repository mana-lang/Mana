#pragma once

namespace salem {
enum class exit {
    Success,
    CriticalError,
    LOG_LogCounterExhausted,
    LOG_LogCounterIllegalPath,
    CLI_HelpArgUsed,
    CLI_MissingSrcFile,
    LEX_TokenizationFailed,
};

consteval int exit_code(exit exit_code) {
    return static_cast<int>(exit_code);
}
} // namespace salem
