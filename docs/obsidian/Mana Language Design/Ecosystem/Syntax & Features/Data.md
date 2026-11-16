
##### Expressions
```kotlin
import std.fmt
    
fn Main() {
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
Data in Mana is *immutable* by default. Therefore, the compiler does not refer to them as variables, but instead as *data bindings*, or just *bindings*. This *requires* them to be initialized within the declaration.

`data` is a *contextual* keyword. It is only a keyword when it introduces a declaration, and may be used as an identifier anywhere else.
```kotlin  
fn Main() {
    data two = 2
    data four = two + two 
    four += 1 // won't compile
}
```
>[!error] Error
>Attempt to assign to immutable data bound by `four`

##### Mutability
To bind a name to *mutable data* in Mana, you must qualify a declaration with the `mut` keyword. 

Mutability is a property of the *data*, not of the binding. `mut data` reads exactly as it behaves: the name is bound to *mutable data*.
```kotlin
import std.fmt
    
fn Main() {
	mut data number = 15
	number += 7
	fmt.Print("Catch-{number}")
}
```
>[!tip] Output
>Catch-22

##### Default Initialization
In the absence of an initializer, data will still be initialized with a default value.
```kotlin
data x: i32

fmt.Print("{x}")
```
>[!tip] Output
> 0
- For *primitive* types, the default value is the result of conversion from `0` to the given type.
- For *enumerated* types, the default value is the *first* enumeration.
- *Composite* types will default each of their individual fields.

You may only omit an initializer for a declaration including a type annotation.
```kotlin
data x
```
>[!error] Error
> Binding `x` has neither an initializer nor an annotation.

##### Initialization Blocks
A data declaration may *target* a block with the *target operator* `=>`. 
This is an *initialization block*:
```kotlin
data x: i32 => {
	// 'x' has already been default-initialized here
	// within this block, 'x' is mutable
	RunSomeFunc()
	DoSomeTest()
	x = FetchResult()
}
// 'x' is immutable from here on
```
Within an initialization block, initialization happens in three steps:
1. The data is *default-initialized*.
2. The block runs. Within it, the data is *mutable*. Assignments within the block are regular assignments to the default-initialized data, and because it has a default value, you may read it before your first assignment.
3. When the block ends, initialization is complete. Unless it was declared `mut`, the data is *immutable* from then on.

Initialization blocks allow you to build up data step by step, without having to make it mutable.
```kotlin
data player: Player => {
	player.name = LoadName()
	player.health = PlayerMaxHealthDefault
	
	if hardcore {
		player.lives = 1
	} else {
		player.lives = 5
	}
}
```

Since the data is default-initialized first, a declaration with an initialization block *must* include a type annotation.

A declaration may also target control flow, such as an `if` or a `loop`. See [Control Flow](Control%20Flow.md).

##### Copy Semantics
When data is assigned to a binding, it is *always* treated as a *copy*. The new binding holds its own value, and changes made to the original are *never* visible through it.

- A *copy* simply *duplicates* the data. Where possible, this will manifest as a "*shallow copy*."
- A *reference* only *refers* to the original data, taking up a *fixed* amount of *additional* space
- A *move* gives the data a new owner, and *invalidates* the original binding

```kotlin
import std.fmt
    
fn main() {
	mut data hey = "hey"
	data hi = hey       // this is a copy
	mut data hiya = hey // this is a copy
	hey = "hello"
	
	mut data five = 5
	data num = five // this is a copy
	five = 1
	
	fmt.Print(hey, five, hi, num, hiya)
}
```
>[!tip] Output
> hello1hey5hey

This does *not* mean Mana always duplicates the data in memory. When the compiler can prove that a copy is indistinguishable from a *reference* or a *move* - for example, because the original is never modified while the new binding is alive, or because the original is never used again - it is free to reference or move the data instead, or simply alias the two bindings.

In other words, optimisations may *never* affect behaviour in an observable way, other than allowing the same behaviour to execute faster.

Short of unnoticeable optimisations, references and moves in Mana are *always explicit*. 

##### Assignment Annotation
By default, assignment *copies*. You can explicitly tell Mana to handle the assignment differently using one of the two *assignment operators*: `&` to *reference*, and `$` to *move*.
```kotlin
import std.fmt
    
fn main() {
	data spell = "Stupefy"
	
	data copy = spell  // copy
	data ref  = &spell // reference
	data move = $spell // move
	
	fmt.Print(ref) // ok
	fmt.Print(copy) // ok
	fmt.Print(move) // ok
	
	fmt.Print(spell) // won't compile
}
```
>[!danger] Error
> Data `spell` was invalidated by an earlier move, and cannot be used

In the above example, `spell` became invalidated, because it was *moved* into `move`. A *move* in Mana signifies that data has been given a new *owner* - it does *not* invalidate references.

A move *ends* the original binding, the same way leaving its scope would. It does *not* modify the data, so immutable data may be moved.

This feature is an important part of how Mana handles *ownership*, which will be discussed later.

##### Type annotations
All data in Mana is *strongly and statically typed*. 

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
Unassigned data declarations are zeroed by default in Mana. To leave it uninitialized, you must do so explicitly with the `@[NoInit]` attribute.
```kotlin
import std.fmt

fn main() {
	// data will be uninitialized.
    @[NoInit] mut data x: bool
    
    // using it is either a compile error, or undefined behaviour
    fmt.Print(x) 
}
```
>[!danger] Error
>Data `x` was not initialized, but an attempt was made to read from it

This attribute can *only* be used with *mutable* data declarations. Immutable data bindings *cannot* be left uninitialized, as they would be permanently unusable.

Immutable declarations with an [initialization block](#Initialization%20Blocks) are no exception. There is no need for `@[NoInit]` there: whenever a block writes its data before reading it, the default value is never observed, so the compiler is free to skip default initialization entirely.

It also does this *outside* of initialization blocks, so `@[NoInit]` is generally unnecessary.
 
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

##### Lists
Lists may be declared via a *list-expression*, and indexed via the *indexing operator*.
```kotlin
data numbers = [5, 10, 15, 20, 25]
fmt.Print("{numbers[2]}")
```
>[!tip] Output
>15

List types are deduced from their members. If: 
- *All* elements in a *list-expression* are of the *same type* as its *first* element
	- That list is an *array-expression*
- *Any* element in a *list-expression* has a type different from its *first* element
	- That list is a *tuple-expression* 

List types are annotated by surrounding a data type with square brackets.
```kotlin
data tuple = [3.5, 2.2859, 8.3, "how'd i get here", 16.323]
data values: [f32] = [3.5, 2.2859, 8.3, "how'd i get here", 16.323]
```
>[!danger] Error
> Data `values` was declared as an array of `f32`, but was assigned a `string`

The *array-type-specifier* takes the form `[T, N]` where:
- `T` is any extant type
- `N` is any integral constant

The integral constant specifies the *size* of the array, and may be omitted in any scenario where the array is immediately assigned.

If the array is *not* immediately assigned, its size *must* be specified in the declaration.

An array which is not immediately assigned will have all its values *zeroed* by default. Much like scalar data, if you want it to be uninitialized, it must be annotated with `@[NoInit]`.
```kotlin
// creates immutable 'x' of size '5' with type 'f64'
// all its elements will have the value '0.0'
data x: [f64, 5] 

// error
data y: [f64] 

// will be uninitialized
@[NoInit] mut data z: [i32, 8]

// error
@[NoInit] data w: [i32, 8]
```
>[!danger] Error
> Data `y` was declared as an array of `f64`, but was not assigned, and has no size specifier
> 
> Data `w` was annotated with `@[NoInit]`, but it is immutable. There is no situation where it could be used

An *empty-list* expression `[]` is valid, but its type *must* be specified in the declaration.

The *tuple-type-specifier* takes the form `[T1, T1, T2, T3, T1, Etc]` where each entry is any extant type, matching *exactly* with the order of declaration of the tuple's literal.