	
##### Typing
All data in **Mana** has a *type*. 

A data type describes that data's size, how it's laid out in memory, and how its contents are to be interacted with.

**Mana** is *statically typed*, which means *all* types of *all* data *must* be known at compile time.
##### Primitive Types
Primitive types are also called *built-in types*. These types are part of the language itself, and they represent different shapes of the fundamental form of data: numbers.

While some types aren't really interacted with in numeric form, all *primitives* are fundamentally represented by some numeric value, since that is all that computers know.
```rust
	// integers of different signages and sizes
	// the numbers reflect their size in bits
u8, u16, u32, u64
i8, i16, i32, i64

	// platform-dependent size, either 32 or 64-bit
isize, usize

	// boolean type, may be either 'true' or 'false'
bool 

	// real numbers of different sizes
f32, f64

	// text types
[u8]   // treated by mana as utf-8 string
char   // unicode scalar value, 32 bits in size

	// function type
fn
```


Besides the primitive types, Mana also has some special type-related keywords.
```C#
	// same as u8, except when handling text
	// [byte] is interpreted differently from [u8]
	// it is simply treated as an array of 8-bit integers
byte

	// offers an interface over the [u8] type
string

	// unit type
none
```

##### Representation
Every type in **Mana** has a fixed *size* and *alignment*, both of which are known at compile time.

Primitives are represented as follows:
- *Integers* use *two's complement*, with the size given in their name. `isize` and `usize` are the size of a pointer on the target platform.
- *Real numbers* follow the *IEEE 754* standard. `f32` is single-precision, and `f64` is double-precision.
- `bool` takes up *1 byte*, and may *only* hold `0` (`false`) or `1` (`true`).
	- Converting an integer to `bool` yields `true` for *any* non-zero value; `bool(x)` is equivalent to `x != 0`.
- `char` takes up *4 bytes*, and may *only* hold a valid *Unicode scalar value*.
	- Strings do *not* consist of `char`s. A string is a UTF-8 encoded `[u8]`, and a `char` is a single *decoded* code point.
- `none` takes up *0 bytes*.

The byte order (*endianness*) of all data is that of the target platform.

##### Integer Overflow
Integer arithmetic whose result exceeds the range of its type *overflows*. By default, **Mana** treats overflow as a *bug*:
- In *checked* builds, overflow *traps*, halting execution. See [Build Modes](Build%20Modes.md).
- In *unchecked* builds, overflow *wraps* around. It is *never* undefined behaviour.

When overflow is intended, it must be stated *explicitly* with one of two attributes:
- `@[Wrap]` - the result wraps around.
- `@[Saturate]` - the result is clamped to the *minimum* or *maximum* value of its type, in *every* build.
```rust
data x: u8 = 250

data a = @[Wrap] x + 10     // 4
data b = @[Saturate] x + 10 // 255

data y: u8 = 3

data c = @[Wrap] y - 5      // 254
data d = @[Saturate] y - 5  // 0
```

Both attributes apply to `+`, `-` and `*`, as well as to signed division, where `i_min / -1` is the only operation that can overflow. 

They may be placed before a single *expression*, or before a *scope* or *function*, in which case they apply to all integer arithmetic within it.

> [!note]
> Saturating arithmetic is not free; it typically costs a comparison per operation. However, most CPUs offer native saturating instructions for SIMD operations, where it is often free.


##### None
`none` is a special keyword which is both a type and a value, but represents the *absence* of a value. In essence, `none`  is a value of type `none`. 

What this means is:
- Functions
	- You may specify `none` as the return type of a function, which will be the same as specifying no return type. 
	- A function with return type `none` can *only* return `none`.
	- The following two statements are equivalent:
		- `return`
		- `return none`
	- By default, operators for composite types are empty functions which return `none`.
		- While this means that for two bindings `a, b` of a type `T`, the operation `a + b` *would* compile (the result of that expression is `none`, the compiler *may* warn for such expressions, and can be configured not to.
- Data
	- You may assign `none` to a binding during initialization, and that will deduce its type to be `none`.  
		- You may annotate a binding with type `none`, which will default its value to `none`
		- This means the following statements are all equivalent:
			- `data x = none`
			- `data x: none`
			- `data x: none = none`
	- Data of type `none` cannot meaningfully be used in any expressions.
	- Data of type `none` can be represented with 0 bits of information.
- Pattern Matching
	- You may have *one* match arm that matches to `none`
	- This arm represents not having matched anything. When all other matches fail, a `match` block will match to `none`.

In short, `none` is the way with which you describe the *absence* of things in Mana.

##### Composite Types
Composite types can be created using the `type` keyword.

A **Composite Type** block consists of the type keyword and the type's name, followed by a block of *fields* (*type members*), each with their own type annotated.
```rust
type Foo {
	a: i32
	b: f64
	c: string
}
```
```kotlin
// implicit
data foo = Foo {102, 43.3, "hello"}

// explicit
data bar = Foo {.b = 43.3
				.c = "hello"} // 'a' will be 0
```

For *implicit* initialization, the fields *must* be initialized in the *same* order they were declared in their type declaration, and *all* data members *must* be initialized.

*Explicit* initialization does *not* have these requirements. Any fields left uninitialized in an explicit initializer will be *defaulted*.

Typically, it is preferable to use *explicit* initialization.

Both types of initialization are done via a *braced-list*, which means they may either be separated by commas, or by newlines.

If there is no *braced-list* in the declaration, *all* members are defaulted.
```kotlin
data bax = Foo
```
In this example:
- `a` will be `0`
- `b` will be `0.0`
- `c` will be `""`

##### Default Values
You *may* declare a *default value* for a field. Type members with default values will use their default value instead of their zero-value when omitted from an explicit initializer.
```rust
type Foo {
	a: i32
	b: f64 = 32.99
	c: string
}
```
```kotlin
data foo = Foo {.a = 102, .c = "hello"} // 'b' will be 32.99
```

##### Memory Layout
**Mana** lays out data in memory following the same rules as **C**. This makes **Mana** data directly compatible with **C** and **C++**, without any conversion.
- The fields of a composite type are laid out in their *declaration order*. Each field is placed at its natural alignment, with *padding* inserted where necessary.
- The elements of an array are *contiguous* in memory.
- The contents of padding bytes are *unspecified*. As a result, the default `operator==` for composite types compares them *field by field*, never byte by byte.

Since field order determines layout, the compiler will *warn* when the order of a type's fields wastes space on padding.
```rust
type Foo {
	a: i32   // 4 bytes + 4 padding
	b: f64   // 8 bytes
	c: bool  // 1 byte + 7 padding
	d: i64   // 8 bytes
	e: u16   // 2 bytes + 6 padding
}
```
>[!warning] Warning
> Type `Foo` takes up 40 bytes, 17 of which are padding. Reordering its fields could reduce its size to 24 bytes.

##### Locking Mutable Data
Fields may be annotated with the `@[Locked]` attribute to indicate that *only* mutable associated functions may modify them.

**Mana** does *not* have the concept of granular private access specifiers for individual type members. However, sometimes you want a value which may be modified by an interface function, but not by anything else. In these scenarios, you would want to use `@[Locked]`.
```rust
// 'a' may only be modified by interface functions, 
// even if `Foo` has been declared 'mut'
type Foo {
	a: i32 @[Locked]
	b: f64 = 32.99
	c: string
}

interface Bar for type Foo {
	mut fn Baz() -> i32 {
		.a += 2
		return .a
	}
}
```
Attempting to modify `[Locked]` data results in a *compile error*.

You may also specify a `[Locked]` block:
```rust
type Foo {
	@[Locked] {
		a: i32
		b: f64 = 32.99
	}
	
	c: string
}
```
> [!note]
`[Locked]` data is restricted to only be modifiable by interface functions. However, **Mana** allows anywhere to create an associated function for a type. `[Locked]` is merely an indicator that a type member *shouldn't* freely be modified, for some reason or another.
