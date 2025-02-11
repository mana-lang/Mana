#include <sigil/error/error-sink.hpp>

namespace sigil {
void ErrorSink::PrintError(Error& error) {}

void ErrorSink::AnalyzeNode(Error& error) {}

std::span<const Error> ErrorSink::PeekTop() const {
    return errors.empty() ? ({}) : std::span(&errors.back(), 1);
}

void ErrorSink::PrintTop() {
    if (not errors.empty()) {
        PrintError(errors.back());
    }
}

void ErrorSink::PrintAll() {}

void ErrorSink::Output() {}

void ErrorSink::Report(const Error& error) {
    errors.emplace_back(error);
}

void ErrorSink::Report(const ParseNode& problem, const ml::i64 offset, const ErrorCode error_code) {
    errors.emplace_back(Error(&problem, error_code, offset));
}

}  // namespace sigil