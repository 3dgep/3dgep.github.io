# 2D Software Rasterizer

A 2D software rasterizer in C++23, built step by step in the
2D Software Rasterizer series on [3dgep.com](https://www.3dgep.com/).

- `rast/` is the rasterizer library. It grows with every part.
- `examples/` has one program per part, such as `examples/part-01-framebuffer`.

Every part has a Git tag (`part-01-framebuffer`, `part-02-clipping`, ...) with the code at the end of that article.

## Building

You need CMake 3.28 or newer and a C++23 compiler with `<mdspan>` and `<print>`:
Visual Studio 2026, GCC 15, or Clang 19 with libc++.
CMake downloads SDL3 and stb_image for you.

On Windows:

```
cmake --preset windows
cmake --build --preset windows-release
```

On Linux or macOS (with Ninja installed):

```
cmake --preset unix
cmake --build --preset unix-release
```

The programs end up in `build/<preset>/bin`.

For the web, you need [Emscripten](https://emscripten.org/) and Ninja.
[cmake/EmscriptenToolchain.cmake](cmake/EmscriptenToolchain.cmake) finds Emscripten through the `EMSDK` environment variable
(set when you activate the Emscripten SDK), the `EMSCRIPTEN` environment variable, or `em-config` on the `PATH`
(for an Emscripten installed with a package manager):

```
cmake --preset web
cmake --build --preset web
```
