
##### When
The `when` keyword is like `if`, but for *compile-time*. It is used for *conditional compilation* and *generics*, and can *only* evaluate `const` expressions.
```kotlin
when std.BuildTarget == std.Platforms.Linux.x64 {
	DoSomething()
} else when std.BuildTarget == std.Platforms.Windows.x86 {
	DoSomethingElse()
}
```
In the above example, either `DoSomething` or `DoSomethingElse` will get compiled, and the other block will get omitted from the build output completely.


##### When Match
You may also create a `match`-expression with the `when` keyword. This allows for denser and cleaner *when-statements*.

Base enumerations, tags, and variants (if their data is known at compile-time) *may* be used in *when-expressions*.

```rust
when match std.BuildTarget {
	std.Platforms.Linux       => DoLinuxThing()
	std.Platforms.Windows     => DoWindowsThing() =>
	std.Platforms.Windows.x86 => AlsoDoX86Thing()
	std.Platforms.MacOS       => DoMacThing()
	none => DoDefault()
}
```


##### Where
The `where` keyword imposes a series of constraints on a *type*, *type interface* or *generic function*, which *must* be resolvable at compile-time.

Constrained types have fields bound by their constraints
```rust
// in below example:
// field 'a' must be of an integral type
// field 'b' must be of a type containing:
//           - an interface with a Fetch() function
//                               - that returns a value of the same type as 'a'
//                                                             - aka type T
type Foo<T, U> {
	a: T => where std.Type.IsIntegral<T>
	b: U => where (u: U) => u.Fetch() -> type(a)
}
```
In short,  a constrained type is a generic type that satisfies  *either* of the following:
 - A valid `when` expression evaluation
 - A statement that would be valid with the given type

A `where` clause would fail if it cannot be resolved at compile time.

It may contain *either* a compile-time boolean expression (aka a *when-expression*) *or* a parenthesized parameter list (as in an invocable), followed by *either* a target-operator followed by a single statement *or* a braced-scope set of statements, all of which must compile given the types of the arguments presented in the argument list.

All statements in the braced-scope set may optionally be assigned an expected-return value.
```rust
// A is a type where:
// a value of this type is convertible to any Decimal type (f32, f64)
// a value of this type may access interface function `Floor()`
// a value of this type accesses interface function 'Sqrt()'
//                       - which returns a value convertible to Decimal type
type Bar<A> {
	a: A where (x: A) {
		std.Type.ConvertibleTo(std.Type.Decimal, type(x))
		x.Floor()
		x.Sqrt() -> std.Type.ConvertibleTo(std.Type.Decimal)
	}
}
```


You may also declare a constraint interface
```rust
interface SomeConstraint where {
	// list of constraints...
}

type Foo<SomeConstraint> {
	a: SomeConstraint // will have constraint applied, 
					 // no `where` clause necessary
} 
```

Interfaces of types implementing this constraint *must* implement any functions defined by the constraint
```rust
type interface DoesThing where {
	fn DoThing() -> i32
	fn DoOtherThing() -> f64
}

type Foo: DoesThing {
	a: i32
	b: string
}

interface Bar for type Foo {
	fn Blah() {
		// do something
	}
	
	fn DoOtherThing() -> f32 {
		// do other thing
	}
}
```
>[!danger] Error
> Type `Foo` does not satisfy constraint `DoesThing`:
> - Function `fn DoThing() -> i32` is not implemented
> - Function `fn DoOtherThing() -> f32` does not match return requirement: `f64`

For types wishing to implement multiple interfaces:
```rust
type Foo: interface {
	DoesThing
	DoesOtherThing
	HasThing
} => {
	a: i32
	b: string // etc.	
}
```

Functions belonging to a constrained interface are bound by that type's constraints.
```Rust
type Foo {
	a: i32
	b: string
}

type T where (t: T) => t.Bar() -> Baz for
interface Bar for type Foo {
	mut fn Doohickey() -> T {
		// do some stuff, return constrained T
	}
}
```

Arguments used in an invocation of a constrained generic function are bound by that function's constraints.
```rust
type T where std.Type.IsNumber<T> for
fn Add(a, b: T) {
	return a + b
}

Add("s", "b")
```
>[!danger] Error
> Invocation does not satisfy constraints for `Add(a, b: T)`:
> - Constraint `std.Type.IsNumber<type("s")>` evaluates to `false`
> - Constraint `std.Type.IsNumber<type("b")>` evaluates to `false`