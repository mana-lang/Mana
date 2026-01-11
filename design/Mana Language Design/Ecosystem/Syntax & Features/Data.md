
##### Expressions
```kotlin
import std.fmt
    
fn main() {
    fmt.PrintLine(5 + 2)
    fmt.Print("Hello" + " world!\n")
    fmt.Print("I have {} oranges", 3)
}
```
>[!tip] Output
>7
> Hello world!
> I have 3 oranges

##### Declarations
Data in **Mana** is *immutable* by default. Therefore, we don't refer to them as variables, but instead as *data bindings*, or just *data* for short. A singular data binding is called a *datum*. 
```kotlin  
fn main() {
    data two = 2
    data four = two + two 
    four += 1 // won't compile
}
```
>[!danger] Error
>Datum `four` is not mutable. You can make it mutable by annotating its declaration:
> `mut data four: f32 = two + two`

##### Mutability
To make data mutable, it must be annotated with the `mut` keyword. A mutable data binding is effectively a variable.
```kotlin
import std.fmt
    
fn main() {
	mut data number = 15
	number += 7
	fmt.Print("Catch-{number}")
}
```
>[!tip] Output
>Catch-22

##### Assignment Deduction
**Mana** makes certain assumptions about data when it's assigned to other data.
Depending on the context, it may *copy*, *reference*, or *move* the data.

- A *copy* simply *duplicates* the data
- A *reference* takes up a *fixed* amount of *additional* space by only *referring* to the actual data
- A *move* takes up the *same* amount of space, and *invalidates* the original data

**Mana** will, by default, attempt to *copy* assigned data, unless it would be cheaper to *reference* that data instead. How it determines this depends on the context, but the *memory size* of the data being assigned is always a key factor.

**Mana** will *almost never* perform a *move* by default, because a move *invalidates* the data being assigned from.

Keep in mind that these rules *only* apply when assigning to `data`. If the data is *mutable*, it will always be copied; **Mana** wouldn't invisibly create a mutable reference, as that would be extremely bug-prone.
```kotlin
import std.fmt
    
fn main() {
	mut data hey = "hey"
	data hi = hey       // this is a reference
	mut data hiya = hey // this is a copy
	hey = "hello"
	
	mut data five = 5
	data num = five // this is a copy
	five = 1
	
	fmt.Print(hey, five, hi, num, hiya)
}
```
>[!tip] Output
> hello1hello5hey

##### Assignment Annotation
You can explicitly tell **Mana** how you want it to handle the assignment using one of the three *assignment operators*.
```kotlin
import std.fmt
    
fn main() {
	data spell = "Stupefy"
	
	data copy = $spell
	data ref  = &spell
	data move = ~spell
	
	fmt.Print(ref) // ok
	fmt.Print(copy) // ok
	fmt.Print(move) // ok
	
	fmt.Print(spell) // won't compile
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
import std.fmt
    
fn main() {
	data x: i32 = 5
	
	// 'x' and '2' are silently converted to f64 here
	data f: f64 = x * 2  
	
	// unassigned data is zeroed by default
	data z: i64
	
	fmt.Print("x: {x}\nf: {f}\nz: {z}")
}
```
>[!tip] Output
> x: 5
> f: 10.0
> z: 0

##### Uninitialized Data
Unassigned data declarations are zeroed by default in **Mana**. To leave it uninitialized, you must do so explicitly with the `none` keyword.
```kotlin  
import std.fmt

fn main() {
	// data will be uninitialized.
    data x: bool = none
    
    // using it is either a compile error, or undefined behaviour
    fmt.Print(x) 
}
```
>[!danger] Error
>Data `x` was not initialized, but an attempt was made to read from it

Note: In the above example, `x` can never be initialized, because it's not `mut`.
 
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

##### Arrays
Arrays may be declared with the *list* operator, and indexed the *same* way.
```kotlin
data numbers = [1, 2, 3, 4, 5]
fmt.Print("{numbers[2]}")
```
>[!tip] Output
 3

Array types are deduced from their members. If all members in a list literal are of the *same* type, that list is an *array*. If any of the types differ, that list will be treated as a *tuple* instead.

To enforce that a data member is an array, you may annotate it with the array type specifier.
```kotlin
data tuple = [3.5, 2.2859, 8.3, "how'd i get here", 16.323]
data values: [f32] = [3.5, 2.2859, 8.3, "how'd i get here", 16.323]
```
>[!danger] Error
> Data `values` was declared as an array of `f32`, but was assigned a `string`

The array type specifier may contain either a type, or a type and an integer value, separated by a comma. The value specifies the *size* of the array, though this may be omitted in any scenario where the array is immediately assigned.

If the array is not immediately assigned, its size *must* be specified in the declaration.

An array which is not immediately assigned will have all its values *zeroed* by default. Much like scalar data, if you want it to be uninitialized, it must be assigned `none`.
```kotlin
// creates immutable 'x' of size '5' with type 'f64'
// all its elements will have the value '0.0'
data x: [f64, 5] 

// error
data y: [f64] 

// will be uninitialized
mut data z: [i32, 8] = none

// error
data w: [i32, 8] = none
```
>[!danger] Error
> Data `y` was declared as an array of `f64`, but was not assigned, and has no size specifier
> 
> Data `w` was assigned `none`, but it is immutable. There is no situation where it could be used

An empty-list declaration `[]` is valid, but the type and size *must* be specified.