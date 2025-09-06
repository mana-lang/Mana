
##### Typing
All data in **Mana** has a *type*. 

A data type describes that data's size, how it's laid out in memory, and how its contents are to be interacted with.

To begin with, there are several *primitive types* in **Mana**:
```rust
	// integers of different signages and sizes
	// the numbers reflect their size in bits
u8, u16, u32, u64
i8, i16, i32, i64

	// platform-dependent size, either 32 or 64-bit
isize, usize

	// same as u8, except when handling text
	// [byte] is interpreted differently from [u8]
byte

	// boolean type, may be either 'true' or 'false'
bool 

	// real numbers of different sizes
f32, f64

	// text types
[u8]   // treated by mana as utf-8 string
string // interface for [u8]
char   // unicode scalar value, 32 bits in size

	// non-type
null
```

**Mana** is *statically typed*, which means *all* types of *all* data *must* be known at compile time.
