From a fresh install:
```
[gba-player/gba-rom] $ mkdir build
[gba-player/gba-rom] $ cmake -B build --toolchain=cmake/gba.toolchain.cmake -DCMAKE_INSTALL_PREFIX="$PWD/build"
[gba-player/gba-rom] $ cd build
[gba-player/gba-rom/build] $ make -j
```