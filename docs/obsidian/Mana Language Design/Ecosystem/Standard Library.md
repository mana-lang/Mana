##### Standard Library
This page lists *planned* modules of the standard library.

##### Format
`std.Format` provides text formatting and output, such as `Print` and `PrintLine`.

##### Procedure
`std.Procedure<S>` holds *any* procedure meeting the [signature](Syntax%20&%20Features/Procedures.md#The%20%60fn%60%20Keyword) `S`, including procedures with fields. It *owns* a copy of its procedure.

A `std.Procedure` is the size of two pointers, and *only* accepts procedures that can be copy-initialized. The copy of a procedure with fields is stored in memory from the [current allocator](Syntax%20&%20Features/Memory.md#Allocator%20Capture).

##### Timer
`std.Timer` records the time when it is initialized, and again when it is destroyed. It stores the difference between the two.
