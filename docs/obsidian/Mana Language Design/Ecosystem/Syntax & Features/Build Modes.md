
##### Build Modes
Mana programs may be built in one of two modes: *checked* or *unchecked*. The mode is chosen at compile time, and has the same meaning for both **Circe** and **Salem**.

Some bugs can only be detected while a program is running. Mana defines a list of these, called *runtime-checked errors*.
- In a *checked* build, the compiler inserts a check before every operation that could cause a runtime-checked error. If a check fails, execution halts and reports where the error occurred.
- In an *unchecked* build, no checks are inserted. Where the language defines a result for the operation, that result is produced. Otherwise, the behaviour is *undefined*.

Checked builds trade performance for catching bugs where they happen.

> [!note]
> Build modes are unrelated to the static analysis performed by **Sceptre**. Static analysis happens at compile time, and only produces warnings; it never changes the code that gets generated.

##### Runtime-Checked Errors
| Error                                           | Unchecked behaviour |
| ----------------------------------------------- | ------------------- |
| [Integer overflow](Types.md#Integer%20Overflow) | Wraps around        |
| [Floating-point to integer conversion](Types.md#Conversions) out of range, or of NaN | Clamped to the range of the target type; NaN yields `0` |
| Integer division or remainder by zero           | Undefined behaviour |
| [Dereferencing `null_addr`](Pointers.md#Null)   | Undefined behaviour |
| [Out-of-bounds array index](Data.md#Lists)      | Undefined behaviour |

##### Compiler Flags
Compilers may offer flags that relax the guarantees the language makes about program behaviour. The behaviour described by this specification applies *only* to builds without such flags. Builds using them are *not* guaranteed to behave identically to default builds, nor across **Circe** and **Salem**.

Compiler flags are set per **Trove**, and apply *only* to the code written in it, wherever that code is compiled. A generic or inlined procedure keeps the flags of the Trove that declares it.

Such flags include:
- *Floating-point relaxation*: floating-point operations may be fused or reordered.
- *Primitive strict aliasing*: every primitive type is treated as [strict](Pointers.md#Strict%20Aliasing).

**Sceptre** warns where code passes pointers into a Trove whose flags make assumptions about them that the calling code does not.
