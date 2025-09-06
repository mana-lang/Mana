##### Loop
```rust
// 'while true'
loop {
	do_something()
}

// repeats 5 times
loop 5 {
	do_something()
}

// for (int i = 0; i <= 5; ++i)
loop ~5 i {
// 'loop 5~ i' to count from 5 to 0
	thing[i].do_something()
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
loop { ... } if x == 5
// keep in mind that the 'if' must be
// on the same line as '}'
// or terminated with a semicolon
// or it will be considered a separate if statement

// within this loop, 'x' is temporarily mutable
// and is initially zeroed
data x = loop {
	// do stuff
	x = something()
} // after the loop block, 'x' is immutable

loop {
	if some_condition() {
		skip // jumps to start of loop
	} else if other_cond() {
		break // exits current loop
	}
}

// loops may be labeled
loop A: 2..10 i {
	loop B: if i % 5 == 0 {
		if cond() {
			skip -> A // go back to the start of A
		}
		break -> A // exits from outer loop A
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

Expressions on the left side of arms in a match block are compared to the matched *subject*.
If the expression matches, it will execute that arm.
```rust
match x {
	Animal.Dog => DoDogThing()
	Animal.Cat => {
			DoSome()
			CatThings()
		}
	Animal.Lizard => DoLizardThings()
	_ => DoGeneralThings()
}
```

It's important to note that `match` will execute *only* the *first* arm that matches, and it will run comparison *from top to bottom*. To continue matching after a match has succeeded, the *match operator* `=>` must be used at the *end* of a match arm.

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
	_ => DoGeneralThings()
}
```

To match against *multiple* options, use the `|` operator.
```rust
match x {
	Animal.Dog | Animal.Cat => DoCatDogThing()
	_ => PrintError()
}
```