#include <hex/ast/parser.hpp>
#include <hex/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

#include <algorithm>

namespace hex {
using namespace ast;

Parser::Parser(const TokenStream&& tokens)
    : tokens_ {tokens}
    , cursor_ {} {}

Parser::Parser(const TokenStream& tokens)
    : tokens_ {tokens}
    , cursor_ {} {}

bool Parser::Parse() {
    const auto& top_token = tokens_.front();

    if (top_token.type != TokenType::_module_) {
        LogErr(
            "Improper token stream format. Top-level token was: '{}' instead of "
            "'_module_'",
            magic_enum::enum_name(top_token.type)
        );
        return false;
    }

    ast_.rule = Rule::Module;
    ast_.tokens.push_back(tokens_.front());

    cursor_ = 1;
    while (ProgressedAST(ast_)) {}

    if (CurrentToken().type != TokenType::Eof) {
        LogErr("It appears we did not parse the entire file.");
        return true; // TODO: handle this error and return false instead
    }
    return true;
}

auto Parser::ViewAST() const -> const Node& {
    return ast_;
}

auto Parser::ViewTokens() const -> const TokenStream& {
    return tokens_;
}

void Parser::PrintAST() const {
    Log("Printing AST for module '{}'", ast_.tokens[0].text);
    PrintAST(ast_);
}

void Parser::PrintAST(const Node& root, std::string prepend) const {
    Log("{}[{}]", prepend, magic_enum::enum_name(root.rule));

    prepend.append("== ");

    if (not root.branches.empty()) {
        for (const auto& node : root.branches) {
            PrintAST(*node, prepend);
        }
    }

    std::ranges::replace(prepend, '=', '-');

    for (const auto& [type, text, position] : root.tokens) {
        if (type == TokenType::Terminator) {
            continue;
        }
        Log("{} [{}] -> {}", prepend, magic_enum::enum_name(type), text);
    }

    if (root.tokens.size() > 1) {
        Log("");
    }
}

bool Parser::IsPrimitive(TokenType token_type) const {
    switch (token_type) {
        using enum TokenType;

    case KW_i32:
    case KW_i64:

    case KW_u32:
    case KW_u64:

    case KW_f32:
    case KW_f64:

    case KW_char:
    case KW_string:
    case KW_byte:
        return true;

    default:
        return false;
    }
}

auto Parser::CurrentToken() const -> const Token& {
    return tokens_[cursor_];
}

auto Parser::PeekNextToken() const -> const Token& {
    return tokens_[cursor_ + 1];
}

auto Parser::NextToken() -> const Token& {
    return tokens_[++cursor_];
}

auto Parser::CycleToken() -> const Token& {
    return tokens_[cursor_++];
}

// this function makes no fucking sense
// void Parser::AddTokensTo(Node& node, TokenType delimiter) {
//     while (CurrentToken().type != delimiter) {
//         if (not ProgressedAST(node)) {
//             return;
//         }
//     }
// }

// inclusive
void Parser::AddTokensTo(Node& node, const TokenType delimiter) {
    do {
        node.tokens.push_back(CycleToken());
    } while (CurrentToken().type != delimiter);
}

void Parser::AddTokensTo(Node& node, const i64 count) {
    for (i64 i = 0; i < count; ++i) {
        node.tokens.push_back(CycleToken());
    }
}

void Parser::AddCurrentTokenTo(Node& node) const {
    if (cursor_ < tokens_.size()) {
        node.tokens.push_back(CurrentToken());
    }
}

// TODO: Make transmit functions take reference to token_stream instead
// also: make these make sense
void Parser::TransmitTokens(Node& sender, Node& receiver) const {
    auto receive = receiver.tokens;  // why are we receiving to a temporary??
    for (const auto& send : sender.tokens) {
        receive.push_back(send);
    }

    sender.tokens.clear();
}

void Parser::TransmitTokens(Node& sender, Node& receiver, TokenRange range) const {
    const auto [breadth, offset] = range;

    if (breadth + offset > sender.tokens.size()) {
        LogErr("TransmitTokens error: range was too large");
        return;
    }

    for (i64 i = 0; i < breadth; ++i) {
        receiver.tokens.emplace_back(sender.tokens[offset]);
    }
}

bool Parser::ProgressedAST(Node& node) {
    // don't process eof (final token) before quitting
    if (cursor_ + 1 >= tokens_.size() - 1) {
        return false;
    }

    if (CurrentToken().type == TokenType::Terminator) {
        ++cursor_;
        return true;
    }

    const bool matched_something = Matched_Expression(node);

    ++cursor_;
    return matched_something;
}

// expressions can recurse, but are always non-terminal
bool Parser::Matched_Expression(Node& node) {
    auto& expr = node.NewBranch();
    expr.rule  = Rule::Expression;

    auto& next_node = expr.NewBranch();
    return Matched_Unary(next_node) || Matched_Literal(next_node);
}

// literal = number | string | KW_true | KW_false | KW_null
bool Parser::Matched_Literal(Node& node) {
    switch (CurrentToken().type) {
        using enum TokenType;

    case Lit_Int:
    case Lit_Float:
    case Lit_Char:
    case Lit_String:
    case Lit_true:
    case Lit_false:
    case Lit_null:
        node.rule = Rule::Literal;
        AddCurrentTokenTo(node);
        return true;
    default:
        return false;
    }
}

// unary = ("-" | "!") expr
bool Parser::Matched_Unary(Node& node) {
    using enum TokenType;
    switch (CurrentToken().type) {
    case Op_Minus:
    case Op_LogicalNot:
        switch (PeekNextToken().type) {
        case Lit_Int:
        case Lit_Float:
        case Lit_true:
        case Lit_false:
            node.rule = Rule::Unary;
            AddTokensTo(node, 2);
            return true;

        default:
            return false;
        }

        // TODO: care that this path may be indicative of an error
        // that's for future me to resolve
        default:
            return false;
    }
}

}  // namespace hex