##### Memory Management
Mana manages memory *deterministically*. There is no garbage collector, and memory is managed identically by **Circe** and **Salem**.

Dynamic memory is obtained through *allocators*. Data may *never* outlive the allocator it was allocated from.
##### Allocator Types
An allocator type is declared with `alloc type`, and implements the *allocator interface* with `interface for alloc type`.
```rust
alloc type Pool {
	memory: Block
	slot_size: usize
	free_list: usize
}

interface for alloc type Pool {
	fn Alloc(mut *This, size: usize, align: usize) -> Block { }
	fn Free(mut *This, block: Block) { }
	fn Enter(*This) -> usize { }
	fn Exit(mut *This, token: usize) { }
}
```
- `Alloc` is *required*.
- `Free` is *optional*. Without it, freeing memory does nothing.
- `Enter` and `Exit` are *optional*, but must be implemented *together*.

##### Allocator Declarations
Allocator types may *only* be instantiated by `alloc` declarations. Allocators are *always* mutable.

Allocators may be declared in *any* scope, and are destroyed when their scope exits, like any other data. They are destroyed only after *all* data in their scope has been destroyed. For allocators declared at module scope, this is all module data of the program. Their order among one another follows the same rules as [module data](Data.md#Module%20Data).
```rust
alloc pool: Pool
alloc bullets = Pool {.slot_size = 64}

data p: Pool // error
```
Allocators are named in the `alloc` namespace, much like [tags](Tags.md) are in the `Tag` namespace: `alloc.pool`, `alloc.bullets`. The allocators of another module follow the usual disambiguation rules, as in `Foo.alloc.pool`.

##### Default Allocators
The standard library provides a general-purpose heap allocator, `std.alloc.default`, which is the *default allocator*, and an arena allocator, `std.alloc.arena`. Compiler flags may select their implementations.

##### Scoped Allocators
A scope or function annotated with `@[Allocator(name)]` uses the named allocator as its *current allocator*. Names are taken from the `alloc` namespace, so `@[Allocator(pool)]` refers to `alloc.pool`, and `@[Allocator(Foo.pool)]` to `Foo.alloc.pool`. The initial namespace may always be omitted if it is the same as the current module's namespace.

If the allocator implements `Enter` and `Exit`, `Enter` is called when the scope is entered, and `Exit` is called with its result whenever the scope is exited.
```rust
@[Allocator(std.arena)] {
	data visible = Cull(level)
	Draw(visible)
} // the arena rewinds, freeing 'visible'
```

##### Allocator Capture
Data that allocates *captures* the *current allocator* when it is initialized, and uses it for its entire lifetime.

The current allocator is that of the innermost `@[Allocator]` scope, or the default allocator otherwise.

##### Pages
Memory is obtained from the operating system in *pages*, through three functions provided by the compiler in `std.pages`. All allocators, including the [default allocators](#Default%20Allocators), are built on them.
```rust
fn Size() -> usize           // the page size of the target platform
fn Map(size: usize) -> Block // 'size' is rounded up to whole pages
fn Unmap(block: Block)
```
Memory returned by `Map` is *zeroed*.

A `Block` is a region of memory:
```rust
type Block {
	address: usize
	size: usize
}
```
> [!note]
> The type of `address` is provisional, until pointers are specified.

Under **Salem**, `Map` and `Unmap` call the operating system directly. Under **Hex**, they call the *page source* of the host, which may be provided through the embedding API, and otherwise defaults to the operating system.