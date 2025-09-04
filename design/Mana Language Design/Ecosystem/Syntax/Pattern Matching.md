*Pattern matching* in **Mana** works roughly the same as **Rust**'s pattern matching.

```rust
match x {
	Animal.Dog => do_dog_thing()
	Animal.Cat => {
			do_some()
			cat_things()
		}
	Animal.Lizard => do_lizard_things()
	_ => 
}
```
