#pragma once

namespace salem {

enum class Exit {
    Success,
    CriticalError,
    Log_LogCounterExhausted,
    Log_LogCounterIllegalPath,
    CLI_HelpArgUsed,
    CLI_MissingSrcFile,
    Lexer_TokenizationFailed,
};

consteval int ExitCode(Exit exit_code) {
    return static_cast<int>(exit_code);
}

}
