
##### When
The `when` keyword is like `if`, but for *compile-time*. It is used for *conditional compilation* and *generics*, and can *only* evaluate `const` expressions.
```kotlin
when std.BuildTarget == std.Platforms.Linux.x64 {
	DoSomething()
} else when std.BuildTarget == std.Platforms.Windows.x32 {
	DoSomethingElse()
}
```
In the above example, either `DoSomething` or `DoSomethingElse` will get compiled, and the other block will get omitted completely.