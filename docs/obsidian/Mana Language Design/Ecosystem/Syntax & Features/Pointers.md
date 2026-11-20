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
mut data health = 100
mut data armor = 50
data level = 3

data reader = &health     // *i32
data writer = mut &health // mut *i32
data invalid = mut &level // error: 'level' is immutable

mut data target = &health
target = &armor           // 'target' is mutable, so it may be re-pointed
```

A pointer to a pointer is written `**T`. Each `*` carries its own access, as in `mut *mut *T`.

##### Dereferencing
The *dereference* punctuator `*` yields the data a pointer points to. 

For a pointer `p`, the data `*p` yields is *only* mutable when `p` has *write access*.
In order words, its type *must* be `mut *T`
```kotlin
mut data health = 100
data writer = mut &health
data reader = &health

*writer = 5              // writes 'health'
data total = *writer + 1 // reads 'health'
*reader = 5              // error: 'reader' only has read access
```

Member access through `.`  *automatically* dereferences a pointer to a composite type. A prefix `*` applies *after* member access, so `*a.b` dereferences `a.b`
```kotlin
data head: *Node = &first
data v = head.next.v           // dereferences 'head' and 'head.next'
data next = *head.next         // a copy of the Node 'head.next' points to
data invalid = mut &head.v     // error: 'head' only has read access
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
An alias is may *only* be declared in *local-scope*. To hold an address in a [parameter](Procedures.md#Parameters) or [field](Types.md#Composite%20Types), you must use a pointer.