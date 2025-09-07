
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

	// non-type
null
```


Besides the primitive types, **Mana** also has some special type-related keywords.
```C#
	// same as u8, except when handling text
	// [byte] is interpreted differently from [u8]
	// it is simply treated as an array of 8-bit integers
byte

	// offers an interface over the [u8] type
string

	// non-type
	// data of this type may not be declared
	// and functions can not return this type
null
```

##### Custom Types
You may also create your own types in **Mana**. You do so using the `type` keyword.

A user-defined type block consists of the type keyword and the type's name, followed by a block of *type members* each with their own type specified.
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


##### Flexibility
Types may be annotated with the `Flexible` attribute. 

This allows **Salem** (but *not* **Circe**) to do manipulate the type under the hood to *optimize* it for *cache locality* based on context such as its access patterns.

In the below case, type `Foo` is taking up more space than necessary due to *alignment requirements*. It is taking up 40 bytes, while only using 23 bytes of data. On its own, 17 bytes of wasted data isn't that big of a deal; however, once you have 100K `Foo` objects that you are accessing regularly, you will encounter far more cache misses than necessary, leading to a degradation in performance.

These issues can compound and cause your game to run at lower frame-rates, which gives you less room to add fancy assets, effects and features.
```rust
type Foo {
	a: i32   // 4 bytes + 4 padding
	b: f64   // 8 bytes
	c: bool  // 1 byte + 7 padding
	d: i64   // 8 bytes
	e: u16   // 2 bytes + 6 padding
}
```

With the `[Flexible]` attribute, **Salem** can automatically reorder these type members to waste the least amount of space.

You can *still write the type the same way*, and **Salem** will reorder the type *under the hood*.

This reduces the padding down to 1: 
```rust
[Flexible]
type Foo {
	a: i32   // 4 bytes
	b: f64   // 8 bytes
	c: bool  // 1 byte
	d: i64   // 8 bytes
	e: u16   // 2 bytes + 1 padding
}
```

Because the type has been *reordered* to:
```rust
type Foo {
	b: f64   // 8 bytes
	d: i64   // 8 bytes
	a: i32   // 4 bytes
	c: bool  // 1 byte
	e: u16   // 2 bytes + 1 padding
}
```

Note that **Salem** will reorder *as few members as possible* to respect as much of the specified order as it can. As a result, the first two and last three members are still in alphabetical order, respectively.

Reordering is only one of the things the `[Flexible]` attribute does. The other is far bigger, and is related to *access patterns*.

Suppose you create a `[Foo, 5]`. Ordinarily, **Mana** orders that array as follows:

`[a][b][c][d][e][a][b][c][d][e][a][b][c][d][e][a][b][c][d][e][a][b][c][d][e]`

Each of the members are laid out sequentially in memory.

If the type is `[Flexible]` however, **Salem** might do something like this:

`[a][a][a][a][a][b][b][b][b][b][c][c][c][c][c][d][d][d][d][d][e][e][e][e][e]`

It *reordered* the array to optimize for accessing the same member in each `Foo`, rather than skipping over to the next `Foo` on every access.