
##### Expressions
```kotlin
import std.io.print
    
fn main() {
    std.println(5 + 2)
    std.print("Hello" + " world!\n")
    std.print("I have {} oranges", 3)
}
```
>[!tip] Output
>7
> Hello world!
> I have 3 oranges


##### Declarations
Data in **Mana** is *immutable* by default
```kotlin  
fn main() {
    data two = 2
    data four = two + two 
    four += 1 // won't compile
}
```
>[!danger] Error
>Data `four` is not mutable. You can make it mutable by annotating its declaration:
> `mut data four: f32 = two + two`

##### Mutability
To make data mutable, it must be annotated with the `mut` keyword
```kotlin
import std.io.print
    
fn main() {
	mut data number = 15
	number += 7
	std.print("Catch-{number}")
}
```
>[!tip] Output
>Catch-22

##### Assignment
**Mana** makes certain assumptions about data when it's assigned to other data.
Depending on the context, it may *copy*, *reference*, or *move* the data.

- A *copy* simply *duplicates* the data, and takes up more space as a result
- A *reference* takes up a fixed amount of space by only *referring* to the actual data container
- A *move* takes up the same amount of space, and invalidates the original data container

**Mana** will, by default, attempt to *copy* assigned data, unless it would be cheaper to *reference* that data instead. How it determines this depends on the context, but the *memory size* of the data being assigned is always a key factor.

**Mana** will *almost never* perform a *move* by default, because a *move* invalidates the data being assigned from.

Keep in mind that these rules *only* apply to `data` - `mut data` will always be copied, because **Mana** wouldn't invisibly create a mutable reference, as that would be extremely bug-prone.
```kotlin
import std.io.print
    
fn main() {
	data hey = "hello"
	data hi = hey // this is a reference
	
	data five = 5
	data num = five // this is a copy
	
	std.print(hey, five, hi, num)
}
```
>[!tip] Output
> hello5hello5

##### Assignment Operator
You can explicitly tell **Mana** how you want it to handle the assignment using one of the three *assignment operators*.
```kotlin
import std.io.print
    
fn main() {
	data spell = "Stupefy"
	
	data copy = $spell
	data ref  = &spell
	data move = ~spell
	
	std.print(ref) // ok
	std.print(copy) // ok
	std.print(move) // ok
	
	std.print(spell) // won't compile
}
```
>[!danger] Error
> Data `spell` was invalidated by an earlier move, and cannot be used

In the above example, `spell` became invalidated, because it was *moved* into `move`. A *move* in **Mana** signifies that data has been given a new *owner* - it does *not* invalidate references.

This feature is an important part of how **Mana** handles *ownership*, which will be discussed later.


##### Type annotations
All data in **Mana** is *strongly and statically typed*. 

This means the types of all values must be known at compile-time, and data may not be assigned a value of a type that doesn't match its own.

Data declarations may have *explicit* type annotations. This may be useful if you want to convert certain types, or ensure a variable's type doesn't silently change e.g. in the case of a function's return type being modified unexpectedly.
```kotlin
import std.io.print
    
fn main() {
	data x: i32 = 5
	
	// 'x' and '2' are silently converted to f64 here
	data f: f64 = x * 2  
	
	// unassigned data is zeroed by default
	data z: i64
	
	std.print("x: {x}\nf: {f}\nz: {z}")
}
```
>[!tip] Output
> x: 5
> f: 10.0
> z: 0

##### Uninitialized Data
Unassigned data declarations are zeroed by default in **Mana**. To leave it uninitialized, you must do so explicitly with the `null` keyword.
```kotlin  
import std.io.print

fn main() {
	// data will be uninitialized.
    data x: bool = null
    
    // using it is either a compile error, or undefined behaviour
    std.print(x) 
}
```
>[!danger] Error
>Data `x` was not initialized, but an attempt was made to read from it

##### Constants
Compile-time constants are created with the `const` keyword.
They *must* have their type annotated, and cannot be unassigned.
Constants may only be assigned *constant expressions*. 
This includes other constants, as well as any other expressions that evaluate to a constant, such as value literals and constant procedures.

```kotlin
const Pi: f64 = 3.14159265358979
const RadCircle: f64 = 2 * Pi

const SolPlanets = 8 
const Something: string
```
>[!danger] Error
> Constant `SolPlanets` was not annotated
> Constant `Something` was not assigned
