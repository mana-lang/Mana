#pragma once

#include <mana/literals.hpp>
#include <sigil/ast/parse-tree.hpp>

#include <memory>
#include <string>
#include <vector>

/// As the ptree gets constructed before the AST,
/// AST nodes assume their ptree input is correct
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

class Literal_F64 final : public Node {
    ml::f64 value;

public:
    explicit Literal_F64(ml::f64 value);

    SIGIL_NODISCARD ml::f64 Get() const;

    void Accept(Visitor& visitor) const override;
};

class BinaryExpr final : public Node {
    char op;
    Ptr  left, right;

public:
    explicit BinaryExpr(const ParseNode& node);
    explicit BinaryExpr(char op, const ParseNode& left, const ParseNode& right);

    SIGIL_NODISCARD char GetOp() const;
    SIGIL_NODISCARD auto GetLeft() const -> const Node&;
    SIGIL_NODISCARD auto GetRight() const -> const Node&;

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
