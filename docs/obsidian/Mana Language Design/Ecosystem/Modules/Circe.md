
<h1><center><i>Circe</i></center></h1>
<h4><center><b>Mana Bytecode Compiler</b></center></h4>

---
**Circe** is the bytecode generation module for the **Mana** language. It transforms the Abstract Syntax Tree (AST) produced and validated by **Sigil** into executable bytecode (**Hexe**) for the Mana Virtual Machine (**Hex**).

Circe operates by traversing the AST and mapping its semantic structure to the low-level instructions expected by Hex.

Like Sigil, Circe follows a philosophy of "correct by construction." It assumes that the input AST has been fully validated by Sigil's `SemanticAnalyzer`. Therefore, Circe does not perform type checking or semantic validation; it focuses exclusively on efficient bytecode emission and register allocation.

> [!tip] Compilation Pipeline
> **AST ➾ Bytecode Generation ➾ Serialization ➾ Hexecutable (.hexe)**
### Bytecode Generation
The `BytecodeGenerator` class implements **Sigil**'s `ast::Visitor` interface to walk the AST and populate a `Hexe` bytecode structure with the appropriate instructions, including:
- **Data Bindings**: Register allocation and lifetime tracking for data declarations.
- **Control Flow**: Resolution of jump distances for loops (`Loop`, `LoopIf`, etc.) and conditional branches (`If`).
- **Expressions**: Resolution of unary and binary expressions into VM instructions.
- **Literals**: Stored in the Hexe Constant Pool to be assigned to data bindings at runtime.

### CLI Integration
Circe provides a command-line interface that drives the entire compilation process. It allows for configurable output paths and provides optional verbose debugging information, including:
- **Compilation Metrics**: Time taken and token counts.
- **Emission Statistics**: Instruction size and constant pool volume.
- **Artifact Inspection**: Text emission of the `ParseTree` or `TokenStream` for debugging purposes.

All logging within the module is handled via a dedicated `spdlog` instance, consistent with the rest of the Mana modules.