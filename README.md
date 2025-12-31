`ob`
---

`ob` is a "pure object-oriented language" borrowing syntax from Self and
Smalltalk, as well as a library implementing said language. The project's
main goal is to allow users to embed and extend `ob` into applications with
ease.

## Building from source

As of now, `ob` uses the Meson build system, thus building the program
should be as simple as:

``` bash
export $BUILD_DIRECTORY = ".build" # Or another value

git clone --recursive https://github.com/nqr2/ob
cd ob

meson setup $BUILD_DIRECTORY
meson compile -C $BUILD_DIRECTORY

# For building the documentation, after setting up Meson above
# ninja -C $BUILD_DIRECTORY docs
```

While there is a `meson.options` file, the only effect it has is on
whether the tests are built or not.
