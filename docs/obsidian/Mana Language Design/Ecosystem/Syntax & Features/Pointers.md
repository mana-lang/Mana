##### Pointers
A *pointer* is data holding the *address* of other data.

The type of a pointer is written `*T`, where `T` is the type of the data it points to. 

A pointer has the size and alignment of `usize`. 

At runtime, a pointer is *only* an address; the type it points to exists only at compile time.

Like any data, a pointer may only be *re-pointed* if it is mutable.

> [!note]
> Null, pointer arithmetic and conversions are not yet specified.

##### Access
The type of a pointer includes its *access* to the data it points to. `*T` may only *read* that data, while `mut *T` may also *write* it.

The *address-of* punctuator `&` yields an *address-value* whose type is the pointer type of its operand. 

`&x` where `x` is of type `T` yields an *address-value* of type `*T` with *read* access to `x`. 
`mut &x` conversely yields an *address-value* of type `mut *T` with *write* access, and *requires* `x` to also be mutable.
```kotlin
mut data a = 1
mut data c = 2
data b = 3

data p = &a     // *i32
data q = mut &a // mut *i32
data r = mut &b // error: 'b' is immutable

mut data s = &a
s = &c          // 's' is mutable, so it may be re-pointed
```

##### Dereferencing
The *dereference* punctuator `*` yields the data a pointer points to. Writing through a pointer *requires* it to have *write* access.

Member access through `.` dereferences a pointer to a composite type *automatically*. A prefix `*` applies *after* member access, so `*a.b` dereferences `a.b`.
```kotlin
mut data a = 1
data p = mut &a
data q = &a

*p = 5          // writes 'a'
data b = *p + 1 // reads 'a'
*q = 5          // error: 'q' only has read access

data head: *Node = &first
data v = head.next.value // dereferences 'head' and 'head.next'
data n = *head.next      // a copy of the Node 'head.next' points to
```

##### Aliases
An *alias* is a binding to existing data, declared with the `alias` keyword. It is initialized with an address: `&x` aliases `x` with *read* access, and `mut &x` with *write* access.

An alias is used *exactly* like the data it aliases. It holds *no* data of its own, so it can *never* be re-pointed, and `mut alias` does not exist.

Pointers *cannot* be aliased.

Aliases *cannot* be dereferenced. Attempting to do so is a compile error.
```kotlin
mut data player = Player {}

alias health = &player.health
alias position = mut &player.position

position.x += 1 // writes 'player.position.x'
health = 5      // error: 'health' only has read access
```
Parameters may also be aliases. See [Parameters](Procedures.md#Parameters).