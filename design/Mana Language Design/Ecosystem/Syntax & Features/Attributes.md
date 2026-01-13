You may annotate declarations with Attributes to give them new properties or restrictions.

An attribute is defined by an `@` symbol followed by a *bracketed-list* of (optional trailing) comma-separated Attribute names:

`@[Pure, Inline]`

Below is a list of valid Attributes:

- *Pure*
	- Usage: *Before* function declaration
	- Effect: Raises compile error if a function has any (potential) side effects
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
- *Flexible*
	- Usage:  *Before* a type declaration
	- Effect: Will aggressively optimise for cache locality at the cost of having unpredictable data layouts. 
- *Discardable*
	- Usage: *Before* a function with a return type
	- Effect: Will provide a compiler warning if the return value of this function is not used


##### Attribute Groups
Multiple attributes may be grouped under a single name for convenience.

```rust
@PureInline = @[Pure, Inline]

@[PureInline]
fn Foo() // etc
```
