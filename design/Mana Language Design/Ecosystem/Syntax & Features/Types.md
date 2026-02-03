	
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
