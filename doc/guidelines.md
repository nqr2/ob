# Coding guidelines.

For convenience, I've decided to set some guidelines to follow (mostly so I
don't forget them).

## Naming

*All* public symbols must have `ob` or `OB` as a prefix, which can then be
continued by the name of a "logical grouping" (see `obstr` for *str*ings,
`obobj` for *obj*ects, and so on), and then an underscore. This prefix should
only be uppercase for macros and constants, and lowercase anywhere else.

Macros and constants should always be in `UPPER_SNAKE_CASE`, types should be in
`PascalCase`, while functions should be in `snake_case`.

Function pointers must have a `Fn` prefix, followed by the "name" of the
function (see `FnVisit`, `FnAllocate`).

For types that are mostly used as pointers (see `Object`, `String`, `Allocator`),
there should be an abbreviated typedef (`Obj`, `Str`).

## Programming

Global variables are forbidden.

## Documenting

Nothing here yet.

## TODO
- Use vocabulary from RFC 2119?
- What *else* should go on each section?
- Add notes on how to @brief and describe files.
- How to document stuff.
