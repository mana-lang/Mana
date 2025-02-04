#include <sigil/ast/parse_tree.hpp>

namespace sigil::ast {

Node::Node(const Rule r)
    : rule {r}
    , parent {nullptr} {}

Node::Node(Node* p, const Rule r)
    : rule {r}
    , parent {p} {}

Node& Node::NewBranch(const Rule new_rule) {
    // because the module node is the root, it's actually useless to list it as a parent
    return *branches.emplace_back(std::make_shared<Node>(rule == Rule::Module ? nullptr : this, new_rule));
}

void Node::PopBranch() {
    branches.pop_back();
}

void Node::RemoveBranch(const i64 idx) {
    branches.erase(branches.begin() + idx);
}

void Node::RemoveBranchFromTail(const i64 idx) {
    branches.erase(branches.end() - idx);
}

SIGIL_NODISCARD bool Node::IsRoot() const {
    return parent == nullptr;
}

void Node::AcquireBranchOf(Node& target, const i64 index) {
#ifdef SIGIL_DEBUG
    if (target.branches[index].get() == this) {
        LogErr("Can not acquire branches of self");
        return;
    }
#endif

    branches.emplace_back(target.branches[index]);
    branches.back()->parent = this;
    target.RemoveBranch(index);
}

void Node::AcquireBranchesOf(Node& target, const i64 start, const i64 end) {
    for (i64 i = start; i <= end; ++i) {
#ifdef SIGIL_DEBUG
        if (target.branches[i].get() == this) {
            LogErr("Can not acquire branches of self");
            return;
        }
#endif
        branches.emplace_back(target.branches[i]);
        branches.back()->parent = this;
    }

    // erase removes up to the penultimate element, but we want inclusive removal
    target.branches.erase(target.branches.begin() + start, target.branches.begin() + end + 1);
}

void Node::AcquireBranchesOf(Node& target, const i64 start) {
    for (i64 i = start; i < target.branches.size(); ++i) {
#ifdef SIGIL_DEBUG
        if (target.branches[i].get() == this) {
            LogErr("Can not acquire branches of self");
            return;
        }
#endif
        branches.emplace_back(target.branches[i]);
        branches.back()->parent = this;
    }
    target.branches.erase(target.branches.begin() + start, target.branches.end());
}

void Node::AcquireTailBranchOf(Node& target) {
#ifdef SIGIL_DEBUG
    if (target.branches.back().get() == this) {
        LogErr("Can not acquire branches of self");
        return;
    }
#endif
    branches.emplace_back(target.branches.back());
    branches.back()->parent = this;
    target.branches.erase(target.branches.end() - 1);
}
}  // namespace hex::ast