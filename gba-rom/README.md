From a fresh install:
```
[gba-player/gba-rom] $ mkdir build
[gba-player/gba-rom] $ cmake -S ./ -B build --toolchain=cmake/gba.toolchain.cmake
[gba-player/gba-rom] $ cd build
[gba-player/gba-rom/build] $ make -j
```