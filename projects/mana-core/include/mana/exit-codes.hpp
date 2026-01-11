#pragma once

namespace mana {
enum class ExitCode {
    Success,
    UnknownCriticalError,

    NoFileProvided,

    LexerError,
    ParserError,

    OutputOpenError,
    OutputWriteError,
};

consteval int Exit(ExitCode exit_code) {
    return static_cast<int>(exit_code);
}

}  // namespace mana
