[miniKanren](http://minikanren.org/) (logic programming) implementation for C++ using coroutines.

# Motivation

See miniKanren for the main explanation.
As a simple example consider an `append` function, which combines two lists
([sample](sample/src/main.cpp)). Given such a single function implementation multiple kinds of queries
can be performed against it: the obvious one, given two lists - what's the combined list;
but also, given a combined list - which arguments can be used to obtain such a list;
given a combined list and one of the arguments - what's the other argument.

# Usage

Key operations are the following: `eq` unifies values; `fresh` and `conde` roughly
follow the miniKanren meanings.

`RelationFuture` type defines the coroutine. The key operations mentioned above need
to be awaited (`co_await`).

`Run` class is used to run the queries, provides iterators over the results.

To make the solution search efficient unification should be able to look into parts of
values (like list elements) - this is described by the `Fragmentable` concept.

`minikanrenpp/minikanren.hpp` is the all-in-one header.

## Other

`xvariant` is like `std::any`, but allows to restrict the stored types to fulfil
a certain static interface.

# Building

Uses CMake.

Can be as simple as
```
mkdir build; cd build
cmake -DENABLE_SAMPLE=on ..
cmake --build .
```
