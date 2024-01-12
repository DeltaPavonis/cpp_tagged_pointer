# cpp_tagged_pointer

A type-tagged pointer supporting runtime type identification and dynamic dispatch, written in C++20. Based on the `pbrt::TaggedPointer` type from [pbrt-v4](https://github.com/mmp/pbrt-v4).

## Overview

In C++, using pointers to abstract base classes incurs storage overhead. Specifically, instances of polymorphic objects behind a pointer to their abstract base class include an extra, hidden pointer to a virtual function table. This pointer is needed in order for virtual function calls to be correctly resolved at runtime; it is needed to dynamically dispatch virtual function calls to the correct derived type. On modern systems, these pointers are 8 bytes in size. This means that every instance of an object that inherits from an abstract base class incurs a storage penalty of 8 bytes. This overhead quickly adds up for programs that make wide use of polymorphism.

All of this motivates the `TaggedPointer` class, which represents a pointer to one of a given set of types which additionally dynamically maintains its current type <b>without requiring any extra storage beyond the original pointer</b>. This is done by storing the current type as a pointer tag; by "folding" the information necessary to perform runtime type identification into the pointer address itself. `TaggedPointer` thus allows for dynamic dispatch and runtime polymorphism <b>without the traditional storage overhead from virtual table pointers</b>.

See `example.cpp` for a generalizable `TaggedPointer` usage example.

## Usage
To use `TaggedPointer`, simply add `#include "tagged_pointer.h"` to your program.

## Acknowledgements
As mentioned above, this implementation was heavily inspired by the one for `pbrt::TaggedPointer`, used in [pbrt-v4](https://github.com/mmp/pbrt-v4).

