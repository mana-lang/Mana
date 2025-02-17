#pragma once

#include <sigil/ast/parse-tree.hpp>
#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>

#include <memory>
#include <string>
#include <vector>

namespace sigil::ast {
namespace ml = mana::literals;

class Visitor;

/// As the ptree gets constructed before the AST,
/// AST nodes assume their ptree input is correct
class Node {
public:
    using Ptr       = std::shared_ptr<Node>;
    virtual ~Node() = default;

    virtual void Accept(Visitor& visitor) const = 0;
};

template <typename T>
concept NodeType = std::is_base_of_v<Node, T>;

class Artifact final : public Node {
    std::string      name;
    std::vector<Ptr> children;

public:
    explicit Artifact(const std::string_view name)
        : name(name) {}

    SIGIL_NODISCARD auto GetName() const -> std::string_view;
    SIGIL_NODISCARD auto GetChildren() const -> const std::vector<Ptr>&;

    void Accept(Visitor& visitor) const override;

    template <NodeType NT, typename... Args>
    void AddChild(Args&&... args) {
        children.push_back(std::make_shared<NT>(std::forward<Args>(args)...));
    }
};

template <typename T>
class Literal final : public Node {
    T value;

public:
    explicit Literal(T value)
        : value(value) {}

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

class BinaryExpr final : public Node {
    std::string op;
    Ptr         left, right;

public:
    explicit BinaryExpr(const ParseNode& node);
    explicit BinaryExpr(const std::string& op, const ParseNode& left, const ParseNode& right);

    SIGIL_NODISCARD std::string_view GetOp() const;
    SIGIL_NODISCARD auto             GetLeft() const -> const Node&;
    SIGIL_NODISCARD auto             GetRight() const -> const Node&;

    void Accept(Visitor& visitor) const override;

private:
    static Ptr ConstructChild(const ParseNode& node);
    explicit BinaryExpr(const ParseNode& node, ml::i64 depth);
};

class UnaryExpr final : public Node {
    char op;
    Ptr  val;

public:
    explicit UnaryExpr(const ParseNode& node);

    void Accept(Visitor& visitor) const override;

    SIGIL_NODISCARD char GetOp() const;
    SIGIL_NODISCARD auto GetVal() const -> const Node&;
};

}  // namespace sigil::ast
