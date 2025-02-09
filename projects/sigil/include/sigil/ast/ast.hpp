#pragma once

#include <sigil/ast/rule.hpp>

#include <mana/literals.hpp>

#include <memory>
#include <vector>

namespace sigil {
class ParseNode;
}

namespace sigil::ast {
namespace ml = mana::literals;

class Visitor;

class Node {
public:
    using Ptr       = std::shared_ptr<Node>;
    virtual ~Node() = default;

    virtual void Accept(Visitor& visitor) const = 0;
};

template <typename T>
concept NodeType = std::is_base_of_v<Node, T>;

class Module final : public Node {
    std::string      name;
    std::vector<Ptr> children;

public:
    explicit Module(const std::string_view name)
        : name(name) {}

    SIGIL_NODISCARD auto GetName() const -> std::string_view;
    SIGIL_NODISCARD auto GetChildren() const -> const std::vector<Ptr>&;

    void Accept(Visitor& visitor) const override;

    template <NodeType NT, typename... Args>
    void AddChild(Args&&... args) {
        children.push_back(std::make_shared<NT>(std::forward<Args>(args)...));
    }
};

class LiteralF64 final : public Node {
    ml::f64 value;

public:
    explicit LiteralF64(ml::f64 value);

    SIGIL_NODISCARD ml::f64 Get() const;

    void Accept(Visitor& visitor) const override;
};

class BinaryOp final : public Node {
    char op;
    Ptr  left, right;

public:
    explicit BinaryOp(char op, const ParseNode& left, const ParseNode& right);

    SIGIL_NODISCARD char GetOp() const;
    SIGIL_NODISCARD auto GetLeft() const -> const Node&;
    SIGIL_NODISCARD auto GetRight() const -> const Node&;

    void Accept(Visitor& visitor) const override;

private:
    void ConstructChild(const ParseNode& node, Ptr& target);
};

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void Visit(const LiteralF64& node) = 0;
    virtual void Visit(const BinaryOp& node)   = 0;
    virtual void Visit(const Module& node)     = 0;
};

}  // namespace sigil::ast
