---
title: "2D Software Rasterizer Part 8: Blending"
description: "Draw transparent shapes and sprites by blending their colors with the colors behind them, with source over destination, premultiplied alpha, additive blending, blend factors and operations like Direct3D and Vulkan, an exact division by 255 without a division, and blending in linear light with an sRGB pixel format, like a GPU."
slug: 2d-software-rasterizer-8-blending
lastUpdated: 2026-11-28
---

In [part 7](/2d-software-rasterizer-7-texturing/), we drew sprites with transparent backgrounds, but a pixel could only be drawn or not drawn at all. In this part, colors become truly transparent. A half-transparent red over blue makes purple, a shadow darkens the ground below it, and a glow brightens everything it covers. This is *blending*, and with it, the alpha channel that every `Color` has had since part 1 finally does something.

In this part, you will:

1. learn what alpha means, and how *source over destination* combines two colors,
2. see why *premultiplied alpha* makes blending simpler,
3. add *additive* blending for glows and light,
4. describe every blend with *blend factors* and a *blend operation*, like Direct3D and Vulkan,
5. divide by 255 exactly, without a division,
6. blend in *linear light* with an sRGB pixel format, like a GPU does,
7. and make every primitive of the rasterizer blend.

At the end of this part, your program shows this:

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-8-blending/index.html" title="Transparent shapes over a gray checkerboard of 320 by 180 pixels, scaled up four times: a white bar with black bars over it that go from transparent to opaque, three half-transparent circles in red, green and blue that overlap, a gold and a blue translucent rectangle, a pink rectangle that fades in and out, an orange triangle that fades out towards one corner, and an orange glow that follows the mouse" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Move the mouse over the demo to move the glow. Click the demo first, so that it gets the keys. <kbd>F1</kbd> switches the blend state, <kbd>F2</kbd> switches the format of the color target, and <kbd>F3</kbd> splits the screen between the two formats.

</div>

The code for this part is at the [`part-08-blending`](https://github.com/3dgep/2d-software-rasterizer/tree/part-08-blending) tag of the [2d-software-rasterizer](https://github.com/3dgep/2d-software-rasterizer) repository. You can also see [everything that changed since part 7](https://github.com/3dgep/2d-software-rasterizer/compare/part-07-texturing...part-08-blending).

## Alpha

The alpha of a color says how much of a pixel it covers. An alpha of 1 (255 in a byte) covers the pixel completely, so the color is *opaque*. An alpha of 0 covers nothing, so the color is invisible. In between, the color covers part of the pixel, and the color behind it shows through the rest. The idea is older than you might think. Alvy Ray Smith tells its history in a short memo [\[1\]](#cite-1). Alpha was invented in the 1970s at the New York Institute of Technology, by Ed Catmull (who also gave us texture mapping in part 7) and Smith himself.

### Source Over Destination

When we draw a color, the color we draw is called the *source*, and the color that's already in the pixel is the *destination*. Thomas Porter and Tom Duff described how to combine them in 1984 [\[2\]](#cite-2), with a set of operators for *compositing* images [\[3\]](#cite-3). The one we need almost all the time is *over*, the source over the destination. With the alpha of the source written as $a$, from 0 to 1, every channel of the result is

$$
c = c_s \, a + c_d \, (1 - a)
$$

A part $a$ of the pixel shows the source, and the rest, $1 - a$, shows the destination. With $a = 1$, the result is the source. With $a = 0$, it's the destination. With $a = \frac{1}{2}$, it's halfway between them.

The result has an alpha too:

$$
a_\text{result} = a + a_d \, (1 - a)
$$

That's how much the two layers cover together. The source covers $a$ of the pixel, and the destination covers $a_d$ of the rest. When the destination is opaque, like the background of a game, the result is opaque too.

### Premultiplied Alpha

In the formula of *over*, the source's color is always multiplied by its alpha. So we can do that multiplication once, ahead of time, and store colors with their channels already multiplied by their alpha. Those colors are called *premultiplied*. A half-transparent white, $(1, 1, 1, \frac{1}{2})$, is $(\frac{1}{2}, \frac{1}{2}, \frac{1}{2}, \frac{1}{2})$ when it's premultiplied. With premultiplied colors, *over* becomes

$$
c = c_s + c_d \, (1 - a)
$$

That looks like a small saving, one multiplication less per channel. But Porter and Duff already used premultiplied colors, Jim Blinn explains why they are the better choice [\[4\]](#cite-4), and Shawn Hargreaves explains what that means for games [\[5\]](#cite-5). These are the most useful reasons.

- With premultiplied colors, a color is *how much light it adds* and alpha is *how much it hides* of what's behind it. Those two are independent. A premultiplied color with an alpha of 0, but with red, green and blue, hides nothing and adds light. So one kind of blending can draw both a smoke cloud that blocks the view and a flame that glows, even in one texture.
- Blending premultiplied colors with each other works out. Two layers can be combined first and then drawn over a third, with the same result as drawing them one by one.
- When texels are blended with each other, like *bilinear filtering* does in part 12, straight colors leak the color of invisible texels into the result, and give sprites dark or bright fringes. Premultiplied texels don't, because an invisible texel is $(0, 0, 0, 0)$.

### Additive Blending

Not everything that's transparent hides what's behind it. Light, fire, sparks and glows only *add* to the colors behind them:

$$
c = c_d + c_s \, a
$$

Additive blending never darkens a pixel. Where many glows overlap, the result gets brighter and brighter, until it's white. The order in which the glows are drawn doesn't matter, because addition doesn't care about order. That isn't true for *over*, where the last layer drawn is in front.

### Blend Factors and Operations

Look at the three formulas again. Each one multiplies the source by something, multiplies the destination by something, and adds the two. Direct3D and Vulkan describe every blend that way [\[6\]](#cite-6) [\[7\]](#cite-7):

$$
c = (c_s \cdot f_s) \;\text{op}\; (c_d \cdot f_d)
$$

$f_s$ and $f_d$ are *blend factors*, chosen from a short list. They can be 0, 1, the alpha of the source or the destination, the color of the source or the destination, or one minus any of those. *op* is the *blend operation*. It's usually an addition, but it can also be a subtraction (either way around), or the smaller or the larger of the two colors. Alpha gets factors and an operation of its own.

| Blend | $f_s$ | $f_d$ | op |
|---|---|---|---|
| *Over* | `SrcAlpha` | `InvSrcAlpha` | `Add` |
| Premultiplied *over* | `One` | `InvSrcAlpha` | `Add` |
| Additive | `SrcAlpha` | `One` | `Add` |
| Multiply | `DestColor` | `Zero` | `Add` |
| Screen | `One` | `InvSrcColor` | `Add` |

*Multiply* multiplies the two colors. It can only darken, like a colored glass filter or a shadow, and white leaves the destination as it is. *Screen* is its opposite, $c_s + c_d (1 - c_s)$. It can only lighten, and black leaves the destination as it is. Both ignore the alpha of the source, so they're meant for opaque colors, or for premultiplied colors where a transparent color is black for Screen.

## Blending in Linear Light

In part 6, we saw that our colors are *sRGB* values, which aren't proportional to the amount of light. An sRGB byte of 128 isn't half as bright as 255, but only about 22%. The sRGB standard [\[8\]](#cite-8) [\[9\]](#cite-9) defines how to convert between the two. To *decode* a channel $c$ from 0 to 1 to linear light,

$$
c_\text{linear} =
\begin{cases}
\dfrac{c}{12.92} & \text{if } c \le 0.04045 \\[1ex]
\left(\dfrac{c + 0.055}{1.055}\right)^{2.4} & \text{otherwise}
\end{cases}
$$

and *encoding* does the opposite. The curve is close to $c^{2.2}$. It's often called *gamma* [\[10\]](#cite-10).

Blending with sRGB numbers gives the wrong amount of light. Draw black with an alpha of $\frac{1}{2}$ over white. The bytes give $255 \cdot \frac{1}{2} = 128$ (rounded), which is only 22% of the light of white, so the result looks far too dark. In linear light, half of white's light is $0.5$, which encodes to the byte 188. That's what a half-transparent black glass in front of a white wall really looks like. The same happens with colored layers, where the overlap of half-transparent colors looks too dark and muddy in sRGB numbers.

A GPU solves this with the *format* of the render target. In Direct3D, a render target with the format `DXGI_FORMAT_B8G8R8A8_UNORM` stores bytes and blends them as they are. With `DXGI_FORMAT_B8G8R8A8_UNORM_SRGB`, the bytes in memory are still sRGB, but the GPU decodes the destination to linear light before it blends [\[11\]](#cite-11) [\[12\]](#cite-12), and encodes the result again when it writes it. Vulkan has the same formats, like `VK_FORMAT_B8G8R8A8_SRGB` [\[13\]](#cite-13). We'll do exactly the same, with a `PixelFormat` on our `Surface`.

Try both below. The two rectangles are layers with their own color and alpha, and the strip at the bottom blends black over white, with an alpha from 0 on the left to 1 on the right. Switch between `Unorm` and `UnormSrgb` and watch the strip. With `Unorm`, the strip is already dark in the middle. With `UnormSrgb`, it looks like an even gradient. Hover over a pixel to see the numbers of its blend. Pick a preset from the *blend state* menu, or change the factors and the operation yourself.

<iframe class="embed embed-demo embed-alpha-blending" src="/demos/alpha-blending.html" title="Two overlapping translucent rectangles over a checkerboard, with a color and an alpha slider for each, menus for the blend state, its source factor, its operation and its destination factor, buttons for the format of the color target (Unorm, UnormSrgb), and a strip that blends black over white with an alpha from 0 to 1. Hovering a pixel shows the source, the destination and the result of every blend." loading="lazy" allowfullscreen></iframe>

## Pixel Formats

The format belongs to the surface, in `Surface.hpp`:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Surface.hpp#L20-L26"
```

Our pixels are `0xAARRGGBB` numbers. On a little-endian CPU, which is every CPU that you're likely to compile this on, the lowest byte of a number comes first in memory [\[14\]](#cite-14). So the bytes of a pixel are blue, green, red, alpha, in that order, and that's why the DXGI formats with this layout are called `B8G8R8A8`. *Unorm* means *unsigned normalized*. A byte from 0 to 255 stands for a number from 0 to 1.

The surface gets its format when it's made:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Surface.hpp#L31-L42" ins={4,12}
```

The format is a parameter of the constructor, with `B8G8R8A8Unorm` as its default argument, so the surfaces of the earlier parts don't change. `Format` returns it. The surface stores it in a new member, `m_Format`.

The constructor in `Surface.cpp` stores the format:

```cpp source="2d-software-rasterizer-part-08/rast/src/Surface.cpp#L17-L24" ins={1,4}
```

The format of a surface never changes, like the format of a texture on a GPU. To draw with another format, the example of this part makes a second surface.

## Blend States

The blending is in two new files, `Blend.hpp` and `Blend.cpp`. `Blend.hpp` starts with the blend factors:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Blend.hpp#L11-L27"
```

These are the factors that Direct3D 12 calls `D3D12_BLEND` and Vulkan `VkBlendFactor` [\[15\]](#cite-15) [\[16\]](#cite-16). Both have a few more, like a constant blend factor and factors for a second source color, but these ten are the ones that we need. For the alpha channel, there's no separate color, so the `Color` factors use the alpha of that color.

The blend operations come next:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Blend.hpp#L29-L38"
```

`Min` and `Max` ignore the factors and compare the colors themselves, like in Direct3D and Vulkan.

A `BlendState` holds the factors and operations for the colors and for alpha:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Blend.hpp#L40-L56"
```

The members have the same names as the members of `D3D12_RENDER_TARGET_BLEND_DESC`. `blendEnable` turns blending on or off. Without blending, the source replaces the destination, like all the drawing of the earlier parts. The default member initializers describe exactly that, with `One` for the source, `Zero` for the destination, and blending off. The defaulted `operator==` compares two blend states member by member [\[17\]](#cite-17).

Writing out six factors and operations for every draw would get tiresome, so `BlendState` has presets for the usual blends:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Blend.hpp#L58-L105"
```

These are the rows of the table from the start of this part. The presets use *designated initializers* [\[18\]](#cite-18) (`.srcBlend = ...`), so every preset only names the members that are different from the defaults. The members must be named in the order they are declared. All presets except `Opaque` blend alpha with `One` and `InvSrcAlpha`, which gives $a + a_d (1 - a)$, how much the two layers cover together. The presets are `constexpr`, so `BlendState::AlphaBlend()` costs nothing at run time.

The header declares the function that blends two colors:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Blend.hpp#L107-L111"
```

`Blend` takes the format of the color target, which decides whether it blends bytes or linear light.

Premultiplying a color depends on the format too:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Blend.hpp#L113-L119"
```

In linear light, *premultiplied* means that the linear color is multiplied by alpha. So a color that will be blended into a `B8G8R8A8UnormSrgb` target must be premultiplied in linear light, and one for a `B8G8R8A8Unorm` target as bytes. The second `Premultiply` does that for every pixel of a surface, like a texture.

The last two functions convert one channel:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Blend.hpp#L121-L123"
```

`SrgbToLinear` decodes an sRGB byte to linear light, and `LinearToSrgb` encodes it again.

## Dividing by 255

With bytes, a factor is a number from 0 to 255 instead of 0 to 1, so the formula of *over* becomes

$$
c = \frac{c_s \, a + c_d \, (255 - a)}{255}
$$

rounded to the nearest whole number. A division is one of the slowest things that a CPU does with integers. Dividing by 256 would be a cheap shift by 8 bits, but it gives the wrong result. White over white would be $\frac{255 \cdot 255}{256} = 254$, not 255, and every blend would darken the image a little. Jim Blinn found a way to divide by 255 with shifts and additions, exactly [\[19\]](#cite-19):

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L14-L25"
```

Adding 128 first turns the rounding down of the shifts into rounding to the nearest. Then $x + \lfloor x / 256 \rfloor$, shifted right by 8 bits, is about $x \cdot \frac{257}{65536}$, and $\frac{257}{65536}$ is just a tiny bit more than $\frac{1}{255}$. For every $x$ from 0 to $255 \cdot 255$, the largest product of two bytes, the result is exactly $x / 255$ rounded to the nearest whole number. Blinn called this trick "three wrongs make a right". The `static_assert`s check a few values at compile time [\[20\]](#cite-20), and the tests of this part check every product of two bytes.

With `Add`, the sum of the two weighted colors can be up to $2 \cdot 255 \cdot 255$, beyond the range where the trick is exact. But any sum of $255 \cdot 255$ or more is 255 or more after the division, and the result is clamped to 255 anyway. So we clamp the sum to $255 \cdot 255$ first, and the result stays exact.

## The sRGB Tables

The transfer functions of sRGB are the formulas from before:

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L27-L36"
```

`DecodeSrgb` and `EncodeSrgb` work on one channel, from 0 to 1. They call `std::pow`, which is slow [\[21\]](#cite-21), so we don't call them for every pixel.

A byte has only 256 values, so decoding can look up the result in a table:

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L38-L46"
```

The table is filled once, when the program starts, by a lambda that is called right away [\[22\]](#cite-22) (the `()` at the end). `DecodeTable[c]` is the linear light of the byte `c`, exactly.

Encoding goes from a `float` to a byte, so it can't have an entry for every value. Instead, linear light is split into 4096 steps:

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L48-L59"
```

Why 4096? sRGB spends more of its bytes on dark colors, where our eyes see small differences. The step from the sRGB byte 0 to 1 is only $0.0003$ in linear light, so the table must be fine enough to give even the darkest bytes a step of their own. With 4096 steps, every byte survives the trip to linear light and back, and every encoded value is within 1 of the exact result. The tests check both.

The two public functions use the tables:

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L156-L166"
```

`LinearToSrgb` clamps first, because `Add` can make a channel brighter than 1, and `Subtract` can make it negative. Then it rounds to the nearest step of the table.

## Blending Colors

Blending a channel needs the channel of both colors, and the alpha of both colors, for the factors:

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L61-L70"
```

`Channel` is a class template [\[23\]](#cite-23), because bytes are blended with `std::uint32_t` (from 0 to 255) and linear light with `float` (from 0 to 1).

A factor is one of the values of the channel, 0, 1, or one minus a value:

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L72-L100"
```

`Factor` is a function template too, so the same `switch` works for bytes and for floats. `one` is the value that stands for 1, which is 255 for bytes.

A channel as bytes is blended with integers:

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L102-L124"
```

Both weighted values are still 255 times too large, so the operation works on numbers up to $255 \cdot 255$, and `DivideBy255` divides the result. `Add` clamps the sum first, as we saw [\[24\]](#cite-24). The subtractions clamp at 0 by never subtracting the larger number from the smaller. This is used for the red, green and blue of a `B8G8R8A8Unorm` target, and for alpha in both formats.

A channel in linear light is blended with floats:

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L126-L152"
```

The same factors and operations, from 0 to 1. `LinearToSrgb` encodes the result, and clamps it to the range from 0 to 1.

`Blend` puts it together:

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L168-L200"
```

Without blending, the source is the result. Otherwise, alpha is blended first, always as a byte, because alpha isn't light. The lambda `bytes` fills in a `Channel` with the alphas of both colors. Then the red, green and blue are blended as bytes, or decoded with `DecodeTable` and blended in linear light, depending on the format.

### Premultiply

`Premultiply` multiplies the channels by alpha, in the way that the format blends:

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L202-L214"
```

For bytes, that's `DivideBy255` again. For linear light, the channel is decoded, multiplied, and encoded. A premultiplied color in an sRGB target is the sRGB encoding of the premultiplied linear color, which is what `BlendLinear` decodes again.

The version for a surface premultiplies every pixel:

```cpp source="2d-software-rasterizer-part-08/rast/src/Blend.cpp#L216-L222"
```

`Data` gives the pixels as one long span, so a range-based `for` loop visits them all.

The library lists the new files in `rast/CMakeLists.txt`:

```cmake source="2d-software-rasterizer-part-08/rast/CMakeLists.txt#L1-L16" ins={2,11}
```

## Blending in the Rasterizer

`Rasterizer.hpp` includes the new header:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Rasterizer.hpp#L3-L9" ins={1}
```

`Blend.hpp` includes `Surface.hpp` itself, for `PixelFormat`.

The blend state is part of the rasterizer's state, like the front face and the cull mode:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Rasterizer.hpp#L61-L68" ins={1-5,7}
```

Everything that's drawn after `SetBlendState` blends with that state, until it's set again. The format isn't part of this state, because it belongs to the color target. `Clear` never blends, like clearing a render target on a GPU. It sets the pixels to the color, whatever their alpha.

The rasterizer stores the blend state:

```cpp source="2d-software-rasterizer-part-08/rast/include/rast/Rasterizer.hpp#L110-L116" ins={6}
```

A default `BlendState` has blending off, so every example of the earlier parts draws exactly as before.

### Writing a Pixel

Every primitive writes its pixels through one small function at the top of `Rasterizer.cpp`:

```cpp source="2d-software-rasterizer-part-08/rast/src/Rasterizer.cpp#L17-L22"
```

`WritePixel` takes a reference to the pixel, because blending needs the pixel's color (the destination), and then replaces it with the result.

Some blends are just copies:

```cpp source="2d-software-rasterizer-part-08/rast/src/Rasterizer.cpp#L24-L42"
```

When the source is multiplied by 1 and the destination by 0, and they're added, the result is the source. That's the case for `One` and `Zero`, and also for `SrcAlpha` and `InvSrcAlpha` when the color is opaque. So an opaque color drawn with `AlphaBlend` or `PremultipliedAlpha` hides the destination completely. Additive blending is never a copy, because it adds to the destination even when the source is opaque.

`SetPixel` writes through `WritePixel`:

```cpp source="2d-software-rasterizer-part-08/rast/src/Rasterizer.cpp#L84-L90" ins={5}
```

The format comes from the color target, at the moment of drawing.

`FillRect` keeps its fast path:

```cpp source="2d-software-rasterizer-part-08/rast/src/Rasterizer.cpp#L92-L121" ins={11-22,25-28}
```

When the result is a copy, the rows are still filled with `std::fill_n`, as fast as before. Otherwise, every pixel is blended on its own, because every pixel has its own destination color. `FillCircle` and `DrawRect` use `FillRect`, so they blend too.

`DrawLine` writes each pixel through `WritePixel`:

```cpp source="2d-software-rasterizer-part-08/rast/src/Rasterizer.cpp#L224-L229" ins={2,5-6}
```

The pixel is a reference into the surface, chosen by the major axis, like the two assignments of part 3 did.

`DrawCircle` does the same in both of its lambdas:

```cpp source="2d-software-rasterizer-part-08/rast/src/Rasterizer.cpp#L315-L330" ins={2,5,12}
```

The midpoint walk of part 4 never visits a pixel twice, which matters now. A pixel that's blended twice would be twice as opaque as its neighbors.

### Triangles

The loop of the triangles gets the blend state and the format:

```cpp source="2d-software-rasterizer-part-08/rast/src/Rasterizer.cpp#L482-L523" ins={1-4,8-9,29}
```

The color that `shade` returns is written with `WritePixel`. The top-left rule of part 5 makes sure that triangles that share an edge never draw a pixel twice, which also matters now. With blending, a seam drawn by both triangles would show up as a darker line through a transparent quad.

The single-color `FillTriangle` passes them on:

```cpp source="2d-software-rasterizer-part-08/rast/src/Rasterizer.cpp#L547-L558" ins={10-11}
```

The color's alpha decides how much of the triangle shows, like with `FillRect`.

Gouraud shading interpolates the alpha of the vertex colors too (part 6 already did that), so a triangle can fade from opaque to transparent:

```cpp source="2d-software-rasterizer-part-08/rast/src/Rasterizer.cpp#L560-L582" ins={16,21-22}
```

The shading lambda has a name now, `shade`, which keeps the call to `RasterizeTriangle` short.

The textured `FillTriangle` only does the alpha test without blending:

```cpp source="2d-software-rasterizer-part-08/rast/src/Rasterizer.cpp#L584-L619" ins={14-16,18,29,35}
```

With blending, the transparent texels of a sprite blend away by themselves, and the half-transparent texels at its edges blend smoothly with the background. The alpha test would only cut those off.

## How Fast Is It?

Flexibility has a price. For every channel of every pixel, the blend looks up two factors with a `switch` and picks the operation with another one. Linear light adds two table lookups, a few float operations, a clamp and a conversion to an index. The example of this part measures both formats when you press <kbd>F4</kbd>, by blending a translucent color with `AlphaBlend` over a whole surface many times. On the computer this article was written on, a Release build gave these times for one million pixels:

| | No blending | `B8G8R8A8Unorm` | `B8G8R8A8UnormSrgb` |
|---|---|---|---|
| Native (MSVC) | 0.08 ms | 11.8 ms | 13.1 ms |
| WebAssembly (in a browser) | 0.21 ms | 8.6 ms | 15.4 ms |

The biggest difference is between blending and not blending at all, about a hundred times. Filling a row with `std::fill_n` lets the compiler write many pixels at once, while our blend is a function call per pixel, with several `switch`es per channel, and the compiler can't see through any of them, because the blend state is only known at run time. In the second half of the series, we'll make the common blend states fast. With `if constexpr` and a blend state as a template argument, the compiler can build a blend function for each preset with no `switch` at all, and inline it into the pixel loops. That's the price of keeping the code easy to follow for now.

## The Example

The example draws transparent shapes over a checkerboard, with one blend state after another. To print the name of a blend state, it keeps the name with the state:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L23-L28"
```

A `NamedBlendState` is a name and a blend state, nothing more.

<kbd>F1</kbd> switches between these:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L30-L38"
```

`std::array` deduces its element type and size from the initializers (class template argument deduction) [\[25\]](#cite-25). Because the presets are `constexpr`, the whole array is a compile-time constant.

The formats get a name too:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L40-L43"
```

There are only two formats, so a conditional expression is enough.

Most shapes are a named color with another alpha:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L45-L49"
```

`WithAlpha` copies the red, green and blue of the color, and replaces its alpha.

The glow is a texture that the example makes itself:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L51-L69"
```

Every texel is white. Its alpha is 1 in the middle of the texture and falls off to 0 at the edge of the circle, squared, so the glow fades out softly. Outside the circle, the alpha is 0.

A checkerboard shows what's behind the transparent shapes:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L71-L84"
```

The squares are 10 by 10 pixels, light and dark gray.

### Drawing the Shapes

`DrawShapes` draws everything with the blend state:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L86-L97"
```

The lambda `color` premultiplies a color when the blend state is `PremultipliedAlpha`, in the format of the color target, so the shapes look the same as with `AlphaBlend`. Thanks to the defaulted `operator==`, the blend state can be compared with the preset. The checkerboard is drawn without blending. It's the opaque background.

The first shapes are the bars:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L99-L105"
```

Black with an alpha of 0, 64, 128, 192 and 255 over white. `std::min` keeps the last alpha at 255, because $4 \cdot 64$ is 256. Compare the middle bar in the two formats with <kbd>F3</kbd>.

Then come three circles that overlap:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L107-L110"
```

Each circle is half transparent, so where they overlap, their colors mix, and where all three overlap, the checkerboard still shows through.

Translucent rectangles come next:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L112-L116"
```

The gold rectangle is drawn first, so the blue one is in front of it where they overlap. The pink rectangle's alpha follows a sine, so it fades in and out.

The triangle fades out:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L118-L121"
```

Its right vertex has an alpha of 0. Gouraud shading interpolates the alpha with the colors, so the triangle fades from opaque on the left to invisible at that vertex.

The glow is drawn last:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L123-L136"
```

The glow follows the mouse, or circles around when the mouse is outside the surface. It's a quad of two textured triangles, like the sprites of part 7, with orange vertex colors that tint the white texture. It always uses `Additive`, whatever the blend state of the other shapes, so it only brightens what's behind it.

### Measuring

<kbd>F4</kbd> measures how fast a format blends:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L138-L152"
```

`MeasureBlending` blends a half-transparent color over the whole surface 50 times, and divides the time by the number of pixels. The result is printed to the console, or to the browser's developer console on the web.

### The Main Loop

`main` makes two color targets:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L154-L179"
```

The two surfaces have the same size, one with each format. `target` points to the one that's shown, and `blendIndex` is the index of the blend state in `blendStates`.

The keys switch the blend state, the format, the split screen, and measure:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L181-L201"
```

<kbd>F1</kbd> moves to the next blend state, and back to the first one after the last. <kbd>F2</kbd> switches the format by switching between the two surfaces, because the format of a surface never changes.

Then the shapes are drawn:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L203-L213"
```

`blendState` is a reference to the chosen blend state. Without the split screen, the rasterizer draws into the chosen surface, and the window shows it.

With the split screen, the example draws into both:

```cpp source="2d-software-rasterizer-part-08/examples/part-08-blending/main.cpp#L215-L237"
```

The same shapes are drawn into both surfaces. Then the right half of the sRGB surface is copied over the right half of the other one, with a black line in the middle. The left half shows bytes, and the right half linear light.

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-8-blending/index.html" title="Transparent shapes over a gray checkerboard of 320 by 180 pixels, scaled up four times: a white bar with black bars over it that go from transparent to opaque, three half-transparent circles in red, green and blue that overlap, a gold and a blue translucent rectangle, a pink rectangle that fades in and out, an orange triangle that fades out towards one corner, and an orange glow that follows the mouse" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The example of part 8. Click the demo first, so that it gets the keys. <kbd>F1</kbd> switches the blend state, <kbd>F2</kbd> switches the format of the color target, and <kbd>F3</kbd> splits the screen between the two formats.

</div>

## Why Not Store Linear Light?

If linear light is right for blending, why not store every surface in linear light, and convert to sRGB only once, when the window shows it? That's how modern 3D renderers work, and SDL3 can even do the last step, with a renderer in a linear colorspace [\[26\]](#cite-26). But it needs more than 8 bits per channel.

sRGB isn't only a quirk of old monitors. It's also a way to compress light into a byte, by spending more of the 256 steps on dark colors, where our eyes see small differences. In linear light, 8 bits are far too few for the darks. The first step above black, $\frac{1}{255}$, is already as bright as the sRGB byte 13, so every darker color would become black or that one step, with visible bands in every dark gradient. Linear light needs about 12 bits or more per channel, in practice 16-bit half floats or 32-bit floats, so a pixel would take 8 or 16 bytes instead of 4. Every operation of the rasterizer would move two to four times as much memory.

So, like most 2D renderers and like the GPUs with an sRGB render target, we keep 8-bit sRGB pixels, and only convert to linear light where it matters, while blending. A float color target, with lighting and *tone mapping* at the end, is a better fit for a 3D renderer.

## What's Next

We can draw transparent shapes and sprites now. But everything we draw is still placed with pixel coordinates, and turning or scaling a sprite takes the math of `TurnedCorners` from part 7, by hand. In part 9, we'll describe positions, rotations and scales with 3 by 3 matrices, combine them into transforms, and add a 2D camera that can move around a world that's larger than the screen.

## References

<span id="cite-1"></span>\[1\] A. R. Smith, “Alpha and the History of Digital Compositing”, Microsoft Technical Memo 7, 1995. \[Online\]. Available: [http://alvyray.com/Memos/CG/Microsoft/7_alpha.pdf](http://alvyray.com/Memos/CG/Microsoft/7_alpha.pdf). \[Accessed: 28- Nov- 2026\].

<span id="cite-2"></span>\[2\] T. Porter and T. Duff, “Compositing Digital Images”, ACM SIGGRAPH Computer Graphics, vol. 18, no. 3, pp. 253–259, 1984. \[Online\]. Available: [https://doi.org/10.1145/964965.808606](https://doi.org/10.1145/964965.808606).

<span id="cite-3"></span>\[3\] Wikipedia, “Alpha compositing”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Alpha_compositing](https://en.wikipedia.org/wiki/Alpha_compositing). \[Accessed: 28- Nov- 2026\].

<span id="cite-4"></span>\[4\] J. F. Blinn, “Compositing, Part 1: Theory”, IEEE Computer Graphics and Applications, vol. 14, no. 5, pp. 83–87, 1994. \[Online\]. Available: [https://doi.org/10.1109/38.310740](https://doi.org/10.1109/38.310740).

<span id="cite-5"></span>\[5\] S. Hargreaves, “Premultiplied alpha”, shawnhargreaves.com, 2009. \[Online\]. Available: [https://shawnhargreaves.com/blog/premultiplied-alpha.html](https://shawnhargreaves.com/blog/premultiplied-alpha.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-6"></span>\[6\] Microsoft, “D3D12_RENDER_TARGET_BLEND_DESC structure”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_render_target_blend_desc](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_render_target_blend_desc). \[Accessed: 28- Nov- 2026\].

<span id="cite-7"></span>\[7\] Khronos Group, “VkPipelineColorBlendAttachmentState”, Vulkan Documentation, 2026. \[Online\]. Available: [https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineColorBlendAttachmentState.html](https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineColorBlendAttachmentState.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-8"></span>\[8\] IEC, “Multimedia systems and equipment – Colour measurement and management – Part 2-1: Colour management – Default RGB colour space – sRGB”, IEC 61966-2-1:1999, 1999.

<span id="cite-9"></span>\[9\] Wikipedia, “sRGB”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/SRGB](https://en.wikipedia.org/wiki/SRGB). \[Accessed: 28- Nov- 2026\].

<span id="cite-10"></span>\[10\] Wikipedia, “Gamma correction”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Gamma_correction](https://en.wikipedia.org/wiki/Gamma_correction). \[Accessed: 28- Nov- 2026\].

<span id="cite-11"></span>\[11\] Microsoft, “DXGI_FORMAT enumeration”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format](https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format). \[Accessed: 28- Nov- 2026\].

<span id="cite-12"></span>\[12\] Microsoft, “Converting data for the color space”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/converting-data-color-space](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/converting-data-color-space). \[Accessed: 28- Nov- 2026\].

<span id="cite-13"></span>\[13\] Khronos Group, “VkFormat”, Vulkan Documentation, 2026. \[Online\]. Available: [https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormat.html](https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormat.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-14"></span>\[14\] Wikipedia, “Endianness”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Endianness](https://en.wikipedia.org/wiki/Endianness). \[Accessed: 28- Nov- 2026\].

<span id="cite-15"></span>\[15\] Microsoft, “D3D12_BLEND enumeration”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_blend](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_blend). \[Accessed: 28- Nov- 2026\].

<span id="cite-16"></span>\[16\] Khronos Group, “VkBlendFactor”, Vulkan Documentation, 2026. \[Online\]. Available: [https://registry.khronos.org/vulkan/specs/latest/man/html/VkBlendFactor.html](https://registry.khronos.org/vulkan/specs/latest/man/html/VkBlendFactor.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-17"></span>\[17\] cppreference.com, “Default comparisons”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/default_comparisons](https://en.cppreference.com/w/cpp/language/default_comparisons). \[Accessed: 28- Nov- 2026\].

<span id="cite-18"></span>\[18\] cppreference.com, “Aggregate initialization, Designated initializers”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers](https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers). \[Accessed: 28- Nov- 2026\].

<span id="cite-19"></span>\[19\] J. F. Blinn, “Three Wrongs Make a Right”, IEEE Computer Graphics and Applications, vol. 15, no. 6, pp. 90–93, 1995. \[Online\]. Available: [https://doi.org/10.1109/38.469535](https://doi.org/10.1109/38.469535).

<span id="cite-20"></span>\[20\] cppreference.com, “static_assert declaration”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/static_assert](https://en.cppreference.com/w/cpp/language/static_assert). \[Accessed: 28- Nov- 2026\].

<span id="cite-21"></span>\[21\] cppreference.com, “std::pow”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/numeric/math/pow](https://en.cppreference.com/w/cpp/numeric/math/pow). \[Accessed: 28- Nov- 2026\].

<span id="cite-22"></span>\[22\] cppreference.com, “Lambda expressions”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/lambda](https://en.cppreference.com/w/cpp/language/lambda). \[Accessed: 28- Nov- 2026\].

<span id="cite-23"></span>\[23\] cppreference.com, “Class template”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/class_template](https://en.cppreference.com/w/cpp/language/class_template). \[Accessed: 28- Nov- 2026\].

<span id="cite-24"></span>\[24\] cppreference.com, “std::min”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/algorithm/min](https://en.cppreference.com/w/cpp/algorithm/min). \[Accessed: 28- Nov- 2026\].

<span id="cite-25"></span>\[25\] cppreference.com, “Class template argument deduction (CTAD)”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/class_template_argument_deduction](https://en.cppreference.com/w/cpp/language/class_template_argument_deduction). \[Accessed: 28- Nov- 2026\].

<span id="cite-26"></span>\[26\] SDL, “SDL_Colorspace”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_Colorspace](https://wiki.libsdl.org/SDL3/SDL_Colorspace). \[Accessed: 28- Nov- 2026\].
