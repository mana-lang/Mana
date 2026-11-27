##### Typing
All data in Mana has a *type*. 

A data type describes that data's size, how it's laid out in memory, and how its contents are to be interacted with.

Mana is *statically typed*, which means *all* types of *all* data *must* be known at compile time.

##### Values
Mana has *one* value model, which applies to *every* type. Initialization, assignment, copying, moving, taking addresses and operators behave the same way whether the data is a primitive, an array, a composite, a function, or a pointer.

Wherever data of type `T` is expected, *any* expression producing a `T` is accepted.

Each category of type has its own *literal* syntax:
```rust
24, 3.5, "hi"               // primitive
[5, 10, 15]                 // array
Foo {.a = 102}              // composite
Colour.Red                  // enumerated
Action.Jump {.height = 2.0} // variant
fn(x: i32) -> i32 { }       // function
null_addr                   // pointer
```
A type name on its own is *not* a value. `T(x)` is a *conversion*, not a literal.

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
Every type in Mana has a fixed *size* and *alignment*, both of which are known at compile time.

Primitives are represented as follows:
- *Integers* use *two's complement*, with the size given in their name. `isize` and `usize` are the size of a pointer on the target platform.
- *Real numbers* follow the **IEEE 754** standard. `f32` is single-precision, and `f64` is double-precision.
	- Floating-point operations are performed *exactly* as written, unless relaxed by a [compiler flag](Build%20Modes.md#Compiler%20Flags).
- `bool` takes up *1 byte*, and may *only* hold `0` (`false`) or `1` (`true`).
	- Converting an integer to `bool` yields `true` for *any* non-zero value; `bool(x)` is equivalent to `x != 0`.
- `char` takes up *4 bytes*, and may *only* hold a valid *Unicode scalar value*.
	- Strings do *not* consist of `char`s. A string is a UTF-8 encoded `[u8]`, and a `char` is a single *decoded* code point.
- `none` takes up *0 bytes*.

The byte order (*endianness*) of all data is that of the target platform.

### Integer Overflow
Integer arithmetic whose result exceeds the range of its type *overflows*. By default, Mana treats overflow as a *bug*:
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

Both attributes apply to `+`, `-` and `*`, to signed division, where `i_min / -1` is the only operation that can overflow, and to [conversions](#Conversions) between integer types. 

They may be placed before a single *expression*, or before a *scope* or *function*, in which case they apply to all integer arithmetic within it.

> [!note]
> Saturating arithmetic is not free; it typically costs a comparison per operation. However, most CPUs offer native saturating instructions for SIMD operations, where it is often free.

##### Conversions
`T(x)` *converts* `x` to type `T` by invoking the *conversion operator* `operator T` on `x`. If `x` is already of type `T`, `T(x)` yields `x`. Otherwise, without such a specialization, `T(x)` is a compile error. A type that begins with a punctuator is parenthesized, as in `(*i32)(x)`.

A conversion operator is declared like any [operator specialization](Procedures.md#Operators). Its operand is its *only* parameter, and it returns `T`, so its return type is not written.

Data is accepted where data of *another* type is expected *only* if an *implicit conversion* to that type exists. A conversion operator annotated with `@[Implicit]` is an implicit conversion: the language invokes it wherever data of type `T` is expected. The language invokes at most *one* conversion to reach an expected type.
```rust
interface for type Kelvin {
	operator Celsius as (*This) {
		return Celsius {.degrees - 273.15}
	}

	@[Implicit]
	operator f64 as (*This) {
		return .degrees
	}
}

data boiling = Kelvin {373.15}
data celsius = Celsius(boiling) // invokes 'operator Celsius'
data degrees: f64 = boiling     // the language invokes 'operator f64'
data invalid: Celsius = boiling // error: no implicit conversion from 'Kelvin' to 'Celsius'
```

Between primitives, the standard library declares the following conversion operators:
- *Integer to integer* yields the same value. If the value is outside the range of the target type, the conversion [overflows](#Integer%20Overflow). It is implicit when the target type can hold *every* value of the source type.
- *Integer to floating-point* yields the nearest representable value. It is implicit when *every* value of the source type is represented exactly: from integers of up to 16 bits to `f32`, and of up to 32 bits to `f64`.
- *Floating-point to floating-point* yields the nearest representable value. It is implicit from `f32` to `f64`.
- *Floating-point to integer* discards the fraction. If the result is outside the range of the target type, or the value is NaN, the conversion traps in *checked* builds; in *unchecked* builds it is clamped to the range of the target type, and NaN yields `0`.
- *Integer to `bool`* yields `true` for any non-zero value.
- *`bool` to integer* yields `0` or `1`.

`byte` converts as an 8-bit unsigned integer type. `isize` and `usize` have *no* implicit conversions to or from other primitive types.

An integer literal is of type `i64`, and a literal with a decimal point is of type `f64`.

Where data of type `T` is expected, an invocation, including that of an operator, is resolved in two steps. First, the specialization that returns `T` is invoked, if one accepts its arguments. Otherwise, its arguments select the specialization by their types, and the result is converted to `T`. A constant expression is resolved the same way; its value is converted *only* in the second step.

Within `T(x)`, data of type `T` is expected of `x`, so `x` is resolved in the same two steps, except that its second step may invoke *any* conversion operator to `T`.

When arguments select a specialization, one they match *without* converting data is chosen over one that requires a conversion. A constant argument matches a parameter of an integer type when it is of an integer type, and a parameter of a floating-point type when it is of a floating-point type, if its value can be represented in it. If more than one specialization matches equally, the invocation is a compile error.

A [constant](Data.md#Constants) expression is converted at *compile time*. If its value can be represented in the expected type, it is treated as if it were of that type to begin with; a floating-point type represents it rounded to the nearest value. Otherwise, the conversion is a compile error.
```rust
data x: i32 = 5

data a = 7 / 2          // i64: 3
data b: f64 = 7 / 2     // 3.5
data c: f64 = x * 2     // 10.0
data d: u8 = 255
data e: f32 = 1.5
data half = f64(7 / 2)  // 3.5
data small = u8(x)      // 5

data f: u8 = 300        // error: 300 cannot be represented as 'u8'
data g: u8 = x          // error: no implicit conversion from 'i32' to 'u8'
data h: string = 7 / 2  // error: no implicit conversion from 'i64' to 'string'
```

##### None
`none` is a special keyword which is both a type and a value, but represents the *absence* of a value. In essence, `none`  is a value of type `none`. 

What this means is:
- Functions
	- You may specify `none` as the return type of a function, which will be the same as specifying no return type. 
	- A function with return type `none` can *only* return `none`.
	- The following two statements are equivalent:
		- `return`
		- `return none`
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

A *composite-type-block* consists of the type keyword and the type's name, followed by a *field-list*.

A *field-list* consists of *field-declarations*, which consist of a name, followed by a `:` colon, followed by a type annotation.
```rust
type Foo {
	a: i32
	b: f64
	c: string
}
```

In a *field-list* or a [parameter-list](Procedures.md#Parameters), names may be grouped under commas to share a single annotation. Everything right of the colon is shared by the group, while anything left of a name applies *only* to that name.
```rust
type Vec2 {
	x, y: f32
}

fn Blend(mut a, b: Colour) { } // only 'a' is mutable
```

The mutability of each field is determined by the mutability of its instance. A *field-list* only *declares* fields, which are instantiated along with their owner, so fields may *not* be declared `mut`

For *implicit* initialization, the fields *must* be initialized in the *same* order they were declared in their type declaration, and *all* data members *must* be initialized.

*Explicit* initialization does *not* have these requirements. Any fields left uninitialized in an explicit initializer will be *defaulted*, unless annotated with `@[NoDefault]`.

Typically, it is preferable to use *explicit* initialization.

Both types of initialization are done via a *braced-list*, which means they may either be separated by commas, or by newlines.
```kotlin
// implicit
data foo = Foo {102, 43.3, "hello"}

// explicit
data bar = Foo {.b = 43.3
				.c = "hello"} // 'a' will be 0
```

A declaration without an initializer defaults *all* members. So does an empty *braced-list*.
```kotlin
data bav: Foo
data bax = Foo {} // identical
```
In both cases:
- `a` will be `0`
- `b` will be `0.0`
- `c` will be `""`

To store an *address-value* in a field, it must be a [pointer](Pointers.md). 
```rust
type Foo {
	a: *i32
	b: mut *f64
	c: string
}
```

##### Default Values
You *may* declare a *default value* for a field. Type members with default values will use their default value instead of their zero-value when omitted from an explicit initializer.

Fields annotated with `@[NoDefault]` *must* be provided by every initializer. Annotating a *type* with `@[NoDefault]` applies it to all of its fields without a default value. Data of such a type cannot be default-initialized.
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

##### Type Aliases
A *type-alias* is another name for an existing type, declared with `type alias`.

A *type-alias* and the type it names are the *same* type.
```kotlin
type alias Health = i32
type alias AddingFunction = *operator() as (i32, i32) -> i32
```

##### Memory Layout
Mana lays out data in memory following the same rules as **C**. This makes Mana data directly compatible with **C** and **C++**, without any conversion.
- The fields of a composite type are laid out in their *declaration order*. Each field is placed at its natural alignment, with *padding* inserted where necessary.
- The elements of an array are *contiguous* in memory.
- A type is zero-size when its data needs no storage: `none`, a type whose fields are all zero-size, a fieldless composite type, or an array of zero length or of zero-size elements. Bindings of such a type occupy no storage. Every instance of such a type has the same address, unique to that type. Zero-size data takes no part in the layout of what contains it, and its address is that of its own type.
- The contents of padding bytes are *unspecified*. As a result, the default `operator==` for composite types compares them *field by field*, never byte by byte.

Since field order determines layout, **Sceptre** will *warn* when the order of a type's fields wastes space on padding.
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
Fields may be annotated with the `@[Locked]` attribute to indicate that *only* methods may modify them, through a `mut *This` receiver.

Mana does *not* have the concept of granular private access specifiers for individual type members. However, sometimes you want a value which may be modified by an interface function, but not by anything else. In these scenarios, you would want to use `@[Locked]`.
```rust
// 'a' may only be modified by interface functions, 
// even if `Foo` has been declared 'mut'
type Foo {
	a: i32 @[Locked]
	b: f64 = 32.99
	c: string
}

interface Bar for type Foo {
	fn Baz(mut *This) -> i32 {
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
`[Locked]` data is restricted to only be modifiable by interface functions. However, Mana allows anywhere to create a method for a type. `[Locked]` is merely an indicator that a type member *shouldn't* freely be modified, for some reason or another.
