# capsh

The capability-aware shell starts applications from inside Capsicum's
*capability mode*.
This allows for untrusted applications to be **sandboxed from inception**.

**Note:** currently, this software only works on a version of FreeBSD that
supports direct execution of `ld-elf.so.1` with an explicit file descriptor
argument, i.e., `12-CURRENT` post-`r318431`.


## Build it:

1. Download, build and install [libpreopen](https://github.com/musec/libpreopen)
1. Build `capsh` with [CMake](https://cmake.org) and (optionally)
   [Ninja](https://ninja-build.org):
    ```sh
    $ git clone https://github.com/musec/capsh
    $ cd capsh
    $ mkdir build
    $ cd build
    $ cmake -G Ninja ..    # or leave out Ninja to generate Makefiles
    $ ninja                # or make, or gmake
    ```

## Use it:

Currently, `capsh` can only do one thing: execute a single binary, specified
by absolute path, from within a Capsicum sandbox.
Also, the binaries it can execute aren't very interesting (we aren't really
leveraging much of [libpreopen](https://github.com/musec/libpreopen) yet),
so you may have to content yourself with:

```sh
$ ./src/capsh /bin/echo "hi"
hi
```
