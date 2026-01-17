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

When taking an array as a parameter, the corresponding length parameter must
come before the pointer to the array (so `f(len, items)`, not `f(items, len)`).

### Header structure

Ideally, a public header should be arranged as:
- Header guards
- The file's documentation
- #includes from the same library
- #includes from dependencies
- #includes from the standard library
- Constants
- Type definitions
- Functions
- Macros

## Documenting

All public headers should include a section as follows:

``` c
/** @file
 *
 * @brief (brief description goes here)
 * (...)
 */
```

If the item is "descriptively named enough" to make documentation redundant,
then it can be omitted. As an example, for a function like
`bool string_equal(lhs, rhs)`, there is no need to add something such as
"Compares two strings, and returns `true` if both are equal".

When documenting the semantics of a function, avoid writing about
implementation details if unneeded.

## TODO
- Use vocabulary from RFC 2119?
- What *else* should go on each section?
- Add notes on how to @brief and describe files.
- How to document stuff.

# Commits

## Messages

Commit messages should follow the format `<section>: <action>` where "section"
refers to the "logical section of code" the commit modifies, and "action"
being "written as if ordering the repository to change, and not as if
describing your changes", compare:

- Remove array bounds checks (yes)
- Remove*d* array bounds checks (no)

As an example, if I were to add a new option to the `tools/obc` binary, I'd
write `tools/obc: Add a new option (to do this and that and...)`.

I often add `(oops)` to small bugfixes from oversights and the like, this is
not required.

## Size

I personally prefer small and "atomic" changes (in general).
