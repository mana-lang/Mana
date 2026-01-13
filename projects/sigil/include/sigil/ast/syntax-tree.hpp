#pragma once

#include <sigil/ast/parse-tree.hpp>
#include <sigil/ast/visitor.hpp>
#include <sigil/core/concepts.hpp>

#include <mana/vm/primitive-type.hpp>
#include <mana/literals.hpp>

#include <memory>
#include <string>
#include <vector>
#include <charconv>
#include <unordered_map>

#include <magic_enum/magic_enum.hpp>

#include <sigil/core/logger.hpp>

namespace sigil::ast {
namespace ml = mana::literals;


class Visitor;

using NodePtr = std::shared_ptr<class Node>;

// As the ptree gets constructed before the AST,
// AST nodes assume their ptree input is correct
class Node {
public:
    virtual ~Node() = default;

    virtual void Accept(Visitor& visitor) const = 0;
};

class Statement final : public Node {
    NodePtr child;

public:
    explicit Statement(NodePtr&& node);

    SIGIL_NODISCARD const NodePtr& GetChild() const;

    void Accept(Visitor& visitor) const override;
};

template <typename T>
concept NodeType = std::is_base_of_v<Node, T>;

class StatementContainer {
protected:
    std::vector<NodePtr> statements;

public:
    template <NodeType NodeT, typename... Args>
    void AddStatement(Args&&... args) {
        statements.emplace_back(std::make_shared<Statement>(
                std::make_shared<NodeT>(std::forward<Args>(args)...)
            )
        );
    }

    void AddStatement(NodePtr&& node) {
        statements.emplace_back(std::make_shared<Statement>(std::move(node)));
    }
};

class Artifact final : public Node, public StatementContainer {
    std::string name;

public:
    explicit Artifact(const std::string_view name)
        : name(name) {}

    SIGIL_NODISCARD auto GetName() const -> std::string_view;
    SIGIL_NODISCARD auto GetChildren() const -> const std::vector<NodePtr>&;

    void Accept(Visitor& visitor) const override;
};

class Identifier final : public Node {
    std::string name;

public:
    explicit Identifier(const ParseNode& node);

    SIGIL_NODISCARD std::string_view GetName() const;
    void Accept(Visitor& visitor) const override;
};

// TODO: make MutableDataDeclaration to encode meaning in the type rather than a bool
class DataDeclaration final : public Node {
    std::string name;
    NodePtr initializer;
    bool is_mutable;

public:
    explicit DataDeclaration(const ParseNode& node);

    SIGIL_NODISCARD std::string_view GetName() const;
    SIGIL_NODISCARD const NodePtr& GetInitializer() const;
    SIGIL_NODISCARD bool IsMutable() const;

    void Accept(Visitor& visitor) const override;
};

class Assignment final : public Node {
    std::string identifier;
    std::string op;
    NodePtr value;

public:
    explicit Assignment(const ParseNode& node);

    SIGIL_NODISCARD std::string_view GetIdentifier() const;
    SIGIL_NODISCARD const NodePtr& GetValue() const;
    SIGIL_NODISCARD std::string_view GetOp() const;

    void Accept(Visitor& visitor) const override;
};

class Scope final : public Node, public StatementContainer {
    std::unordered_map<std::string, DataDeclaration*> datums;

public:
    explicit Scope(const ParseNode& node);

    void Accept(Visitor& visitor) const override;

    SIGIL_NODISCARD const std::vector<NodePtr>& GetStatements() const;
};

class If final : public Node {
    NodePtr condition;
    NodePtr then_block;
    NodePtr else_branch;

public:
    explicit If(const ParseNode& node);

    SIGIL_NODISCARD const NodePtr& GetCondition() const;
    SIGIL_NODISCARD const NodePtr& GetThenBlock() const;
    SIGIL_NODISCARD const NodePtr& GetElseBranch() const;

    void Accept(Visitor& visitor) const override;
};

class Loop final : public Node {
    NodePtr body;

public:
    explicit Loop(const ParseNode& node);

    SIGIL_NODISCARD const NodePtr& GetBody() const;

    void Accept(Visitor& visitor) const override;
};

class LoopIf : public Node {
    NodePtr condition;
    NodePtr body;

public:
    explicit LoopIf(const ParseNode& node);

    SIGIL_NODISCARD const NodePtr& GetCondition() const;
    SIGIL_NODISCARD const NodePtr& GetBody() const;

    void Accept(Visitor& visitor) const override;
};

// semantically distinguishes from LoopIf without wasted padding
class LoopIfPost final : public LoopIf {
public:
    explicit LoopIfPost(const ParseNode& node);
    void Accept(Visitor& visitor) const override;
};

class LoopRange final : public Node {
    NodePtr start;
    NodePtr end;
    NodePtr body;

    std::string counter;

public:
    explicit LoopRange(const ParseNode& node);

    SIGIL_NODISCARD const NodePtr& GetStart() const;
    SIGIL_NODISCARD const NodePtr& GetEnd() const;
    SIGIL_NODISCARD const NodePtr& GetBody() const;

    SIGIL_NODISCARD std::string_view GetCounter() const;

    void Accept(Visitor& visitor) const override;
};

class LoopFixed final : public Node {
    std::string counter;
    NodePtr limit;
    NodePtr body;
    bool inclusive;
    bool counts_down;

public:
    explicit LoopFixed(const ParseNode& node);

    SIGIL_NODISCARD const NodePtr& GetLimit() const;
    SIGIL_NODISCARD const NodePtr& GetBody() const;
    SIGIL_NODISCARD std::string_view GetCounter() const;

    SIGIL_NODISCARD bool HasCounter() const;
    SIGIL_NODISCARD bool IsInclusive() const;
    SIGIL_NODISCARD bool CountsDown() const;

    void Accept(Visitor& visitor) const override;
};

class LoopControl : public Node {
    NodePtr condition;
    std::string label;

public:
    explicit LoopControl(const ParseNode& node);

    SIGIL_NODISCARD const NodePtr& GetCondition() const;
    SIGIL_NODISCARD std::string_view GetLabel() const;

    SIGIL_NODISCARD bool HasLabel() const;
    SIGIL_NODISCARD bool HasCondition() const;

    void Accept(Visitor& visitor) const override;
};

class Break final : public LoopControl {
public:
    explicit Break(const ParseNode& node);
    void Accept(Visitor& visitor) const override;
};

class Skip final : public LoopControl {
public:
    explicit Skip(const ParseNode& node);
    void Accept(Visitor& visitor) const override;
};

template <LiteralType T>
class Literal final : public Node {
    T value;

public:
    explicit Literal(T value)
        : value(value) {}

    explicit Literal(const std::string_view str) {
        std::from_chars(str.data(), str.data() + str.size(), value);
    }

    SIGIL_NODISCARD T Get() const {
        return value;
    }

    void Accept(Visitor& visitor) const override {
        visitor.Visit(*this);
    }
};

template <>
class Literal<void> final : public Node {
public:
    void Accept(Visitor& visitor) const override {
        visitor.Visit(*this);
    }
};

class ArrayLiteral final : public Node {
    std::vector<NodePtr> values;
    mana::PrimitiveType type;

public:
    explicit ArrayLiteral(const ParseNode& node);

    SIGIL_NODISCARD const std::vector<NodePtr>& GetValues() const;
    SIGIL_NODISCARD mana::PrimitiveType GetType() const;

    void Accept(Visitor& visitor) const override;

private:
    NodePtr ProcessValue(const ParseNode& elem);
};

class BinaryExpr final : public Node {
    std::string op;
    NodePtr left, right;

public:
    explicit BinaryExpr(const ParseNode& node);
    explicit BinaryExpr(const std::string& op, const ParseNode& left, const ParseNode& right);
    explicit BinaryExpr(std::string_view op, const ParseNode& left, const ParseNode& right);

    SIGIL_NODISCARD std::string_view GetOp() const;

    SIGIL_NODISCARD auto GetLeft() const -> const Node&;
    SIGIL_NODISCARD auto GetRight() const -> const Node&;

    void Accept(Visitor& visitor) const override;

private:
    explicit BinaryExpr(const ParseNode& binary_node, ml::i64 depth);
};

class UnaryExpr final : public Node {
    std::string op;
    NodePtr val;

public:
    explicit UnaryExpr(const ParseNode& node);

    void Accept(Visitor& visitor) const override;

    SIGIL_NODISCARD std::string_view GetOp() const;
    SIGIL_NODISCARD const Node& GetVal() const;
};

NodePtr CreateExpression(const ParseNode& node);

template <typename SC>
    requires std::is_base_of_v<StatementContainer, SC>
void PropagateStatements(const ParseNode& node, SC* root) {
#define Add template AddStatement

    for (const auto& stmt : node.branches) {
        for (const auto& n : stmt->branches) {
            using enum Rule;

            switch (n->rule) {
            case DataDeclaration:
                root->Add<class DataDeclaration>(*n);
                break;
            case If:
                root->Add<class If>(*n);
                break;
            case Loop:
                root->Add<class Loop>(*n);
                break;
            case LoopIf:
                root->Add<class LoopIf>(*n);
                break;
            case LoopIfPost:
                root->Add<class LoopIfPost>(*n);
                break;
            case LoopRange:
                root->Add<class LoopRange>(*n);
                break;
            case LoopFixed:
                root->Add<class LoopFixed>(*n);
                break;
            case LoopControl:
                if (n->tokens[0].type == TokenType::KW_break) {
                    root->Add<Break>(*n);
                    break;
                }
                if (n->tokens[0].type == TokenType::KW_skip) {
                    root->Add<Skip>(*n);
                    break;
                }
                Log->error("Unexpected loop control statement. Token was '{}'",
                           magic_enum::enum_name(n->tokens[0].type)
                );
                break;
            default:
                if (auto expr = CreateExpression(*n)) {
                    root->AddStatement(std::move(expr));
                }
                break;
            }
        }
    }

#undef Add
}
} // namespace sigil::ast
