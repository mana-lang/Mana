
*Variants* in **Mana** define a *sum type*. They provide a way to enumerate with a payload.

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

Variants are similar to *enums* in that they indicate a list of possibilities; however, variants can hold *any* kind of data, whereas enums are always some kind of integral constant.

While variants have a *discriminant*, this is opaque to the user. 