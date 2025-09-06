##### Procedure
The term *procedure* is used as a catch-all term for *executable things* in **Mana**. This includes:
- Functions
- Constant Functions
- Pure Functions
- Type Functions
- Polymorphic Functions
	- Generic Functions
	- Multi-Functions
	- Operators
- Closures
- Delegates
- Functors

##### Functions
Functions in **Mana** are declared with the `fn` keyword. 

They consist of a *name*, followed by a set of *parentheses* containing an *optional* list of *parameters*, separated by *commas*. They may also *optionally* specify a return type.

*All* valid variable names are *also* valid function names.

A parameter's type *must* be specified. Multiple parameters of the same type *may* be grouped under commas sharing the same type annotation.
 
If a return type is *not* specified, it is *deduced* to be the type of the *first* return expression.
If no return expression exists, the return type is deduced to be *null*.
```rust
fn Add(a: i32, b: i32) {
	return a + b
}

fn Subtract(a, b: f32) -> f32 {
	return a - b
}
```

 The `null` return type *may* be specified, in which case returning any value from a function results in a compile error.
```rust
import std.fmt

fn PrintIf(b: bool) -> null {
	if not b: return false // error
	
	fmt.Print("Printing")
}

fn main() {
	PrintIf(true)
}
```
>[!danger] Error
> Function `PrintIf` attempted to return a value despite having `null` return type
##### Parameters
```rust
fn Foo(a: i32, b: mut &string, c: [f64, 5]) -> mut &string
```
##### Inline
Functions *may* be annotated with the `[Inline]` *attribute* to declare the function *inlined*. 

An *inlined function* has two properties:
1. It will be expanded in-place at its call site instead of performing the usual function call
2. It will attempt to execute at compile time, if possible
	- In this case, inlined calls are replaced with their computed values instead 
```rust
[Inline]
fn Sqrt(x: f64) -> f64 {
	return x * x
}
```
If a function *cannot* be inlined, the compiler will raise an error.

##### Constant Functions
*Constant functions* are written exactly the same as regular functions, with three major differences:

1. They must be annotated with the `const` keyword
2. They execute at compile time
3. Every part of a constant function is *also* `const`

Constant functions *must* be executable at compile time. This means:
- Any arguments passed to a constant function call *must* also be known at compile time. 
- The result of a constant function must be *interchangeable* with *constant data*.
- Constant functions are implicitly *inlined*, even without `[Inline]`

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

##### Pure Functions
*Pure functions* are written exactly the same as regular functions, with five major differences:

1. They must be annotated with the [Pure] *attribute*
2. They may *not* have *mutable references* as parameters
3. They may *not* have *mutable ownership* over their parameters
4. They may *not write* to *mutable data* that was not declared within their function body
5. They may *not* return *mutable references*

A pure function, in essence, is not allowed to have *side effects*. This means it cannot modify state outside of itself. This includes any data that would normally be modifiable by a type function.

Data local to the pure function's scope may be freely modified *within function bounds*.

Failing to meet a pure function's requirements results in a *compile error*.
```rust
[Pure]             // OK -- immutable ref
fn DotProduct(v1, v2: &Vec2) {
	return v1.x * v2.x + v1.y * v2.y
}

// Pure often pairs well with Inline
[Pure, Inline]
fn Sqrt(v: i32) {
	return v * v
}
```


##### Type Functions
```rust
type Foo {
	a: i32
	b: f64
	c: bool
}

interface Bar for type Foo {
	mut fn Baz() -> i32 {
		this.a *= 3
		return this.a
	}
	
	[Pure, Inline]
	fn Fuzz() -> f64 {
		return this.b * Pi / 180
	}
}

// if only one interface exists for a type
// it gets imported automatically when that type is imported
// otherwise, you must explicitly import the interface
// to be able to use it with the type
// 'import SomeModule.Foo::Bar'
// or all of them with 'import SomeModule.Foo::*'

data foo = Foo {1, 2, false}
data x = foo.Fuzz()
```


**TODO:** 
- **describe attribute-specifiers**
- **add "attribute groups" where commonly used sets of attributes can be grouped under a new name**