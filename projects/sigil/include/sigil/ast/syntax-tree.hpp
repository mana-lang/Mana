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
    explicit Statement(const NodePtr&& node);

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
            std::make_shared<NodeT>(std::forward<Args>(args)...)));
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

class Scope final : public Node, public StatementContainer {
public:
    explicit Scope(const ParseNode& node);

    void Accept(Visitor& visitor) const override;

    const std::vector<NodePtr>& GetStatements() const;
};

class If final : public Node {
    NodePtr condition;
    NodePtr then_block;
    NodePtr else_branch;

public:
    If(const ParseNode& node);
    // If(NodePtr condition, NodePtr then_block, NodePtr else_branch = nullptr);

    const NodePtr& GetCondition() const;
    const NodePtr& GetThenBlock() const;
    const NodePtr& GetElseBranch() const;

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
    ArrayLiteral(const ParseNode& node);

    const std::vector<NodePtr>& GetValues() const;
    mana::PrimitiveType GetType() const;

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
    explicit BinaryExpr(const std::string_view op, const ParseNode& left, const ParseNode& right);

    SIGIL_NODISCARD std::string_view GetOp() const;

    SIGIL_NODISCARD auto GetLeft() const -> const Node&;
    SIGIL_NODISCARD auto GetRight() const -> const Node&;

    void Accept(Visitor& visitor) const override;

private:
    static NodePtr ConstructChild(const ParseNode& operand_node);
    explicit BinaryExpr(const ParseNode& binary_node, ml::i64 depth);
};

class UnaryExpr final : public Node {
    std::string op;
    NodePtr val;

public:
    explicit UnaryExpr(const ParseNode& unary_node);

    void Accept(Visitor& visitor) const override;

    SIGIL_NODISCARD std::string_view GetOp() const;
    SIGIL_NODISCARD const Node& GetVal() const;
};

template <typename SC>
    requires std::is_base_of_v<StatementContainer, SC>
void PropagateStatements(const ParseNode& node, SC* root) {
#define Add template AddStatement

    for (const auto& stmt : node.branches) {
        for (const auto& n : stmt->branches) {
            using enum Rule;

            switch (n->rule) {
            case Equality:
            case Comparison:
            case Term:
            case Factor:
                root->Add<BinaryExpr>(*n);
                break;
            case Unary:
                root->Add<UnaryExpr>(*n);
                break;
            case ArrayLiteral:
                root->Add<ast::ArrayLiteral>(*n);
                break;
            case IfBlock:
                root->Add<If>(*n);
                break;
            default:
                break;
            }
        }
    }

#undef Add
}



} // namespace sigil::ast
