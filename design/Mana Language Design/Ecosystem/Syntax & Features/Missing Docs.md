Below is a list of features that still need documentation, but haven't been explored yet for some reason or another:

##### Concurrency Model
It's currently unclear which concurrency model would be best for Mana, but probably a simple thread splitting keyword alongside a robust threading library would be good. Simple example:

```rust
import std.fmt

fn SayHi() {
  loop 5 {
    fmt.Print("Hi ")
  }
}

fn Main() {
  split SayHi()

  loop 5 {
    fmt.Print("Woah ")
  }
}
```
>[!tip] Output
>Hi Hi Woah Hi Woah Woah Woah Hi Woah Hi

This, of course, does not address things such as the capability to perform atomic operations and ensure thread safety. Whether this is to be handled by the threading library or some other way is something that requires considerable deliberation, and so this is left for when the language is more mature and robust.


##### Imports & VTUs
While the design for how Virtual Translation Units work is solid, there is still work to be done around the implementation details of how to acquire a list of symbols off of imports without doing a full compilation.

The smartest idea currently seems to be to perform the parsing and semantic analysis steps to obtain a valid symbol table, which can then be used to resolve dependencies between Artifacts. However, this is something that is likely best left shelved until after all procedure forms and UDT declarations have been implemented. The implementation of those may have considerable impacts on the implementation of imports.


##### Error Handling
I simply have not done enough research on language-level error handling mechanisms to feel confident implementing any particular one. I personally dislike OOP-style "exceptions," but I can't deny that there is likely some benefit to stack unwinding. On the other hand, I can't say that Rust's monad style of error handling is the ideal either.

This isn't something that is being overlooked, but as with everything else in this file, I feel as though having established other language features may glean some insight into what would work best. I'm a big proponent of "design for what people are already doing," and so if I find myself wishing for or tending towards a certain form of error handling, then I may bake that into the language if that seems appropriate.

With that said, Mana already has support for `variant`s, so that means we get Rust-style monadic error propagation (ie. `Result<T, E>`) out the gate. Though the question remains whereas `variant` will resemble Rust-enums enough for that to be a sensible thing to do.

Research should also be done into languages that "panic."


##### Asserts
Everybody loves asserts. I wouldn't have an issue baking something like `assert` and `assert when` into the language, however, I feel as though this should be put on hold until the error handling mechanism has been determined, as that may render asserts moot.


##### Iterators
What is an iterator? ~~(And how much does it weigh?)~~

What the exact definition of "iterator" is in Mana remains to be seen. I know that it will be the mechanism via which `for x in y` statements can decompose collections into a loop over their elements, but how exactly this will be achieved remains to be seen.


##### Type conversions
This one is thankfully (hopefully) not that complex. It is simply on hold until the type system is solid. For the time being, native conversions are automatically done between integral primitives. The syntax will likely look like `TypeName(x)`, e.g. `i32(foo)`


##### Type Aliasing
This one should also be simple, but is useless until a proper type system exists.


##### Strings
UTF-8? UTF-32? ASCII? String conversions? Views? Slices?

Strings get very complicated very fast. For the time being, we'll just support ASCII natively and, very soon, UTF-8. After that, some serious thought will have to be put into strings.


##### Runtime Polymorphism
This one is definitely a doozy. Considering how many games rely on runtime polymorphism, it may be a requirement. Still, I would like to do research to determine whether there are more performant alternatives that provide the same amount of intuitive flexibility and expressive power.


##### FFI
The dooziest dooze of them all. How Hex interfaces with foreign functions is one question, and then there's also the question of how Salem would generate binaries that are compatible with C or C++ shared libraries. The Hex question has to be answered first of course, but it should likely wait until after literally everything in this file has been covered. There's no point dealing with C/C++ ABIs when Mana's ABI is so unstable it may as well not exist.

The attribute system is nice because it lets us introduce these things in the form of something like `@[External]` rather than introducing a completely new keyword.


##### Memory Allocation
Mana must provide a means to allocate raw memory from the OS, and the `std` library should make doing so easy and safe. But the core mechanism does need to exist.

However, this is completely irrelevant until Salem enters active development. Hex should perform garbage collection and memory management as a runtime.

How memory-related operations interface with Hex is also a point of consideration.

