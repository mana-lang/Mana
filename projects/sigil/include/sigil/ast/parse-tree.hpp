#pragma once

#include <sigil/ast/rule.hpp>
#include <sigil/ast/token.hpp>

#include <mana/literals.hpp>

#include <memory>
#include <vector>

namespace sigil {

class ParseNode {
    ParseNode* parent;

public:
    using Ptr = std::shared_ptr<ParseNode>;

    ast::Rule        rule;
    TokenStream      tokens;
    std::vector<Ptr> branches;

    explicit ParseNode(ast::Rule rule = ast::Rule::Undefined);
    explicit ParseNode(ParseNode* parent, ast::Rule rule = ast::Rule::Undefined);

    SIGIL_NODISCARD ParseNode& NewBranch(ast::Rule new_rule = ast::Rule::Undefined);

    void PopBranch();
    void RemoveBranch(ml::i64 idx);
    void RemoveBranchFromTail(ml::i64 idx);

    SIGIL_NODISCARD bool IsRoot() const;
    SIGIL_NODISCARD bool IsLeaf() const;

    void AcquireBranchOf(ParseNode& target, ml::i64 index);
    void AcquireBranchesOf(ParseNode& target, ml::i64 start, ml::i64 end);
    void AcquireBranchesOf(ParseNode& target, ml::i64 start);
    void AcquireTailBranchOf(ParseNode& target);
};

}  // namespace sigil
