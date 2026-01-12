
##### If
`if` statements in **Mana** evaluate a *boolean* and execute code when that boolean evaluates to *true*.

An `if` statement may be followed by an `else` statement, which itself may contain an `if` condition.

When the `else` statement contains no `if` statement, it is executed when *all* preceding conditions have failed.
```kotlin
data x = true
data y = 3 < 7

if x {
	DoSomeStuff()
} else if y {
	DoSomeOtherStuff()
} else {
	DoSomeCrazyStuff()
}
```

##### Loop
```rust
// 'while true'
loop {
	DoSomething()
}

// repeats 5 times
loop 5 {
	DoSomething()
}

// for (int i = 0; i <= 5; ++i)
loop ~5 i {
// 'loop 5~ i' to count from 5 to 0
	thing[i].DoSomething()
}

// all loops are scoped, so from here on out
// assume that the missing scope blocks are for brevity only


// for (int i = 0; i < 5; ++i)
// same as '~5', but exclusive
loop ..5 i 

// for (int i = 3; i <= 5; ++i)
loop 3~5 i  
loop 3..5 i // exclusive variant
	
// while x == 5
loop if x == 5

// do while x == 5
loop { /* some code */ } if x == 5
// keep in mind that the 'if' must be on the same line as '}'
// or terminated with a semicolon
// for it to be part of the loop

// within this loop, 'x' is temporarily mutable
// and is initially zeroed
data x => loop {
	// do stuff
	x = Something()
} // after the loop block, 'x' is immutable

loop {
	if SomeCondition() {
		skip // jumps to start of loop
	} else if OtherCondition() {
		break // exits current loop
	}
}

// loops may be labeled
loop A: 2..10 i {
	loop B: if i % 5 == 0 {
		if cond {
			skip => A // go back to the start of A
		}
		break => A // exits from outer loop A
	}
}
```


##### Ranged Iteration
In **Mana**, you can iterate over a *range* of elements using the `for` keyword.

```kotlin
data values = [55, 23, 99]

for elem in values {
	fmt.PrintLine("{elem}")
}
```
>[!tip] Output
>55
>23
>99

##### Match
*Pattern matching* in **Mana** works roughly the same as **Rust**'s pattern matching.

They are blocks which may contain a series of statements separated by the *map operator* `=>`

Expressions on the left side of *match arms* in a match block are compared to the matched *subject*. If the expression matches, it will execute that arm.
```rust
match x {
	Animal.Dog => DoDogThing()
	Animal.Cat => {
			DoSome()
			CatThings()
		}
	Animal.Lizard => DoLizardThings()
	none => DoGeneralThings()
}
```
Keep in mind that, because the left side of a match arm is compared against the *subject*, this means that all left-side match arm expressions *must* be *valid comparisons* against the subject.

This means that either a built-in comparison exists, or a valid *equality operator* has been defined for the two types being compared, just like with any other comparison.


It's important to note that `match` will execute *only* the *first* arm that matches, and it will run comparisons *from top to bottom*. To continue matching after a match has succeeded, the *match operator* `=>` must be used at the *end* of a match arm.

If the match arm is a *single* expression, then it must be used *after* the expression.
Otherwise, it must be placed on an *otherwise empty* statement at the *end* of an expression block.
```rust
match x {
	Animal.Dog => DoDogThing() => // keep matching
	Animal.Cat => {
			DoSome()
			CatThings()
			=> // keep matching
		}
	Animal.Lizard => DoLizardThings()
	none => DoGeneralThings()
}
```

To match against *multiple* options, use the `|` operator.
```rust
match x {
	Animal.Dog | Animal.Cat => DoCatDogThing()
	_ => PrintError()
}
```