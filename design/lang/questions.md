## Execution model and boundaries
- What are the unit(s) of compilation vs interpretation (module, package, file, namespace)? How do you declare “this module is interpreted, that one is compiled”?
- Is the interpreter a bytecode VM, an AST-walker, or something else? If bytecode, do you plan a single bytecode format usable by both the interpreter and AOT/JIT backends?
- Will there be a JIT tier (baseline + optimizing), or only interpreted and AOT-compiled?

## Hot reload semantics
- What’s your model for state preservation across reloads? Do you support live data migration of struct/class layout changes, function signature changes, enum changes?
- How do you handle in-flight frames/calls when a function is replaced? Quiesce and swap, lazy swap on next call, or always-indirect dispatch via stubs?
- What is the strategy for ABI stability within a process across different codegens (interpreted vs compiled)?

## Language/runtime capabilities
- Reflection and metadata: do you plan compile-time reflection, runtime reflection, or hybrid? This is crucial for hot reload and editor tooling.
- Memory model: GC, ARC, arenas, or manual with ownership/borrowing? How will this interact with real-time constraints (e.g., no unpredictable pauses)?
- Concurrency: what primitives are planned (jobs, actors, channels, async/await)? Any determinism goals for lockstep networking or replays?

## Tooling and workflows
- How do developers toggle a module’s mode (interpreted/compiled)? Build tags, config file, or in-language annotations?
- Debugging: can you set breakpoints across both modes seamlessly? Single-step from compiled into interpreted code?
- Incremental builds: do you envision a persistent compiler daemon/compiler server for fast compiles, plus hot reload for interpreted parts?

## Interoperability
- What’s the FFI story (C/C++, Rust)? Can compiled Mana expose a stable ABI for engine integration, while hot-reload gameplay lives in interpreted modules?
- Asset/tool pipeline: will scripts/tools written in Mana run inside the same runtime, tapping into reflection and editor integration?

## Safety and correctness
- Type/system features planned (sum types, generics, traits/interfaces, ownership, pattern matching)? Any soundness guarantees that aid live patching?
- Versioning: will you track module versions and perform compatibility checks at reload time with structured error messages and suggested migrations?

## Performance and determinism
- Do you target deterministic floating point and reproducible scheduling when desired (e.g., for replays), even across modes?
- Will compiled code be able to directly call interpreted code in tight loops, or will you encourage boundary batching to minimize overhead?

## Packaging and distribution
- Can a shipped game include some modules in interpreted mode for live modding? If yes, how do you secure/sandbox those modules?
- Do you plan AOT cross-compilation targets and a minimal runtime for consoles/mobile?

## Editor and UX
- Live edit-and-continue: do you plan partial re-typechecking/recompilation with structural hashing to keep feedback sub-100ms?
- Error reporting: will diagnostics include hot-reloadability guidance (e.g., “changing field order breaks ABI; add explicit version/migration”)?
