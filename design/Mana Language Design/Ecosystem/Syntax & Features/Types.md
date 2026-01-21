	
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


Besides the primitive types, **Mana** also has some special type-related keywords.
```C#
	// same as u8, except when handling text
	// [byte] is interpreted differently from [u8]
	// it is simply treated as an array of 8-bit integers
byte

	// offers an interface over the [u8] type
string

	// unit type/non-type
	// data of this type may not be declared
	// and functions can not return this type
none
```
Attempting to create data of type `none` will result in a compile error. Setting a function's return type to `none` indicates it *may not* return any value; it would be equivalent to a `void` function in *C*.
##### Custom Types
You may also create your own types in **Mana**. You do so using the `type` keyword.

A user-defined type block consists of the type keyword and the type's name, followed by a block of *type members* or *fields*, each with their own type specified.
```rust
type Foo {
	a: i32
	b: f64
	c: string
}
```

Data of this type is declared as normal, and its members' data *must* be initialized individually in one of two ways:
```kotlin
// implicit
data foo = Foo {102, 43.3, "hello"}

// explicit
data bar = Foo {.b = 43.3
				.a = 102
				.c = "hello"}
```

For *implicit* initialization, the data members *must* be initialized in the *same* order they were declared in their type declaration, and *all* data members *must* be initialized.

*Explicit* initialization does *not* have these requirements. Any members left uninitialized in an explicit initializer will be *zeroed*.

Typically, it is preferable to use *explicit* initialization.

Both types of initialization are done via a *list*, which means they may either be separated by commas, or by newlines.

##### Default Values
You *may* declare a *default value* for a *type member*. Type members with default values will use their default value instead of their zero-value when omitted from an explicit initializer.
```rust
type Foo {
	a: i32
	b: f64 = 32.99
	c: string
}
```
```kotlin
data foo = Foo {.a = 102, .c = "hello"}
```

##### Locking Mutable Data
Mutable data may be annotated with the `@[Locked]` attribute to indicate that anything from an outer scope is not allowed to write to it. This is particularly useful inside custom types.

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
