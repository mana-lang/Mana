##### Attribute Annotations
You may annotate declarations with Attributes to give them new properties or restrictions, or to hint to the compiler about what you want it to do with that particular segment of code.

Attributes exist for language features which do not quite justify a keyword. Mana strives to use as few keywords as possible, and attributes help reduce more niche or highly-specific keywords.

An attribute is defined by an `@` symbol followed by a *bracketed-list* of (optional trailing) comma-separated Attribute names:

`@[Pure, Inline]`

Below is a list of valid Attributes:

- *Hot*
	- Usage: *Before* function declaration
		- Effect: Function will be hot-reloadable through **Hex**
	- Usage: *Before* module declaration *or* at the top of a file *without* a module declaration
		- Effect: *All* functions within this module will have the `@[Hot]` attribute applied to them
- *NoInit*
	- Usage: *Before* a *mutable* data declaration
	- Effect: Data will be left uninitialized
- *NoDefault*
	- Usage: *Before* a field or parameter, *or* before a type or function declaration
	- Effect: The field or parameter has no default, and *must* be provided explicitly. On a type or function, this applies to all fields or parameters without a default value.
- *Forbid*
	- Usage: *Before* an operator specialization in a type interface
	- Effect: Invoking that particular specialization is a compile error.
- *Pure*
	- Usage: *Before* function declaration
		- Effect: Raises compile error if the function breaks any of the [pure function](Procedures.md#Pure%20Functions) rules
	- Usage: *Before* a signature type, wherever a type is written
		- Effect: The type holds *only* pure procedures of that signature
- *Locked*
	- Usage:
		- *After* type member declaration
		- *Inside* type block, declaring its own block containing type members
	- Effect: Raises compile error when attempting to mutate designated type member(s)
- *Inline*
	- Usage: *Before* function declaration
	- Effect: 
		- Will run the function at compile-time, if possible, and replace the call site with its result
		- If it cannot run at compile-time, it will instead replace the call with its contents, if possible
		- If this is not possible, it remains a regular function call
- *Discardable*
	- Usage: *Before* a function with a return type that isn't `none`
	- Effect: Discarding return values of this function will *not* raise a compile error.
- *Discard*
	- Usage: *Before* an invocation-expression
	- Effect: Will suppress the "Return value unused" compile error
- *Wrap*
	- Usage: *Before* an expression, scope, or function declaration
	- Effect: Overflowing integer arithmetic *wraps around*, so going 1 above the maximum value for that size takes you to the minimum, and vice versa.
- *Saturate*
	- Usage: *Before* an expression, scope, or function declaration
		- Effect: Overflowing integer arithmetic is *clamped* to the minimum or maximum value of the type's size.
- *Allocator(name)*
	- Usage: *Before* a scope or function declaration
	- Effect: The named allocator becomes the current allocator within it. See [Memory](Memory.md#Scoped%20Allocators).
- *Outlive(n)*
	- Usage: *Before* a module-level data or allocator declaration
	- Effect: Destroys it after all unannotated declarations of its Artifact, in descending order of `n`, so that `1` is destroyed last. See [Module Data](Data.md#Module%20Data).
- *Timer*
	- Usage: *Inside* a scope block, from the moment you want to measure the execution of a block.
	- Effect: Records the time at the moment of the attribute's declaration, and again when that scope is exited. The difference is stored in a configurable global `ScopedTimer` type.


##### Attribute Groups
Multiple attributes may be grouped under a single name for convenience.
```rust
@PureInline = @[Pure, Inline]

@[PureInline]
fn Foo() // etc
```

Keep in mind that you *cannot* group attributes with differing usage specs. For example, you cannot write the following:
```rust
@NotAGoodIdea = @[Pure, Timer]
```
This would result in a compile error, because `Pure` and `Timer` are placed in different locations.