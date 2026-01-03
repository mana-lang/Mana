
##### Procedure
The term *procedure* is used as a catch-all term for *executable things* in **Mana**. This includes:
- Functions
- Constant Functions
- Pure Functions
- Type Interface Functions
- Polymorphic Functions
	- Multi-Functions
	- Operators
	- Generic Functions
- Closures
- Delegates
- Functors

In **Mana**, *everything is data*. This includes *functions*.

When you declare a function, you are creating *executable data*; it is helpful to think of **Mana** functions as no different from other data declarations.

For example, the two declarations below are identical.
```rust
fn Foo() -> i32 {
	return 5 + 6
}

data Foo: fn() -> i32 {
	return 5 + 6
}
```

A function in **Mana** may be thought of as data containing information about how to *process* other data.

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

##### The `fn` Type
The `fn` keyword denotes a special data declaration of type `fn`. The components of this type describe:
- What types of parameters the function takes (*if any*)
- What type of value the function returns (*if any*)
- A block of statements to be executed by the function (*if any*)

Data of type `fn` may be executed by putting it in an *invocation expression*, also known as a function call. In these docs we differentiate between the two because a conventional function is only one type of **Procedure**, all of which are covered in this document.

Data of type `fn` may be declared in *any* situation where data may be declared.
```rust
/// Type member
type Foo {
	a: f64
	b: fn(f64, string) -> i16
	c: string
}

/// Module scope
fn Bar(a: f64, b: string) -> i16 {
	fmt.Print("{a} | {b}")
}

data foo = Foo {.a = 95.4, .b = Bar, .c = "hey"}

// invoked via member access operator
foo.b(22.7, foo.c) // identical to Bar(22.7, "hey")


/// Parameter
fn Invoke(baz: fn(f64, string) -> i16, a: f64, b: string) {
	baz(a, b)
}

// both of these are effectively the same
Invoke(foo.b, foo.a, foo.c)
Invoke(Bar, 95.4, "hey")


/// Within a function block
fn Blep() {
	data x = 32
	fn Closure(): x -> i32 {
		fmt.PrintLine("{x}")
	}
	
	Closure()
}
```

##### Inline
Functions *may* be annotated with the `@[Inline]` *attribute* to declare the function *inlined*. 

An *inlined function* has two properties:
1. It will be expanded in-place at its call site instead of performing the usual function call
2. It will attempt to execute at compile time, if possible
	- In this case, inlined calls are replaced with their computed values instead 
```rust
@[Inline]
fn Sqrt(x: f64) -> f64 {
	return x * x
}
```
If a function *cannot* be inlined, the compiler *may* raise a *warning*. The function will then behave as normal.

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

1. They must be annotated with the `@[Pure]` *attribute*
2. They may *not* have *mutable references* as parameters
3. They may *not* have *mutable ownership* over their parameters
4. They may *not write* to *mutable data* that was not declared within their function body
5. They may *not* return *mutable references*

A pure function, in essence, is not allowed to have *side effects*. This means it cannot modify state outside of itself. This includes any data that would normally be modifiable by a type function.

Data local to the pure function's scope may be freely modified *within function bounds*.

Failing to meet a pure function's requirements results in a *compile error*.
```rust
@[Pure]            // OK -- immutable ref
fn DotProduct(v1, v2: &Vec2) {
	return v1.x * v2.x + v1.y * v2.y
}

// Pure often pairs well with Inline
@[Pure, Inline]
fn Sqrt(v: i32) {
	return v * v
}
```

##### Type Interface Functions
You may associate functions to types by creating *type interfaces*. 

Type interface functions are mostly like regular functions, save for a few key differences:
- They may be declared `mut`
	- `mut` functions may modify type members
		- This includes `@[Locked]` type members
	- They may *only* be invoked on mutable data of that type
- They may access members of that type via the `this` keyword
	- `this` may be omitted, and the members accessed with a leading access operator `.`
```rust
type Foo {
	a: i32 @[Locked]
	b: f64
	c: bool
}

interface Bar for type Foo {
	mut fn Baz() -> i32 {
		.a *= 3
		return .a
	}
	
	@[Pure, Inline]
	fn Fuzz() -> f64 {
		return .b * Pi / 180
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

##### Multi-Functions
Multi-functions are *polymorphic functions* which

##### Operators

##### Generic Functions

##### Closures

##### Delegates

##### Functors
Functors are *types* with a defined `()` operator.

```rust
type Functor {
	a: i32
	b: string
}

interface for type Functor {
	operator () => mut fn(x: i32) -> i32 {
		.a += x
		return .a
	}
}

data foo = Functor { .a = 12, .b = "hey" }
data bar = foo(32)
fmt.Print("{bar}")
```
> [!tip] Output
> 44