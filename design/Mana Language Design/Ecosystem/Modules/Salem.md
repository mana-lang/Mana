
<h1><center><i>Salem</i></center></h1>
<h4><center><b>Mana Native Compiler</b></center></h4>

---
**Salem** is the native compiler for the **Mana** language. While **Circe** focuses on rapid bytecode generation for the virtual machine, Salem is designed to be the heavy-lifting back-end that produces optimized machine code, on top of providing a solid level of code analysis to be able to detect subtle bugs and give high-quality error reports.

Salem will remain in stasis as a project until Circe, Hex and Sigil are mature enough for Mana to be competitive with Lua. At that point, it will integrate LLVM as its code generation back-end to produce AOT binary executables that optionally have Hex embedded within them, primed to run `@[Hot]` code.
### Future Goals & Roadmap
As the ecosystem evolves, Salem is intended to implement several advanced features:

- **Native Code Generation**: Moving beyond bytecode to produce highly optimized binaries for target platforms (starting with x64 Windows and Linux).
- **Data Layout Optimization**: Automatically reordering members in `@[Flexible]` data types to minimize padding and improve cache locality—a core part of Mana's performance philosophy.
- **Safety & Performance Analysis**: Providing deep-tissue analysis of code to spot potential performance bottlenecks or safety violations.
- **Errors for Humans**: Offering configurably detailed error and warning diagnostics, formatted in a way that's easy to understand.
- **Hybrid Execution**: Automatically embedding the **Hex** VM into native executables to support `@[Hot]` code reloading, allowing for a seamless blend of native performance and runtime flexibility.

Salem is named after the black cat mascot of the Mana project, symbolizing its role as a watchful and helpful companion to the developer.