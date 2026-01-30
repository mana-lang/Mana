
##### Procedure
The term *procedure* is used as a catch-all term for *executable things* in **Mana**. This includes:
- Functions
- Closures
- Delegates
- Invocators
- Associates

All procedures may also have the following **Attributes**:
- Constant
- Pure
- Inline
- Hot

Additionally, procedures can be *polymorphic*. This includes:
- Functions
- Operators


##### Executable Data
In **Mana**, *everything is data*. This includes *functions*.

When you declare a procedure, you are creating *executable data*; it is helpful to think of Mana functions as no different from other data declarations.

For example, the two declarations below are identical.
```rust
fn Foo() -> i32 {
	return 5 + 6
}

data Foo: fn = () -> i32 {
	return 5 + 6
}
```

A function in Mana may be thought of as data containing information about how to *process* other data.

Data which may be invoked to process other data is referred to as being *invocable*.

##### Functions
Functions in Mana are declared with the `fn` keyword. 

They consist of a *name*, followed by a *parameter-list* expression containing an *optional* list of *parameters*, separated by *commas*. They may also optionally specify a *return* type.

*All* valid datum names are *also* valid function names.

A parameter's type *must* be specified. Multiple parameters of the same type *may* be grouped under commas sharing the same type annotation.
 
If a return type is *not* specified, it is *deduced* to be `none`.

 Returning *any* value from a function with `none` return type results in a compile error.
```rust
import std.fmt

fn Subtract(a, b: f32) -> f32 {
	return a - b
}

fn PrintIf(b: bool) {
	if not b {
		return false // error
	}
	
	fmt.Print("Printing")
}

fn main() {
	PrintIf(true)
}
```
>[!danger] Error
> Function `PrintIf` attempted to return a value despite having `none` return type
##### Parameters
```rust
fn Foo(a: i32, b: mut &string, c: [f64, 5]) -> mut &string
```

##### The `fn` Type
The `fn` keyword denotes a special data declaration of type `fn`. The components of this type describe:
- What types of parameters the function takes (*if any*)
- What type of value the function returns (*if any*)
- A block of statements to be executed by the function (*if any*)

Data of type `fn` may be executed by putting it in an *invocation expression*, also known as a function call. In these docs we differentiate between the two because a conventional function is only one type of procedure, all of which are covered in this document.

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

##### Entry Point
A standalone Mana program will always start execution from the special `Main` function. Other functions may still be called from outside Mana, e.g. when using Mana for scripting; however, *Main* is the typical place for execution to begin and end.

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

##### Associated Functions
You may *associate* functions to types by creating *type interfaces*. 

Associate functions are mostly like regular functions, save for a few key differences:
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

// when multiple associate functions share the exact same signature, 
// and both are imported at once, 
// you must disambiguate them with the scope resolution operator '::'

interface Bru for type Foo {
	mut fn Baz() -> i32 {
		.a *= 27
		return .a
	}
}

data bru = foo.Bru::Baz()
data baz = foo.Baz()
```
>[!danger] Error
> Call to associate 'Baz()' is ambiguous. 
> Possible options:
> - Bru::Baz()
> - Bar::Baz() 
##### Polyfunctions
Polyfunctions are *polymorphic functions*, meaning that there are two or more functions that share the same name, but have different argument lists and, if so, may have different return types as well.

Each definition of a polyfunction is a *specialization* of that function.
```rust
fn PrintValue(a, b: i32) -> i32 {
	data p = a + b
	fmt.PrintLine("I'm an integer: {p}")
	
	return p
}

fn PrintValue(a, b: f32) -> f64 {
	data p = a + b
	fmt.PrintLine("I'm a decimal: {p}")
	
	return p
}

fn PrintValue(a: string) {
	fmt.PrintLine("I'm a string, I don't return anything: {a}")
}

fn Main() {
	data a = PrintValue(12, 34)
	data b = PrintValue(1.2, 34.5)
	
	PrintValue("Woah")
}
```
> [!tip] Output
> I'm an integer: 46
> I'm a decimal: 35.7
> I'm a string, I don't return anything: Woah
##### Operators
"Mana is data-centric. Everything in Mana is is data, even functions."

*But what does this actually mean?*

Mana's focus is exclusively on data and transformations. As part of this philosophy, the only way to transform data is through *operators*. 

The way to think about Mana's data-orientation is:
- Types *describe* data
- Bindings *contain* data
- Operators *transform* data

In essence; *functions in Mana are just specialized operators.*

By specializing an operator, you are describing to Mana how a certain type interacts with transformations.

You can specialize operators in the *interface block* of any given type. Because operators are the "source" of invocability in Mana, they do not contain the `fn` keyword in their declaration. Instead they bind directly to a *parenthesized-list* expression and have an optional return type. 
```rust
type Vec2 {
	x: f32
	y: f32
}

interface for type Vec2 {
	operator + => (other: &Vec2) -> Vec2 {
		return Vec2 {.x + other.x, .y + other.y}
	}
}

fn Main() {
	data a = Vec2 {23, 43}
	data b = Vec2 {97.1, 664.5}
	data c = a + b // 123.1, 707.5
}
```

By default, operators for composite types return `none`. To make them return something else, you have to specialize them.
```rust
type Vec3 {
	a: f32
	b: f32
	c: f32
}

fn Main() {
	data a = Vec3 {1, 2, 3}
	data b = Vec3 {4, 5, 6}
	data c = a + b
}
```
>[!danger] Error
> `Vec3::operator+` does not take two arguments because it is unspecialized.
> Consider writing a specialization in the interface for `Vec3`

##### Generic Polyfunctions
```rust
type T for
fn Add(a, b: T) -> T {
	return a + b
}

fn Sub<T>(a, b: T) -> T {
	return a - b
}
```

##### Closures
Closures are essentially syntax sugar for ad-hoc invocators.
- Defined with the `fn` keyword inside another invocable
- May capture surrounding data
    - If unspecified, capture is inferred by Mana’s type-checking semantics
- Capture semantics are part of the closure’s type
- May be named or anonymous
- Otherwise behave like functions

Tentative syntax:
```rust
data x = 32
data y = "hello"
data v = Vec2 {5, 10}

data closure = fn(a: i32) => [$x, &y, mut &v] -> i32 {
	fmt.Print(.y) // captures are members of the closure invocator
	
	.v.x = a
	.v.y = .x
	
	return a * x
}
```

Capturing surrounding type would have to be with a named `this`
```rust

// this: ID <- 'ID' becomes standin for `this`,
// because closure has its own `this`
data f = fn() => [mut this: foo] {
	.foo.something = SomethingElse()
}
```

##### Delegates
- Type-safe multicast invocable references
- May have an arbitrary number of invocables bound to them
- Invocation forwards to all bound invocables
- Lightweight and suitable for event-driven behavior
- Commonly used for event-driven behavior

##### Invocators
Invocators are *types* with a specialized `()` operator.

They have all the state of any user-defined type, with associated invocation behaviour. They can be thought of as functions that carry state.

```rust
type Invocator {
	a: i32
	b: string
}

interface for type Invocator {
	operator () => mut (x: i32) -> i32 {
		.a += x
		return .a
	}
}

data foo = Invocator { .a = 12, .b = "hey" }
data bar = foo(32)
fmt.Print("{bar}")
```
> [!tip] Output
> 44
