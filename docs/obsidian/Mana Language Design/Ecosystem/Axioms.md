##### Axioms
Mana is built on top of its **Fundamental Axioms**.

It is designed such that every rule either *is* an axiom, or follows from its axioms.

The axioms are designed to be applied *homogeneously*: each one applies to *everything* it describes, *without* special cases.

Each axiom states *invariants* and *defaults*. An invariant holds in *every* program. A default holds wherever the program writes nothing else.

A default is replaced *only* by writing its replacement: with unique, specific syntax, such as `mut` or `$`, or with an *attribute*. An attribute replaces defaults for the declaration, expression or scope it annotates. Types and invariants hold under every attribute.

##### Data
All data has a *type*. A type describes its data's size, alignment and representation in memory. Every type has a fixed size and alignment, and the type of *all* data is known at compile time.

Types *describe* data, and operators *transform* it.

Emergent behaviour:
- [The `fn` keyword](Syntax%20%26%20Features/Procedures.md#The%20%60fn%60%20Keyword) declares a type and instantiates it, so a procedure is data.
- A procedure without fields is zero bytes in size, and invoking it is [resolved from its type](Syntax%20%26%20Features/Procedures.md#Declaring%20%60fn%60).
- At runtime, a [pointer](Syntax%20%26%20Features/Pointers.md#Pointers) is *only* an address; the type it points to exists only at compile time.
- [Zero-size](Syntax%20%26%20Features/Types.md#Memory%20Layout) data occupies no storage.

##### One Value Model
Mana has *one* value model, which applies to *every* type. Initialization, assignment, copying, moving, taking addresses and operators behave the same way whether the data is a primitive, an array, a composite, a function, or a pointer.

Each category of type has its own [literal](Syntax%20%26%20Features/Types.md#Values) syntax. A type name on its own is *not* a value.

Emergent behaviour:
- Data of *every* type is [default-initialized](Syntax%20%26%20Features/Data.md#Default%20Initialization) in the absence of an initializer.
- A pointer may only be re-pointed if it is mutable, like any data.
- The captures of a closure are [fields](Syntax%20%26%20Features/Procedures.md#Closures), initialized like any fields.

##### Bindings
Data is held by *bindings*. Data held by a binding is *bound*. All other data is *unbound*: literals, the results of invocations, moved data (`$x`) and *address-values* (`&x`).

- Unbound data is *never* copied. It becomes bound by whatever it initializes.
- Unbound data is *immutable*, and its lifetime lasts until the end of the statement it appears in.
- Bound data that initializes or is assigned to a binding is *copied*.
- A binding holds data of its type from its declaration until it is invalidated or its lifetime ends.
- Mutability is a property of bound data. A binding is *immutable*, unless declared `mut`.
- The lifetime of a binding ends when it exits scope, and the lifetime of bound data ends with the binding holding it. Data is destroyed *exactly once*.

Emergent behaviour:
- A [parameter](Syntax%20%26%20Features/Procedures.md#Parameters) is a `data` declaration, initialized by its argument.
- A [move](Syntax%20%26%20Features/Data.md#Assignment%20Annotation) acts on bindings: it binds data to a new binding, and leaves its source invalidated or default-initialized.
- The result of an `operator[]` or unary `operator*` specialization that returns a pointer is [bound](Syntax%20%26%20Features/Procedures.md#Operators).
- An [alias](Syntax%20%26%20Features/Pointers.md#Aliases) is a binding to existing data, and holds no data of its own.

##### Operators
Every transformation of data is the invocation of an operator.

*Every* operator may be specialized. An operator is invoked *only* through its own syntax, by the language, or through a pointer to its specialization.

A specialization defines what an operator does to *data*. What it does to *bindings* is fixed by the language.

Emergent behaviour:
- The lifecycle of data is [five operators](Syntax%20%26%20Features/Procedures.md#Lifecycle%20Operators).
- A [conversion](Syntax%20%26%20Features/Types.md#Conversions) `T(x)` invokes `operator T`.
- A function is an instance of a type with an `operator()` specialization.
- A [binding operator](Syntax%20%26%20Features/Procedures.md#Freestanding%20Operators) may *only* be specialized with a receiver.

##### As Written
What is written is what happens.

- Optimizations *never* affect behaviour in an observable way, other than allowing the same behaviour to execute faster.
- Addresses and moves are *always* explicit.

Emergent behaviour:
- An [argument](Syntax%20%26%20Features/Procedures.md#Invocation%20Arguments) is passed *exactly* as written.
- [Assignment](Syntax%20%26%20Features/Data.md#Assignment%20Semantics) copies; `#`, `&` and `$` annotate any other handling. A copy written as `#x` is *never* elided.
- Floating-point operations are performed *exactly* as written.
- Intended [overflow](Syntax%20%26%20Features/Types.md#Integer%20Overflow) is stated with `@[Wrap]` or `@[Saturate]`.
- Uninitialized data is declared with `@[NoInit]`, a discarded value with `@[Discard]`, and a reinterpretation with `@[Reinterpret]`.
- Invocation through `()` *never* dereferences.

##### Access
Data is written *only* through *write access*. Write access originates from a mutable binding, and is carried by `mut &x` and by pointers of type `mut *T`.

Every operation preserves or drops access.

Emergent behaviour:
- `mut &x` [requires](Syntax%20%26%20Features/Pointers.md#Access) `x` to be mutable.
- The data `*p` yields is mutable *only* when `p` has write access.
- At deeper levels of indirection, write access may *only* be dropped if none of the higher levels have write access.
- A [conversion](Syntax%20%26%20Features/Pointers.md#Opaque%20Pointers) or [reinterpretation](Syntax%20%26%20Features/Pointers.md#Reinterpreting%20Pointers) of a pointer may not exceed the access of its source.
- Unbound data is immutable, so `5 += position` is a compile error.
- The `mut *This` specialization of an operator is used *only* where its result is written.

##### Expected Types
Wherever data of type `T` is expected, *any* expression producing a `T` is accepted.

Data of *another* type is accepted *only* through an *implicit conversion*, and the language invokes at most *one* conversion to reach an expected type.

Emergent behaviour:
- An invocation where data of type `T` is expected is [resolved in two steps](Syntax%20%26%20Features/Types.md#Conversions).
- A constant expression is converted at compile time, and is a compile error if its value cannot be represented in the expected type.
- Annotating a declaration with a [signature](Syntax%20%26%20Features/Procedures.md#The%20%60fn%60%20Keyword) checks a constraint, and leaves its type unchanged.
