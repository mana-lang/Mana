
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
Hex is currently exclusively a bytecode interpreter, but the aim is to convert it into an embeddable Mana runtime that can either execute bytecode or JIT machine code. The aim is to maximize speed despite all the flexibility offered by Hex.

A medium-term goal is to have Mana & Hex have enough features that it can serve as a replacement for **Lua**. While Lua is a very powerful and ubiquitous language, it can be a little cumbersome to embed in an engine, as a lot of glue is required. The goal is to start by offering an excellent C++ interface to easily integrate Mana code into any C++ project, and be able to utilize it as a scripting language much like Lua, but with far less setup and glue, on top of being able to make use of Mana's game development-centric features. 

C++ was chosen as Mana's primary interface language because the vast majority of game engines are built with it. While the ultimate endgame for Mana would be for game engines to be written in it, this would never happen as long as Mana is neither mature and robust enough to be worth such a time investment, nor sees any widespread use. 

A far more realistic outcome is that Mana can potentially be adopted by smaller engines and game projects and, once people start seeing its value, perhaps one day larger engines can integrate it as an optional scripting language, before any serious engines would be built with it.

Due to its nature as a hybrid language, it should have some amount of ABI compatibility with C++, which would enable Mana code to gradually enter existing engines in scripting form, and eventually replace or extend entire binary modules (e.g. as a `.dll`). The runtime would remove a lot of the typical glue needed to make this go smoothly and work well.

Hex being such a low-overhead, high performance, and plug-and-play runtime would ensure that adoption isn't a complete fantasy.