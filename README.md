`ob`
---

`ob` is a "pure object-oriented language" borrowing syntax from Self and
Smalltalk, as well as a library implementing said language. The project's
main goal is to allow users to embed and extend `ob` into applications with
ease.

## Building from source

As of now, `ob` uses the CMake build system, so building the program
should be as simple as:

``` bash
export BUILD_DIRECTORY=".build" # Or another value

git clone https://github.com/nqr2/ob
cd ob

# For just building
cmake --workflow --preset "debug" # or "release"

# Or for building and testing
# cmake --workflow --preset "debug-and-test"

# For building the documentation, after setting up CMake above
# cmake --build --preset documentation
```

## Debugging

`ob` is fairly incomplete and full of bugs, and so there are a couple tools for
debugging, notably:

- bin/ob accepts a `-v` parameter, which sets the verbosity of the logs, and an
  `-e` parameter for reading from the command line.
- tools/obc compiles `ob` code into bytecode and emits it to stdout, and has a
  similar `-e` parameter.
- tools/dis reads bytecode from a file or stdin and prints a description of
  it's contents.

A few examples include:

``` sh
echo "'Hello' print." > FILE

# Print the bytecode from a file.
tools/obc FILE | tools/dis

# Or from the command line instead.
tools/obc -e "'Hello' print." | tools/dis

# Or show some logs when interpreting.
bin/ob -v 5 FILE

# Or also from the command line.
bin/ob -v 5 -e "'Hello' print."
```
