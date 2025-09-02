
<h1><center><i>Mana Overview</i></center></h1>

---
**Mana** itself consists of several major components, as well as some tools to support a productive ecosystem.

![](ManaEcosystem.png)

##### Salem
**Salem** is the *native compiler*. It's what compiles **Mana** code to machine code. It's designed to be friendly, especially towards beginners, and is capable of both safety and performance checks, on top of the usual stuff you'd expect out of a compiler. 

**Salem** will also *automatically* embed **Hex** into any **Mana** executable that has [hot] code in it. This allows **Hex** to track and execute the relevant code. 

**Salem** is also its own mascot, a black cat.

##### Hex
**Hex** is the *virtual machine* that executes **Mana** *bytecode*. It's what allows [hot] code to be recompiled whilst the program is running. It does this by *tracking* any specified "hot files" and invoking **Circe** on the file when it notices a change. **Circe** will then recompile the relevant code so **Hex** can reload it.

##### Circe
**Circe** is the *bytecode JIT compiler*. It compiles [hot] **Mana** code to `.hexe` *bytecode* files that may be executed by **Hex** - also called *hexecutables*. 

Unlike **Salem**, **Circe** does not perform any special checking. It also does not reorganize data layouts or do anything else to optimize or speed up your code. This is because **Circe**'s primary purpose is to compile to bytecode as quickly as possible.

**Circe** can be invoked by **Salem**, **Hex**, or standalone.

**Circe**'s mascot is a magpie.

##### Sigil
**Sigil** is the parsing front-end shared between **Salem** and **Circe**. 


### Tools
##### Conjure
**Conjure** is a build system and package manager for **Mana**, akin to **Rust**'s **Cargo**. It helps developers set up and manage projects and dependencies.

**Mana** projects are called **Troves**, which are made up of **Artifacts**. **Artifacts** are *Virtual Translation Units*. This means **Mana** does not see source files; it instead acts like everything contained within a named module is a source file.

By default, modules are named after their source file, so you can still treat **Artifacts** like regular TUs if that's what you're used to.

While **Artifacts** may be accessed as if they're contained within one another, they are imported *stand-alone*. This means `import std.io.print` will *only* import the `print` **Artifact**.

##### Codex
**Codex** is the **Mana** debugger.
Likely not happening anytime soon.

##### Grimoire
**Grimoire** is the collective name for all **Mana** editing-related tools. LSP, editor plugins, auto-formatters; if it falls under "Mana text editing," it falls under the **Grimoire** project.

