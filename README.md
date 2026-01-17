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

git clone --recursive https://github.com/nqr2/ob
cd ob

cmake -B "$BUILD_DIRECTORY" # any options here, as -DOPTION=VALUE
cmake --build "$BUILD_DIRECTORY"

# For building the documentation, after setting up CMake above
# cmake --build "$BUILD_DIRECTORY" --target docs
```

As of now, there are options for building the tests and documentation
(`BUILD_TESTS` and `BUILD_DOCS` respectively), and for enabling certain
features (`ENABLE_ASSERT` and `ENABLE_LOG`), though these don't do anything
for now.
