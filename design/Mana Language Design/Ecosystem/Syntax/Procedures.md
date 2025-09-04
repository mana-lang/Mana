The term *procedure* is used as a catch-all term for *executable things* in **Mana**. This includes:
- Functions
- Constant Functions
- Pure Functions
- Polymorphic Functions
	- Generic Functions
	- Multi-Functions
- Type Functions
- Closures
- Delegates
- Functors


##### Functions
Functions in **Mana** are declared with the `fn` keyword. 

They consist of a *name*, followed by a set of *parentheses* containing an *optional* list of *parameters*, separated by *commas*. They may also *optionally* specify a return type.

*All* valid variable names are *also* valid function names.

A parameter's type *must* be specified. Multiple parameters of the same type *may* be grouped under commas sharing the same type annotation.
 
If a return type is *not* specified, the return type is *deduced* to be *the first type returned*.

```rust
fn add(a: i32, b: i32) {
	return a + b
}

fn subtract(a, b: f32) -> in f32 {
	return a - b
}
```

Return statements *may* be empty. This means the function's return type is `null` and it returns nothing. The `null` return type *may* be specified, in which case returning any value from a function results in a compile error.
```rust
import std.io.print

// identical to 'fn print_if(b: bool) -> null'
fn print_if(b: bool) {
	if not b: return
	
	std.print("Printing")
}

fn main() {
	print_if(true)
}
```
>[!tip] Output
> Printing


The return type *may* be annotated with the `in` keyword to declare the function *inlined*. An inlined function has two properties:

1. It will be expanded in-place at its call site instead of performing the usual function call
2. It will attempt to execute at compile time, if possible
	- In this case, inlined calls are replaced with their compile-time values instead 

If a function *cannot* be inlined, the compiler will raise an error.
```rust
fn sqrt(x: f64) -> f64 in {
	return x * x
}

// even if you're omitting the return type, 
// you must still specify 'in'
// to inline a function
fn sqrt(v: Vec2) -> in {
	return Vec2 {
		v.x * v.x
		v.y * v.y
	}
}

// or if you're returning nothing at all
fn print_smth(smth: string) -> in {
	std.print(smth)
}
```

##### Constant Functions
*Constant functions* are written exactly the same as regular functions, with three major differences:

1. They must be annotated with the `const` keyword
2. They execute at compile time
3. Every part of a constant function is *also* `const`

Constant functions *must* be executable at compile time. This means:
- Any arguments passed to a constant function call *must* also be known at compile time. 
- The result of a constant function must be *interchangeable* with *constant data*.
- Constant functions are implicitly *inlined*, and the `in` keyword does *nothing*

Just like *constant data*, constant functions *must* have their return type specified.
```rust

// equivalent to 'const pi: f32 = 3.14159265358979'
const fn Pi() -> f64 {
	return 3.14159265358979
}

const fn ToRadians(degrees: f64) -> f64 {
	return degrees * Pi() / 180
}
```
