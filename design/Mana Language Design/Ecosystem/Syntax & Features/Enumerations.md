
**Mana** provides three means of creating enumerations; *enums*, *tags* and *variants*.

Mana's *enums*, like many other languages, offer a simple means of creating labeled values. However, they can do a lot more than that.
##### C++ Style Enums
If you're familiar with the **C++** `enum class`, that is how Mana enums work in their most basic form.
```kotlin
enum Colour {
	Red    // 0
	Green  // 1
	Blue   // 2
}

data colour = Colour.Red

// you can specify what the underlying type of an enum is
enum Animal : u8 {
	Cat     = 2 // enums will count up from the latest value
	Dog    // 3
	Lizard // 4
}

data animal = Cat // won't compile
```
>[!danger] Error
> Attempted to assign unknown value `Cat` to data `animal`. Did you mean `Animal.Cat`?

Much like in C++, *enums* in Mana are *scoped* and can *not* be implicitly converted to their underlying type by default.

##### Extensibility
**Mana** also offers the ability to extend an enum by *appending* new labels to it. To make an enum extensible, you must annotate it with the `mut` keyword.

You may only append labels to an extensible enum from another enum. If you wish to add single values, this is possible by creating an *anonymous enum* inline. That label will then be parented to the enum it's being appended to. 
```kotlin
mut enum Dog {
	Terrier
	Bulldog
	Spaniel
}

// in another file
enum Dog += enum { Malamute }

// or entire sets
enum Dog += enum { BassetHound, Beagle, }

// it is not allowed to remove labels of the same parent
enum Dog -= Dog.Beagle
```
>[!danger] Error
> Attempted to remove label `Malamute` from enum `Dog`, but `Dog` is the parent of `Malamute`

You can also append from named enums.
```kotlin
mut enum Weapon {
	Glock
	Katana
	Shoe
}

// constant enums may still be appended to mut enums
enum MeleeWeapon {
	Zweihander
	Halberd
	Khopesh
	Flail
}

// append 'MeleeWeapon' labels to 'Weapon'
enum Weapon += MeleeWeapon

// these labels will be treated as though they are part of 'Weapon'
data flail = Weapon.Flail

// 'MeleeWeapon' is still considered its own valid enum
data melee_flail = MeleeWeapon.Flail

// enums can easily be serialized to strings
std.println("{1}: {2}", enum.AsString(flail), enum.Underlying(flail))
std.print("{1}: {2}", enum.AsString(melee_flail), enum.Underlying(melee_flail))
```
>[!tip] Output
> Weapon.Flail: 6
> MeleeWeapon.Flail: 3

You can remove labels whose original parent differs from the enum they are appended to.
```kotlin
// continuing from before...

enum Weapon -= MeleeWeapon.Zweihander

// this label is no longer a part of 'Weapon'
data big_sword = Weapon.Zweihander // error, won't compile

// other values still remain a part of it
data long_reach = Weapon.Khopesh

// unless you remove the entire enum
enum Weapon -= enum MeleeWeapon
```
> [!note] Note
Appends made in global scope *cannot* be removed.

It is either a *compile error* or *undefined behaviour* to use an enum after its label has been removed, depending on the circumstance. 

However, enum appends are *scope-bound*; they *automatically* get removed at the end of the scope where they were added.
```kotlin
// continuing from before...

enum Weapon += MeleeWeapon.Khopesh
data sword = Weapon.Khopesh

// compile error, 'Weapon.Khopesh' is still in use
enum Weapon -= MeleeWeapon.Khopesh 

// -----
// another example
{
	enum Weapon += MeleeWeapon.Khopesh
}
// compile error, enum 'Weapon' has no label 'Khopesh'
data sword = Weapon.Khopesh


// -----
enum Weapon += MeleeWeapon.Khopesh

{
	data sword = Weapon.Khopesh
	// do something with `sword`
}

enum Weapon -= MeleeWeapon.Khopesh // ok
```
> [!warning] Warning
While it is fine to *append* to an enum, it is generally *discouraged* to explicitly remove from it, as that may lead to unexpected bugs. It is better to allow removal to happen automatically.

##### Underlying type
To override the default behaviour of enums, such as changing their enumeration values, you must *explicitly* specify their underlying type.  This is because enums will, by default, be the smallest underlying type they can count up to, which is `u8` for the vast majority of cases.

However, you may want negative values for enum fields, or to utilize them as bit flags. To prevent unexpected behaviour in those cases, annotation is mandatory when overriding enum fields.

The underlying type *must* be an *integral* type, such as `i32`. 
```kotlin
// the following enum declaration won't compile
enum Unspecified {
	A = 10
	B = 20
}

// but now it will
enum Specified : u8 {
	A = 10
	B = 20
}
```
> [!note] Note
> Even though `bool` is technically an integral type, it is uniquely *not* permitted for use as an enum's underlying type.

By default, each next label will increase the previous label's value by 1. However, this behaviour can be overridden by specifying *at least two* label values explicitly. Mana will then follow the pattern described by the programmer until it encounters another set of *two* explicit values, or the enum ends.
```kotlin
// normally, enums increment...
enum Countdown : u8 {
	Ten    = 10
	Nine   = 9
	Eight // 8
	Seven // 7
	Six   // etc... -> WILL underflow to 255 eventually, will warn if so
}

enum Skip : u8 {
	Zero
	Two    = 2
	Four   = 4
	Six   // 6
	Eight // etc...
}

// it only considers the last two entries
enum Fibonacci : u8 {
	A  = 1
	B  = 1 // still unsure if this should be an error
	C  = 2
	D  = 3
	E  = 5
	F // 7 
	G // 9
	// not quite fibonacci...
}

// specifically, only the last two /specific/ entries
enum Pattern : u8 {
	A   = 3
	B   = 6
	C  // 9
	D   = 10 // specifically specific
	E  // 14
	F   = 15
	G   = 17
	H  // 19, etc
}

```

