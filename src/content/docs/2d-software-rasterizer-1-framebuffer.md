---
title: "2D Software Rasterizer Part 1: The Framebuffer"
description: "Start a 2D software rasterizer in C++23: a CMake project with SDL3, a 32-bit ARGB color, a framebuffer viewed through std::mdspan, and one streaming texture that puts it on screen."
slug: 2d-software-rasterizer-1-framebuffer
lastUpdated: 2026-11-28
---

Your GPU draws millions of triangles every frame, and most of the time you never see how it does it. You hand it vertices and a shader, and pixels come out at the other end. In this series, we build that machinery ourselves. A *software rasterizer* runs on the CPU and decides, for every pixel on the screen, which color it gets. Nothing is hidden, because every pixel is written by code that you wrote.

We start with the simplest possible thing, a block of memory that holds the pixels, and grow it into a renderer that draws lines, circles, triangles, textures, transparent shapes and text. In the second half of the series, we make it fast. We split the screen into tiles, draw them on all the cores of your CPU, and process several pixels at once with SIMD instructions.

In this first part, you will:

1. set up a CMake project that downloads SDL3 for you,
2. learn how a 32-bit color is packed into a single integer,
3. build a *framebuffer*, a 2D grid of pixels in memory, and view it with C++23's `std::mdspan`,
4. show the framebuffer in a window, through a single texture that we update every frame, with keys for fullscreen and VSync,
5. and fill it with an animated gradient.

At the end of this part, your program shows this:

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-1-framebuffer/index.html" title="An animated gradient: red increases to the right, green increases downwards, and diagonal stripes of blue move towards the bottom-right corner" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result of part 1, running in your browser. Every pixel of it is computed by the CPU, and the GPU only shows the finished image.

</div>

The code for this part is on GitHub, at the [`part-01-framebuffer`](https://github.com/3dgep/2d-software-rasterizer/tree/part-01-framebuffer) tag of the [2d-software-rasterizer](https://github.com/3dgep/2d-software-rasterizer) repository.

## The Project

You need CMake 3.28 or newer and a compiler that supports C++23, including the `<mdspan>` and `<print>` headers. Visual Studio 2026, GCC 15 and Clang 19 (with libc++) all do. You don't need to install any libraries, because CMake downloads them when you configure the project.

The repository has two parts. The `rast` folder is the rasterizer library, which grows with every part of the series. The `examples` folder has a small program for every part, which uses the library to show what that part is about:

```txt
2d-software-rasterizer
├── CMakeLists.txt
├── CMakePresets.json
├── cmake
│   ├── EmscriptenToolchain.cmake
│   └── web-shell.html
├── rast
│   ├── CMakeLists.txt
│   ├── include/rast
│   │   ├── Color.hpp
│   │   ├── Surface.hpp
│   │   └── Window.hpp
│   └── src
│       ├── Surface.cpp
│       └── Window.cpp
└── examples
    ├── CMakeLists.txt
    └── part-01-framebuffer
        ├── CMakeLists.txt
        └── main.cpp
```

The headers are in `include/rast`, so a program includes them as `<rast/Color.hpp>`, and it's always clear which library a header comes from. The `cmake` folder is only used for the web build, which runs the examples in your browser (like the one at the end of this article).

### The Top-Level CMakeLists.txt

The `CMakeLists.txt` in the root of the repository starts by asking for C++23:

```cmake source="2d-software-rasterizer-part-01/CMakeLists.txt#L1-L10" title="CMakeLists.txt"
```

`CMAKE_CXX_EXTENSIONS OFF` asks for standard C++ (`-std=c++23`) instead of the GNU dialect (`-std=gnu++23`), so the code compiles the same way with every compiler. Every executable goes into the `bin` folder of the build, which makes them easy to find.

Next, CMake downloads [SDL3](https://www.libsdl.org/):

```cmake source="2d-software-rasterizer-part-01/CMakeLists.txt#L12-L27" title="CMakeLists.txt"
```

SDL opens the window, handles the keyboard and the mouse, and puts our pixels on the screen. That's all we use it for. It never draws a line or a triangle for us. `FetchContent` clones SDL at a fixed release (`GIT_TAG`), so everyone who builds the project gets the same version. We build SDL as a static library, so its code becomes part of each executable and there's no DLL to copy next to it. `SYSTEM` treats SDL's headers as system headers, which means that their warnings don't show up in our build.

Our own code gets all the warnings that the compiler has to offer:

```cmake source="2d-software-rasterizer-part-01/CMakeLists.txt#L29-L39" title="CMakeLists.txt"
```

`rast_set_warnings` turns on the high warning levels for one target. With MSVC, `/permissive-` also makes the compiler follow the standard more strictly, and `/utf-8` reads the source files as UTF-8. With GCC and Clang, `-Wconversion` warns about every conversion that can lose information, like an `int` that is stored in a `std::uint8_t`. A rasterizer converts between integers and floats all the time, and we want each of those conversions to be visible in the code. Finally, CMake goes into the `rast` and `examples` folders.

### The rast Library

The library's `CMakeLists.txt` lists its files:

```cmake source="2d-software-rasterizer-part-01/rast/CMakeLists.txt" title="rast/CMakeLists.txt"
```

The include directory is `PUBLIC`, so every program that links `rast` can include its headers. SDL is linked `PRIVATE`, because only `Window.cpp` uses it. A program that uses the library never has to include an SDL header, and that's what keeps the rasterizer independent of SDL. The `EMSCRIPTEN` part is for the web build, and we come back to it when we look at `Window::Present`.

### The Examples

Every example is built with the same function, in `examples/CMakeLists.txt`:

```cmake source="2d-software-rasterizer-part-01/examples/CMakeLists.txt" title="examples/CMakeLists.txt"
```

`rast_add_example` makes an executable from the source files that you give it, links it to `rast` and turns on the warnings. For the web, it makes a web page instead, `index.html`, in a folder of its own. Then every part's folder only needs a single line:

```cmake source="2d-software-rasterizer-part-01/examples/part-01-framebuffer/CMakeLists.txt" title="examples/part-01-framebuffer/CMakeLists.txt"
```

When a new part adds an example, it adds a folder with a file like this one, and one `add_subdirectory` line to `examples/CMakeLists.txt`. The examples of the earlier parts stay in the repository, and they keep building while the library grows.

### Building

`CMakePresets.json` has a preset for each platform. On Windows, open a terminal in the repository's folder and run:

```powershell
cmake --preset windows
cmake --build --preset windows-release
```

The first command configures the project for Visual Studio 2026 (this is when SDL is downloaded). The second one builds it. You will find `part-01-framebuffer.exe` in `build\windows\bin\Release`. You can also open the folder in Visual Studio (**File** > **Open** > **Folder**), which reads the presets by itself. On Linux and macOS, use the `unix` preset (`cmake --preset unix` and `cmake --build --preset unix-release`), which needs [Ninja](https://ninja-build.org/).

## Colors

A screen is a grid of pixels, and every pixel has a color. The color is a mix of red, green and blue light, so we store three numbers per pixel. We use one byte (0 to 255) for each of them, and add a fourth byte for *alpha*, the opacity, which we need when we blend transparent shapes in a later part. Four bytes make 32 bits, which fits exactly in a `std::uint32_t`.

The order of the four bytes in that integer is called the *pixel format*. We use `ARGB8888`, which means alpha, red, green and blue, with 8 bits each, from the most significant bits to the least significant bits. Here are the bits of an opaque orange:

<svg class="bits-diagram" viewBox="0 0 700 112" role="img" aria-label="The 32 bits of orange, 0xFFFF8000: alpha 255, red 255, green 128 and blue 0, 8 bits each">
<text class="title" x="30" y="20">orange: 0xFFFF8000</text>
<text class="label alpha" x="110.0" y="41" text-anchor="middle">alpha: 255 (0xFF)</text>
<text class="count" x="110.0" y="91" text-anchor="middle">bits 31-24</text>
<rect class="field alpha" x="30.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit alpha" x="40.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="50.0" y1="50" x2="50.0" y2="74"/>
<text class="bit alpha" x="60.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="70.0" y1="50" x2="70.0" y2="74"/>
<text class="bit alpha" x="80.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="90.0" y1="50" x2="90.0" y2="74"/>
<text class="bit alpha" x="100.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="110.0" y1="50" x2="110.0" y2="74"/>
<text class="bit alpha" x="120.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="130.0" y1="50" x2="130.0" y2="74"/>
<text class="bit alpha" x="140.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="150.0" y1="50" x2="150.0" y2="74"/>
<text class="bit alpha" x="160.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="170.0" y1="50" x2="170.0" y2="74"/>
<text class="bit alpha" x="180.0" y="66.5" text-anchor="middle">1</text>
<text class="label red" x="270.0" y="41" text-anchor="middle">red: 255 (0xFF)</text>
<text class="count" x="270.0" y="91" text-anchor="middle">bits 23-16</text>
<rect class="field red" x="190.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit red" x="200.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="210.0" y1="50" x2="210.0" y2="74"/>
<text class="bit red" x="220.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="230.0" y1="50" x2="230.0" y2="74"/>
<text class="bit red" x="240.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="250.0" y1="50" x2="250.0" y2="74"/>
<text class="bit red" x="260.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="270.0" y1="50" x2="270.0" y2="74"/>
<text class="bit red" x="280.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="290.0" y1="50" x2="290.0" y2="74"/>
<text class="bit red" x="300.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="310.0" y1="50" x2="310.0" y2="74"/>
<text class="bit red" x="320.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="330.0" y1="50" x2="330.0" y2="74"/>
<text class="bit red" x="340.0" y="66.5" text-anchor="middle">1</text>
<text class="label green" x="430.0" y="41" text-anchor="middle">green: 128 (0x80)</text>
<text class="count" x="430.0" y="91" text-anchor="middle">bits 15-8</text>
<rect class="field green" x="350.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit green" x="360.0" y="66.5" text-anchor="middle">1</text>
<line class="cell green" x1="370.0" y1="50" x2="370.0" y2="74"/>
<text class="bit green" x="380.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="390.0" y1="50" x2="390.0" y2="74"/>
<text class="bit green" x="400.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="410.0" y1="50" x2="410.0" y2="74"/>
<text class="bit green" x="420.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="430.0" y1="50" x2="430.0" y2="74"/>
<text class="bit green" x="440.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="450.0" y1="50" x2="450.0" y2="74"/>
<text class="bit green" x="460.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="470.0" y1="50" x2="470.0" y2="74"/>
<text class="bit green" x="480.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="490.0" y1="50" x2="490.0" y2="74"/>
<text class="bit green" x="500.0" y="66.5" text-anchor="middle">0</text>
<text class="label blue" x="590.0" y="41" text-anchor="middle">blue: 0</text>
<text class="count" x="590.0" y="91" text-anchor="middle">bits 7-0</text>
<rect class="field blue" x="510.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit blue" x="520.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="530.0" y1="50" x2="530.0" y2="74"/>
<text class="bit blue" x="540.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="550.0" y1="50" x2="550.0" y2="74"/>
<text class="bit blue" x="560.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="570.0" y1="50" x2="570.0" y2="74"/>
<text class="bit blue" x="580.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="590.0" y1="50" x2="590.0" y2="74"/>
<text class="bit blue" x="600.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="610.0" y1="50" x2="610.0" y2="74"/>
<text class="bit blue" x="620.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="630.0" y1="50" x2="630.0" y2="74"/>
<text class="bit blue" x="640.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="650.0" y1="50" x2="650.0" y2="74"/>
<text class="bit blue" x="660.0" y="66.5" text-anchor="middle">0</text>
<image href="/images/claude-logo.svg" x="679" y="91" width="16.8" height="16.8" opacity="0.35"><title>Made with Claude</title></image>
</svg>

<div class="caption">

The 32 bits of a color in the `ARGB8888` format. Alpha is in the highest 8 bits, then red, green, and blue in the lowest 8 bits.

</div>

Written in hexadecimal, a color looks like `0xAARRGGBB`, with two hex digits per channel. Every hex digit is exactly 4 bits, so you can read the channels straight from the number. The orange above is `0xFFFF8000`, which is alpha `FF` (255), red `FF` (255), green `80` (128) and blue `00` (0). In the same way, `0xFFFF0000` is opaque red, `0xFF00FF00` is opaque green, and `0xFF000000` is opaque black.

`Color.hpp` defines the `Color` struct, which wraps one of those integers:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Color.hpp#L1-L22"
```

The comment above the struct repeats the bit layout, so you don't have to look it up when you read the code. The only member is `argb`, the packed integer, and a default `Color` is opaque black. The constructor that takes a `std::uint32_t` is `explicit`, so a plain number never turns into a color by accident. You have to write `Color{ 0xFFFF0000 }`.

Most of the time, you will make a color from its channels:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Color.hpp#L24-L29"
```

Each channel is converted to a 32-bit integer first, and then shifted to its place. Red is shifted 16 bits to the left, which moves it from bits 0–7 to bits 16–23. Because the four channels don't overlap, combining them with `|` (bitwise or) puts them all in one integer. The conversion before the shift matters. In an expression, a `std::uint8_t` is promoted to a signed `int`, so without it, `a << 24` would put the alpha bits in the sign bit of an `int`. Converting each channel to `std::uint32_t` first keeps all the arithmetic unsigned.

Reading a channel back does the opposite:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Color.hpp#L31-L36"
```

To read red, we shift it back down 16 bits, so that it's in the lowest byte. The `static_cast<std::uint8_t>` then keeps only that lowest byte, and drops alpha, which is still above it. That's why we don't need a mask like `& 0xFF` here. The defaulted `operator==` compares two colors by comparing their `argb` members.

Everything in `Color` is `constexpr`, so the compiler can check it while it compiles:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Color.hpp#L64-L69"
```

The first `static_assert` checks that a `Color` is exactly 4 bytes, with no padding. We rely on that later, when we copy the whole surface into a texture in one go. The other two are tests of the packing and unpacking. If you get a shift wrong, the code doesn't compile.

:::note[Bytes in memory]
`ARGB8888` describes the bits of a 32-bit integer, not the order of the bytes in memory. On a little-endian CPU (every x86 and almost every ARM CPU), the least significant byte comes first, so the bytes of a pixel are stored as blue, green, red, alpha. SDL calls this a *packed* pixel format. As long as we only read and write whole `std::uint32_t` values, the byte order doesn't matter, and that's what `Color` does.
:::

### Named Colors

Code like `Color{ 0x64, 0x95, 0xED }` is hard to read. You have to decode the numbers to know which color it is. So `Color` has named constants, like `Color::Red` and `Color::CornflowerBlue`. The names are the 148 color keywords of CSS, the language that styles web pages, so you can look up any of them in your browser's color picker or on a page like MDN's [list of named colors](https://developer.mozilla.org/en-US/docs/Web/CSS/named-color). There is one extra constant, `Color::Transparent`, with all four channels at 0.

The constants are declared inside the struct, below `operator==`. This is the start of the list (the full list is in `Color.hpp`):

```cpp title="Color.hpp"
    // Named colors: Transparent, and the 148 color keywords of CSS (like CornflowerBlue).
    // They are declared here and defined below the struct, because a struct
    // is incomplete until its closing brace, and a constexpr variable needs a complete type.
    static const Color Transparent;
    static const Color
        AliceBlue, AntiqueWhite, Aqua, Aquamarine, Azure, Beige, Bisque, Black, BlanchedAlmond,
        Blue, BlueViolet, Brown, BurlyWood, CadetBlue, Chartreuse, Chocolate, Coral, CornflowerBlue,
        // ...
};
```

A `static` data member belongs to the struct, not to each color, so there is only one `Color::Red` in the whole program. You would like to write `static constexpr Color Red{ 0xFF, 0x00, 0x00 };` here, but that doesn't compile. Inside its own definition, until the closing brace, `Color` is an *incomplete type*. The compiler doesn't know its size or its constructors yet, so it can't make a `constexpr` `Color` there. That's why the constants are only *declared* in the struct, with `static const`, and each declaration can name many constants at once.

Below the struct, where `Color` is complete, each constant is *defined*:

```cpp title="Color.hpp"
// The values of the named colors.
inline constexpr Color Color::Transparent         { 0x00000000 };
inline constexpr Color Color::AliceBlue           { 0xF0, 0xF8, 0xFF };
inline constexpr Color Color::AntiqueWhite        { 0xFA, 0xEB, 0xD7 };
inline constexpr Color Color::Aqua                { 0x00, 0xFF, 0xFF };
inline constexpr Color Color::Aquamarine          { 0x7F, 0xFF, 0xD4 };
// ...

static_assert(Color::CornflowerBlue == Color{ 0xFF6495ED });
static_assert(Color::Transparent.A() == 0);
```

The definition may add `constexpr`, so the constants can be used at compile time, like in the `static_assert`s at the end. `inline` allows the definitions to be in a header. Every `.cpp` file that includes `Color.hpp` sees the same definitions, and `inline` tells the linker that they are all the same variable, instead of reporting it as defined more than once. Transparent uses the constructor that takes the packed `0xAARRGGBB` value, because it's the only one with an alpha of 0. All the others are opaque.

Watch out for `Color::Green`. In CSS, `green` is a dark green (`0x008000`), and the pure green with the green channel at 255 is called `Lime`. In the same way, `Gray` (`0x808080`) is darker than `DarkGray` (`0xA9A9A9`). The names come from the colors of the X Window System of the 1980s, and they're not always logical, but they are what every web developer knows.

## The Surface

A *framebuffer* is a block of memory with one color for every pixel of the screen. Memory is one long row of bytes, so we have to decide how a 2D grid of pixels is stored in it. We store the pixels row by row. First the pixels of the top row, from left to right, then the second row, and so on. This is called *row-major* order. In an image that is $w$ pixels wide, the pixel at column $x$ and row $y$ is at index

$$
i = y \cdot w + x
$$

You could write `pixels[y * width + x]` everywhere, but it's easy to swap `x` and `y`, or to use the height instead of the width. C++23 has a better tool for this, `std::mdspan`. It's a *view* of memory that you already have (it doesn't own the memory, like `std::span`), with a number of dimensions and an *extent* (a size) for each one. It does the index calculation for you.

`Surface.hpp` starts by naming the views that we use:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Surface.hpp#L1-L15"
```

`std::dextents<int, 2>` means two dimensions, with sizes that are only known at run time (the `d` is for *dynamic*), indexed with an `int`. The first dimension is the column `x` and the second is the row `y`, so you index a `PixelView` as `pixels[x, y]`, the same order as in a texture lookup in a shader. The comma is C++23's *multidimensional subscript operator*. Before C++23, `operator[]` could only take one argument.

The third template argument, the *layout*, decides where `pixels[x, y]` is in memory. `std::mdspan`'s default layout is `std::layout_right`, where the right-most index changes fastest in memory. With that layout, you would have to write `pixels[y, x]` to get row-major memory. We use `std::layout_left` instead, where the left-most index changes fastest. Going from `pixels[x, y]` to `pixels[x + 1, y]` moves one pixel forward in memory, and going to `pixels[x, y + 1]` moves a whole row (`width` pixels) forward. That's exactly the formula above, so the memory is still stored row by row. Only the order of the indices is different. `ConstPixelView` is the read-only version, for code that only reads the pixels.

:::caution[x first, but rows first]
`pixels[x, y]` puts `x` first, but in memory the pixels are still stored one row after the other. So a loop over all pixels must still have `y` in the outer loop and `x` in the inner loop, to go through memory from front to back. (In matrix terms, `std::layout_left` is called *column-major*. For an image, the "columns" of that matrix are our rows, which is why the name sounds like the opposite of what we're doing.)
:::

We use `int` for the indices instead of `std::size_t`. Starting in the next part, we work with coordinates that can be negative or fall outside of the screen, and signed integers make that arithmetic much less surprising.

The `Surface` class owns the pixels and hands out views of them:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Surface.hpp#L17-L41"
```

The pixels are stored in a `std::vector<Color>`, which frees its memory by itself. `Pixels()` makes a new view every time it's called, from the vector's data and the surface's width and height (in that order, the same order as the indices). Making a view costs almost nothing, and it means that a view can never point to memory that has been moved. If a `Surface` is copied or moved, the next call to `Pixels()` points to the right place. The views are only valid as long as the surface exists, the same as a pointer.

`Data()` returns all the pixels as a `std::span`, one row after the other. That's useful when the position of a pixel doesn't matter, for example to fill the whole surface with one color.

The constructor is in `Surface.cpp`:

```cpp source="2d-software-rasterizer-part-01/rast/src/Surface.cpp"
```

It makes the vector big enough for `width * height` colors. Each of them is a default `Color`, so a new surface is opaque black. The multiplication is done with `std::size_t`, because the product of two `int`s could overflow for a very large surface. The `assert` catches a surface with a zero or negative size in a debug build.

## The Window

The framebuffer is in the CPU's memory, but the screen is driven by the GPU. So, once per frame, we have to copy the pixels to the GPU and ask it to show them. That's the job of the `Window` class.

We use SDL's 2D renderer for this, but only for one thing. We create one *texture* (an image in GPU memory) with the same size as our surface, copy the surface into it every frame, and draw it to fill the window. SDL doesn't draw anything else. Every pixel that you see is computed by our code.

### Window.hpp

`Window.hpp` doesn't include SDL. It only says that the SDL types exist:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Window.hpp#L1-L14"
```

These are *forward declarations*. The header only stores pointers to these types, and you can store a pointer to a type without knowing what's inside it. So a program that includes `Window.hpp` doesn't get SDL's headers, and it doesn't need SDL's include directory.

The public part of the class starts with the functions that every program needs:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Window.hpp#L19-L39"
```

Opening a window can fail, for example on a machine without a display. Instead of throwing an exception, `Create` returns a `std::expected<Window, std::string>`. It contains either a `Window`, or a `std::string` that says what went wrong, and the caller has to check which one it got. A constructor can't return an error, which is why `Create` is a static function and the constructor is private.

A `std::expected<Window, ...>` has to be able to move the `Window` into place, so `Window` has a move constructor. It can't be copied, because two windows would then own the same SDL objects. The move assignment operator is deleted, because there is only ever one window in our programs, and a defaulted move assignment would destroy the old window without shutting SDL down.

`PollEvents` handles the window's events once per frame, and `Present` shows a surface in the window. The comment above the class lists the keys that every window understands. We come back to them in [VSync and Fullscreen](#vsync-and-fullscreen).

The private part starts with the default constructor, a helper function for the title bar, and a deleter:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Window.hpp#L49-L60"
```

Each SDL object is owned by a `std::unique_ptr`, so it's destroyed automatically. But SDL objects aren't freed with `delete`. Each type has its own function, like `SDL_DestroyWindow`, so the `std::unique_ptr` gets a *deleter*. `SdlDeleter` has an `operator()` for each type, and the `std::unique_ptr` calls the one that matches its pointer. The deleter's functions are defined in `Window.cpp`, where SDL is included.

Then come the members:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Window.hpp#L62-L79"
```

The order of the members matters. C++ destroys members in the reverse order of their declaration, so the texture is destroyed first, then the renderer, and the window last. A texture belongs to a renderer, and a renderer belongs to a window, so this is the order in which SDL wants them destroyed. `m_TextureWidth` and `m_TextureHeight` remember the size of the texture, so that we can tell when the surface's size changes. `m_Title` is the title that the program asked for, and the other members are for VSync and the frame rate, which we look at later.

### Window.cpp

`Window.cpp` is the only file that includes SDL:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L1-L12"
```

`emscripten.h` is only included in the web build, where the compiler defines `__EMSCRIPTEN__`.

`Create` initializes SDL and creates the window and the renderer:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L21-L54"
```

In SDL3, most functions return `true` when they succeed, and functions that create something return `nullptr` when they fail. In both cases, `SDL_GetError` tells us why. Each failure returns a `std::unexpected` with a message, which turns into a `std::expected` that holds an error. When something fails after `SDL_Init`, we call `SDL_Quit` ourselves, because the `Window` that would normally do it is never returned.

The title is a `std::string_view`, which doesn't have to end with a zero character. So we store it in `m_Title`, a `std::string`, which gives us a C string for `SDL_CreateWindow` (and we need the title again later). The window is resizable. Passing `nullptr` as the name of the renderer lets SDL pick the best one for the platform (Direct3D on Windows, Metal on macOS, OpenGL or Vulkan on Linux, and WebGL in the browser).

Finally, `Create` turns VSync on, and starts the timer for the frame rate. Both are explained in [VSync and Fullscreen](#vsync-and-fullscreen).

The destructor shuts everything down again:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L56-L66"
```

A `Window` that has been moved from has a null `m_Window`, and it must not call `SDL_Quit`, because the `Window` that it was moved into still uses SDL. The SDL objects must be destroyed before `SDL_Quit`, so the destructor resets them first, in the same order as the members would be destroyed.

`PollEvents` processes all the events that are waiting:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L68-L103"
```

SDL puts every event (a key press, a mouse movement, a resized window) in a queue, and `SDL_PollEvent` takes them out one by one until the queue is empty. `SDL_EVENT_QUIT` means that the user closed the window, and then `PollEvents` returns `false`, which stops the program's main loop.

All the other events that we handle are key presses. When you hold a key down, the operating system repeats it after a moment, and SDL sets `event.key.repeat` for those repeats. We skip them, so that holding <kbd>V</kbd> doesn't switch VSync on and off many times per second. <kbd>F11</kbd> and <kbd>Alt</kbd>+<kbd>Enter</kbd> switch between a window and fullscreen, and <kbd>V</kbd> switches VSync on or off. <kbd>Esc</kbd> leaves fullscreen when the window is fullscreen, and closes it otherwise, so you can never get stuck in fullscreen. `event.key.key` is the *key code*, the key as it's labeled on the keyboard, so <kbd>V</kbd> is the key with a V on it, on any keyboard layout.

### Present

`Present` does the actual work, in three steps. The first part creates the texture:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L105-L126"
```

The texture is created on the first call, when `m_Texture` is still null, and again when the size of the surface changes. Its format is `SDL_PIXELFORMAT_ARGB8888`, the same as our `Color`, so SDL never has to convert our pixels. `SDL_TEXTUREACCESS_STREAMING` tells SDL that we will change the whole texture every frame, so SDL sets it up for frequent updates from the CPU.

The window is usually larger than the surface (the example opens a window that is twice as large). `SDL_SCALEMODE_NEAREST` scales the texture up without blurring it, so every pixel of the surface becomes a sharp square on the screen. That's what we want for a rasterizer, where you often need to see individual pixels. `SDL_SetRenderLogicalPresentation` makes the renderer think in pixels of the surface instead of pixels of the window. With `SDL_LOGICAL_PRESENTATION_LETTERBOX`, it scales the texture as large as it fits and adds black bars when the window has a different shape.

The second part copies the pixels:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L128-L134"
```

`SDL_UpdateTexture` copies pixels from CPU memory into the texture. The `nullptr` means that we update the whole texture, and `surface.Data().data()` points to the first pixel of the surface. The last argument is the *pitch* of our pixels, the number of bytes from the start of one row to the start of the next one. Our surface stores its rows right after each other, so the pitch is simply the width times 4 bytes. That's not true for every image. Many libraries and GPU drivers add unused bytes at the end of each row, so that each row starts at a multiple of 64 or 256 bytes, and then the pitch is larger than the width times the size of a pixel. The texture probably has a pitch of its own, but SDL takes care of that for us.

:::note[Why not lock the texture?]
SDL can also *lock* a streaming texture with `SDL_LockTexture`, which gives you a pointer and a pitch, so that you write the pixels yourself and `SDL_UnlockTexture` sends them to the GPU. SDL's documentation recommends that for textures that change often. But locking only saves work if you draw straight into the memory that it gives you, and we can't do that. That memory is write-only, and it doesn't keep the previous frame, while our rasterizer needs to read the pixels it already drew (to blend a transparent shape on top of them, for example). So we keep our own `Surface`, and copy it once per frame. For that copy, `SDL_UpdateTexture` does the same work as locking with the Direct3D renderer, and it saves a copy with the OpenGL renderers, which upload straight from our pixels.
:::

The last part shows the texture:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L136-L150"
```

`SDL_RenderClear` clears the window to black, which is the color of the bars around the texture. `SDL_RenderTexture` draws the whole texture, and the two `nullptr`s mean the whole texture and the whole (logical) window. `SDL_RenderPresent` shows the result.

In the browser, a program can't run a loop that never ends, because the browser only shows a new frame when the code returns to it. `emscripten_sleep(0)` returns to the browser for a moment and continues where it left off. That only works with Emscripten's *Asyncify* feature, which is why `rast/CMakeLists.txt` adds `-sASYNCIFY` to every program that links the library. This keeps the examples simple. They can use a normal `while` loop, on every platform. With VSync on, the browser build waits a little longer, with `WaitForAnimationFrame`, which we look at in the next section.

### VSync and Fullscreen

*VSync* (vertical synchronization) makes `SDL_RenderPresent` wait for the screen's next refresh, 60 times per second on most screens. Without it, the program draws as many frames as it can, and you may see *tearing*, where the top of the screen shows one frame and the bottom shows the next one. VSync also keeps the program from using a whole CPU core for frames that nobody sees. But sometimes you want it off. Later in the series, we measure how long a frame takes, and VSync would hide that, because every frame would take at least 1/60th of a second.

The window can also cover the whole screen, which is useful when you want to look at the pixels up close. The public part of `Window.hpp` has functions for both:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Window.hpp#L41-L47"
```

`SetVSync` and `SetFullscreen` switch these modes, and the other two functions tell you whether they're on. `PollEvents` calls them when you press <kbd>V</kbd>, <kbd>F11</kbd> or <kbd>Alt</kbd>+<kbd>Enter</kbd>, but a program can call them too.

`SetVSync` is in `Window.cpp`:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L165-L181"
```

On the desktop, `SDL_SetRenderVSync` tells the renderer to wait for every refresh (1), or not to wait at all (`SDL_RENDERER_VSYNC_DISABLED`). We only change `m_VSync` when that worked, so it always tells the truth. In the browser, SDL can't do this for us, because it only works for programs that let Emscripten run their main loop, and our examples run their own loop. So the browser build only remembers the setting, and `Present` uses it.

That's what the function at the top of `Window.cpp` is for:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L14-L19"
```

`EM_ASYNC_JS` defines a C++ function whose body is JavaScript. The browser calls the function that you give to `requestAnimationFrame` right before it draws its next frame, which happens at the refresh rate of the screen. So waiting for that is the browser's version of VSync. The `await` only works because of Asyncify, which pauses our C++ code until the promise resolves. With VSync off, `Present` calls `emscripten_sleep(0)` instead, which gives the browser just enough time to do its own work.

Fullscreen takes two small functions:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L183-L195"
```

`SDL_SetWindowFullscreen` switches the window between fullscreen and a normal window. By default, SDL uses *borderless fullscreen*. The window covers the screen at the desktop's resolution, and the monitor doesn't switch to another video mode, so it's quick and doesn't make the screen flicker. Our surface stays the same size, and the letterboxing in `Present` scales it up to fit. `IsFullscreen` checks the `SDL_WINDOW_FULLSCREEN` flag of the window. In the browser, fullscreen makes the canvas cover the whole screen.

Without text on the screen (that comes in part 10), it's hard to tell whether VSync is on, so the window shows it in its title bar, together with the frame rate. `Present` counts the frames at its end:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L152-L163"
```

`SDL_GetTicksNS` returns the time since SDL started, in nanoseconds. Every half second, the number of frames is divided by the time that passed, which gives the number of frames per second. Updating the title every frame would make it flicker and would be hard to read, so we only do it twice per second.

The title is made by `UpdateTitle`:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L197-L202"
```

`std::format` puts the program's title, the frame rate (`{:.0f}` rounds it to a whole number) and the VSync state together, like `Part 1: The Framebuffer | 60 fps | VSync on`. `SetVSync` calls `UpdateTitle` too, so the title changes as soon as you press <kbd>V</kbd>. With VSync on, you should see the refresh rate of your screen. With VSync off, the frame rate is much higher, and how much higher depends on your CPU.

At the bottom of `Window.cpp` are the deleter's three functions, which call SDL's destroy functions.

## The Gradient

Now we can draw. The example in `examples/part-01-framebuffer/main.cpp` fills the surface with a gradient, and animates it. It starts with the includes and a small helper function:

```cpp source="2d-software-rasterizer-part-01/examples/part-01-framebuffer/main.cpp#L1-L19"
```

The example only includes the library's headers and the standard library, never SDL. We compute the colors with floats between 0 and 1, which is easier than working with bytes, and `ToByte` converts such a value to a channel between 0 and 255:

$$
c = \lfloor 255 \cdot \operatorname{clamp}(v, 0, 1) + 0.5 \rfloor
$$

`std::clamp` keeps the value between 0 and 1, so a value that is slightly outside of that range doesn't wrap around to a completely different color. Adding 0.5 before the conversion rounds to the nearest integer, because a `static_cast` to an integer type always rounds down (for positive numbers). Without it, only a value of exactly 1.0 would give you 255.

`main` opens the window first:

```cpp source="2d-software-rasterizer-part-01/examples/part-01-framebuffer/main.cpp#L21-L35"
```

The surface is 640 by 360 pixels, and the window is twice as large, so every pixel of the surface is shown as 2 by 2 pixels on the screen. If `Create` fails, `window` holds an error instead of a window. `std::println` prints the error message to `stderr`, and the program ends. Otherwise, `window` behaves like a pointer to the `Window`, so we use `->` to call its functions. `start` remembers when the program started, so that we can animate the gradient over time.

The main loop runs until the window is closed:

```cpp source="2d-software-rasterizer-part-01/examples/part-01-framebuffer/main.cpp#L37-L62"
```

Every frame, `time` is the number of seconds since the start, as a `float`. Then we visit every pixel of the surface, row by row, with `y` in the outer loop. That's the same order as the pixels are stored in memory, so the CPU reads and writes memory from front to back, which is the fastest way to go through it.

For every pixel, `u` and `v` are its position as a fraction of the width and the height. They are 0 at the top-left pixel and 1 at the bottom-right pixel. Red follows `u`, so the surface gets redder to the right. Green follows `v`, so it gets greener towards the bottom. The bottom-right corner, which has both, is yellow. Blue is a wave:

$$
b = \frac{1}{2} + \frac{1}{2} \sin\big(2t - \pi (u + v)\big)
$$

The sine goes from -1 to 1, so $b$ goes from 0 to 1. Along a diagonal line, where $u + v$ is the same, all pixels have the same amount of blue. As time goes on, the stripes of blue move towards the bottom-right corner, and a new stripe starts every $\pi$ seconds.

Finally, `pixels[x, y]` stores the color in the surface, and `Present` shows it in the window.

The result looks like this. (This is the same program, built for the web with the `web` preset.)

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-1-framebuffer/index.html" title="An animated gradient: red increases to the right, green increases downwards, and diagonal stripes of blue move towards the bottom-right corner" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The animated gradient of part 1. Click the demo first, so that it gets the keys. <kbd>Alt</kbd>+<kbd>Enter</kbd> or the button in the top-right corner switches to fullscreen, and <kbd>Esc</kbd> stops the demo, like in the desktop version (reload the page to start it again).

</div>

The surface has 640 × 360 = 230,400 pixels, so the loop computes a sine more than 230,000 times per frame, plus a few conversions per pixel. That's no problem for a modern CPU at 60 frames per second, even in a debug build. But it gives you an idea of the amount of work, because everything that we draw in this series is done for every pixel, every frame. In the second half of the series, we measure how long a frame takes, and we make it faster.

## What's Next

We now have a surface that we can write pixels to, and a window that shows it. In [part 2](/2d-software-rasterizer-2-clipping/), we look at what happens when we draw outside of the surface. We add a fast `Clear`, keep every drawing operation inside the surface, and add *scissor rectangles*, which limit drawing to a part of the screen.
