<h1 align="center">The Mana Programming Language</h1>

<p align="center">
  <img src="design/logo-text-white-bg.png" alt="Mana — a purple and blue gradient flame logo with the lowercase white text 'mana' beside it." width="600">
</p>
<p align="center">
  <em>Write fast, run fast.</em>
</p>

<p align="center">
  <em>A data-centric language purpose-built for game development.</em>
</p>


<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/C%2B%2B-23-blue.svg" alt="C++23">
  <img src="https://img.shields.io/badge/status-in%20development-orange" alt="Status: In Development">
</p>

---

**Mana** is a statically-typed, data-centric programming language purpose-built for game development. It features a hybrid execution model, where source code can be selectively compiled to native machine code or to JIT-able bytecode for partial hot-reload execution in a register-based VM, without the need for glue code.

```rust
import std.fmt

fn Main() {
    data message = "Hello from Mana!"
    fmt.PrintLine("{message}")

    data scores = [5, 10, 15, 20]
    mut data total: i32 // defaults to 0

    for score in scores {
        total += score
    }
    fmt.Print("Total: {}", total)
}
```

### Development
Mana is currently in the **very early** stages of development. Some things may be broken or incomplete, and some references may lead to nowhere. Most notably, the code example above won't compile yet, as `import` statements have not yet been implemented; therefore, printing values is done with `PrintV` for the time being, which does not yet support string interpolation. 

This is actively being worked on, and these issues will be taken care of in due time.

### Key Features

- **Data-centric design**
    - Everything in Mana (including functions) is data, encouraging a data-centric mental model that helps you understand the flow of data through your program.
- **Hybrid execution**
    - Selectively compile to native code or bytecode from the same source, and **Hot-Reload** bytecode with **Hex**, Mana's speedy, register-based VM.
- **Statically typed** 
    - With type inference: `data x = 42` 
    - Or annotations: `data x: i32 = 42`
- **Expressive Control Flow**
    - Simple yet flexible loops
        - infinite: `loop { }`
        - fixed: `loop 8 { }`
        - conditional: `loop if x > 5 { }`
            - post-block eval: `loop { } if x > 5`
        - counted: `loop 8 => i { }`
        - range-based: `loop 0..10 => i { }`
        - and more
    - Iterating ranges with `for`
        - `for val in values`
    - Pattern matching with `match`
- **Explicit & Deduced Assignment**
    - Control how data is passed around via explicit assignment operators, or allow Mana to deduce the best assignment form for you.
- **Zero-cost abstractions**
    - Never incur any hidden costs in native code
- **Advanced Enumeration**
    - Simple yet extensible enumerations with `enum`
    - Hierarchical, tree-like labels with `Tag`
        - Fast comparisons across hierarchy levels with `in`, `in?` and `==`
    - Sum types with `variant`
- **Compile-time Evaluation & Constraints**
    - Conditional compilation with `when`
    - Generic type constraints with `where`
    ```kotlin
    when std.BuildTarget == std.Platforms.Linux.x64 { ... }
    type Foo<T> { a: T => where std.Type.IsIntegral<T> }
    ```
- **Powerful Procedures**
    - First-class functions via Mana's flexible operator specialization model
    - Function constraints with Attributes such as `@[Inline]` and `@[Pure]`
    - Polymorphism for functions and operators
    - Built-in delegates
- **Powerful Type Interface System**
    - Create compile-time composite type interfaces with `interface for type Foo`
        - Keeps transformations and APIs separate from data
    - Name interfaces for separate applications on the same type (`interface Bar for type Foo`)
    - Import named interfaces by-module
        - Easily disambiguate colliding interfaces
    - Extend existing interfaces from anywhere
    - Lock type fields to make them read-only beyond interface bounds

See the [design documentation](design/Mana%20Language%20Design/) for the full language details. The documentation is an Obsidian vault, and so it is best read with Obsidian. However, the files are still Markdown, so they should be legible in any MD reader.

### Modules

| Module | Type | Description |
|--------|------|-------------|
| **Sigil** | Library | Shared, modular frontend, including semantic analysis for LSP  |
| **Circe** | Executable / Library | Bytecode compiler |
| **Hex** | Executable / Library | Register-based virtual machine |
| **Salem** | Executable | Native/AOT compiler |

### Building

**Requirements:** C++23 compatible compiler with computed-goto support (GCC 13+ or Clang 17+), CMake 3.28+

```bash
git clone https://github.com/mana-lang/Mana.git
cd mana

# Configure and build (Debug)
cmake -B .build/debug -DCMAKE_BUILD_TYPE=Debug
cmake --build .build/debug

# Or Release
cmake -B .build/release -DCMAKE_BUILD_TYPE=Release
cmake --build .build/release
```

Dependencies (`spdlog`, `Catch2`) are fetched automatically via CMake's `FetchContent`.

### Running
As CMake can output to arbitrary paths, this example assumes Hex and Circe's executables live in the same folder as the Mana source file, for simplicity.
```bash
# Compile a .mn file to bytecode and execute it
./circe hello-world.mn
./hex hello-world.hexe
```

### Running Tests
Mana is currently in the middle of an overhaul of its test suite. Instructions for running tests will be placed here in the future.

### Code Examples

See the [`assets/examples/`](assets/examples/) directory for example `.mn` files covering many of Mana's current and planned features.

**Be aware** that the `examples` folder contains Mana code as it is planned to work in the future; for code samples that will actually compile and run, see [`assets/samples/`](assets/samples/)

### Project Status

The current focus is on maturing the hot-reloadable side of Mana to be competitive with Lua as an embeddable scripting language. 

Salem will remain in stasis until Mana is a viable option as a scripting language for development with C++ game engines.

| Component | Status |
|-----------|--------|
| Lexer | ✅ Functional |
| Parser | ✅ Functional |
| Semantic Analysis | ✅ Functional |
| Bytecode Compiler (Circe) | 🟡 In progress |
| VM (Hex) | 🟡 In progress |
| LSP / Editor Support (Grimoire) | 🟡 In progress |
| Native Compiler (Salem) | 🔴 Future |
| Build System (Conjure) | 🔴 Future |

### Planned Ecosystem

- **Conjure** — Build system and package manager
- **Codex** — Debugging tools
- **Grimoire** — LSP, editor plugins, and formatters
- **Arcana** — Game development framework

<p align="center">
  <img src="design/Mana%20Language%20Design/_Assets/ManaEcosystem.png" alt="Mana ecosystem diagram showing Sigil, Circe, Hex, Salem, Conjure, Grimoire, Codex, and libraries" width="700">
</p>

### Project Model

Mana's build system, **Conjure**, organizes code into **Troves** and **Artifacts**.

**Artifacts** are *Virtual Translation Units* (VTUs) — Mana doesn't see source files directly. Instead, it treats everything within a named module as a single translation unit, allowing modules to be extended from anywhere.
<p align="center">
  <img src="design/Mana%20Language%20Design/_Assets/Info_Artifacts.png" alt="Diagram showing how source files compose into Artifacts (Virtual Translation Units)" width="800">
</p>

**Troves** are a named set of Artifacts; in other words, a package.
<p align="center">
  <img src="design/Mana%20Language%20Design/_Assets/Info_Troves.png" alt="Diagram showing how Artifacts are organized hierarchically within a Trove" width="800">
</p>

### Editor Support

A VS Code syntax highlighting extension is available in [`extensions/mana/`](extensions/mana/). This is the beginning of what will eventually become the **Grimoire** project. For now, simple syntax highlighting will do. Once the project is extended to include LSP support, Grimoire will be moved to its own repository.

### License
[MIT](LICENSE)