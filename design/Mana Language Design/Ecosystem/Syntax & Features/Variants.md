*Variants* in **Mana** work very similarly to **Rust** *enums*.

```kotlin
variant Action {
	Idle
	Jump {height: f32}
	ChangePosition {Vec3}
	ChangeColour {i32, i32, i32}
	Attack {
		target: &Entity
		ability: Ability
	}
}
```

Variants are similar to *enums* in that they indicate a list of possibilities, however, variants can hold *any* kind of data, whereas enums are always some kind of integral constant. This makes Variants a *sum type* and allows you to utilise all the power you would expect from Rust enums.