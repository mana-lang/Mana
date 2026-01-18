#include <sigil/ast/semantic-analyzer.hpp>

namespace sigil::ast {

void SemanticAnalyzer::Visit(const Artifact& artifact) {}
void SemanticAnalyzer::Visit(const Scope& node) {}
void SemanticAnalyzer::Visit(const MutableDataDeclaration& node) {}
void SemanticAnalyzer::Visit(const DataDeclaration& node) {}
void SemanticAnalyzer::Visit(const Identifier& node) {}
void SemanticAnalyzer::Visit(const Assignment& node) {}
void SemanticAnalyzer::Visit(const If& node) {}
void SemanticAnalyzer::Visit(const Loop& node) {}
void SemanticAnalyzer::Visit(const LoopIf& node) {}
void SemanticAnalyzer::Visit(const LoopIfPost& node) {}
void SemanticAnalyzer::Visit(const LoopRange& node) {}
void SemanticAnalyzer::Visit(const LoopFixed& node) {}
void SemanticAnalyzer::Visit(const Break& node) {}
void SemanticAnalyzer::Visit(const Skip& node) {}
void SemanticAnalyzer::Visit(const UnaryExpr& node) {}
void SemanticAnalyzer::Visit(const BinaryExpr& node) {}
void SemanticAnalyzer::Visit(const ArrayLiteral& array) {}
void SemanticAnalyzer::Visit(const Literal<ml::f64>& literal) {}
void SemanticAnalyzer::Visit(const Literal<ml::i64>& literal) {}
void SemanticAnalyzer::Visit(const Literal<void>& node) {}
void SemanticAnalyzer::Visit(const Literal<bool>& literal) {}

} // namespace sigil::ast
