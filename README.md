=======
# Chani - Simulflow Debugger

Chani is a 8086/DOS emulator and debugger, aiming to be useful for reverse 
engineering DOS games.

Parts of Chani are currently hard-coded to support the game Dune by Cryo 
Interactive.

To run Dune, copy the files `DNCDPRG.EXE` and `DUNE.DAT` into your build folder 
and run `chani DNCDPRG.EXE`

## Building

Chani uses [CMake][cmake] for building build files. Create a build directory 
either inside or outside the source tree and run cmake with the path f the 
source directory as an argument.

On a unixy platform, to build for debug use:

```sh
cmake <path-to-source>
make -j
```

Please be aware that a debug build is very slow.

To build for release use:

```sh
cmake -DCMAKE_BUILD_TYPE=Release <path-to-source>
make -j
```

# License

Chani is made available under the terms of the [MIT][mit] license. see the file 
`LICENSE` for details.

The components contained in the `3rdparty` folder are included for convenience 
and are available under individual terms. See their license terms or details.

[cmake]: https://cmake.org
[mit]: https://opensource.org/licenses/MIT
