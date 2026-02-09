
##### Procedure
The term *procedure* is used as a catch-all term for *executable things* in **Mana**. This includes:
- Functions
- Associates
- Invocators
- Closures
- Delegates

Procedures *may* also have the following **Attributes**:
- Pure
- Inline
- Hot
- Discardable

Certain procedures may be qualified with the `mut` or `const` keywords.

Additionally, procedures can be *polymorphic*.

##### Functions
Functions in Mana are typically declared with the `fn` keyword. 

They consist of a *function signature* followed by a *function body*.

Function signatures consist of a *name*, followed by a *parameter-list*, followed by an *optional* return type. 

The *parameter-list* consists of a set of parentheses containing an *optional*, comma-separated list of *parameters*.

A *parameter* consists of a *name*, followed by a colon, followed by a *type-name*.

A parameter's type may *not* be omitted. Multiple parameters of the same type *may* be grouped under commas sharing the same type annotation.
 
If a *return* type is *not* specified, it is *assumed* to be `none`.
Returning any value *other* than `none` from such a function results in a compile error.

The *function body* consists of a *scope* whose contents take one of two forms:
- Zero or more statements
- Only and *exactly* one expression

Should the function body contain only and exactly one expression, then the function will return that expression. This expression's evaluated type *must* then also match the function's return type.

*All* valid binding names are *also* valid function names.
```rust
import std.fmt

fn Subtract(a, b: f32) -> f32 {
	a - b
}

fn PrintIf(b: bool) {
	if not b {
		return false // error
	}
	
	fmt.Print("Printing")
}

fn Main() {
	PrintIf(true)
}
```
>[!danger] Error
> Function `PrintIf` attempted to return a value despite having `none` return type

##### Parameters
```rust
fn Foo(a: i32, b: mut &string, c: [f64, 5]) -> mut &string
```

##### Polyfunctions
Polyfunctions are *polymorphic functions*, meaning that there are two or more functions that share the same name, but have different parameters -- and, if so, may have different return types as well.

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

##### Generic Polyfunctions
```rust
type T for
fn Add(a, b: T) -> T {
	a + b
}

// shorthand, for when there are no constraints
fn Sub<T>(a, b: T) -> T {
	a - b
}
```

##### Associates
You may *associate* functions to types by creating *type interfaces*. 

Associate functions are mostly like regular functions, save for a few key differences:
- They may be declared `mut`
	- `mut` functions may modify type members
		- This includes `@[Locked]` type members
	- They may *only* be invoked on *mutable* bindings of that type
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
		.b * Pi / 180
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

interface Bem for type Foo {
	mut fn Baz() -> i32 {
		.a *= 27
		return .a
	}
}

data bru = foo.Bem::Baz()
data baz = foo.Baz()
```
>[!danger] Error
> Call to associate `foo.Baz()` is ambiguous. 
> Possible options:
> - Bem::Baz()
> - Bar::Baz() 

##### Operators
Operators in Mana are specializable polyfunctions.

You can specialize operators in the *interface block* of any given type. 

Operator specializations do *not* use the `fn` keyword in their declaration; instead, they bind directly to a *parameter-list* and have an *optional* return type. 
```rust
type Vec2 {
	x: f32
	y: f32
}

interface for type Vec2 {
	operator+ => (other: &Vec2) -> Vec2 {
		Vec2 {.x + other.x, .y + other.y}
	}
}

fn Main() {
	data a = Vec2 {23, 43}
	data b = Vec2 {97.1, 664.5}
	data c = a + b // 123.1, 707.5
}
```

By default, *most* operators for composite types return `none` and take no parameters. To make them return something else, you have to specialize them.

Comparison operators, however, check for equality among all fields.
```rust
type Vec3 {
	a: f32
	b: f32
	c: f32
}

fn Main() {
	data a = Vec3 {1, 2, 3}
	data b = Vec3 {4, 5, 6}
	data c = a == b // c == false
	data d = a + b
}
```
>[!danger] Error
> `Vec3::operator+` does not take two arguments because it is unspecialized.
> Consider writing a specialization in the interface for `Vec3`

##### Invocators
"Invocator" is just a term for types with a specialized `operator()`

A binding of an invocator is invocable like any other function. However, they carry state like any binding.
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

mut data foo = Invocator { .a = 12, .b = "hey" }
data bar = foo(32)
fmt.Print("{bar}\n{foo.a}")
```
> [!tip] Output
> 44
> 44

##### Closures
Closures are essentially syntax sugar for ad-hoc invocators.
- Defined with the `fn` keyword inside another invocable
- May capture surrounding data with a *capture clause*
	- Capture clause follows a binding operator after the return type, but before the function's scope
	- Captured bindings may contain assignment operators
		- In their absence, captures follow Mana's *assignment deduction rules*.
	- Captured bindings are part of the closure's type
	- Closure's size is *at least* the size of all its captures
	- Closure will store captures in the order that requires the *least* amount of padding
- May be named or anonymous
- Will deduce the return type to the first return expression, if return type is left unspecified
- Otherwise behave like functions

```kotlin
data x = 32
data y = "hello"
data v = Vec2 {5, 10}

// closures may be named
data closure = fn(a: i32) -> i32 => x, y, mut &v {
	fmt.Print(.y) // captures are members of the 'closure' invocator
	
	.v.x = a
	.v.y = .x
	
	return a * x
}
@[Discard] closure(55)

// or anonymous	
foo.Filter(fn() => x, &y {.x % .y})
		.Map(fn(t: i32) => x, mut &v {t - .x + .v.Cycle()})

```

You can capture a reference to the surrounding type with an alias to `this`
```rust
// this: ID <- 'ID' becomes standin for enclosing `this`,
// because closure has its own `this`
.Handle(fn() => x, mut this: foo {
	foo.something = SomethingElse(.x)
})
```

##### Executable Data
"Mana is data-centric. Everything in Mana is data, *including functions*."

*But what does this actually mean?*

Mana's focus is exclusively on data and its transformations. As part of this philosophy, the *only* way to transform data is through *operators*. 

The way to think about Mana's data-centrism is:
- Types *describe* data
- Bindings *contain* data
- Operators *transform* data

In essence; *functions in Mana are just specialized operators.*

By specializing an operator, you are describing to Mana how a certain type ought to be transformed in expressions containing that operator.

This does raise a question, though: 

*What about `fn`?*

##### The `fn` Type
The `fn` keyword denotes a special data declaration of type `fn`. 
`fn` is a special primitive whose sole purpose is to describe "normal functions."

What makes `fn` special is that it is a type which allows you to concisely declare `operator()` specializations for it.

The two declarations below are identical.
```rust
fn AddSix(a: i32) -> i32 {
	a + 6
}

data AddSix = fn(a: i32) -> i32 {
	a + 6
}
```

This is the first thing that `fn` makes convenient for us. What this assignment is masking is the differentiation between the `fn` type and the `fn` value. Below is the same declaration, but without any type deduction:
```rust
data AddSix: fn(i32) -> i32 => fn(a: i32) -> i32 {
	a + 6
}
```
It very quickly turns into a mess. 

Additionally, you may have noticed that this declaration is somewhat recursive; `fn` is both a type as well as a value. Why is this?

This is related to how `fn` is a type. Consider the syntax that instantiates a compound type:
```rust
data x: Foo = Foo {1, 2, 3}
```

In this case, we are also writing `Foo` twice; `Foo` is the type, and `Foo {1, 2, 3}` is a value of that type. `fn` is no different, because it is itself a type. 

The type form is the function signature, which you would write when storing a particular kind of function. The value form still includes the `fn` keyword to make it clear that we are instantiating a function.

To get an idea of what an `fn` declaration does under the hood, we can rewrite the above declaration to look more like a "normal" invocator:
```kotlin
interface __Foo for type fn {
	operator() => (a: i32) -> i32 {
		a + 6
	}
}

data Foo: __Foo
```

Which then lets us write:
```kotlin
data x = Foo(6) // x == 12
```

In essence, `fn` exists to preserve Mana's philosophy while still allowing us to conveniently write "standalone" functions. It's a keyword representing a type which may be instantiated with special syntax.

##### Declaring `fn`
Data of type `fn` may be declared in *any* situation where data may be declared. When declaring a field of type `fn`, its signature *must* be included.
```rust
/// Field
type Foo {
	a: f64
	b: fn(f64, string) -> i16
	c: string
}

/// Module (global) scope
fn Bar(a: f64, b: string) -> i16 {
	fmt.Print("{a} | {b}")
	return 5
}

data foo = Foo {.a = 95.4, .b = Bar, .c = "hey"}

// invoked via member access operator
foo.b(22.7, foo.c) // identical to Bar(22.7, "hey")


///            Parameter
fn Invoke(baz: fn(f64, string) -> i16, a: f64, b: string) {
	baz(a, b)
}

// both of these are effectively the same
Invoke(foo.b, foo.a, foo.c)
Invoke(Bar, 95.4, "hey")


/// Within function scope, as a closure
fn Blep() {
	data x = 32
	fn Closure() => x {
		fmt.PrintLine("{x}")
		return x * 2
	}
	
	data y = Closure() // y == 64
}
```

Any specific `fn` signature may be aliased:
```kotlin
type AddingFunction = fn(i32, i32) -> i32

data Add = AddingFunction(a, b: i32) -> i32 { a + b }

data x = Add(12, 34) // x == 46
```

This is particularly useful when repeatedly storing the same kind of function signature in a field, or when passing around the same kind of function signature among functions.
##### Delegates
- Type-safe (multicast) invocable references
- May have an arbitrary number of invocables bound to them
- Invocation forwards to all bound invocables
- Lightweight 
- Commonly used for event-driven behavior

##### Entry Point
A standalone Mana program will always start execution from the special `Main` function. Other functions may still be called from outside Mana, e.g. when using Mana for scripting; however, *Main* is the typical place for execution to begin and end.

In the future, there are considerations to have an `@[Entry]` attribute, which could override the default-expected entry point.

##### Inline
Functions *may* be annotated with the `@[Inline]` *attribute* to declare the function *inlined*. 

An *inlined function* has two properties:
1. It will be expanded in-place at its call site instead of performing the usual function call
2. It will attempt to execute at compile time, if possible
	- In this case, inlined calls are replaced with their computed values instead 
```rust
@[Inline]
fn Sqrt(x: f64) -> f64 {
	x * x
}
```
If a function *cannot* be inlined, the compiler *may* raise a *warning*. The function will then behave as normal.

##### Constant Functions
*Constant functions* are written exactly the same as regular functions, with three major differences:

1. They must be qualified with the `const` keyword
2. They execute at compile time
3. Every part of a constant function is *also* `const`

Constant functions *must* be executable at compile time. This means:
- Any arguments passed to a constant function call *must* also be known at compile time. 
- The result of a constant function must be *interchangeable* with *constant data*.
- Constant functions are implicitly *inlined*, even without `@[Inline]`

Just like *constant data*, constant functions *must* have their return type specified.
```rust
// equivalent to 'const pi: f64 = 3.14159265358979'
const fn Pi() -> f64 {
	3.14159265358979
}

const fn ToRadians(degrees: f64) -> f64 {
	degrees * Pi() / 180
}
```

##### Pure Functions
*Pure functions* are written exactly the same as regular functions, with five major differences:

1. They must be annotated with the `@[Pure]` *attribute*
2. They may *not* have *mutable references* as parameters
3. They may *not* have *mutable ownership* over their parameters
4. They may *not write* to *mutable data* that was not declared within their function body
5. They may *not* return *mutable references*

A pure function, in essence, is not allowed to have *side effects*. This means it cannot modify state outside of itself. This includes any data that would normally be modifiable by an associate.

Data local to the pure function's scope may be freely modified within that scope. It may *not* be transmitted beyond its bounds.

Dynamic allocations within a pure function *must* be deallocated by the end of said function, *unless* this allocation is returned by the function, thus relinquishing ownership to the caller.

Pure functions may *not* call functions which may result in auxiliary side effects.

Failing to meet a pure function's requirements results in a *compile error*.
```rust
@[Pure]           // OK -- immutable ref
fn DotProduct(v1, v2: &Vec2) -> Vec2 {
	v1.x * v2.x + v1.y * v2.y
}

// Pure often pairs well with Inline
@[Pure, Inline]
fn Sqrt(v: i32) -> i32 {
	v * v
}
```

##### Discarding Values
When you invoke a function which returns a meaningful value (i.e. not `none` or some other monostate type) Mana will assume you actually want to use that value. Not using such a value results in a compile error.

```rust
fn Add(a, b: i32) -> i32 {
	a + b
}

fn Main() {
	Add(5, 7)
}
```
>[!danger] Error
> Discarding value of invocation `Add(5, 7)`

However, you may wish to intentionally discard this value, because you are invoking a function for its side effects alone. In this case, you may use the `@[Discard]` attribute before the discarding expression.
```rust
fn Add(a, b: i32) -> i32 {
	a + b
}

fn Main() {
	@[Discard] Add(5, 7) // Ok, compiler knows you're discarding intentionally
}
```

Additionally, you may be writing a function where you know its resulting value is only optionally used. In such cases, you may annotate the function with `@[Discardable]`
```rust
@[Discardable]
fn Pop() -> SomeType {
	data ret = .coll[.length - 1]
	--.length
	
	return ret
}

thing.Pop() // Ok, we don't have to use the value
```