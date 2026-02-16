#include <sigil/ast/parse-tree.hpp>
#include <sigil/core/logger.hpp>

namespace sigil {
using namespace mana::literals;
using namespace ast;

ParseNode::ParseNode(const Rule r)
    : parent {nullptr},
      rule {r} {}

ParseNode::ParseNode(ParseNode* p, const Rule r)
    : parent {p},
      rule {r} {}

ParseNode& ParseNode::NewBranch(const Rule new_rule) {
    // because the module node is the root, it's useless to list it as a parent
    return *branches.emplace_back(
        std::make_shared<ParseNode>(rule == Rule::Artifact ? nullptr : this, new_rule)
    );
}

void ParseNode::PopBranch() {
    branches.pop_back();
}

void ParseNode::RemoveBranch(const i64 idx) {
    branches.erase(branches.begin() + idx);
}

void ParseNode::RemoveBranchFromTail(const i64 idx) {
    branches.erase(branches.end() - idx);
}

bool ParseNode::IsRoot() const {
    return parent == nullptr;
}

bool ParseNode::IsLeaf() const {
    return branches.empty();
}

void ParseNode::AcquireBranchOf(ParseNode& target, const i64 index) {
#ifdef SIGIL_DEBUG
    if (target.branches[index].get() == this) {
        Log->error("Can not acquire branches of self");
        return;
    }
#endif

    branches.emplace_back(target.branches[index]);
    branches.back()->parent = this;
    target.RemoveBranch(index);
}

void ParseNode::AcquireBranchesOf(ParseNode& target, const i64 start, const i64 end) {
    for (i64 i = start; i <= end; ++i) {
#ifdef SIGIL_DEBUG
        if (target.branches[i].get() == this) {
            Log->error("Can not acquire branches of self");
            return;
        }
#endif
        branches.emplace_back(target.branches[i]);
        branches.back()->parent = this;
    }

    // erase removes up to the penultimate element, but we want inclusive removal
    target.branches.erase(target.branches.begin() + start, target.branches.begin() + end + 1);
}

void ParseNode::AcquireBranchesOf(ParseNode& target, const i64 start) {
    for (i64 i = start; i < target.branches.size(); ++i) {
#ifdef SIGIL_DEBUG
        if (target.branches[i].get() == this) {
            Log->error("Can not acquire branches of self");
            return;
        }
#endif
        branches.emplace_back(target.branches[i]);
        branches.back()->parent = this;
    }
    target.branches.erase(target.branches.begin() + start, target.branches.end());
}

void ParseNode::AcquireTailBranchOf(ParseNode& target) {
#ifdef SIGIL_DEBUG
    if (target.branches.back().get() == this) {
        Log->error("Can not acquire branches of self");
        return;
    }
#endif
    branches.emplace_back(target.branches.back());
    branches.back()->parent = this;
    target.branches.erase(target.branches.end() - 1);
}
} // namespace sigil
