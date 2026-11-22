
##### Build Modes
Mana programs may be built in one of two modes: *checked* or *unchecked*. The mode is chosen at compile time, and has the same meaning for both **Circe** and **Salem**.

Some bugs can only be detected while a program is running. Mana defines a list of these, called *runtime-checked errors*.
- In a *checked* build, the compiler inserts a check before every operation that could cause a runtime-checked error. If a check fails, execution halts and reports where the error occurred.
- In an *unchecked* build, no checks are inserted. Where the language defines a result for the operation, that result is produced. Otherwise, the behaviour is *undefined*.

Checked builds trade performance for catching bugs where they happen.

> [!note]
> Build modes are unrelated to the static analysis performed by **Sigil**. Static analysis happens at compile time, and only produces warnings; it never changes the code that gets generated.

##### Runtime-Checked Errors
| Error                                           | Unchecked behaviour |
| ----------------------------------------------- | ------------------- |
| [Integer overflow](Types.md#Integer%20Overflow) | Wraps around        |
| Integer division or remainder by zero           | Undefined behaviour |
| [Dereferencing `null_addr`](Pointers.md#Null)   | Undefined behaviour |

##### Compiler Flags
Compilers may offer flags that relax the language's guarantees, such as allowing floating-point operations to be fused or reordered. The behaviour described by this specification applies *only* to builds without such flags. Builds using them are *not* guaranteed to behave identically to default builds, nor across **Circe** and **Salem**.
