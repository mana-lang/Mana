## Hot reload semantics
- What’s the model for state preservation across reloads? Do we support live data migration of composite type layout changes, function signature changes, enum changes?
- How do we handle in-flight frames/calls when a function is replaced? Quiesce and swap, lazy swap on next call, or always-indirect dispatch via stubs?
- What is the strategy for ABI stability within a process across different codegens (interpreted vs compiled)?

## Language/runtime capabilities
- Reflection and metadata: we allow for compile-time reflection, but will there also be support for runtime reflection? This is significant for editor tooling.
- Memory model: GC, ARC, arenas, manual with ownership/borrowing. Which are best suitable for which scenarios? How will this interact with real-time constraints (e.g. no unpredictable pauses)?
- Concurrency: what primitives are planned (jobs, actors, channels, async/await)? Any determinism goals for lockstep networking or replays?

## Tooling and workflows
- Debugging: can user set breakpoints across both modes seamlessly? Single-step from compiled into interpreted code?
- Hot-reload recompiles: When do we trigger a recompile from a running executable? What tradeoffs do we make?

## Interoperability
- What’s the FFI story (C/C++, Rust)? Can compiled Mana expose a stable ABI for engine integration?

## Safety and correctness
- How will we implement data race protection?
- To what extent do we allow the user to write unsafe code?
- What will Mana's testing model be? 
  - Will Conjure have built-in testing features?

## Packaging and distribution
- Can a shipped game include some modules in interpreted mode for live modding? If yes, how do we secure/sandbox those modules?
