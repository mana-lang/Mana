#pragma once

#include <sigil/ast/parse-tree.hpp>
#include <sigil/ast/visitor.hpp>
#include <sigil/core/concepts.hpp>

#include <mana/vm/primitive-type.hpp>
#include <mana/literals.hpp>

#include <memory>
#include <string>
#include <vector>

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

class Artifact final : public Node {
    std::string          name;
    std::vector<NodePtr> children;

public:
    explicit Artifact(const std::string_view name)
        : name(name) {}

    SIGIL_NODISCARD auto GetName() const -> std::string_view;
    SIGIL_NODISCARD auto GetChildren() const -> const std::vector<NodePtr>&;

    void Accept(Visitor& visitor) const override;

    template <typename NodeT, typename... Args>
        requires std::is_base_of_v<Node, NodeT>
    void AddChild(Args&&... args) {
        children.emplace_back(std::make_shared<Statement>(
            std::make_shared<NodeT>(std::forward<Args>(args)...)));
    }
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
    mana::PrimitiveType  type;

public:
    ArrayLiteral(const ParseNode& node);

    const std::vector<NodePtr>& GetValues() const;
    mana::PrimitiveType         GetType() const;

    void Accept(Visitor& visitor) const override;

private:
    NodePtr ProcessValue(const ParseNode& elem);
};

class BinaryExpr final : public Node {
    std::string op;
    NodePtr     left, right;

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
    explicit       BinaryExpr(const ParseNode& binary_node, ml::i64 depth);
};

class UnaryExpr final : public Node {
    std::string op;
    NodePtr     val;

public:
    explicit UnaryExpr(const ParseNode& unary_node);

    void Accept(Visitor& visitor) const override;

    SIGIL_NODISCARD std::string_view GetOp() const;
    SIGIL_NODISCARD const Node&      GetVal() const;
};
} // namespace sigil::ast
