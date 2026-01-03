
##### Tags
*Tags* are a hierarchical labeling system in **Mana**.

They are essentially hierarchical enums, which may be compared against their hierarchy through various comparison operators. 

They can efficiently compress a lot of categorical state information into a small space, and allow for expressive querying against other tags.

Unlike most keywords in **Mana**, the `Tag` keyword is capitalized.
```kotlin
Tag Entity {
	Ally
	Enemy
	
	// sub-tags may be defined inline
	Neutral {
		Villager
		Drunkard
		Orphan
	}
}

// or in their own blocks
Tag Entity.Ally {
	Paladin
	Priest
	Mage
}

Tag Entity.Enemy {
	Zombie
	Warlock
	Vampire
}

// tags do not occupy their type's name
// instead, they're added to the global 'Tag' type
struct Entity {
	data tag: Tag.Entity
	data bark: string
	data health: i32
}

// the 'Tag' type must be specified when using tag values
data janet = Entity {
	.tag = Tag.Entity.Enemy.Zombie
	.bark = "I thought zombies became stupid. This sucks."
	.health = -100
}

// this wouldn't compile
data john = Entity {
	.tag = Entity.Neutral.Orphan
	.bark = "Will someone adopt me"
	.health = 2
}
```
>[!danger] Error 
struct `Entity` contains no member `Neutral`. Did you mean `Tag.Entity.Neutral`?

You can compare hierarchical tags with one another, including different levels of their hierarchy. Tag comparisons
```kotlin
// the tag expression 'A in B' describes 
// whether /all/ the tags of A are present in B
if Tag.Entity.Enemy in Tag.Entity.Enemy.Zombie // true

// this expression is non-commutative
if janet.tag in Tag.Entity.Enemy // false

// checks if /any/ tags in their hierarchies match
if john.tag in? janet.tag // true

// equals for exact comparison
if Tag.Entity.Enemy == janet.tag // false
```

The `Tag` keyword also includes some functionality to manipulate tags.
```kotlin
if Tag::parent(john.tag) == Tag.Entity.Neutral // true

if Tag::root(john.tag) == Tag::root(janet.tag) // true, Tag.Entity

// new tags can be created at runtime
if Tag::leaf(john.tag) == Tag.Orphan 

// new_tag == Tag.Entity.Neutral.Orphan.Painter
mut data new_tag = john.tag + Tag {Painter}

// new_tag == Tag.Neutral.Orphan.Painter
new_tag -= Tag.Entity

new_tag -= Tag // won't compile
```

##### Tag Imports
Normally, Tags belonging to a foreign Artifact must be accessed via the scope resolution operator `::`

`SomeArtifact::Tag.Status.Burning`

One may expose