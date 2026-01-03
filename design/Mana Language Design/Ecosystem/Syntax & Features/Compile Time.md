
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
You may also create a `match`-expression with the `when` keyword. This allows for denser and cleaner *when-expressions*.

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