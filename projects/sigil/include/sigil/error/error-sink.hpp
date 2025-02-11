#pragma once

#include <sigil/error/error.hpp>

#include <span>
#include <vector>

namespace sigil {

class ErrorSink {
    std::vector<Error> errors;

    void PrintError(Error& error);
    void AnalyzeNode(Error& error);

    SIGIL_NODISCARD std::span<const Error> PeekTop() const;

    void PrintTop();
    void PrintAll();

public:
    void Output();
    void Report(const Error& error);
    void Report(const ParseNode& problem, ml::i64 token_offset, ErrorCode error_code);
};

}  // namespace sigil
