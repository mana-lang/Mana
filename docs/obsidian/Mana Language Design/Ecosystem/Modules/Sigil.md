
<h1><center><i>Sigil</i></center></h1>
<h4><center><b>Mana Language Frontend</b></center></h4>

---
**Sigil** is a modular front-end for the Mana language that retains every step of the compilation process, as well as ensuring that the output for each step is a complete and valid input for the next.

What this means is that, for example, the output of Sigil's `Lexer` (a token stream) is a complete and valid series of tokens for Sigil's `Parser`. The parser does not need to ensure that the token stream is formatted correctly, because this is already guaranteed by the lexer.

In other words, Sigil's transformation and analysis components do *not* perform any input validation or sanitation. Therefore, *if the input to any Sigil component is ill-formed, its behaviour is undefined.* 

This is by design. It allows Sigil's components to avoid redundant verification, enabling each component to focus purely on producing a complete and valid output. 


> [!tip] Data Pipeline
> **String ➾ Token Stream ➾ Parse Tree ➾ Abstract Syntax Tree ➾ Error Sink**

> [!tip] Analysis Pipeline
> **Source ➾ Lexical Analysis ➾ Syntax Analysis ➾ Semantic Analysis ➾ Error Report**

The semantic analyzer does not modify the AST. Instead, it will record useful information about the AST that can be passed off to code generators such as Circe.

All errors encountered by Sigil will be recorded to an external **ErrorSink**. The error sink may be configured and swapped out freely, however it uses distinct error codes to analyze the source code itself, and can output configurable, comprehensive, detailed, and well-formatted errors to any `spdlog` logger.

Sigil is designed to both serve as the front-end to Mana's compilers as well as the primary driver behind semantic analysis interfaces such as LSP, which should afford a relatively easy time in the implementation of Mana language plugins for different code editors.