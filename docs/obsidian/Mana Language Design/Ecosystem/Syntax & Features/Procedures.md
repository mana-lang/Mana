
##### Procedure
The term *procedure* is used as a catch-all term for *executable things* in Mana. This includes:
- Functions
- Methods
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
 
If a *return* type is *not* specified, it is *assumed* to be `none`.
Returning any value *other* than `none` from such a function results in a compile error.

The *function body* is a *scope*. A function returns a value *only* through `return`.

*All* valid binding names are *also* valid function names.
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

fn Main() {
	PrintIf(true)
}
```
>[!danger] Error
> Function `PrintIf` attempted to return a value despite having `none` return type

##### Parameters
A parameter consists of a *name*, followed by a colon, followed by a *type-name*. The name may be qualified with `mut`.

A parameter's type may *not* be omitted, except for a [field parameter](Procedures.md#Field%20Parameters). Parameters may be [grouped](Types.md#Composite%20Types) like fields.
```rust
fn Foo(a: i32, b: mut *string, c: [f64, 5]) -> mut *string
```
A parameter may declare a *default value*, just like a [field](Types.md#Default%20Values).

An *argument-list* initializes a function's parameters: *implicitly*, providing all parameters in order, or *explicitly*, by name.

Parameters omitted from an *argument-list* are *default-initialized*, unless annotated with `@[NoDefault]`

```rust
fn Spawn(kind: EnemyKind, count: i32 = 1, position: Vec2) { }

Spawn(Goblin, 3, origin)                  // implicit
Spawn(.kind = Goblin, .position = origin) // 'count' is 1
Spawn(.kind = Goblin)                     // 'count' is 1, position is {0, 0}
```

Parameter names and default values belong to the *declaration* of a callable binding, field or parameter, *not* to its type. They are written in its signature annotation, or in the `fn` literal that initializes it. A [signature type](Procedures.md#The%20%60fn%60%20Keyword) consists *only* of parameter and return types, so a callable whose declaration writes *no* parameter names must be given *every* argument, in order.
```rust
data spawn: fn(EnemyKind, i32, Vec2) = Spawn
data named: fn(kind: EnemyKind, count: i32 = 1, position: Vec2) = Spawn

spawn(Goblin, 1, origin) // ok
spawn(.kind = Goblin)    // error: 'spawn' declares no parameter names or defaults
named(.kind = Goblin)    // ok: 'count' is 1
```

Re-binding a callable binding does *not* change its parameter names or defaults.
```rust
mut fn Attack(target: *Enemy, damage: i32 = 10) { }
fn Heal(target: *Enemy, amount: i32 = 5) { }

Attack = Heal
Attack(.target = enemy) // invokes 'Heal' with a 'damage' of 10
```

A parameter is identical to a `data` *declaration*, initialized by its argument when the function is invoked. The only difference in declaration syntax is the omission of the `data` keyword, for convenience. 

A *parameter-list* may declare mutability, unlike a [field-list](Types.md#Composite%20Types), because a parameter is an immediate instantiation of its binding within its function, whereas the elements of a *field-list* are subject to the mutability of their instantiation.

A parameter that refers to other data is *always* a [pointer](Pointers.md), of type `*T` or `mut *T`.

##### Invocation Arguments
An argument is passed *exactly* as written: `x` passes a *copy*, `&x` an address with *read* access, `mut &x` an address with *write* access, and `$x` *moves* it.
```rust
fn Foo(v: Vec2, 
	   mut w: Vec2, 
	   p: *i32, 
	   mut q: mut *i32) {
}

Foo(v, w, &x, mut &y)
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
	return a + b
}

// shorthand, for when there are no constraints
fn Sub<T>(a, b: T) -> T {
	return a - b
}
```

##### Methods
You may add *methods* to types by creating *type interfaces*. 

Within an *interface-block*, `this` is a type: a [pointer](Pointers.md) to the block's associated type. `this` has *read* access, and `mut this` has *write* access.

A method is a function whose *first* parameter is of type `this`. That parameter is its *receiver*. *Only* the first parameter may be of type `this`, and a function without one has *no* receiver.

The receiver is initialized from the data the method is invoked on, *not* by the *argument-list*: as `&x` for a `this` receiver, and as `mut &x` for a `mut this` receiver.

A receiver whose name is omitted is named `this`. Its members may be accessed with a leading access operator `.`, whatever its name.
```rust
type Vec2 {
	x: f32
	y: f32
}

interface for type Vec2 {
	fn Scale(mut this, x: f32) {
		.x *= x
		.y *= x
	}
	
	fn Dot(v: this, other: Vec2) -> f32 {
		return .x * other.x + .y * other.y // '.x' is 'v.x'
	}
}

mut data a = Vec2 {3, 4}
data b = Vec2 {1, 2}

a.Scale(2)        // 'this' is 'mut &a', 'x' is 2
data d = b.Dot(a) // 'v' is '&b'
b.Scale(2)        // error: 'b' is immutable
```

> [!note]
> How a function without a receiver is invoked is not yet specified.

```rust
type Foo {
	a: i32 @[Locked]
	b: f64
	c: bool
}

interface Bar for type Foo {
	fn Baz(mut this) -> i32 {
		.a *= 3
		return .a
	}
	
	@[Pure, Inline]
	fn Fuzz(this) -> f64 {
		return .b * Pi / 180
	}
}

// if only one interface exists for a type
// it gets imported automatically when that type is imported
// otherwise, you must explicitly import the interface
// to be able to use it with the type
// 'import SomeModule.Foo::Bar'
// or all of them with 'import SomeModule.Foo::*'

mut data foo = Foo {1, 2, false}
data x = foo.Fuzz()

// when multiple methods share the exact same signature, 
// and both are imported at once, 
// you must disambiguate them with the scope resolution operator '::'

interface Bem for type Foo {
	fn Baz(mut this) -> i32 {
		.a *= 27
		return .a
	}
}

data bru = foo.Bem::Baz()
data baz = foo.Baz()
```
>[!danger] Error
> Call to method `foo.Baz()` is ambiguous. 
> Possible options:
> - Bem::Baz()
> - Bar::Baz() 

##### Mutated Fields
Within an *interface-block* you may declare a *mutated-field-list*.

A *mutated-field-list* may *exclusively* contain fields belonging to the type the interface is for.

*Every* function of that *interface-block* may mutate the listed fields, even through a `this` receiver, and even on *immutable* data. Outside that interface, the listed fields follow the usual rules.

The compiler treats listed fields as *mutable data*. 

A *mutated-field-list* is intended for mutations that are *not* observable through the interface, such as locking a mutex.
```rust
type Cache {
	key: i32
	mutex: Mutex
}

interface for type Cache {
	mut { mutex }
	
	fn Get(this) -> i32 {
		.mutex.Lock() // ok, even on an immutable 'Cache'
		// ...
		.mutex.Unlock()
	}
}

data cache = Cache {}
cache.Get()        // ok
cache.mutex.Lock() // error: 'cache' is immutable
```

##### Operators
Operators in Mana are specializable polyfunctions.

You can specialize operators in the *interface block* of any given type. 

Operator specializations are declared like functions, with `operator` and the operator's symbol in place of `fn` and a name. 
```rust
type Vec2 {
	x: f32
	y: f32
}

interface for type Vec2 {
	operator+(this, other: Vec2) -> Vec2 {
		return Vec2 {.x + other.x, .y + other.y}
	}
}

fn Main() {
	data a = Vec2 {23, 43}
	data b = Vec2 {97.1, 664.5}
	data c = a + b // 123.1, 707.5
}
```

An operator has *no* specialization for a composite type, unless the language provides one or the type specializes it. Invoking an operator without a matching specialization is a compile error.

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
> `Vec3` has no specialization of `operator+`.
> Consider writing a specialization in the interface for `Vec3`

A specialization defines what an operator does to *data*. What it does to *bindings* is fixed by the language, and cannot be specialized.

*Every* [operator](Operators.md) may be specialized, and *no* operator may be invoked by name. An operator is *only* invoked through its own syntax, or by the language.

Invoking an operator specialization annotated with `@[Forbid]` is a compile error. Without a parameter list, *every* specialization of that operator is forbidden.
```rust
interface for type Quat {
	@[Forbid] operator+                             // all specializations
	@[Forbid] operator{}(mut this, x, y, z, w: f32) // only this specialization
}
```

Specializations of `operator[]` and unary `operator*` *must* return a [pointer](Pointers.md). The result of the operator is the data that pointer points to, so it is considered [bound](Data.md#Bound%20and%20Unbound%20Data).
```rust
type Inventory {
	slots: [Item, 16]
}

interface for type Inventory {
	operator[](this, index: usize) -> *Item {
		return &.slots[index]
	}
	
	operator[](mut this, index: usize) -> mut *Item {
		return mut &.slots[index]
	}
}

mut data inventory = Inventory {}
inventory[2] = sword     // writes an 'Item' 
data held = inventory[2] // a copy
```

When specializations with a `this` and a `mut this` receiver both apply, such as for `operator[]` and unary `operator*`, the `mut this` one is used *only* where its result is written.
```rust
inventory[2] = sword              // 'mut this' operator[]
data held = inventory[2]          // 'this' operator[]
data writable = mut &inventory[2] // 'mut this' operator[]
```

##### Freestanding Operators
An operator specialization may also be declared outside an *interface-block*, wherever a function may be declared. A *freestanding* specialization takes *every* operand as a parameter.

A [binding operator](Operators.md#Binding%20Operators) may *only* be specialized with a receiver, which is the data of that binding.

A specialization with a receiver and a freestanding specialization with the same operand types are the *same* specialization.
```rust
operator*(scale: f32, v: Vec2) -> Vec2 {
	return Vec2 {scale * v.x, scale * v.y}
}

operator+=(a: Vec2, b: Vec2) { } // error: 'operator+=' is a binding operator

interface for type Vec2 {
	operator+=(mut this, other: Vec2) {
		.x += other.x
		.y += other.y
	}
}

mut data position = Vec2 {1, 2}
position += Vec2 {3, 4}      // ok
data scaled = 2.0 * position // ok
5 += position                // error: '5' is not mutable
```

Freestanding specializations are imported with the Artifact that declares them. If several imported Artifacts declare the same specialization, invoking it is a compile error, unless *exactly one* of those Artifacts is imported with `::operator`.
```rust
// both declare 'operator+(a: Vec2, b: Vec2)'
import geometry.vec
import physics.vec_ops

import geometry.vec::operator

data c = a + b // uses 'geometry.vec'
```

##### Field Parameters
The *parameter-list* of a method or operator specialization may contain *field parameters.* A field parameter is a field that was previously declared in the *field-list* of that *interface-block*'s associated type, written with a leading access operator `.`

The argument given for a field parameter becomes the value of that field *before* the body runs. The type annotation of a field parameter *may* be omitted.
```rust
interface for type Vec2 {
	fn Set(mut this, .x, .y) { }
	
	fn Scale(mut this, x: f32) {
		.x *= x
		.y *= x
	}
}

mut data v = Vec2 {1, 2}
v.Set(5, 10)  // 'v' is {5, 10}
v.Scale(2)    // 'v' is {10, 20}
v.Set(.y = 3) // 'v' is {10, 3}
```
A field parameter may *only* be declared after a `mut this` receiver; this includes operator specializations.

When a field parameter is omitted from an explicit argument list, its field keeps its value.

The names in a *parameter-list* *must* be unique. The name of a field parameter is the name of its field, so a field parameter and a parameter of the same name may *not* share a parameter-list.
```rust
fn Foo(mut this, .x: f32, x: f32) // error
```

Unless specialized, the `operator{}` of a type takes one field parameter for each of its fields, in declaration order.

*Every* specialization of `operator{}` *must* declare a field parameter for each field annotated with `@[NoDefault]`.

##### Lifecycle Operators
The lifecycle of data is defined by five operators:
- `operator{}` - *construction*, invoked by a *braced-list*
- `operator#` - *copy-initialization*
- `operator=` - *assignment*
- `operator$` - *move*
- `operator!{}` - *destruction*
```rust
interface for type LogFile {
	operator{}(mut this, path: string) {
		.handle = OpenFile(path)
	}
	
	operator!{}(mut this) {
		Close(.handle)
	}
	
	@[Forbid] operator#
	@[Forbid] operator=
}

data log = LogFile {"game.log"}
data copy = log  // error
data moved = $log // ok
```

Specializing `operator{}` will *only* override the default braced-list if its parameters after its receiver *exactly* match the field order of its respective *interface-block*'s associated type, just like any other operator specialization. 

A specialization of `operator{}` initializes each field from its field parameter, or with its default if it has none. Its body then runs on the initialized data, like an [initialization block](Data.md#Initialization%20Blocks). A specialization without parameters besides its receiver is invoked by both `data x: T` and `T {}`.

`operator#` is a unary operator, invoked by `#x`, and by the language whenever data is initialized as a *copy* of other data: in declarations, parameters, fields and list elements. Its *only* parameter is its receiver, the data being copied, and the specialization returns the copy.
```rust
interface for type Buffer {
	operator#(this) -> Buffer {
		return Buffer {.data = Duplicate(.data), .size = .size}
	}
}

data b = a // invokes the 'operator#' of 'a'
```
Unless specialized, it copies each field with that field's own `operator#`. As a result, data with a field that forbids `operator#` cannot be copied by default either.

`operator=` *exclusively* specializes assignment. Unless specialized, it assigns each field with that field's own `operator=`, *overwriting* the old data.

`operator$` defines how data arrives at its new binding when it is moved. What the source binding is left holding is decided by the language, *not* by `operator$`; see [Assignment Annotation](Data.md#Assignment%20Annotation).

> [!note]
> The signature and default behaviour of `operator$` are provisional, until ownership is specified.

`operator!{}` is invoked *exactly once*, when data reaches the end of its lifetime. 
Moving data does *not* invoke its `operator!{}`; its new binding destroys it instead. 

After its body, the `operator!{}` of each of the type's fields is invoked, in *reverse* declaration order. Unless specialized, the body of `operator!{}` is empty. 

The `operator!{}` of a primitive does nothing. 

The default state of a type *must* be safe to destroy.

Data whose `operator!{}` has been invoked is said to be *destroyed*.

The lifetime of a binding ends when it exits scope. 

The lifetime of bound data ends with the binding holding it.

The bindings of a scope are destroyed in *reverse* declaration order.

When the binding of an array ends, the `operator!{}` of each of its elements is invoked, in *reverse* index order.

The lifetime of [unbound](Data.md#Bound%20and%20Unbound%20Data) data lasts until the end of the statement it appears in.

##### Invocators
"Invocator" is just a term for types with a specialized `operator()`

A binding of an invocator is invocable like any other function. However, they carry state like any binding.
```rust
type Invocator {
	a: i32
	b: string
}

interface for type Invocator {
	operator()(mut this, x: i32) -> i32 {
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
	- Capture clause follows a target operator after the return type, but before the function's scope
	- Each capture is written `name = value`
		- A capture of a binding under its own name may omit the name: `x` is `x = x`, and `&y` is `y = &y`
		- The values of a capture clause know *exclusively* of the enclosing scope, *not* of the closure's parameters
	- Captured bindings may contain assignment annotations
		- In their absence, a capture is a *copy*, as in any assignment.
	- Captured bindings are part of the closure's type
	- The capture clause declares the closure's fields and initializes them at once. Like any fields, they may *not* be declared `mut`; a closure writes its own fields through a `mut this` receiver.
	- `&y` and `mut &y` capture *pointers*, of type `*T` and `mut *T` respectively.
	- Closure's size is *at least* the size of all its captures
	- Closure will store captures in the order that requires the *least* amount of padding
- Within a closure, `this` is a pointer to the closure's *own* type
	- Its captures are reached through its [receiver](Procedures.md#Methods), like the fields of any receiver
	- A closure without a receiver cannot reach its captures
- The enclosing `this` is captured like any other data, under another name: `owner = this`
- May be named or anonymous
- Will deduce the return type to the *first* return expression, if return type is left unspecified
- Otherwise behave like invocators

```kotlin
data x = 32
data y = "hello"
data v = Vec2 {5, 10}

// closures may be named
mut data closure = fn(mut this, a: i32) -> i32 => x, &y, v {
	fmt.Print(.y) // captures are members of the 'closure' invocator
	
	.v.x = a
	.v.y = .x
	
	return a * .x
}
@[Discard] closure(55)

// or anonymous	
foo.Filter(fn(this) => x { return .x % 5 })
		.Map(fn(mut this, t: i32) => x, v { return t - .x + .v.Cycle() })

```

Capturing the enclosing `this`
```rust
.Handle(fn(this) => x, owner = this {
	.owner.something = SomethingElse(.x)
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

##### The `fn` Keyword
The `fn` keyword declares an ad-hoc type with an `operator()` specialization, and immediately instantiates it. Its optional [capture-clause](Procedures.md#Closures) denotes the type's fields. Without a capture-clause, that type has *no* fields, and is exactly the function's *signature type*.

The two declarations below are identical.
```rust
fn AddSix(a: i32) -> i32 {
	return a + 6
}

data AddSix = fn(a: i32) -> i32 {
	return a + 6
}
```

This is the first thing that `fn` makes convenient for us. What this initialization is masking is the differentiation between the function's type and its value. Below is the same declaration, but without any type deduction:
```rust
data AddSix: fn(i32) -> i32 = fn(a: i32) -> i32 {
	return a + 6
}
```
It very quickly turns into a mess. 

Additionally, you may have noticed that this declaration is somewhat recursive; `fn` appears in both the type and the value. Why is this?

Consider the syntax that instantiates a compound type:
```rust
data x: Foo = Foo {1, 2, 3}
```

In this case, we are also writing `Foo` twice; `Foo` is the type, and `Foo {1, 2, 3}` is a value of that type. `fn` works the same way: `fn(i32) -> i32` is a *signature type*, which holds *any* procedure without fields of that signature, and `fn(a: i32) -> i32 { }` is such a procedure.

The type form is the function signature, which you would write when storing a particular kind of function. The value form still includes the `fn` keyword to make it clear that we are instantiating a function.

To get an idea of what an `fn` declaration does under the hood, we can rewrite the above declaration to look more like a "normal" invocator:
```kotlin
type __AddSix { } // no capture-clause, so no fields

interface for type __AddSix {
	operator()(this, a: i32) -> i32 {
		return a + 6
	}
}

data AddSix = __AddSix {}
```

Which then lets us write:
```kotlin
data x = AddSix(6) // x == 12
```

In essence, `fn` exists to preserve Mana's philosophy while still allowing us to conveniently write "standalone" functions. It's a keyword which declares a type and instantiates it in one step.

##### Declaring `fn`
A function may be declared in *any* situation where data may be declared. A field holding a function *must* be declared with a signature type.
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
	fn Closure(this) => x {
		fmt.PrintLine("{.x}")
		return .x * 2
	}
	
	data y = Closure() // y == 64
}
```

A function binding or field is exactly the size of one pointer. A closure binding is *at least* the size of its captures.

To hold *any* procedure of a given signature, annotate its signature type with `@[Proc]`. A `@[Proc] fn` *owns* a copy of its procedure, while a `@[Proc] *fn` points to one.
```kotlin
type Button {
	on_click: @[Proc] fn()
}

fn Click() { }

mut data clicks = 0
data counter = fn(this) => mut &clicks { *.clicks += 1 }

data a = Button {.on_click = Click}
data b = Button {.on_click = counter} // 'counter' is copied into 'on_click'
b.on_click()
```
A `@[Proc] fn` is the size of two pointers, and *only* accepts procedures that can be copy-initialized. The copy of a procedure with fields is stored in memory from the [current allocator](Memory.md#Allocator%20Capture).

Any specific `fn` signature may be *type-aliased*:
```kotlin
type alias Callback = @[Proc] fn(i32)

type Button {
	on_press: Callback
	on_release: Callback
}
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
	return x * x
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

A pure function, in essence, is not allowed to have *side effects*. This means it cannot modify state outside of itself. This includes any data that would normally be modifiable by a method.

Data local to the pure function's scope may be freely modified within that scope. It may *not* be transmitted beyond its bounds.

Dynamic allocations within a pure function *must* be deallocated by the end of said function, *unless* this allocation is returned by the function, thus relinquishing ownership to the caller.

Pure functions may *not* call functions which may result in auxiliary side effects.

Failing to meet a pure function's requirements results in a *compile error*.
```rust
@[Pure]           // OK -- read-only pointers
fn DotProduct(v1, v2: *Vec2) -> Vec2 {
	return v1.x * v2.x + v1.y * v2.y
}

// Pure often pairs well with Inline
@[Pure, Inline]
fn Sqrt(v: i32) -> i32 {
	return v * v
}
```

##### Discarding Values
When you invoke a function which returns a meaningful value (i.e. not `none` or some other monostate type) Mana will assume you actually want to use that value. Not using such a value results in a compile error.

```rust
fn Add(a, b: i32) -> i32 {
	return a + b
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
	return a + b
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