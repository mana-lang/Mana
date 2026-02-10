#pragma once

#include <sigil/core/logger.hpp>

#include <sigil/ast/parse-tree.hpp>
#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>
#include <hexe/primitive-type.hpp>

#include <memory>
#include <vector>
#include <charconv>
#include <string_view>

namespace sigil::ast {
namespace ml = mana::literals;

class Visitor;

// As the ptree gets constructed before the AST,
// AST nodes assume their ptree input is correct
class Node {
public:
    virtual ~Node() = default;

    virtual void Accept(Visitor& visitor) const = 0;
};

using NodePtr = std::shared_ptr<Node>;

class Artifact final : public Node {
    std::string_view name;
    std::vector<NodePtr> declarations;

public:
    Artifact(std::string_view name, const ParseNode& node);

    SIGIL_NODISCARD auto GetName() const -> std::string_view;
    SIGIL_NODISCARD auto GetChildren() const -> const std::vector<NodePtr>&;

    void Accept(Visitor& visitor) const override;
};

struct Parameter {
    std::string_view name;
    std::string_view type;
};

class FunctionDeclaration final : public Node {
    std::string_view name;
    std::vector<Parameter> parameters;
    NodePtr body;

    std::string return_type;

public:
    explicit FunctionDeclaration(const ParseNode& node);

    SIGIL_NODISCARD std::string_view GetName() const;
    SIGIL_NODISCARD std::span<const Parameter> GetParameters() const;
    SIGIL_NODISCARD const NodePtr& GetBody() const;
    SIGIL_NODISCARD std::string_view GetReturnType() const;

    void Accept(Visitor& visitor) const override;
};

class Invocation final : public Node {
    std::string_view identifier;
    std::vector<NodePtr> arguments;

public:
    explicit Invocation(const ParseNode& node);

    SIGIL_NODISCARD std::string_view GetIdentifier() const;
    SIGIL_NODISCARD const std::vector<NodePtr>& GetArguments() const;

    void Accept(Visitor& visitor) const override;
};

class Initializer : public Node {
    std::string_view name;
    std::string_view type;
    NodePtr initializer;

public:
    explicit Initializer(const ParseNode& node);

    SIGIL_NODISCARD std::string_view GetName() const;
    SIGIL_NODISCARD std::string_view GetTypeName() const;
    SIGIL_NODISCARD const NodePtr& GetInitializer() const;

    SIGIL_NODISCARD bool HasTypeAnnotation() const;

    void Accept(Visitor& visitor) const override;
};

class DataDeclaration final : public Initializer {
public:
    explicit DataDeclaration(const ParseNode& node);

    void Accept(Visitor& visitor) const override;
};

class MutableDataDeclaration final : public Initializer {
public:
    explicit MutableDataDeclaration(const ParseNode& node);

    void Accept(Visitor& visitor) const override;
};

class Statement final : public Node {
    NodePtr child;

public:
    explicit Statement(NodePtr&& node);

    SIGIL_NODISCARD const NodePtr& GetChild() const;

    void Accept(Visitor& visitor) const override;
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

class LoopIfPost final : public Node {
    NodePtr condition;
    NodePtr body;

public:
    explicit LoopIfPost(const ParseNode& node);

    SIGIL_NODISCARD const NodePtr& GetCondition() const;
    SIGIL_NODISCARD const NodePtr& GetBody() const;

    void Accept(Visitor& visitor) const override;
};

class LoopRange : public Node {
    NodePtr origin;
    NodePtr destination;
    NodePtr body;

    std::string_view counter;

public:
    explicit LoopRange(const ParseNode& node);

    SIGIL_NODISCARD const NodePtr& GetOrigin() const;
    SIGIL_NODISCARD const NodePtr& GetDestination() const;
    SIGIL_NODISCARD const NodePtr& GetBody() const;

    SIGIL_NODISCARD std::string_view GetCounterName() const;

    void Accept(Visitor& visitor) const override;
};

class LoopRangeMutable : public LoopRange {
public:
    explicit LoopRangeMutable(const ParseNode& node);

    void Accept(Visitor& visitor) const override;
};

class LoopFixed final : public Node {
    NodePtr count;
    NodePtr body;

public:
    explicit LoopFixed(const ParseNode& node);

    SIGIL_NODISCARD const NodePtr& GetCountTarget() const;
    SIGIL_NODISCARD const NodePtr& GetBody() const;

    void Accept(Visitor& visitor) const override;
};

class LoopControl : public Node {
    NodePtr condition;
    std::string_view label;

public:
    explicit LoopControl(const ParseNode& node);

    SIGIL_NODISCARD const NodePtr& GetCondition() const;
    SIGIL_NODISCARD std::string_view GetLabel() const;

    SIGIL_NODISCARD bool HasLabel() const;

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

class Return final : public Node {
    NodePtr expr;

public:
    explicit Return(const ParseNode& node);

    SIGIL_NODISCARD const NodePtr& GetExpression() const;

    void Accept(Visitor& visitor) const override;
};

class Assignment final : public Node {
    std::string_view identifier;
    std::string_view op;
    NodePtr value;

public:
    explicit Assignment(const ParseNode& node);

    SIGIL_NODISCARD std::string_view GetIdentifier() const;
    SIGIL_NODISCARD const NodePtr& GetValue() const;
    SIGIL_NODISCARD std::string_view GetOp() const;

    void Accept(Visitor& visitor) const override;
};

template <typename T>
concept NodeType = std::is_base_of_v<Node, T>;

class Scope final : public Node {
    std::vector<NodePtr> statements;

public:
    explicit Scope(const ParseNode& node);

    void Accept(Visitor& visitor) const override;

    SIGIL_NODISCARD const std::vector<NodePtr>& GetStatements() const;

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

class Identifier final : public Node {
    std::string_view name;

public:
    explicit Identifier(const ParseNode& node);

    SIGIL_NODISCARD std::string_view GetName() const;
    void Accept(Visitor& visitor) const override;
};

class BinaryExpr final : public Node {
    std::string_view op;
    NodePtr left, right;

public:
    explicit BinaryExpr(const ParseNode& node);
    explicit BinaryExpr(std::string_view op, const ParseNode& left, const ParseNode& right);

    SIGIL_NODISCARD std::string_view GetOp() const;

    SIGIL_NODISCARD auto GetLeft() const -> const Node&;
    SIGIL_NODISCARD auto GetRight() const -> const Node&;

    void Accept(Visitor& visitor) const override;

private:
    explicit BinaryExpr(const ParseNode& binary_node, ml::i64 depth);
};

class UnaryExpr final : public Node {
    std::string_view op;
    NodePtr val;

public:
    explicit UnaryExpr(const ParseNode& node);

    void Accept(Visitor& visitor) const override;

    SIGIL_NODISCARD std::string_view GetOp() const;
    SIGIL_NODISCARD const Node& GetVal() const;
};

template <typename T> requires std::is_arithmetic_v<T>
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

class StringLiteral final : public Node {
    std::string value;

public:
    explicit StringLiteral(const std::string_view value)
        : value(value) {}

    SIGIL_NODISCARD std::string_view Get() const {
        return value;
    }

    void Accept(Visitor& visitor) const override {
        visitor.Visit(*this);
    }
};

class ArrayLiteral final : public Node {
    std::vector<NodePtr> values;
    hexe::ValueType type;

public:
    explicit ArrayLiteral(const ParseNode& node);

    SIGIL_NODISCARD const std::vector<NodePtr>& GetValues() const;
    SIGIL_NODISCARD hexe::ValueType GetType() const;

    void Accept(Visitor& visitor) const override;

private:
    NodePtr ProcessValue(const ParseNode& elem);
};

NodePtr CreateExpression(const ParseNode& node);
NodePtr CreateDeclaration(const ParseNode& node);
} // namespace sigil::ast
