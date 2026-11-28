---
title: "2D Software Rasterizer Part 7: Texturing"
description: "Load images with stb_image, give every vertex a texture coordinate, and fill triangles with the texels of a texture, with nearest-neighbor sampling, the Wrap, Mirror and Clamp address modes, and an alpha test for sprites."
slug: 2d-software-rasterizer-7-texturing
lastUpdated: 2026-11-28
---

In [part 6](/2d-software-rasterizer-6-barycentric/), every vertex of a triangle got its own color, and the barycentric coordinates blended the colors across the triangle. In this part, the colors come from an image. Every vertex gets a *texture coordinate*, a position on the image, and every pixel of the triangle gets the color of the image at its own texture coordinate. This is *texture mapping* [\[1\]](#cite-1), and it's how almost every 2D game draws its sprites and tiles.

In this part, you will:

1. learn what textures, texels and texture coordinates are,
2. load images with `stb_image`,
3. read the color of a texture at a texture coordinate, with *nearest-neighbor sampling*,
4. decide what happens outside the texture, with the *address modes* Wrap, Mirror and Clamp,
5. fill triangles with a texture, tinted by the vertex colors,
6. and skip the transparent pixels of a sprite with an *alpha test*.

At the end of this part, your program shows this:

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-7-texturing/index.html" title="A little platformer scene of 320 by 180 pixels, scaled up four times: a repeating background of green hills, a ground of grass tiles, a tree, a bush and a sign, a board of nine signs, a big golden key that turns and grows, pink-tinted clouds that drift by, and a green alien in a space helmet that walks to the right and back" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Click the demo first, so that it gets the keys. <kbd>F1</kbd> switches the address mode of the board of signs, and <kbd>F2</kbd> shows the texels as checkerboards.

</div>

The code for this part is at the [`part-07-texturing`](https://github.com/3dgep/2d-software-rasterizer/tree/part-07-texturing) tag of the [2d-software-rasterizer](https://github.com/3dgep/2d-software-rasterizer) repository. You can also see [everything that changed since part 6](https://github.com/3dgep/2d-software-rasterizer/compare/part-06-barycentric...part-07-texturing).

## Textures

A *texture* is an image that we draw onto triangles. It's a grid of colors, just like our `Surface`, so in our rasterizer a texture *is* a `Surface`. To tell the pixels of a texture apart from the pixels on the screen, the pixels of a texture are called *texels* (texture elements) [\[2\]](#cite-2).

Edwin Catmull introduced texture mapping in 1974 [\[3\]](#cite-3), to put images on the curved surfaces of 3D objects. Since then, it has been used for almost everything that a computer draws, from the walls in 3D games to the letters of a font. Paul Heckbert's survey from 1986 [\[4\]](#cite-4) is still a good overview of the ideas.

### Texture Coordinates

Every vertex of a triangle gets a texture coordinate, which says where that vertex is on the texture. A texture coordinate has two parts, $u$ for the horizontal direction and $v$ for the vertical direction. That's why texture coordinates are often called *uv coordinates*. They are *normalized*. The top-left corner of the texture is $(0, 0)$ and the bottom-right corner is $(1, 1)$, however many texels the texture has. So a triangle with the texture coordinates $(0, 0)$, $(1, 0)$ and $(0, 1)$ shows the top-left half of the texture, whether the texture is 8 or 800 texels wide. This is how 3D APIs like Direct3D and Vulkan do it too.

Between the vertices, the texture coordinates are interpolated with the barycentric coordinates, exactly like the colors in part 6:

$$
\begin{aligned}
u(p) &= \lambda_0 u_0 + \lambda_1 u_1 + \lambda_2 u_2 \\
v(p) &= \lambda_0 v_0 + \lambda_1 v_1 + \lambda_2 v_2
\end{aligned}
$$

Then the rasterizer reads the color of the texture at $(u, v)$. Reading a color from a texture is called *sampling* the texture.

### Nearest-Neighbor Sampling

A texture that is $w$ texels wide divides the range of $u$ from 0 to 1 into $w$ equal parts. Texel $i$ covers the values of $u$ from $\frac{i}{w}$ up to $\frac{i + 1}{w}$, so the texel that $u$ falls in is

$$
i = \lfloor u \cdot w \rfloor
$$

and the same for $v$, with the height of the texture. This is *nearest-neighbor sampling* [\[5\]](#cite-5), because the texel that $u$ falls in is the texel whose center is nearest to $u$. The center of texel $i$ is at $u = \frac{i + 0.5}{w}$, halfway between its edges. That matches our pixels, whose centers are at $x + 0.5$.

When a texture is drawn at its own size, with its edges on the edges of the pixels, every pixel center lands exactly on a texel center, and every pixel gets exactly one texel. When it's drawn larger, several pixels get the same texel, and the texels become visible squares. For pixel art, that's exactly what we want. For photos, it looks blocky, and in part 12 we'll blend the four nearest texels instead, with *bilinear filtering*.

### Address Modes

Texture coordinates don't have to stay between 0 and 1. A coordinate of 2.5 is two and a half textures to the right. Then $\lfloor u \cdot w \rfloor$ is a texel index outside the texture, and the *address mode* decides which texel of the texture it becomes. We use the three most common address modes, with the names that Direct3D uses for them [\[6\]](#cite-6) (Vulkan calls them `REPEAT`, `MIRRORED_REPEAT` and `CLAMP_TO_EDGE` [\[7\]](#cite-7)).

- *Wrap* repeats the texture. The texel index is $i \bmod w$, so $u = 1.25$ gives the same texel as $u = 0.25$. With Wrap, a small texture can cover a large area, like a floor or a background.
- *Mirror* repeats the texture too, but every other copy is flipped. A texture and its mirror image together repeat every $2w$ texels. So with $m = i \bmod 2w$, the texel is $m$ when $m < w$, and $2w - 1 - m$ when it's in the flipped copy. $u = 1.25$ gives the same texel as $u = 0.75$. The copies always meet at the same texels, so there's never a visible seam between them.
- *Clamp* clamps the texel index to the range from $0$ to $w - 1$. Outside the texture, the texels at its edges are stretched out.

Try it below. The left half is a grid of screen pixels with a quad of two triangles on it. Drag its corners to change its shape. The right half is an 8 by 8 texture, with $u$ and $v$ from $-1$ to $2$, so it shows the texture and the eight copies around it that the address mode makes. The dashed outline is the quad in texture coordinates, and the dots are the texture coordinates of the pixel centers that the quad covers. Hover over a pixel to see its texture coordinate, the texel that the coordinate falls in, and the texel of the texture that the address mode turns it into. The slider changes the range of the quad's texture coordinates.

<iframe class="embed embed-demo embed-texture-sampling" src="/demos/texture-sampling.html" title="On the left, a grid of screen pixels with a textured quad whose corners can be dragged. On the right, an 8 by 8 texture of a yellow arrow on blue, with a red texel at its top-left corner and a green texel at its bottom-right corner, repeated around itself by the chosen address mode: Wrap, Mirror or Clamp. Hovering a pixel shows its texture coordinate and the texel it picks." loading="lazy" allowfullscreen></iframe>

## Loading Images

Before we can draw textures, we need to load them from image files. Reading a PNG or a JPEG ourselves would be a series of its own, so we use *stb_image* by Sean Barrett [\[8\]](#cite-8). It's a single header file that loads PNG, JPEG, BMP, TGA and a few more formats, and it's in the public domain.

### Adding stb_image

stb has no CMake project and no releases. So the root `CMakeLists.txt` downloads one fixed commit of it, with `FetchContent` [\[9\]](#cite-9), like it downloads SDL:

```cmake source="2d-software-rasterizer-part-07/CMakeLists.txt#L29-L41"
```

`URL` downloads a zip of the repository at that commit, which is faster than cloning its whole history. Because there's no `CMakeLists.txt` in stb, `FetchContent_MakeAvailable` only downloads and unpacks it, and sets `stb_SOURCE_DIR` to its folder. We make an *interface library* for it [\[10\]](#cite-10), a library without code of its own. Linking to `stb` only adds its folder to the include path. That folder is a `SYSTEM` include directory, so the compiler doesn't warn about the code in stb's headers. We can't fix that code, and our warnings should be about our own code.

The `rast` library lists the new files and links to `stb`:

```cmake source="2d-software-rasterizer-part-07/rast/CMakeLists.txt#L1-L20" ins={5,11,18-20}
```

`Sampler.hpp` and `Sampler.cpp` are new, and we'll get to them in a moment. `stb` is linked `PRIVATE`, like SDL, because only `Surface.cpp` includes it. A program that uses `rast` never sees stb's headers.

### Surface::Load

A texture is a `Surface`, so `Surface.hpp` gets two more includes:

```cpp source="2d-software-rasterizer-part-07/rast/include/rast/Surface.hpp#L6-L10" ins={1,4}
```

`<expected>` and `<string>` are for the new function that loads an image.

The class gets that new function:

```cpp source="2d-software-rasterizer-part-07/rast/include/rast/Surface.hpp#L20-L30" ins={3,9-11}
```

`Load` is a `static` function that creates a new `Surface` from an image file. Loading can fail, because the file may not exist or may not be an image. So `Load` returns a `std::expected` [\[11\]](#cite-11), like `Window::Create` in part 1. It holds either the `Surface` or a message that says what went wrong.

`Surface.cpp` includes stb_image:

```cpp source="2d-software-rasterizer-part-07/rast/src/Surface.cpp#L1-L12" ins={6-7,9-12}
```

`stb_image.h` has the declarations and the code of the library in one file. Every file that includes it gets the declarations. The code is only compiled in the one file that defines `STB_IMAGE_IMPLEMENTATION` before it includes the header. Define it in more than one file, and the linker finds the same functions twice.

`Load` asks stb_image for the pixels of the file:

```cpp source="2d-software-rasterizer-part-07/rast/src/Surface.cpp#L25-L49"
```

`stbi_load` returns a pointer to the pixels, row after row from the top, like our `Surface`. The last argument asks for 4 channels per pixel, red, green, blue and alpha, whatever the file has. A file without alpha gets an alpha of 255, fully opaque. The pointer must be freed with `stbi_image_free`, so it goes straight into a `std::unique_ptr` [\[12\]](#cite-12) with `stbi_image_free` as its deleter. Then the memory is freed however `Load` returns. When `stbi_load` fails, it returns a null pointer. Then `std::format` builds the message [\[13\]](#cite-13) from the path and `stbi_failure_reason`, which says why it failed.

stb_image gives us 4 bytes per pixel, in the order red, green, blue, alpha. Our pixels are `0xAARRGGBB` numbers, so we can't copy the bytes. Each pixel is built from its four bytes with the `Color` constructor from part 1.

## Samplers

The rules for sampling a texture are kept in a *sampler*, like in Direct3D and Vulkan. A sampler has nothing to do with any particular texture, so one sampler can sample many textures. Our sampler only has address modes for now. In part 12, it gets a filter too.

`Sampler.hpp` starts with the address modes:

```cpp source="2d-software-rasterizer-part-07/rast/include/rast/Sampler.hpp#L10-L18"
```

These are the three address modes from the start of this part. Direct3D has two more, `BORDER`, which uses a fixed color outside the texture, and `MIRROR_ONCE`, which mirrors the texture once and then clamps. We don't need them.

The sampler has an address mode for each direction:

```cpp source="2d-software-rasterizer-part-07/rast/include/rast/Sampler.hpp#L20-L26"
```

The two directions can have different address modes. A background that repeats from left to right, but not from top to bottom, uses Wrap for `addressU` and Clamp for `addressV`.

A free function samples a texture:

```cpp source="2d-software-rasterizer-part-07/rast/include/rast/Sampler.hpp#L28-L31"
```

`Sample` returns the color of the texel that the texture coordinate falls in.

### The Texel Index

`Sampler.cpp` starts with the texel index along one direction of the texture:

```cpp source="2d-software-rasterizer-part-07/rast/src/Sampler.cpp#L12-L22"
```

This is $\lfloor u \cdot w \rfloor$. The `float` must be converted to an `int`, but that conversion is undefined behavior [\[14\]](#cite-14) [\[15\]](#cite-15) when the value doesn't fit in an `int`, and a texture coordinate can be anything. So the index is clamped to $\pm 2^{30}$ first. That changes nothing for any reasonable coordinate. A `float` has 24 bits of precision, so a coordinate that far outside the texture has no information left about the texel anyway.

### Modulo

Wrap and Mirror need the remainder of a division, but the `%` operator of C++ doesn't give the remainder we want for negative numbers. `-1 % 8` is `-1`, because C++ rounds the division towards zero [\[16\]](#cite-16). We want 7, the last texel of the texture, so a small helper fixes the sign:

```cpp source="2d-software-rasterizer-part-07/rast/src/Sampler.cpp#L24-L29"
```

When the remainder is negative, adding `b` makes it the positive remainder, from 0 up to `b`.

### The Address Modes

`ApplyAddressMode` turns any texel index into the index of a texel inside the texture:

```cpp source="2d-software-rasterizer-part-07/rast/src/Sampler.cpp#L31-L48"
```

Each `case` is one of the formulas from the start of this part. Mirror takes the index modulo $2w$, the length of the texture and its mirror image together. In the first half, the index is used as it is. In the second half, it counts back from the end, so index $w$ becomes $w - 1$, index $w + 1$ becomes $w - 2$, and so on. The `return 0` after the `switch` is never reached, but without it, some compilers warn that the function can end without returning a value.

### Sample

`Sample` puts it all together:

```cpp source="2d-software-rasterizer-part-07/rast/src/Sampler.cpp#L52-L57"
```

It finds the texel index in each direction, applies that direction's address mode, and returns the texel.

## Texture Coordinates on Vertices

A vertex gets its texture coordinate in `Vertex.hpp`:

```cpp source="2d-software-rasterizer-part-07/rast/include/rast/Vertex.hpp#L11-L16" ins={4-5}
```

`texCoord` is a `Vec2`, with `u` in `x` and `v` in `y`. It comes after `color`, so the vertices of part 6, like `{ position, Color::Red }`, still compile. They are *aggregate initialization* [\[17\]](#cite-17), which fills the members in order, and gives the members that are left out their *default member initializers* [\[18\]](#cite-18). That's why `color` gets one now too. A textured vertex without a color is white, and as we'll see, white leaves the texture's colors as they are.

## Textured Triangles

`Rasterizer.hpp` includes the new header:

```cpp source="2d-software-rasterizer-part-07/rast/include/rast/Rasterizer.hpp#L3-L8" ins={3}
```

`Sampler.hpp` includes `Surface.hpp` itself, but `Rasterizer.hpp` keeps including everything it uses.

The rasterizer declares a third `FillTriangle`:

```cpp source="2d-software-rasterizer-part-07/rast/include/rast/Rasterizer.hpp#L95-L100"
```

It takes three vertices, like the `FillTriangle` of part 6, plus the texture and a sampler. The sampler has a *default argument* [\[19\]](#cite-19), `{}`, a `Sampler` with its default address modes, so `FillTriangle(v0, v1, v2, texture)` samples with Wrap.

### Discarding Pixels

Sprites have transparent parts. The character of the example is a 24 by 24 texel image, but most of those texels are the transparent background around it. Blending a transparent color with the color behind it is the topic of part 8. For now, we do what GPUs did before they could blend, and skip those pixels altogether. A pixel shader can *discard* a pixel [\[20\]](#cite-20), so that the pixel keeps the color it has. Our shading function needs to be able to do that too, so the loop of part 6 changes a little:

```cpp source="2d-software-rasterizer-part-07/rast/src/Rasterizer.cpp#L440-L479" ins={4-5,25-28}
```

The shading function can return a `std::optional<Color>` now. The loop only writes the pixel when the optional has a color, and an empty optional, `std::nullopt`, discards the pixel. The shading functions of the other two `FillTriangle`s still return a plain `Color`, which converts to an optional with a value. After inlining, the compiler can see that those optionals always have a value, and remove the check.

### Modulating Colors

The texel is *modulated* by the vertex color, which means they are multiplied, channel by channel:

```cpp source="2d-software-rasterizer-part-07/rast/src/Rasterizer.cpp#L493-L499"
```

Think of every channel as a number from 0 to 1, where 255 is 1. Multiplying a texel by white (all channels 1) leaves the texel as it is, multiplying it by black makes it black, and multiplying it by a color tints the texel with that color. With bytes, $\frac{x \cdot y}{255}$ is the same product, and `+ 127` rounds it to the nearest whole number instead of down. $x \cdot 255 + 127$ divided by 255 is exactly $x$ again, so white really leaves every texel unchanged.

### FillTriangle with a Texture

The new `FillTriangle` interpolates the texture coordinates and samples the texture:

```cpp source="2d-software-rasterizer-part-07/rast/src/Rasterizer.cpp#L538-L570"
```

The setup is the same as in part 6. This time, `s1` and `s2` are references to the swapped vertices instead of copies of their colors, because we need both their colors and their texture coordinates.

For every pixel, the shading function computes the barycentric coordinates, like the `FillTriangle` of part 6. It interpolates the texture coordinate with them, and samples the texture there. Then it multiplies the texel by the interpolated vertex color. The lambda captures everything by reference, with `[&]`, because the texture is too big to copy, and the lambda doesn't outlive the function. It returns either `std::nullopt` or a `Color`, so its return type must be written out as `std::optional<Color>`. Otherwise the compiler can't decide which of the two it returns.

The last step is the *alpha test*. A pixel with an alpha below 128, more transparent than opaque, is discarded. Older 3D APIs, like Direct3D 9, had the alpha test as a fixed part of the pipeline [\[21\]](#cite-21), and modern ones do the same with `discard` in a pixel shader. It gives sprites hard edges, which suits pixel art. Smooth edges need blending, in part 8.

## Assets

The example loads three images. Files that a program loads at run time are called *assets*, and they must be where the program can find them. On the web, there are no files at all. A web page can't open files on your computer, so the assets must be packed into the web build.

`rast_add_example` in `examples/CMakeLists.txt` takes care of both:

```cmake source="2d-software-rasterizer-part-07/examples/CMakeLists.txt#L1-L17" ins={1-7,9-10,14-17}
```

`rast_add_example` gets an optional `ASSETS` argument, the name of a folder next to the example's sources. `cmake_parse_arguments` [\[22\]](#cite-22) finds it among the function's arguments. Everything else, the source files, ends up in `arg_UNPARSED_ARGUMENTS`. `file(GLOB_RECURSE ...)` lists every file in the assets folder [\[23\]](#cite-23), so the build knows when one of them changes. `CONFIGURE_DEPENDS` makes the build look for new files too.

The web build packs the assets:

```cmake source="2d-software-rasterizer-part-07/examples/CMakeLists.txt#L19-L36" ins={13-18}
```

Emscripten's `--preload-file` packs the folder into a file next to the page, `index.data`. The page downloads it before `main` starts, and puts its files in a virtual file system in memory [\[24\]](#cite-24). `@assets` is the name of the folder in that file system, so `"assets/tiles.png"` works on the web exactly like on the desktop. `LINK_DEPENDS` relinks the example when an asset changes, so `index.data` is never out of date.

The desktop build copies the assets instead:

```cmake source="2d-software-rasterizer-part-07/examples/CMakeLists.txt#L37-L50" ins={1-12}
```

A custom target [\[25\]](#cite-25) runs `cmake -E copy_directory_if_different` [\[26\]](#cite-26) before every build of the example, which copies the assets folder next to the executable, but only the files that changed. `$<TARGET_FILE_DIR:...>` is the folder of the executable, which is different for the Debug and Release builds of Visual Studio.

The example opens `assets/tiles.png`, a path relative to the *current working directory*, the folder that the program runs in. When you start the executable from Explorer, that's the folder of the executable. But Visual Studio runs a program in the folder of its project by default, so `VS_DEBUGGER_WORKING_DIRECTORY` [\[27\]](#cite-27) tells it to use the folder of the executable too. If you start the example from a terminal, start it in its own folder.

The example's own `CMakeLists.txt` names its assets folder:

```cmake source="2d-software-rasterizer-part-07/examples/part-07-texturing/CMakeLists.txt"
```

The older examples don't pass `ASSETS`, so nothing changes for them.

### The Images

The images are from *Pixel Platformer* by Kenney [\[28\]](#cite-28), which is free to use for anything, under the CC0 license [\[29\]](#cite-29). `tiles.png` is a *sprite sheet*, also called a *texture atlas* [\[30\]](#cite-30), a single image with many small images on it. It has 20 by 9 tiles of 18 by 18 texels:

<img class="pixel-art" src="/images/2d-software-rasterizer-7/tiles.png" alt="Kenney's Pixel Platformer tile sheet: grass, dirt and snow ground tiles, coins, keys, hearts, gems, signs, trees, bushes, a cactus, water, pipes, clouds, a door, and the digits 0 to 9, each 18 by 18 texels" />

`characters.png` has 9 by 3 frames of 24 by 24 texels, two frames for most characters:

<img class="pixel-art" src="/images/2d-software-rasterizer-7/characters.png" alt="Kenney's Pixel Platformer character sheet: aliens in space helmets in green, blue, pink, yellow and beige, and enemies like blocks, bats, flies and spiked creatures, two frames each, 24 by 24 texels" />

`background.png` is a strip of 48 by 144 texels with green hills, which repeats seamlessly from left to right.

A sprite sheet keeps all the small images in one texture, which is how 2D games usually store them. To draw one sprite, we give the vertices the texture coordinates of that sprite's rectangle. There's one catch. The address modes work on the whole texture, not on one sprite of it. A tile of a sprite sheet can't repeat itself with Wrap, because Wrap would repeat the whole sheet. To repeat one tile, we either draw a quad for every copy, or give the tile a texture of its own. The example does both.

## The Example

The example draws a small platformer scene on the 320 by 180 pixel surface. Every sprite is a *quad*, a rectangle of two triangles, so the example works with rectangles of texture coordinates:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L22-L29"
```

A `TexRect` has the texture coordinates of the top-left corner and of the bottom-right corner of a quad.

The rectangle of a sprite on a sprite sheet is in texels, so it must be divided by the size of the texture:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L31-L38"
```

The texture coordinates of the rectangle's edges are the edges of its texels. When the quad is drawn at the sprite's own size, with its corners on whole pixels, the pixel centers land on the texel centers, and every texel shows up exactly once.

A sprite can be mirrored with its texture coordinates alone:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L40-L44"
```

Swapping $u_0$ and $u_1$ makes $u$ go from right to left across the quad, so the sprite is drawn mirrored. We don't need a second, mirrored copy of the sprite sheet.

A quad is drawn as two triangles:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L46-L58"
```

`DrawQuad` makes a vertex for every corner, with the texture coordinates of that corner. The top corners get one color and the bottom corners another, so a quad can be tinted with a gradient. Both triangles go clockwise, top-left, top-right, bottom-right, and top-left, bottom-right, bottom-left, and they share the diagonal from the top-left corner to the bottom-right corner. The top-left rule of part 5 makes sure that the pixels on the diagonal are drawn exactly once.

Most quads are rectangles at a position:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L60-L65"
```

`Corners` returns the four corners of a rectangle, clockwise from its top-left corner.

A quad can also be turned:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L67-L74"
```

`right` points along the top edge of the turned rectangle, and `down` along its left edge, each half as long as that edge. The corners are the center plus or minus both of them. With an angle of 0, `right` is $(\frac{w}{2}, 0)$ and `down` is $(0, \frac{h}{2})$, so the result is the same as `Corners`. In part 9, we'll do this with transformation matrices.

The tiles of `tiles.png` are 18 by 18 texels:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L76-L80"
```

`Tile` returns the rectangle of the tile in a column and a row of the sheet, both counted from 0.

The characters are 24 by 24 texels:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L82-L86"
```

`Character` does the same for `characters.png`.

### Showing the Texels

To see the texels, the example can swap every texture for a checkerboard of the same size:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L88-L103"
```

Neighboring texels of the checkerboard have different colors, so every texel is a visible square, however large it's drawn. The checkerboard keeps the alpha of the texture, so the alpha test still cuts out the shapes of the sprites.

The board of signs needs a texture of its own:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L105-L119"
```

`Crop` copies a rectangle of texels into a new `Surface`.

The example keeps each texture together with its checkerboard:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L121-L134"
```

The constructor moves the image into `image`, and makes the checkerboard from it. Members are initialized in the order they are declared, so `image` already holds the pixels when `Checkerboard(image)` reads them. The parameter `texture` is empty by then, because it was moved from. `Get` returns the texture to draw with.

Loading a texture can fail:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L136-L145"
```

`LoadTexture` prints the message of `Surface::Load` when it fails, and returns an empty optional.

The address mode is printed when it changes:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L147-L159"
```

`ToString` returns the name of an address mode. We don't have text on the surface yet (that's part 11), so the example prints the name to the console.

### Setting Up

`main` creates the window and loads the textures:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L161-L180"
```

If one of the textures can't be loaded, `LoadTexture` has already printed why, and the example stops.

Then come the sign, the surface and the samplers:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L182-L193"
```

The sign is tile (5, 4) of the tile sheet, the one with the arrow, cropped into its own texture so that the address modes work on it. The background repeats horizontally with Wrap and is clamped vertically. The board of signs starts with Wrap in both directions.

### The Main Loop

The main loop starts with the keys:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L195-L218"
```

<kbd>F1</kbd> switches the board of signs to the next address mode, in both directions at once, and <kbd>F2</kbd> switches between the images and the checkerboards.

Then the example picks its textures:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L220-L226"
```

`Get` returns the images or the checkerboards, so the drawing code below doesn't need to know which one it draws with.

The background is a single quad:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L228-L232"
```

The quad covers the sky, 320 by 144 pixels, and the strip is 48 by 144 texels. $u$ goes from 0 to $\frac{320}{48} \approx 6.67$ across the quad, so the strip is drawn $6\frac{2}{3}$ times, at its own size, and Wrap repeats it. $v$ goes from 0 to 1, because the strip is exactly as high as the quad. The surface isn't cleared anymore, because the background and the ground cover every pixel.

The ground is made of tiles:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L234-L242"
```

The ground is two rows of tiles from the tile sheet, grass on top and dirt below, one quad per tile. These tiles can't repeat with Wrap, because they are on a sprite sheet, so every copy is a quad of its own. The last column of tiles sticks out over the right edge of the surface, and the rasterizer clips it.

A few more tiles stand on the ground:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L244-L250"
```

The tree, the bush and the sign have transparent texels around them. The alpha test discards those pixels, so the background shows through.

The clouds are tinted:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L252-L257"
```

The clouds are three tiles wide on the sheet, and they are drawn twice as large. The top vertices are white and the bottom vertices are pink, so the modulation tints the clouds from white at the top to pink at the bottom, like clouds at sunset. `std::fmod` moves them to the right [\[31\]](#cite-31) and brings them back on the left, and `std::round` keeps their position on whole pixels, so their texels stay sharp squares.

The board of signs shows the address modes:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L259-L262"
```

The quad is three signs wide and high, and its texture coordinates go from $-1$ to $2$. The sign itself is in the middle, where the coordinates are between 0 and 1. With Wrap, the board shows nine signs that all point to the right. With Mirror, the signs in the left and right columns point to the left, and the ones in the top and bottom rows are upside down. With Clamp, there's one sign in the middle, and its edge texels are stretched out to the edges of the board.

A key turns and grows:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L264-L267"
```

The key is three to five times as large as its tile, and it turns slowly. Nearest-neighbor sampling keeps its texels as hard-edged squares, but turned squares on a grid of pixels get ragged edges, and the squares change shape from frame to frame. Press <kbd>F2</kbd> to see the turned texels as a checkerboard.

The last sprite is a character that walks:

```cpp source="2d-software-rasterizer-part-07/examples/part-07-texturing/main.cpp#L269-L286"
```

`distance` goes from 0 to twice the length of the walk, so the character walks to the right in the first half, and back to the left in the second half. Every sixth of a second, the frame switches between the two frames of the green alien, which makes it look like it's walking. The frames look to the left, so walking to the right, the character is flipped with `FlipHorizontally`.

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-7-texturing/index.html" title="A little platformer scene of 320 by 180 pixels, scaled up four times: a repeating background of green hills, a ground of grass tiles, a tree, a bush and a sign, a board of nine signs, a big golden key that turns and grows, pink-tinted clouds that drift by, and a green alien in a space helmet that walks to the right and back" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The example of part 7. Click the demo first, so that it gets the keys. <kbd>F1</kbd> switches the address mode of the board of signs, and <kbd>F2</kbd> shows the texels as checkerboards.

</div>

## What's Next

Our triangles can show images now, and with the alpha test, sprites can have holes. But the alpha test only knows two kinds of pixels, the ones that are drawn and the ones that aren't. In [part 8](/2d-software-rasterizer-8-blending/), we'll blend colors instead, so that sprites can have soft edges, and shapes can be half transparent. We'll see why *premultiplied alpha* makes blending easier, and why blending in sRGB numbers looks wrong.

## References

<span id="cite-1"></span>\[1\] Wikipedia, “Texture mapping”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Texture_mapping](https://en.wikipedia.org/wiki/Texture_mapping). \[Accessed: 28- Nov- 2026\].

<span id="cite-2"></span>\[2\] Wikipedia, “Texel (graphics)”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Texel_(graphics)](https://en.wikipedia.org/wiki/Texel_(graphics)). \[Accessed: 28- Nov- 2026\].

<span id="cite-3"></span>\[3\] E. Catmull, “A Subdivision Algorithm for Computer Display of Curved Surfaces”, Ph.D. dissertation, Dept. of Computer Science, University of Utah, Salt Lake City, UT, USA, 1974.

<span id="cite-4"></span>\[4\] P. S. Heckbert, “Survey of Texture Mapping”, IEEE Computer Graphics and Applications, vol. 6, no. 11, pp. 56–67, 1986. \[Online\]. Available: [https://doi.org/10.1109/MCG.1986.276672](https://doi.org/10.1109/MCG.1986.276672).

<span id="cite-5"></span>\[5\] Wikipedia, “Nearest-neighbor interpolation”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Nearest-neighbor_interpolation](https://en.wikipedia.org/wiki/Nearest-neighbor_interpolation). \[Accessed: 28- Nov- 2026\].

<span id="cite-6"></span>\[6\] Microsoft, “D3D12_TEXTURE_ADDRESS_MODE enumeration”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_texture_address_mode](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_texture_address_mode). \[Accessed: 28- Nov- 2026\].

<span id="cite-7"></span>\[7\] Khronos Group, “VkSamplerAddressMode”, Vulkan Documentation, 2026. \[Online\]. Available: [https://registry.khronos.org/vulkan/specs/latest/man/html/VkSamplerAddressMode.html](https://registry.khronos.org/vulkan/specs/latest/man/html/VkSamplerAddressMode.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-8"></span>\[8\] S. Barrett, “stb: single-file public domain libraries for C/C++”, GitHub, 2026. \[Online\]. Available: [https://github.com/nothings/stb](https://github.com/nothings/stb). \[Accessed: 28- Nov- 2026\].

<span id="cite-9"></span>\[9\] Kitware, “FetchContent”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/module/FetchContent.html](https://cmake.org/cmake/help/latest/module/FetchContent.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-10"></span>\[10\] Kitware, “add_library, Interface Libraries”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/command/add_library.html#interface-libraries](https://cmake.org/cmake/help/latest/command/add_library.html#interface-libraries). \[Accessed: 28- Nov- 2026\].

<span id="cite-11"></span>\[11\] cppreference.com, “std::expected”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected). \[Accessed: 28- Nov- 2026\].

<span id="cite-12"></span>\[12\] cppreference.com, “std::unique_ptr”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/memory/unique_ptr](https://en.cppreference.com/w/cpp/memory/unique_ptr). \[Accessed: 28- Nov- 2026\].

<span id="cite-13"></span>\[13\] cppreference.com, “std::format”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/utility/format/format](https://en.cppreference.com/w/cpp/utility/format/format). \[Accessed: 28- Nov- 2026\].

<span id="cite-14"></span>\[14\] cppreference.com, “Implicit conversions”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/implicit_conversion](https://en.cppreference.com/w/cpp/language/implicit_conversion). \[Accessed: 28- Nov- 2026\].

<span id="cite-15"></span>\[15\] cppreference.com, “Undefined behavior”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/ub](https://en.cppreference.com/w/cpp/language/ub). \[Accessed: 28- Nov- 2026\].

<span id="cite-16"></span>\[16\] cppreference.com, “Arithmetic operators, Multiplicative operators”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/operator_arithmetic#Multiplicative_operators](https://en.cppreference.com/w/cpp/language/operator_arithmetic#Multiplicative_operators). \[Accessed: 28- Nov- 2026\].

<span id="cite-17"></span>\[17\] cppreference.com, “Aggregate initialization”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/aggregate_initialization](https://en.cppreference.com/w/cpp/language/aggregate_initialization). \[Accessed: 28- Nov- 2026\].

<span id="cite-18"></span>\[18\] cppreference.com, “Data members, Member initialization”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/data_members#Member_initialization](https://en.cppreference.com/w/cpp/language/data_members#Member_initialization). \[Accessed: 28- Nov- 2026\].

<span id="cite-19"></span>\[19\] cppreference.com, “Default arguments”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/default_arguments](https://en.cppreference.com/w/cpp/language/default_arguments). \[Accessed: 28- Nov- 2026\].

<span id="cite-20"></span>\[20\] Microsoft, “discard (HLSL)”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/discard--sm4---asm-](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/discard--sm4---asm-). \[Accessed: 28- Nov- 2026\].

<span id="cite-21"></span>\[21\] Microsoft, “Alpha Testing State (Direct3D 9)”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/direct3d9/alpha-testing-state](https://learn.microsoft.com/en-us/windows/win32/direct3d9/alpha-testing-state). \[Accessed: 28- Nov- 2026\].

<span id="cite-22"></span>\[22\] Kitware, “cmake_parse_arguments”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/command/cmake_parse_arguments.html](https://cmake.org/cmake/help/latest/command/cmake_parse_arguments.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-23"></span>\[23\] Kitware, “file, GLOB_RECURSE”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/command/file.html#glob-recurse](https://cmake.org/cmake/help/latest/command/file.html#glob-recurse). \[Accessed: 28- Nov- 2026\].

<span id="cite-24"></span>\[24\] Emscripten, “Packaging Files”, Emscripten Documentation, 2026. \[Online\]. Available: [https://emscripten.org/docs/porting/files/packaging_files.html](https://emscripten.org/docs/porting/files/packaging_files.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-25"></span>\[25\] Kitware, “add_custom_target”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/command/add_custom_target.html](https://cmake.org/cmake/help/latest/command/add_custom_target.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-26"></span>\[26\] Kitware, “cmake(1), Run a Command-Line Tool”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/manual/cmake.1.html#run-a-command-line-tool](https://cmake.org/cmake/help/latest/manual/cmake.1.html#run-a-command-line-tool). \[Accessed: 28- Nov- 2026\].

<span id="cite-27"></span>\[27\] Kitware, “VS_DEBUGGER_WORKING_DIRECTORY”, CMake Documentation, 2026. \[Online\]. Available: [https://cmake.org/cmake/help/latest/prop_tgt/VS_DEBUGGER_WORKING_DIRECTORY.html](https://cmake.org/cmake/help/latest/prop_tgt/VS_DEBUGGER_WORKING_DIRECTORY.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-28"></span>\[28\] Kenney, “Pixel Platformer”, kenney.nl, 2026. \[Online\]. Available: [https://kenney.nl/assets/pixel-platformer](https://kenney.nl/assets/pixel-platformer). \[Accessed: 28- Nov- 2026\].

<span id="cite-29"></span>\[29\] Creative Commons, “CC0 1.0 Universal”, creativecommons.org, 2026. \[Online\]. Available: [https://creativecommons.org/publicdomain/zero/1.0/](https://creativecommons.org/publicdomain/zero/1.0/). \[Accessed: 28- Nov- 2026\].

<span id="cite-30"></span>\[30\] Wikipedia, “Texture atlas”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Texture_atlas](https://en.wikipedia.org/wiki/Texture_atlas). \[Accessed: 28- Nov- 2026\].

<span id="cite-31"></span>\[31\] cppreference.com, “std::fmod”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/numeric/math/fmod](https://en.cppreference.com/w/cpp/numeric/math/fmod). \[Accessed: 28- Nov- 2026\].
