
<h1><center><i>Hex</i></center></h1>
<h4><center><b>Mana Runtime and Virtual Machine</b></center></h4>

---
**Hex** is the high-performance execution engine for the Mana language. It serves as the Virtual Machine (VM) that interprets and executes the register-based bytecode produced by **Circe**. In the future, Hex will also be extended with the capability to execute JITed native code.

Hex is designed as a register-based VM, which offers a more efficient mapping to modern hardware compared to traditional stack-based virtual machines. It consumes `.hexe` bytecode binaries, deserializing them into an executable format and managing the lifecycle of a Mana program's execution.

> [!tip] Execution Flow
> **.hexe file ➾ Deserialization ➾ Instruction Dispatch ➾ Cleanup**

> [!example] Architecture
> **⦿ Register-Based**
> **⦿ Computed-Goto Dispatch**
> **⦿ Variable Payload Instructions** 
> **⦿ Fixed-Size Payloads** *(16-bit unsigned)* 

### Core Components
The module consists of several key systems that facilitate the execution of Mana bytecode:

- **Interpreter**: The core execution loop that uses a dispatch mechanism to process opcodes. It manages an Instruction Pointer (IP) and a set of virtual registers that hold `Value` types.
	- `Value` types are a tagged union type that may optionally be treated as an array in its allocation process to minimize padding
	- Plans exist to manage `Value` allocations with an arena allocator in the future.
- **Disassembler**: A debugging tool that translates raw bytecode back into a human-readable format, displaying instructions, register indices, and resolved constant values.
- **Bytecode Deserializer**: Handles the reconstruction of the `Hexe` executable structure (a constant pool and instruction stream) from a binary `.hexe` file.

### Instruction Set
Hex implements a modern instruction set architecture:
- **Arithmetic**: `Add`, `Sub`, `Mul`, `Div`, `Mod`
- **Comparisons**: `Equals`, `Greater`, `Lesser`, etc.
- **Control Flow**: Relative jumps and conditional branching e.g.`JumpWhenTrue`, `JumpWhenFalse` as well as higher-level ops like `Halt` and `Return`
- **Memory & Constants**: `Move`, `LoadConstant`

### CLI & Debugging
The `hex` executable provides a driver for running Mana programs. When running in debug modes, it offers comprehensive execution metrics, including:
- **Deserialization Latency**: Time taken to load and transform the executable from disk.
- **Performance Metrics**: High-resolution timing of the execution phase.
- **Live Disassembly**: Optional logging of the bytecode stream as it is being processed to assist in compiler debugging.

As with all Mana modules, Hex utilizes a centralized `spdlog` instance for structured logging and error reporting.

### Goals & Future Considerations
Currently, Hex is exclusively a bytecode interpreter. The long-term goal is to develop it into an embeddable Mana runtime that can either execute bytecode or JIT machine code.

The aim is to maximize speed while maintaining Hex's runtime hot-reload flexibility.

A medium-term goal is to give Mana & Hex enough features & stability that it can serve as a practical replacement for **Lua**. While Lua is a very powerful and ubiquitous language, it can be cumbersome to embed into an engine, as a lot of glue is required. By offering an excellent C++ interface to seamlessly integrate Hex into any C++ project, Mana can be used as a scripting language much like Lua, but with far less setup and glue, on top of being able to make use of Mana's game development-centric features. 

C++ was chosen as Mana's primary interface language because it's also the language of choice for most game engines. While the ultimate goal would be to see its use in game engine development, Mana would need to see fairly widespread use for it to reach a level of maturity where it's robust enough to warrant such a massive investment. 

A far more pragmatic outcome is that Mana gets adopted by smaller engines and game projects, and larger engines could integrate it as an optional scripting language. From there, having native engine modules written in Mana (e.g.  as a `.dll`) becomes a trivial endeavor, as facilitating the integration of standalone Mana modules is one of Hex's primary features.

Hex is essentially the heart of Mana. It addresses the core problem Mana solves (fast execution and rapid iteration with expressive syntax) while sidelining the general issue with the adoption of new technologies. 

You can retain your existing workflows and gradually discover and adopt Mana, all thanks to Hex.