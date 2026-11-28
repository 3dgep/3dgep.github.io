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

The headers are in `include/rast`, so a program includes them as `<rast/Color.hpp>`, and it's always clear which library a header comes from. The `cmake` folder is only used for the web build [\[1\]](#cite-1), which runs the examples in your browser (like the one at the end of this article).

### The Top-Level CMakeLists.txt

The `CMakeLists.txt` in the root of the repository starts by asking for C++23 [\[2\]](#cite-2):

```cmake source="2d-software-rasterizer-part-01/CMakeLists.txt#L1-L10" title="CMakeLists.txt"
```

`CMAKE_CXX_EXTENSIONS OFF` asks for standard C++ (`-std=c++23`) instead of the GNU dialect (`-std=gnu++23`) [\[3\]](#cite-3), so the code compiles the same way with every compiler. Every executable goes into the `bin` folder of the build [\[4\]](#cite-4), which makes them easy to find.

Next, CMake downloads [SDL3](https://www.libsdl.org/) [\[5\]](#cite-5):

```cmake source="2d-software-rasterizer-part-01/CMakeLists.txt#L12-L27" title="CMakeLists.txt"
```

SDL opens the window, handles the keyboard and the mouse, and puts our pixels on the screen. That's all we use it for. It never draws a line or a triangle for us. `FetchContent` clones SDL at a fixed release (`GIT_TAG`) [\[6\]](#cite-6), so everyone who builds the project gets the same version. We build SDL as a static library, so its code becomes part of each executable and there's no DLL to copy next to it. `SYSTEM` treats SDL's headers as system headers, which means that their warnings don't show up in our build.

Our own code gets all the warnings that the compiler has to offer:

```cmake source="2d-software-rasterizer-part-01/CMakeLists.txt#L29-L39" title="CMakeLists.txt"
```

`rast_set_warnings` turns on the high warning levels for one target [\[7\]](#cite-7) [\[8\]](#cite-8) [\[9\]](#cite-9). With MSVC, `/permissive-` also makes the compiler follow the standard more strictly [\[10\]](#cite-10), and `/utf-8` reads the source files as UTF-8 [\[11\]](#cite-11). With GCC and Clang, `-Wconversion` warns about every conversion that can lose information [\[12\]](#cite-12), like an `int` that is stored in a `std::uint8_t`. A rasterizer converts between integers and floats all the time, and we want each of those conversions to be visible in the code. Finally, CMake goes into the `rast` and `examples` folders [\[13\]](#cite-13).

### The rast Library

The library's `CMakeLists.txt` lists its files [\[14\]](#cite-14):

```cmake source="2d-software-rasterizer-part-01/rast/CMakeLists.txt" title="rast/CMakeLists.txt"
```

The include directory is `PUBLIC` [\[15\]](#cite-15), so every program that links `rast` can include its headers. SDL is linked `PRIVATE` [\[16\]](#cite-16), because only `Window.cpp` uses it. A program that uses the library never has to include an SDL header, and that's what keeps the rasterizer independent of SDL. The `EMSCRIPTEN` part is for the web build, and we come back to it when we look at `Window::Present`.

### The Examples

Every example is built with the same function, in `examples/CMakeLists.txt`:

```cmake source="2d-software-rasterizer-part-01/examples/CMakeLists.txt" title="examples/CMakeLists.txt"
```

`rast_add_example` makes an executable from the source files that you give it, links it to `rast` and turns on the warnings. For the web, it makes a web page instead, `index.html`, in a folder of its own. Then every part's folder only needs a single line:

```cmake source="2d-software-rasterizer-part-01/examples/part-01-framebuffer/CMakeLists.txt" title="examples/part-01-framebuffer/CMakeLists.txt"
```

When a new part adds an example, it adds a folder with a file like this one, and one `add_subdirectory` line to `examples/CMakeLists.txt`. The examples of the earlier parts stay in the repository, and they keep building while the library grows.

### Building

`CMakePresets.json` has a preset for each platform [\[17\]](#cite-17). On Windows, open a terminal in the repository's folder and run:

```powershell
cmake --preset windows
cmake --build --preset windows-release
```

The first command configures the project for Visual Studio 2026 (this is when SDL is downloaded). The second one builds it. You will find `part-01-framebuffer.exe` in `build\windows\bin\Release`. You can also open the folder in Visual Studio (**File** > **Open** > **Folder**), which reads the presets by itself [\[18\]](#cite-18). On Linux and macOS, use the `unix` preset (`cmake --preset unix` and `cmake --build --preset unix-release`), which needs [Ninja](https://ninja-build.org/) [\[19\]](#cite-19).

## Colors

A screen is a grid of pixels, and every pixel has a color. The color is a mix of red, green and blue light, so we store three numbers per pixel. We use one byte (0 to 255) for each of them, and add a fourth byte for *alpha*, the opacity, which we need when we blend transparent shapes in a later part. Four bytes make 32 bits, which fits exactly in a `std::uint32_t` [\[20\]](#cite-20).

The order of the four bytes in that integer is called the *pixel format*. We use `ARGB8888` [\[21\]](#cite-21), which means alpha, red, green and blue, with 8 bits each, from the most significant bits to the least significant bits. Here are the bits of an opaque orange:

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

The comment above the struct repeats the bit layout, so you don't have to look it up when you read the code. The only member is `argb`, the packed integer, and a default `Color` is opaque black. The constructor that takes a `std::uint32_t` is `explicit` [\[22\]](#cite-22), so a plain number never turns into a color by accident. You have to write `Color{ 0xFFFF0000 }`.

Most of the time, you will make a color from its channels:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Color.hpp#L24-L29"
```

Each channel is converted to a 32-bit integer first, and then shifted to its place. Red is shifted 16 bits to the left, which moves it from bits 0–7 to bits 16–23. Because the four channels don't overlap, combining them with `|` (bitwise or) puts them all in one integer. The conversion before the shift matters. In an expression, a `std::uint8_t` is promoted to a signed `int` [\[23\]](#cite-23), so without it, `a << 24` would put the alpha bits in the sign bit of an `int`. Converting each channel to `std::uint32_t` first keeps all the arithmetic unsigned.

Reading a channel back does the opposite:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Color.hpp#L31-L36"
```

To read red, we shift it back down 16 bits, so that it's in the lowest byte. The `static_cast<std::uint8_t>` then keeps only that lowest byte, and drops alpha, which is still above it. That's why we don't need a mask like `& 0xFF` here. The defaulted `operator==` compares two colors by comparing their `argb` members [\[24\]](#cite-24).

Everything in `Color` is `constexpr`, so the compiler can check it while it compiles [\[25\]](#cite-25) [\[26\]](#cite-26):

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Color.hpp#L64-L69"
```

The first `static_assert` checks that a `Color` is exactly 4 bytes, with no padding. We rely on that later, when we copy the whole surface into a texture in one go. The other two are tests of the packing and unpacking. If you get a shift wrong, the code doesn't compile.

:::note[Bytes in memory]
`ARGB8888` describes the bits of a 32-bit integer, not the order of the bytes in memory. On a little-endian CPU (every x86 and almost every ARM CPU) [\[27\]](#cite-27), the least significant byte comes first, so the bytes of a pixel are stored as blue, green, red, alpha. SDL calls this a *packed* pixel format. As long as we only read and write whole `std::uint32_t` values, the byte order doesn't matter, and that's what `Color` does.
:::

### Named Colors

Code like `Color{ 0x64, 0x95, 0xED }` is hard to read. You have to decode the numbers to know which color it is. So `Color` has named constants, like `Color::Red` and `Color::CornflowerBlue`. The names are the 148 color keywords of CSS [\[28\]](#cite-28), the language that styles web pages, so you can look up any of them in your browser's color picker or on a page like MDN's [list of named colors](https://developer.mozilla.org/en-US/docs/Web/CSS/named-color) [\[29\]](#cite-29). There is one extra constant, `Color::Transparent`, with all four channels at 0.

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

A `static` data member belongs to the struct, not to each color [\[30\]](#cite-30), so there is only one `Color::Red` in the whole program. You would like to write `static constexpr Color Red{ 0xFF, 0x00, 0x00 };` here, but that doesn't compile. Inside its own definition, until the closing brace, `Color` is an *incomplete type* [\[31\]](#cite-31). The compiler doesn't know its size or its constructors yet, so it can't make a `constexpr` `Color` there. That's why the constants are only *declared* in the struct, with `static const`, and each declaration can name many constants at once.

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

The definition may add `constexpr`, so the constants can be used at compile time, like in the `static_assert`s at the end. `inline` allows the definitions to be in a header [\[32\]](#cite-32). Every `.cpp` file that includes `Color.hpp` sees the same definitions, and `inline` tells the linker that they are all the same variable, instead of reporting it as defined more than once. Transparent uses the constructor that takes the packed `0xAARRGGBB` value, because it's the only one with an alpha of 0. All the others are opaque.

Watch out for `Color::Green`. In CSS, `green` is a dark green (`0x008000`), and the pure green with the green channel at 255 is called `Lime`. In the same way, `Gray` (`0x808080`) is darker than `DarkGray` (`0xA9A9A9`). The names come from the colors of the X Window System of the 1980s, and they're not always logical, but they are what every web developer knows.

## The Surface

A *framebuffer* is a block of memory with one color for every pixel of the screen. Memory is one long row of bytes, so we have to decide how a 2D grid of pixels is stored in it. We store the pixels row by row. First the pixels of the top row, from left to right, then the second row, and so on. This is called *row-major* order [\[33\]](#cite-33). In an image that is $w$ pixels wide, the pixel at column $x$ and row $y$ is at index

$$
i = y \cdot w + x
$$

You could write `pixels[y * width + x]` everywhere, but it's easy to swap `x` and `y`, or to use the height instead of the width. C++23 has a better tool for this, `std::mdspan` [\[34\]](#cite-34) [\[35\]](#cite-35). It's a *view* of memory that you already have (it doesn't own the memory, like `std::span`) [\[36\]](#cite-36), with a number of dimensions and an *extent* (a size) for each one. It does the index calculation for you.

`Surface.hpp` starts by naming the views that we use:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Surface.hpp#L1-L15"
```

`std::dextents<int, 2>` means two dimensions [\[37\]](#cite-37), with sizes that are only known at run time (the `d` is for *dynamic*), indexed with an `int`. The first dimension is the column `x` and the second is the row `y`, so you index a `PixelView` as `pixels[x, y]`, the same order as in a texture lookup in a shader. The comma is C++23's *multidimensional subscript operator* [\[38\]](#cite-38) [\[39\]](#cite-39). Before C++23, `operator[]` could only take one argument.

The third template argument, the *layout*, decides where `pixels[x, y]` is in memory. `std::mdspan`'s default layout is `std::layout_right` [\[40\]](#cite-40), where the right-most index changes fastest in memory. With that layout, you would have to write `pixels[y, x]` to get row-major memory. We use `std::layout_left` instead [\[41\]](#cite-41), where the left-most index changes fastest. Going from `pixels[x, y]` to `pixels[x + 1, y]` moves one pixel forward in memory, and going to `pixels[x, y + 1]` moves a whole row (`width` pixels) forward. That's exactly the formula above, so the memory is still stored row by row. Only the order of the indices is different. `ConstPixelView` is the read-only version, for code that only reads the pixels.

:::caution[x first, but rows first]
`pixels[x, y]` puts `x` first, but in memory the pixels are still stored one row after the other. So a loop over all pixels must still have `y` in the outer loop and `x` in the inner loop, to go through memory from front to back. (In matrix terms, `std::layout_left` is called *column-major*. For an image, the "columns" of that matrix are our rows, which is why the name sounds like the opposite of what we're doing.)
:::

We use `int` for the indices instead of `std::size_t`. Starting in the next part, we work with coordinates that can be negative or fall outside of the screen, and signed integers make that arithmetic much less surprising.

The `Surface` class owns the pixels and hands out views of them:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Surface.hpp#L17-L41"
```

The pixels are stored in a `std::vector<Color>` [\[42\]](#cite-42), which frees its memory by itself. `Pixels()` makes a new view every time it's called, from the vector's data and the surface's width and height (in that order, the same order as the indices). Making a view costs almost nothing, and it means that a view can never point to memory that has been moved. If a `Surface` is copied or moved, the next call to `Pixels()` points to the right place. The views are only valid as long as the surface exists, the same as a pointer.

`Data()` returns all the pixels as a `std::span`, one row after the other. That's useful when the position of a pixel doesn't matter, for example to fill the whole surface with one color.

The constructor is in `Surface.cpp`:

```cpp source="2d-software-rasterizer-part-01/rast/src/Surface.cpp"
```

It makes the vector big enough for `width * height` colors. Each of them is a default `Color`, so a new surface is opaque black. The multiplication is done with `std::size_t` [\[43\]](#cite-43), because the product of two `int`s could overflow for a very large surface. The `assert` catches a surface with a zero or negative size in a debug build [\[44\]](#cite-44).

## The Window

The framebuffer is in the CPU's memory, but the screen is driven by the GPU. So, once per frame, we have to copy the pixels to the GPU and ask it to show them. That's the job of the `Window` class.

We use SDL's 2D renderer for this, but only for one thing. We create one *texture* (an image in GPU memory) with the same size as our surface, copy the surface into it every frame, and draw it to fill the window. SDL doesn't draw anything else. Every pixel that you see is computed by our code.

### Window.hpp

`Window.hpp` doesn't include SDL. It only says that the SDL types exist:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Window.hpp#L1-L14"
```

These are *forward declarations* [\[45\]](#cite-45). The header only stores pointers to these types, and you can store a pointer to a type without knowing what's inside it. So a program that includes `Window.hpp` doesn't get SDL's headers, and it doesn't need SDL's include directory.

The public part of the class starts with the functions that every program needs:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Window.hpp#L19-L39"
```

Opening a window can fail, for example on a machine without a display. Instead of throwing an exception, `Create` returns a `std::expected<Window, std::string>` [\[46\]](#cite-46) [\[47\]](#cite-47). It contains either a `Window`, or a `std::string` that says what went wrong, and the caller has to check which one it got. A constructor can't return an error, which is why `Create` is a static function and the constructor is private.

A `std::expected<Window, ...>` has to be able to move the `Window` into place, so `Window` has a move constructor [\[48\]](#cite-48). It can't be copied, because two windows would then own the same SDL objects. The move assignment operator is deleted, because there is only ever one window in our programs, and a defaulted move assignment would destroy the old window without shutting SDL down.

`PollEvents` handles the window's events once per frame, and `Present` shows a surface in the window. The comment above the class lists the keys that every window understands. We come back to them in [VSync and Fullscreen](#vsync-and-fullscreen).

The private part starts with the default constructor, a helper function for the title bar, and a deleter:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Window.hpp#L49-L60"
```

Each SDL object is owned by a `std::unique_ptr` [\[49\]](#cite-49), so it's destroyed automatically. But SDL objects aren't freed with `delete`. Each type has its own function, like `SDL_DestroyWindow`, so the `std::unique_ptr` gets a *deleter*. `SdlDeleter` has an `operator()` for each type, and the `std::unique_ptr` calls the one that matches its pointer. The deleter's functions are defined in `Window.cpp`, where SDL is included.

Then come the members:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Window.hpp#L62-L79"
```

The order of the members matters. C++ destroys members in the reverse order of their declaration [\[50\]](#cite-50), so the texture is destroyed first, then the renderer, and the window last. A texture belongs to a renderer, and a renderer belongs to a window, so this is the order in which SDL wants them destroyed. `m_TextureWidth` and `m_TextureHeight` remember the size of the texture, so that we can tell when the surface's size changes. `m_Title` is the title that the program asked for, and the other members are for VSync and the frame rate, which we look at later.

### Window.cpp

`Window.cpp` is the only file that includes SDL:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L1-L12"
```

`emscripten.h` is only included in the web build, where the compiler defines `__EMSCRIPTEN__`.

`Create` initializes SDL and creates the window and the renderer:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L21-L54"
```

In SDL3, most functions return `true` when they succeed [\[51\]](#cite-51), and functions that create something return `nullptr` when they fail. In both cases, `SDL_GetError` tells us why [\[52\]](#cite-52). Each failure returns a `std::unexpected` with a message [\[53\]](#cite-53), which turns into a `std::expected` that holds an error. When something fails after `SDL_Init`, we call `SDL_Quit` ourselves [\[54\]](#cite-54), because the `Window` that would normally do it is never returned.

The title is a `std::string_view` [\[55\]](#cite-55), which doesn't have to end with a zero character. So we store it in `m_Title`, a `std::string`, which gives us a C string for `SDL_CreateWindow` (and we need the title again later). The window is resizable [\[56\]](#cite-56). Passing `nullptr` as the name of the renderer lets SDL pick the best one for the platform [\[57\]](#cite-57) (Direct3D on Windows, Metal on macOS, OpenGL or Vulkan on Linux, and WebGL in the browser).

Finally, `Create` turns VSync on, and starts the timer for the frame rate. Both are explained in [VSync and Fullscreen](#vsync-and-fullscreen).

The destructor shuts everything down again:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L56-L66"
```

A `Window` that has been moved from has a null `m_Window`, and it must not call `SDL_Quit`, because the `Window` that it was moved into still uses SDL. The SDL objects must be destroyed before `SDL_Quit`, so the destructor resets them first, in the same order as the members would be destroyed.

`PollEvents` processes all the events that are waiting:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L68-L103"
```

SDL puts every event (a key press, a mouse movement, a resized window) in a queue, and `SDL_PollEvent` takes them out one by one until the queue is empty [\[58\]](#cite-58). `SDL_EVENT_QUIT` means that the user closed the window [\[59\]](#cite-59), and then `PollEvents` returns `false`, which stops the program's main loop.

All the other events that we handle are key presses. When you hold a key down, the operating system repeats it after a moment, and SDL sets `event.key.repeat` for those repeats [\[60\]](#cite-60). We skip them, so that holding <kbd>V</kbd> doesn't switch VSync on and off many times per second. <kbd>F11</kbd> and <kbd>Alt</kbd>+<kbd>Enter</kbd> switch between a window and fullscreen, and <kbd>V</kbd> switches VSync on or off. <kbd>Esc</kbd> leaves fullscreen when the window is fullscreen, and closes it otherwise, so you can never get stuck in fullscreen. `event.key.key` is the *key code* [\[61\]](#cite-61), the key as it's labeled on the keyboard, so <kbd>V</kbd> is the key with a V on it, on any keyboard layout.

### Present

`Present` does the actual work, in three steps. The first part creates the texture:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L105-L126"
```

The texture is created on the first call, when `m_Texture` is still null [\[62\]](#cite-62), and again when the size of the surface changes. Its format is `SDL_PIXELFORMAT_ARGB8888`, the same as our `Color`, so SDL never has to convert our pixels. `SDL_TEXTUREACCESS_STREAMING` tells SDL that we will change the whole texture every frame [\[63\]](#cite-63), so SDL sets it up for frequent updates from the CPU.

The window is usually larger than the surface (the example opens a window that is twice as large). `SDL_SCALEMODE_NEAREST` scales the texture up without blurring it [\[64\]](#cite-64), so every pixel of the surface becomes a sharp square on the screen. That's what we want for a rasterizer, where you often need to see individual pixels. `SDL_SetRenderLogicalPresentation` makes the renderer think in pixels of the surface [\[65\]](#cite-65) instead of pixels of the window. With `SDL_LOGICAL_PRESENTATION_LETTERBOX`, it scales the texture as large as it fits and adds black bars when the window has a different shape.

The second part copies the pixels:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L128-L134"
```

`SDL_UpdateTexture` copies pixels from CPU memory into the texture [\[66\]](#cite-66). The `nullptr` means that we update the whole texture, and `surface.Data().data()` points to the first pixel of the surface. The last argument is the *pitch* of our pixels, the number of bytes from the start of one row to the start of the next one. Our surface stores its rows right after each other, so the pitch is simply the width times 4 bytes. That's not true for every image. Many libraries and GPU drivers add unused bytes at the end of each row, so that each row starts at a multiple of 64 or 256 bytes, and then the pitch is larger than the width times the size of a pixel. The texture probably has a pitch of its own, but SDL takes care of that for us.

:::note[Why not lock the texture?]
SDL can also *lock* a streaming texture with `SDL_LockTexture` [\[67\]](#cite-67), which gives you a pointer and a pitch, so that you write the pixels yourself and `SDL_UnlockTexture` sends them to the GPU. SDL's documentation recommends that for textures that change often. But locking only saves work if you draw straight into the memory that it gives you, and we can't do that. That memory is write-only, and it doesn't keep the previous frame, while our rasterizer needs to read the pixels it already drew (to blend a transparent shape on top of them, for example). So we keep our own `Surface`, and copy it once per frame. For that copy, `SDL_UpdateTexture` does the same work as locking with the Direct3D renderer, and it saves a copy with the OpenGL renderers, which upload straight from our pixels.
:::

The last part shows the texture:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L136-L150"
```

`SDL_RenderClear` clears the window to black [\[68\]](#cite-68), which is the color of the bars around the texture. `SDL_RenderTexture` draws the whole texture [\[69\]](#cite-69), and the two `nullptr`s mean the whole texture and the whole (logical) window. `SDL_RenderPresent` shows the result [\[70\]](#cite-70).

In the browser, a program can't run a loop that never ends, because the browser only shows a new frame when the code returns to it. `emscripten_sleep(0)` returns to the browser for a moment [\[71\]](#cite-71) and continues where it left off. That only works with Emscripten's *Asyncify* feature [\[72\]](#cite-72), which is why `rast/CMakeLists.txt` adds `-sASYNCIFY` to every program that links the library. This keeps the examples simple. They can use a normal `while` loop, on every platform. With VSync on, the browser build waits a little longer, with `WaitForAnimationFrame`, which we look at in the next section.

### VSync and Fullscreen

*VSync* (vertical synchronization) makes `SDL_RenderPresent` wait for the screen's next refresh, 60 times per second on most screens. Without it, the program draws as many frames as it can, and you may see *tearing* [\[73\]](#cite-73), where the top of the screen shows one frame and the bottom shows the next one. VSync also keeps the program from using a whole CPU core for frames that nobody sees. But sometimes you want it off. Later in the series, we measure how long a frame takes, and VSync would hide that, because every frame would take at least 1/60th of a second.

The window can also cover the whole screen, which is useful when you want to look at the pixels up close. The public part of `Window.hpp` has functions for both:

```cpp source="2d-software-rasterizer-part-01/rast/include/rast/Window.hpp#L41-L47"
```

`SetVSync` and `SetFullscreen` switch these modes, and the other two functions tell you whether they're on. `PollEvents` calls them when you press <kbd>V</kbd>, <kbd>F11</kbd> or <kbd>Alt</kbd>+<kbd>Enter</kbd>, but a program can call them too.

`SetVSync` is in `Window.cpp`:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L165-L181"
```

On the desktop, `SDL_SetRenderVSync` tells the renderer to wait for every refresh [\[74\]](#cite-74) (1), or not to wait at all (`SDL_RENDERER_VSYNC_DISABLED`). We only change `m_VSync` when that worked, so it always tells the truth. In the browser, SDL can't do this for us, because it only works for programs that let Emscripten run their main loop, and our examples run their own loop. So the browser build only remembers the setting, and `Present` uses it.

That's what the function at the top of `Window.cpp` is for:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L14-L19"
```

`EM_ASYNC_JS` defines a C++ function whose body is JavaScript [\[72\]](#cite-72). The browser calls the function that you give to `requestAnimationFrame` right before it draws its next frame [\[75\]](#cite-75), which happens at the refresh rate of the screen. So waiting for that is the browser's version of VSync. The `await` only works because of Asyncify, which pauses our C++ code until the promise resolves. With VSync off, `Present` calls `emscripten_sleep(0)` instead, which gives the browser just enough time to do its own work.

Fullscreen takes two small functions:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L183-L195"
```

`SDL_SetWindowFullscreen` switches the window between fullscreen and a normal window [\[76\]](#cite-76). By default, SDL uses *borderless fullscreen*. The window covers the screen at the desktop's resolution, and the monitor doesn't switch to another video mode, so it's quick and doesn't make the screen flicker. Our surface stays the same size, and the letterboxing in `Present` scales it up to fit. `IsFullscreen` checks the `SDL_WINDOW_FULLSCREEN` flag of the window [\[77\]](#cite-77). In the browser, fullscreen makes the canvas cover the whole screen.

Without text on the screen (that comes in part 11), it's hard to tell whether VSync is on, so the window shows it in its title bar, together with the frame rate. `Present` counts the frames at its end:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L152-L163"
```

`SDL_GetTicksNS` returns the time since SDL started, in nanoseconds [\[78\]](#cite-78). Every half second, the number of frames is divided by the time that passed, which gives the number of frames per second. Updating the title every frame would make it flicker and would be hard to read, so we only do it twice per second.

The title is made by `UpdateTitle`:

```cpp source="2d-software-rasterizer-part-01/rast/src/Window.cpp#L197-L202"
```

`std::format` puts the program's title [\[79\]](#cite-79) [\[80\]](#cite-80), the frame rate (`{:.0f}` rounds it to a whole number) and the VSync state together, like `Part 1: The Framebuffer | 60 fps | VSync on`. `SetVSync` calls `UpdateTitle` too, so the title changes as soon as you press <kbd>V</kbd>. With VSync on, you should see the refresh rate of your screen. With VSync off, the frame rate is much higher, and how much higher depends on your CPU.

At the bottom of `Window.cpp` are the deleter's three functions, which call SDL's destroy functions.

## The Gradient

Now we can draw. The example in `examples/part-01-framebuffer/main.cpp` fills the surface with a gradient, and animates it. It starts with the includes and a small helper function:

```cpp source="2d-software-rasterizer-part-01/examples/part-01-framebuffer/main.cpp#L1-L19"
```

The example only includes the library's headers and the standard library, never SDL. We compute the colors with floats between 0 and 1, which is easier than working with bytes, and `ToByte` converts such a value to a channel between 0 and 255:

$$
c = \lfloor 255 \cdot \operatorname{clamp}(v, 0, 1) + 0.5 \rfloor
$$

`std::clamp` keeps the value between 0 and 1 [\[81\]](#cite-81), so a value that is slightly outside of that range doesn't wrap around to a completely different color. Adding 0.5 before the conversion rounds to the nearest integer, because a `static_cast` to an integer type always rounds down (for positive numbers). Without it, only a value of exactly 1.0 would give you 255.

`main` opens the window first:

```cpp source="2d-software-rasterizer-part-01/examples/part-01-framebuffer/main.cpp#L21-L35"
```

The surface is 640 by 360 pixels, and the window is twice as large, so every pixel of the surface is shown as 2 by 2 pixels on the screen. If `Create` fails, `window` holds an error instead of a window. `std::println` prints the error message to `stderr` [\[82\]](#cite-82), and the program ends. Otherwise, `window` behaves like a pointer to the `Window`, so we use `->` to call its functions. `start` remembers when the program started [\[83\]](#cite-83), so that we can animate the gradient over time.

The main loop runs until the window is closed:

```cpp source="2d-software-rasterizer-part-01/examples/part-01-framebuffer/main.cpp#L37-L62"
```

Every frame, `time` is the number of seconds since the start, as a `float` [\[84\]](#cite-84). Then we visit every pixel of the surface, row by row, with `y` in the outer loop. That's the same order as the pixels are stored in memory, so the CPU reads and writes memory from front to back, which is the fastest way to go through it.

For every pixel, `u` and `v` are its position as a fraction of the width and the height. They are 0 at the top-left pixel and 1 at the bottom-right pixel. Red follows `u`, so the surface gets redder to the right. Green follows `v`, so it gets greener towards the bottom. The bottom-right corner, which has both, is yellow. Blue is a wave:

$$
b = \frac{1}{2} + \frac{1}{2} \sin\big(2t - \pi (u + v)\big)
$$

The sine goes from -1 to 1 [\[85\]](#cite-85) [\[86\]](#cite-86), so $b$ goes from 0 to 1. Along a diagonal line, where $u + v$ is the same, all pixels have the same amount of blue. As time goes on, the stripes of blue move towards the bottom-right corner, and a new stripe starts every $\pi$ seconds.

Finally, `pixels[x, y]` stores the color in the surface, and `Present` shows it in the window.

The result looks like this. (This is the same program, built for the web with the `web` preset.)

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-1-framebuffer/index.html" title="An animated gradient: red increases to the right, green increases downwards, and diagonal stripes of blue move towards the bottom-right corner" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The animated gradient of part 1. Click the demo first, so that it gets the keys. <kbd>Alt</kbd>+<kbd>Enter</kbd> or the button in the top-right corner switches to fullscreen, and <kbd>Esc</kbd> stops the demo, like in the desktop version (reload the page to start it again).

</div>

The surface has 640 × 360 = 230,400 pixels, so the loop computes a sine more than 230,000 times per frame, plus a few conversions per pixel. That's no problem for a modern CPU at 60 frames per second, even in a debug build. But it gives you an idea of the amount of work, because everything that we draw in this series is done for every pixel, every frame. In the second half of the series, we measure how long a frame takes, and we make it faster.

## What's Next

We now have a surface that we can write pixels to, and a window that shows it. In [part 2](/2d-software-rasterizer-2-clipping/), we look at what happens when we draw outside of the surface. We add a fast `Clear`, keep every drawing operation inside the surface, and add *scissor rectangles*, which limit drawing to a part of the screen.

## References

<span id="cite-1"></span>\[1\] Emscripten, “Emscripten”, emscripten.org, 2026. \[Online\]. Available: [https://emscripten.org/](https://emscripten.org/). \[Accessed: 28- Nov- 2026\].

<span id="cite-2"></span>\[2\] Kitware, “CMAKE_CXX_STANDARD”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/variable/CMAKE_CXX_STANDARD.html](https://cmake.org/cmake/help/latest/variable/CMAKE_CXX_STANDARD.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-3"></span>\[3\] Kitware, “CMAKE_CXX_EXTENSIONS”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/variable/CMAKE_CXX_EXTENSIONS.html](https://cmake.org/cmake/help/latest/variable/CMAKE_CXX_EXTENSIONS.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-4"></span>\[4\] Kitware, “CMAKE_RUNTIME_OUTPUT_DIRECTORY”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/variable/CMAKE_RUNTIME_OUTPUT_DIRECTORY.html](https://cmake.org/cmake/help/latest/variable/CMAKE_RUNTIME_OUTPUT_DIRECTORY.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-5"></span>\[5\] SDL, “Simple DirectMedia Layer”, libsdl.org, 2026. \[Online\]. Available: [https://www.libsdl.org/](https://www.libsdl.org/). \[Accessed: 28- Nov- 2026\].

<span id="cite-6"></span>\[6\] Kitware, “FetchContent”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/module/FetchContent.html](https://cmake.org/cmake/help/latest/module/FetchContent.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-7"></span>\[7\] Kitware, “function”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/command/function.html](https://cmake.org/cmake/help/latest/command/function.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-8"></span>\[8\] Kitware, “target_compile_options”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/command/target_compile_options.html](https://cmake.org/cmake/help/latest/command/target_compile_options.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-9"></span>\[9\] Microsoft, “/w, /W0, /W1, /W2, /W3, /W4, /w1, /w2, /w3, /w4, /Wall, /wd, /we, /wo, /Wv, /WX (Warning level)”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/cpp/build/reference/compiler-option-warning-level](https://learn.microsoft.com/en-us/cpp/build/reference/compiler-option-warning-level). \[Accessed: 28- Nov- 2026\].

<span id="cite-10"></span>\[10\] Microsoft, “/permissive- (Standards conformance)”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance](https://learn.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance). \[Accessed: 28- Nov- 2026\].

<span id="cite-11"></span>\[11\] Microsoft, “/utf-8 (Set source and execution character sets to UTF-8)”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8](https://learn.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8). \[Accessed: 28- Nov- 2026\].

<span id="cite-12"></span>\[12\] GNU Project, “Options to Request or Suppress Warnings”, GCC Online Documentation, 2026. \[Online\]. Available: [https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-13"></span>\[13\] Kitware, “add_subdirectory”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/command/add_subdirectory.html](https://cmake.org/cmake/help/latest/command/add_subdirectory.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-14"></span>\[14\] Kitware, “add_library”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/command/add_library.html](https://cmake.org/cmake/help/latest/command/add_library.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-15"></span>\[15\] Kitware, “target_include_directories”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/command/target_include_directories.html](https://cmake.org/cmake/help/latest/command/target_include_directories.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-16"></span>\[16\] Kitware, “target_link_libraries”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/command/target_link_libraries.html](https://cmake.org/cmake/help/latest/command/target_link_libraries.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-17"></span>\[17\] Kitware, “cmake-presets(7)”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-18"></span>\[18\] Microsoft, “CMake projects in Visual Studio”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio). \[Accessed: 28- Nov- 2026\].

<span id="cite-19"></span>\[19\] Ninja, “Ninja, a small build system with a focus on speed”, ninja-build.org, 2026. \[Online\]. Available: [https://ninja-build.org/](https://ninja-build.org/). \[Accessed: 28- Nov- 2026\].

<span id="cite-20"></span>\[20\] cppreference.com, “Fixed width integer types”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/types/integer](https://en.cppreference.com/w/cpp/types/integer). \[Accessed: 28- Nov- 2026\].

<span id="cite-21"></span>\[21\] SDL, “SDL_PixelFormat”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_PixelFormat](https://wiki.libsdl.org/SDL3/SDL_PixelFormat). \[Accessed: 28- Nov- 2026\].

<span id="cite-22"></span>\[22\] cppreference.com, “explicit specifier”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/explicit](https://en.cppreference.com/w/cpp/language/explicit). \[Accessed: 28- Nov- 2026\].

<span id="cite-23"></span>\[23\] cppreference.com, “Implicit conversions”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/implicit_conversion](https://en.cppreference.com/w/cpp/language/implicit_conversion). \[Accessed: 28- Nov- 2026\].

<span id="cite-24"></span>\[24\] cppreference.com, “Default comparisons”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/default_comparisons](https://en.cppreference.com/w/cpp/language/default_comparisons). \[Accessed: 28- Nov- 2026\].

<span id="cite-25"></span>\[25\] cppreference.com, “constexpr specifier”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/constexpr](https://en.cppreference.com/w/cpp/language/constexpr). \[Accessed: 28- Nov- 2026\].

<span id="cite-26"></span>\[26\] cppreference.com, “static_assert declaration”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/static_assert](https://en.cppreference.com/w/cpp/language/static_assert). \[Accessed: 28- Nov- 2026\].

<span id="cite-27"></span>\[27\] Wikipedia, “Endianness”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Endianness](https://en.wikipedia.org/wiki/Endianness). \[Accessed: 28- Nov- 2026\].

<span id="cite-28"></span>\[28\] W3C, “CSS Color Module Level 4, Named Colors”, w3.org, 2026. \[Online\]. Available: [https://www.w3.org/TR/css-color-4/#named-colors](https://www.w3.org/TR/css-color-4/#named-colors). \[Accessed: 28- Nov- 2026\].

<span id="cite-29"></span>\[29\] MDN, “<named-color>”, MDN Web Docs, 2026. \[Online\]. Available: [https://developer.mozilla.org/en-US/docs/Web/CSS/named-color](https://developer.mozilla.org/en-US/docs/Web/CSS/named-color). \[Accessed: 28- Nov- 2026\].

<span id="cite-30"></span>\[30\] cppreference.com, “Static members”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/static](https://en.cppreference.com/w/cpp/language/static). \[Accessed: 28- Nov- 2026\].

<span id="cite-31"></span>\[31\] cppreference.com, “Type”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/type](https://en.cppreference.com/w/cpp/language/type). \[Accessed: 28- Nov- 2026\].

<span id="cite-32"></span>\[32\] cppreference.com, “inline specifier”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/inline](https://en.cppreference.com/w/cpp/language/inline). \[Accessed: 28- Nov- 2026\].

<span id="cite-33"></span>\[33\] Wikipedia, “Row- and column-major order”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Row-_and_column-major_order](https://en.wikipedia.org/wiki/Row-_and_column-major_order). \[Accessed: 28- Nov- 2026\].

<span id="cite-34"></span>\[34\] cppreference.com, “std::mdspan”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/mdspan](https://en.cppreference.com/w/cpp/container/mdspan). \[Accessed: 28- Nov- 2026\].

<span id="cite-35"></span>\[35\] C. Trott et al., “P0009: MDSPAN”, WG21, 2026. \[Online\]. Available: [https://wg21.link/P0009](https://wg21.link/P0009). \[Accessed: 28- Nov- 2026\].

<span id="cite-36"></span>\[36\] cppreference.com, “std::span”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/span](https://en.cppreference.com/w/cpp/container/span). \[Accessed: 28- Nov- 2026\].

<span id="cite-37"></span>\[37\] cppreference.com, “std::extents, std::dextents”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/mdspan/extents](https://en.cppreference.com/w/cpp/container/mdspan/extents). \[Accessed: 28- Nov- 2026\].

<span id="cite-38"></span>\[38\] cppreference.com, “Operator overloading”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/operators](https://en.cppreference.com/w/cpp/language/operators). \[Accessed: 28- Nov- 2026\].

<span id="cite-39"></span>\[39\] M. Park et al., “P2128: Multidimensional subscript operator”, WG21, 2026. \[Online\]. Available: [https://wg21.link/P2128](https://wg21.link/P2128). \[Accessed: 28- Nov- 2026\].

<span id="cite-40"></span>\[40\] cppreference.com, “std::layout_right”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/mdspan/layout_right](https://en.cppreference.com/w/cpp/container/mdspan/layout_right). \[Accessed: 28- Nov- 2026\].

<span id="cite-41"></span>\[41\] cppreference.com, “std::layout_left”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/mdspan/layout_left](https://en.cppreference.com/w/cpp/container/mdspan/layout_left). \[Accessed: 28- Nov- 2026\].

<span id="cite-42"></span>\[42\] cppreference.com, “std::vector”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/vector](https://en.cppreference.com/w/cpp/container/vector). \[Accessed: 28- Nov- 2026\].

<span id="cite-43"></span>\[43\] cppreference.com, “std::size_t”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/types/size_t](https://en.cppreference.com/w/cpp/types/size_t). \[Accessed: 28- Nov- 2026\].

<span id="cite-44"></span>\[44\] cppreference.com, “assert”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/error/assert](https://en.cppreference.com/w/cpp/error/assert). \[Accessed: 28- Nov- 2026\].

<span id="cite-45"></span>\[45\] cppreference.com, “Class declaration”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/class](https://en.cppreference.com/w/cpp/language/class). \[Accessed: 28- Nov- 2026\].

<span id="cite-46"></span>\[46\] cppreference.com, “std::expected”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected). \[Accessed: 28- Nov- 2026\].

<span id="cite-47"></span>\[47\] V. Botet and J. F. Bastien, “P0323: std::expected”, WG21, 2026. \[Online\]. Available: [https://wg21.link/P0323](https://wg21.link/P0323). \[Accessed: 28- Nov- 2026\].

<span id="cite-48"></span>\[48\] cppreference.com, “Move constructors”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/move_constructor](https://en.cppreference.com/w/cpp/language/move_constructor). \[Accessed: 28- Nov- 2026\].

<span id="cite-49"></span>\[49\] cppreference.com, “std::unique_ptr”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/memory/unique_ptr](https://en.cppreference.com/w/cpp/memory/unique_ptr). \[Accessed: 28- Nov- 2026\].

<span id="cite-50"></span>\[50\] cppreference.com, “Destructors”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/destructor](https://en.cppreference.com/w/cpp/language/destructor). \[Accessed: 28- Nov- 2026\].

<span id="cite-51"></span>\[51\] SDL, “SDL_Init”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_Init](https://wiki.libsdl.org/SDL3/SDL_Init). \[Accessed: 28- Nov- 2026\].

<span id="cite-52"></span>\[52\] SDL, “SDL_GetError”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_GetError](https://wiki.libsdl.org/SDL3/SDL_GetError). \[Accessed: 28- Nov- 2026\].

<span id="cite-53"></span>\[53\] cppreference.com, “std::unexpected”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/utility/expected/unexpected](https://en.cppreference.com/w/cpp/utility/expected/unexpected). \[Accessed: 28- Nov- 2026\].

<span id="cite-54"></span>\[54\] SDL, “SDL_Quit”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_Quit](https://wiki.libsdl.org/SDL3/SDL_Quit). \[Accessed: 28- Nov- 2026\].

<span id="cite-55"></span>\[55\] cppreference.com, “std::basic_string_view”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/string/basic_string_view](https://en.cppreference.com/w/cpp/string/basic_string_view). \[Accessed: 28- Nov- 2026\].

<span id="cite-56"></span>\[56\] SDL, “SDL_CreateWindow”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_CreateWindow](https://wiki.libsdl.org/SDL3/SDL_CreateWindow). \[Accessed: 28- Nov- 2026\].

<span id="cite-57"></span>\[57\] SDL, “SDL_CreateRenderer”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_CreateRenderer](https://wiki.libsdl.org/SDL3/SDL_CreateRenderer). \[Accessed: 28- Nov- 2026\].

<span id="cite-58"></span>\[58\] SDL, “SDL_PollEvent”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_PollEvent](https://wiki.libsdl.org/SDL3/SDL_PollEvent). \[Accessed: 28- Nov- 2026\].

<span id="cite-59"></span>\[59\] SDL, “SDL_EventType”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_EventType](https://wiki.libsdl.org/SDL3/SDL_EventType). \[Accessed: 28- Nov- 2026\].

<span id="cite-60"></span>\[60\] SDL, “SDL_KeyboardEvent”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_KeyboardEvent](https://wiki.libsdl.org/SDL3/SDL_KeyboardEvent). \[Accessed: 28- Nov- 2026\].

<span id="cite-61"></span>\[61\] SDL, “SDL_Keycode”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_Keycode](https://wiki.libsdl.org/SDL3/SDL_Keycode). \[Accessed: 28- Nov- 2026\].

<span id="cite-62"></span>\[62\] SDL, “SDL_CreateTexture”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_CreateTexture](https://wiki.libsdl.org/SDL3/SDL_CreateTexture). \[Accessed: 28- Nov- 2026\].

<span id="cite-63"></span>\[63\] SDL, “SDL_TextureAccess”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_TextureAccess](https://wiki.libsdl.org/SDL3/SDL_TextureAccess). \[Accessed: 28- Nov- 2026\].

<span id="cite-64"></span>\[64\] SDL, “SDL_SetTextureScaleMode”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_SetTextureScaleMode](https://wiki.libsdl.org/SDL3/SDL_SetTextureScaleMode). \[Accessed: 28- Nov- 2026\].

<span id="cite-65"></span>\[65\] SDL, “SDL_SetRenderLogicalPresentation”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_SetRenderLogicalPresentation](https://wiki.libsdl.org/SDL3/SDL_SetRenderLogicalPresentation). \[Accessed: 28- Nov- 2026\].

<span id="cite-66"></span>\[66\] SDL, “SDL_UpdateTexture”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_UpdateTexture](https://wiki.libsdl.org/SDL3/SDL_UpdateTexture). \[Accessed: 28- Nov- 2026\].

<span id="cite-67"></span>\[67\] SDL, “SDL_LockTexture”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_LockTexture](https://wiki.libsdl.org/SDL3/SDL_LockTexture). \[Accessed: 28- Nov- 2026\].

<span id="cite-68"></span>\[68\] SDL, “SDL_RenderClear”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_RenderClear](https://wiki.libsdl.org/SDL3/SDL_RenderClear). \[Accessed: 28- Nov- 2026\].

<span id="cite-69"></span>\[69\] SDL, “SDL_RenderTexture”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_RenderTexture](https://wiki.libsdl.org/SDL3/SDL_RenderTexture). \[Accessed: 28- Nov- 2026\].

<span id="cite-70"></span>\[70\] SDL, “SDL_RenderPresent”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_RenderPresent](https://wiki.libsdl.org/SDL3/SDL_RenderPresent). \[Accessed: 28- Nov- 2026\].

<span id="cite-71"></span>\[71\] Emscripten, “emscripten.h”, Emscripten Documentation, 2026. \[Online\]. Available: [https://emscripten.org/docs/api_reference/emscripten.h.html](https://emscripten.org/docs/api_reference/emscripten.h.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-72"></span>\[72\] Emscripten, “Asyncify”, Emscripten Documentation, 2026. \[Online\]. Available: [https://emscripten.org/docs/porting/asyncify.html](https://emscripten.org/docs/porting/asyncify.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-73"></span>\[73\] Wikipedia, “Screen tearing”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Screen_tearing](https://en.wikipedia.org/wiki/Screen_tearing). \[Accessed: 28- Nov- 2026\].

<span id="cite-74"></span>\[74\] SDL, “SDL_SetRenderVSync”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_SetRenderVSync](https://wiki.libsdl.org/SDL3/SDL_SetRenderVSync). \[Accessed: 28- Nov- 2026\].

<span id="cite-75"></span>\[75\] MDN, “Window: requestAnimationFrame() method”, MDN Web Docs, 2026. \[Online\]. Available: [https://developer.mozilla.org/en-US/docs/Web/API/Window/requestAnimationFrame](https://developer.mozilla.org/en-US/docs/Web/API/Window/requestAnimationFrame). \[Accessed: 28- Nov- 2026\].

<span id="cite-76"></span>\[76\] SDL, “SDL_SetWindowFullscreen”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_SetWindowFullscreen](https://wiki.libsdl.org/SDL3/SDL_SetWindowFullscreen). \[Accessed: 28- Nov- 2026\].

<span id="cite-77"></span>\[77\] SDL, “SDL_GetWindowFlags”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_GetWindowFlags](https://wiki.libsdl.org/SDL3/SDL_GetWindowFlags). \[Accessed: 28- Nov- 2026\].

<span id="cite-78"></span>\[78\] SDL, “SDL_GetTicksNS”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_GetTicksNS](https://wiki.libsdl.org/SDL3/SDL_GetTicksNS). \[Accessed: 28- Nov- 2026\].

<span id="cite-79"></span>\[79\] cppreference.com, “std::format”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/utility/format/format](https://en.cppreference.com/w/cpp/utility/format/format). \[Accessed: 28- Nov- 2026\].

<span id="cite-80"></span>\[80\] SDL, “SDL_SetWindowTitle”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_SetWindowTitle](https://wiki.libsdl.org/SDL3/SDL_SetWindowTitle). \[Accessed: 28- Nov- 2026\].

<span id="cite-81"></span>\[81\] cppreference.com, “std::clamp”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/algorithm/clamp](https://en.cppreference.com/w/cpp/algorithm/clamp). \[Accessed: 28- Nov- 2026\].

<span id="cite-82"></span>\[82\] cppreference.com, “std::println”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/io/println](https://en.cppreference.com/w/cpp/io/println). \[Accessed: 28- Nov- 2026\].

<span id="cite-83"></span>\[83\] cppreference.com, “std::chrono::steady_clock”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/chrono/steady_clock](https://en.cppreference.com/w/cpp/chrono/steady_clock). \[Accessed: 28- Nov- 2026\].

<span id="cite-84"></span>\[84\] cppreference.com, “std::chrono::duration”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/chrono/duration](https://en.cppreference.com/w/cpp/chrono/duration). \[Accessed: 28- Nov- 2026\].

<span id="cite-85"></span>\[85\] cppreference.com, “std::sin”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/numeric/math/sin](https://en.cppreference.com/w/cpp/numeric/math/sin). \[Accessed: 28- Nov- 2026\].

<span id="cite-86"></span>\[86\] cppreference.com, “Mathematical constants”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/numeric/constants](https://en.cppreference.com/w/cpp/numeric/constants). \[Accessed: 28- Nov- 2026\].
