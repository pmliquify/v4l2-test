# V4L2 testing application

## Version 0.1.0

## Build

```
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release ..
$ cmake --build .
```

## Build for aarch64 targets (cross compile)
```
$ sudo apt update
$ sudo apt install binutils-aarch64-linux-gnu g++-aarch64-linux-gnu
$ mkdir build
$ cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=../aarch64-linux-gnu-toolchain.cmake ..
$ cmake --build .
```