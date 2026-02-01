![](logo-text-black-bg.png)
<center><h6><i>A fast, safe, data-centric programming language
<br>purpose-built for game development</i></h6></center> 
---
**Mana** empowers game developers by making it easy to write fast, modular, and reliable code, while still having all the flexibility of a high-level scripting language. 

**Mana**'s motto is to have it both ways: *Write Fast, Run Fast.*

It achieves this by being a **hybrid** language whose runtime can be *selectively embedded* for high-level scripting, or compiled directly to machine code. This allows developers to get high performance while still being able to modify code in a running app, all without requiring any glue or hand-rolled scripting language.

Additionally, **Mana** is designed to let you write expressive and modular code *without* sacrificing performance. 

It is a data-centric language, meaning that everything in **Mana** can be thought of as data; even functions. This allows the developer to reason about their program as the flow of data and its transformations, rather than a series of logic statements. In many ways, **Mana** can be thought of as an imperative inversion of functional programming.

**Salem**, its native/AOT compiler, is designed to spot areas of poor performance based on their *data access patterns*, and can respond to this by warning the user about them, and even suggesting optimized data layouts, access patterns, and branch reduction to achieve the highest performance possible.

Meanwhile **Circe**, its bytecode/JIT compiler, is designed to recompile `@[Hot]` code modules as fast as possible. These are then executed by **Hex**, its low-overhead runtime/VM, which enables the user to *seamlessly prototype* their code as their program is running. All this *without* the need for any glue code, while still maintaining excellent performance.

All of **Mana**'s modules are completely *independent*, which means **Hex** can easily be integrated or embedded into an existing **C++** engine, so that **Mana** may be used as a complete scripting language instead.

**Mana** encourages writing high-performance code with its extremely modular and extensible *static type interface* system, compile-time *reflection* and *lazy evaluation*, and many *zero-cost abstractions*. It has a powerful hierarchical **Tag** system that simplifies complex control flow logic and state machines, *first-class functions* with a unique relationship to data, and a powerful *generic typing* system.

Despite its focus on high performance and expressiveness, **Mana** also does not forego safety. While not quite as safe as **Rust**, **Mana** still has *lifetime-based ownership* constructs and *data race protection*. It is also *statically typed*, allowing many bugs to be caught before your program even runs.

**Mana**'s ultimate goal is to be easy to integrate into existing projects, make it easy to write expressive *and* performant code with virtually no compile-cycle downtime, and make the transition from writing high-level, hot-reloadable scripts to low-level engine code as painless as possible.

