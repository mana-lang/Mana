#include <circe/core/logger.hpp>
#include <circe/bytecode-generator.hpp>
#include <circe/core/cli.hpp>

#include <sigil/ast/lexer.hpp>
#include <sigil/ast/parser.hpp>
#include <sigil/ast/syntax-tree.hpp>
#include <sigil/ast/semantic-analyzer.hpp>

#include <hexec/bytecode.hpp>

#include <mana/exit-codes.hpp>


#include <filesystem>
#include <fstream>

using namespace mana::literals;
using namespace circe;


struct ScopedTimer {
    using Clock     = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::microseconds;

    TimePoint& target;
    Clock::time_point start;

    explicit ScopedTimer(std::chrono::microseconds& output)
        : target(output),
          start(Clock::now()) {}

    ~ScopedTimer() {
        target = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - start);
    }
};

int CompileFrom(const CompileSettings& compile_settings) {
    namespace chrono = std::chrono;
    using namespace mana;

    const auto& in_path = compile_settings.InputFilePath();
    auto out_path       = compile_settings.OutputPath(); // we might have to create our own outpath

    if (not std::filesystem::exists(in_path)) {
        Log->error("Input file '{}' does not exist", in_path.string());
        return Exit(ExitCode::FileNotFound);
    }

    std::chrono::microseconds time_lex, time_parse, time_analysis, time_codegen, time_write, time_total;
    sigil::Lexer lexer;
    sigil::Parser parser;
    sigil::SemanticAnalyzer analyzer;
    BytecodeGenerator codegen;
    u64 output_size;

    {
        ScopedTimer total_timer(time_total);
        {
            ScopedTimer lexer_timer(time_lex);
            if (not lexer.Tokenize(in_path)) {
                Log->error("Failed to tokenize file '{}'", in_path.string());
                return Exit(ExitCode::LexerError);
            }
        }

        {
            ScopedTimer parser_timer(time_parse);
            parser.AcquireTokens(lexer.Tokens());
            if (not parser.Parse()) {
                Log->error("Failed to parse file '{}'", in_path.string());
                return Exit(ExitCode::ParserError);
            }
        }

        if (const auto issues = parser.IssueCount();
            issues > 0) {
            Log->error("Compilation failed with {} issue{}",
                       issues,
                       issues > 1 ? "s" : ""
            );
            return Exit(ExitCode::SyntaxError);
        }

        {
            ScopedTimer analysis_timer(time_analysis);
            parser.AST()->Accept(analyzer);
        }

        if (const auto issues = analyzer.IssueCount();
            issues > 0) {
            Log->critical("Aborting");
            Log->error("Compilation failed with {} issue{}",
                       issues,
                       issues > 1 ? "s" : ""
            );

            parser.PrintParseTree();
            return Exit(ExitCode::SemanticError);
        }

        {
            ScopedTimer codegen_timer(time_codegen);
            codegen.ObtainSemanticAnalysisInfo(analyzer);
            parser.AST()->Accept(codegen);
        }

        {
            ScopedTimer write_timer(time_write);
            // If no output path is provided, use the input filename with .hexec extension
            if (out_path.empty()) {
                out_path = in_path;
                out_path.replace_extension("hexec");
            } else if (std::filesystem::is_directory(out_path) || not out_path.has_filename()) {
                out_path /= in_path.filename().replace_extension(".hexec");
            }

            if (out_path.has_parent_path()) {
                std::filesystem::create_directories(out_path.parent_path());
            }

            std::ofstream out_file(out_path, std::ios::binary);
            if (not out_file) {
                Log->error("Failed to open output file '{}'", out_path.string());
                return Exit(ExitCode::OutputOpenError);
            }
            const auto output = codegen.Bytecode().Serialize();
            out_file.write(reinterpret_cast<const char*>(output.data()), static_cast<std::streamsize>(output.size()));

            if (not out_file) {
                Log->error("Failed to write to output file '{}'", out_path.string());
                return Exit(ExitCode::OutputWriteError);
            }

            output_size = output.size();
        }
    }

    const auto compile_str = fmt::format("Compiled '{}' => '{}'",
                                         in_path.filename().string(),
                                         out_path.filename().string()
    );

    Log->info(compile_str);
    Log->info("Operation completed in {}us", time_total.count());
    Log->info("Output written to '{}'", out_path.string());

    const auto divider = std::string(compile_str.size(), '-');

    if (compile_settings.EmitVerbose()) {
        const auto& bytecode = codegen.Bytecode();
        Log->info(divider);
        Log->info("  Tokens:         {}", lexer.TokenCount());
        Log->info("  Instructions:   {} bytes", bytecode.Instructions().size());
        Log->info("  Constant Pool:  {} constants ({} bytes)",
                  bytecode.ConstantCount(),
                  bytecode.ConstantPoolBytesCount()
        );
        Log->info("  Executable:     {} bytes", output_size);
        Log->info("");
        Log->info("  == Lex:     {}us", time_lex.count());
        Log->info("  == Parse:   {}us", time_parse.count());
        Log->info("  == Analyze: {}us", time_analysis.count());
        Log->info("  == Codegen: {}us", time_codegen.count());
        Log->info("  == Write:   {}us", time_write.count());
        Log->info("");
        Log->info("  ---- Total: {}us", time_total.count());
    }

    if (compile_settings.EmitParseTree()) {
        Log->info("{}\n", divider);
        parser.PrintParseTree();
    }

    if (compile_settings.EmitTokens()) {
        Log->info("{}\n", divider);
        sigil::PrintTokens(parser.ViewTokenStream());
    }

    return Exit(ExitCode::Success);
}

int main(int argc, char** argv) {
    const auto settings = ParseCommandLineCompileSettings(argc, argv);

    if (settings.ErrorCode() != 0 || settings.ShouldExit()) {
        return settings.ErrorCode();
    }

    return CompileFrom(settings);
}
