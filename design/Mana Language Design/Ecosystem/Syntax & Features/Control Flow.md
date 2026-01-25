
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
	DoCrazyStuff()
}

// conditional assignment is also possible via the binding operator
data a: i64 => if x || y {
	a = 89 // inside the bound if, 'a' is temporarily assignable
} else {
	a = 943
}
// 'a' is still immutable here
  
// you can do it on a single line for short conditional assignments
data b: u32 => if x && y { b = 7 } else { b = 27 }
```

##### Loop
```rust
// infinite loop, aka: 'while true'
loop {
	DoSomething()
}

// conditional loop, aka: while x == 5
loop if x == 5 {
	EatSomething()
}

// post-conditional, aka: do {...} while x == 5
loop { 
	SayHi()
} => if x == 5

// fixed loop. runs exactly 8 times
loop 8 {
	DoSomething()
}

// ranged loop
// binds inclusive range from 0 to 8 to a local temporary 'i'
// aka: for (int i = 0; i <= 8; ++i)
loop 8 => i {
	DoSomething()
}

// identical to the above, except it starts at 8 and counts down to 0
// aka: for (int i = 8; i >= 0; --i)
loop 8..0 => i {
	things[i].DoSstuff()
}

// identical to the above. 
// ranges can be arbitrary and will count from lhs to rhs
// aka: for (int i = 3; i <= 8; ++i)
loop 3..8 => i {}  


// the bound temporary is immutable unless otherwise stated
loop 12..0 => mut i {}


// data declarations may also be bound to loops
data x => loop if x < 30 {
	// do stuff
	x = Something()
}


// break and skip statements
loop {
	if SomeCondition() {
		DoSomething()
		skip // jumps to start of loop
	} else if OtherCondition() {
		DoSomethingElse()
		break // exits current loop
	}
}

// break-if and skip-if statements
// allow you to concisely perform conditional skips
loop {
	DoSomething()
	break if SomeCondition()
	
	DoSomethingElse()
	skip if SomeOtherCondition()
	
	DoAnotherThing()
}

// loops may be labeled
loop A: 2 .. arr.BackIndex() => i {
	loop B: if i % 5 == 0 {
		if some_cond {
			skip => A // go back to the start of A
		}
		break => A // exits from outer loop A
	}
}

// you can still use break-if and skip-if
loop A: elems.Size() .. 0 => mut i {
	loop B: if i % 3 == 0 {
		DoStuff()
		skip if cond => A
	}
	break if other_cond => B
}
```


##### Ranged Iteration
You can iterate over a collection using the `for` keyword.

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