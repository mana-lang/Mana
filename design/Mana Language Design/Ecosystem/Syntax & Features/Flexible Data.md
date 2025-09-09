
##### Flexible Data
Types may be annotated with the `Flexible` attribute. 

This allows **Salem** (but *not* **Circe**) to manipulate the type under the hood to *optimize* it for *cache locality* based on context such as its *access patterns*.

In the below case, type `Foo` is taking up more space than necessary due to *alignment requirements*. It is taking up 40 bytes, while only using 23 bytes of data. On its own, 17 bytes of wasted data isn't that big of a deal; however, once you have 100K `Foo` objects that you are accessing regularly, you will encounter far more cache misses than necessary, leading to a degradation in performance.

These issues can compound and cause your game to run at lower frame-rates, which gives you less room to add fancy assets, effects and features.

Let's look at an example of an arbitrary type with tons of padding:
```rust
type Foo {
	a: i32   // 4 bytes + 4 padding
	b: f64   // 8 bytes
	c: bool  // 1 byte + 7 padding
	d: i64   // 8 bytes
	e: u16   // 2 bytes + 6 padding
}
```

With the `@[Flexible]` attribute, **Salem** can automatically reorder these type members to waste the least amount of space.

You can *still write the type the same way*, and **Salem** will reorder the type *under the hood*.

This reduces the padding down to 1: 
```rust
@[Flexible]
type Foo {
	a: i32   // 4 bytes
	b: f64   // 8 bytes
	c: bool  // 1 byte
	d: i64   // 8 bytes
	e: u16   // 2 bytes + 1 padding
}
```

Because the type has been *reordered* to:
```rust
type Foo {
	b: f64   // 8 bytes
	d: i64   // 8 bytes
	a: i32   // 4 bytes
	c: bool  // 1 byte
	e: u16   // 2 bytes + 1 padding
}
```

Note that **Salem** will reorder *as few members as possible* to respect as much of the specified order as it can. As a result, the first two and last three members are still in alphabetical order, respectively.

Reordering of type members is only one of the things the `@[Flexible]` attribute does. The other is related to *access patterns*.

Suppose you create a `[Foo, 5]`. Ordinarily, **Mana** orders that array in memory as follows:

<span style="font-size:0.7em;color:dimgrey"> (split into blocks of 5 for readability) </span>
`[a][b][c][d][e] [a][b][c][d][e] [a][b][c][d][e] [a][b][c][d][e] [a][b][c][d][e]`

Each of the members are laid out sequentially in memory.

If the type is `@[Flexible]` however, **Salem** might do something like this:

`[a][a][a][a][a] [b][b][b][b][b] [c][c][c][c][c] [d][d][d][d][d] [e][e][e][e][e]`

It *reordered* the array to optimize for repeatedly accessing the same member in each `Foo`, rather than skipping over to the next `Foo` on every access.

Reordering may take *any* shape under the hood, as long as it produces the same results.

> [!warning]
When data is marked `@[Flexible]`, it must *not* be used in a way that makes assumptions about its layout. `@[Flexible]` data layouts may change even *between different compilations*, if the data's access pattern has been modified somewhere. 

> [!warning]
The `@[Flexible]` attribute is currently still highly experimental, and as such, data which is marked `@[Flexible]` may *only* contain members which are either primitives, or *also* marked `@[Flexible]`. Mixing flexible and rigid data will lead to a *compile error*.