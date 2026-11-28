---
title: "2D Software Rasterizer Part 2: Clipping"
description: "Coordinate spaces, half-open rectangles, a fast Clear, and a Rasterizer class that clips every shape to the surface and to a scissor rectangle, with the mouse as a spotlight."
slug: 2d-software-rasterizer-2-clipping
lastUpdated: 2026-11-28
---

In [part 1](/2d-software-rasterizer-1-framebuffer/), we computed a color for every pixel of the surface. Every `x` and `y` that we used was inside the surface, because the loops went from 0 to the width and the height. That changes as soon as we draw shapes. A rectangle that moves to the left edge of the screen will, sooner or later, have pixels with a negative `x`. If we write those pixels anyway, we write to memory that isn't ours.

In this part, we make sure that never happens. You will:

1. learn which *coordinate spaces* a 2D rasterizer works with, and what a pixel coordinate means exactly,
2. describe rectangles with *half-open* edges, and compute where two rectangles overlap,
3. clear the surface as fast as the CPU allows,
4. add a `Rasterizer` class that draws into a *color target*, and *clips* every rectangle to it once, instead of checking every pixel,
5. add a *scissor rectangle*, which limits drawing to a part of the surface,
6. and read the mouse and the function keys, so that the example can use the mouse as a spotlight.

At the end of this part, your program shows this:

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-2-clipping/index.html" title="Colored rectangles slide over the edges of a dark surface. Around the mouse, a rectangular spotlight shows them in full color." loading="lazy" allowfullscreen></iframe>

<div class="caption">

Move the mouse over the demo to move the spotlight. Click the demo first, so that it gets the keys, and press <kbd>F1</kbd> to show the outline of the scissor rectangle.

</div>

The code for this part is at the [`part-02-clipping`](https://github.com/3dgep/2d-software-rasterizer/tree/part-02-clipping) tag of the [2d-software-rasterizer](https://github.com/3dgep/2d-software-rasterizer) repository. You can also see [everything that changed since part 1](https://github.com/3dgep/2d-software-rasterizer/compare/part-01-framebuffer...part-02-clipping).

## Coordinate Spaces

A *coordinate space* is a way to give positions a number. A 2D rasterizer works with a few of them, and a surprising number of bugs come from mixing them up. In this part, we meet two.

The first one is the space of the surface, which we already used in part 1. Its origin is the top-left corner of the surface, `x` goes to the right and `y` goes down. That's the same order as the pixels are stored in memory, and it's what screens, image files and graphics APIs use for pixels. Both coordinates are whole numbers, and they name pixels. The surface's pixels go from 0 to the width minus 1, and from 0 to the height minus 1.

It's worth being precise about what `(x, y)` means. A pixel isn't a point but a small square, so pixel `(x, y)` covers the area

$$
[x, x + 1) \times [y, y + 1)
$$

from its top-left corner at $(x, y)$ up to, but not including, the next pixel. Its center is at $(x + 0.5, y + 0.5)$ [\[1\]](#cite-1). We don't need the centers yet, but in part 5 they decide which pixels a triangle covers [\[2\]](#cite-2), and drawing triangles without gaps or overlaps depends on it.

The second space is the space of the window. The mouse is reported in *window pixels*, but our example opens a window that is twice as large as the surface, adds black bars when the shape doesn't match, and can go fullscreen. So the mouse at window position (300, 200) might be over surface pixel (130, 85), or over a black bar, outside the surface. We have to convert one space to the other before we can use the mouse to draw [\[3\]](#cite-3). In part 9, we add a third space, the *world*, where a 2D camera decides which part of a larger scene ends up on the surface.

## Rectangles

Our first shape is the rectangle, and we also use rectangles to describe areas of the surface, like the part that may be drawn. `Rect.hpp` starts with a point:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Rect.hpp#L1-L15"
```

A `Point` is a position in the surface's coordinate space, so it uses `int`s, like the pixel coordinates. The defaulted `operator==` compares both coordinates [\[4\]](#cite-4).

The rectangle stores its four edges:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Rect.hpp#L17-L30"
```

The edges are *half-open* [\[5\]](#cite-5). The `left` column and the `top` row are part of the rectangle, but the `right` column and the `bottom` row are not. They are the first column and row just outside it. That's the same idea as the pixel squares above, and as `begin` and `end` of a C++ range, where `end` points just past the last element.

Half-open edges save a lot of `+ 1`s and `- 1`s. The width is simply `right - left`, a rectangle that starts at `x` with width `w` ends at `x + w` (which is what `FromSize` does), and two rectangles that touch, like `{ 0, 0, 10, 10 }` and `{ 10, 0, 20, 10 }`, share an edge value without sharing a single pixel. When we later fill rectangles with transparent colors, a pixel that's drawn twice would be blended twice, so that matters.

The rest of the struct answers questions about the rectangle:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Rect.hpp#L32-L44"
```

`Width` and `Height` follow from the half-open edges. A rectangle is empty when its right edge is not to the right of its left edge, or its bottom edge is not below its top edge. A rectangle with a zero or negative size has no pixels. `Contains` checks whether a point is inside, with `>=` for the edges that are inside and `<` for the ones that are outside.

Clipping comes down to one question, which pixels are inside two rectangles at the same time. `Intersect` answers it:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Rect.hpp#L46-L56"
```

A pixel is inside both rectangles when it's to the right of both left edges, so the left edge of the overlap is the larger of the two [\[6\]](#cite-6). The same goes for the top edges. And it must be to the left of both right edges, so the right edge of the overlap is the smaller of the two [\[7\]](#cite-7), and the same for the bottom. When the rectangles don't overlap, this gives a rectangle whose right edge is left of its left edge (or whose bottom is above its top), which `IsEmpty` recognizes. So the caller only has to check `IsEmpty`, and never needs a special case.

Try it below. Drag the rectangles `a` and `b` to move them, or drag their bottom-right corners to change their size. The green pixels are the intersection, and the dashed lines are its four edges. Pull the rectangles apart, and the `left` and `right` edges (or the `top` and `bottom` edges) cross each other, which gives the empty intersection a negative width (or height).

<iframe class="embed embed-demo embed-rect-intersect" src="/demos/rect-intersect.html" title="A grid of pixels with two rectangles, a and b, that can be moved and resized. The pixels inside both are the intersection, and the code below the grid shows Intersect(a, b) with the max and min of every edge." loading="lazy" allowfullscreen></iframe>

All of this is `constexpr`, so the file ends with a few tests that run while it compiles [\[8\]](#cite-8) [\[9\]](#cite-9):

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Rect.hpp#L58-L62"
```

The last test checks the two touching rectangles from above. Their intersection is empty, because they don't share any pixels.

## Clearing the Surface

Almost every frame starts by clearing the surface to one color. So it's worth making that fast. `Surface.hpp` includes the new header:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Surface.hpp#L3-L4" ins={2}
```

`Rect.hpp` only depends on the standard library, so every header of the library can include it without pulling in anything else.

The surface uses `Rect` for a new function, and gets a function to clear it:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Surface.hpp#L25-L32" ins={4-8}
```

`Bounds` returns the rectangle that covers every pixel of the surface. With half-open edges, that's simply from `(0, 0)` to the width and the height. We use it to clip shapes to the surface.

`Clear` is in `Surface.cpp`, which now also includes `<algorithm>`:

```cpp source="2d-software-rasterizer-part-02/rast/src/Surface.cpp#L18-L21"
```

You could clear the surface with two loops over `x` and `y`, like the gradient in part 1. But the position of a pixel doesn't matter when every pixel gets the same color. `std::ranges::fill` writes the color to every element of the vector [\[10\]](#cite-10), from the front to the back, and there's nothing else in the loop. That's a loop that compilers recognize. In a release build, they turn it into instructions that store 16 or 32 bytes (4 or 8 pixels) at a time [\[11\]](#cite-11).

You may know `memset` [\[12\]](#cite-12), which fills memory with one byte. It's fast, but it can only clear the surface to a color whose four bytes are the same. That works for `0x00000000` (transparent black) and `0xFFFFFFFF` (opaque white), but not for opaque black, `0xFF000000`. `std::ranges::fill` works for every color, and it's just as fast.

## Why Clip?

Before we draw rectangles, let's see what goes wrong when a rectangle is partly outside the surface. Say the surface is 640 pixels wide, and we write to `pixels[-5, 10]`. `std::mdspan` doesn't check its indices (just like `operator[]` of `std::vector` doesn't) [\[13\]](#cite-13) [\[14\]](#cite-14), so it computes the index $10 \cdot 640 - 5$, which is the fifth pixel from the *end* of row 9. So the left part of the rectangle shows up at the right edge of the surface, one row higher. At the top or the bottom, it's worse. `pixels[100, -1]` is before the first pixel of the vector, and `pixels[100, 360]` is after the last one. Writing there is *undefined behavior* [\[15\]](#cite-15). It may crash, but it may also quietly overwrite other variables of your program.

So every pixel that we write must be inside the surface. Keeping it there is called *clipping* [\[16\]](#cite-16). There are two ways to do it. We can check every pixel just before we write it, or we can make the shape smaller before we draw it, so that it fits on the surface. We'll see both, in a class that does all our drawing from now on.

## The Rasterizer

Until now, the example wrote the pixels of the surface itself. From now on, a `Rasterizer` does the drawing. It's the class that grows with the series. In the coming parts it learns to draw lines, circles, triangles and textures. Drawing needs some state that we don't want to pass to every function, like the surface to draw into and the scissor rectangle that we add below. That state lives in the `Rasterizer`.

### Rasterizer.hpp

The class starts with its *color target*:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Rasterizer.hpp#L1-L22"
```

The color target is the surface that holds the colors of the pixels that we draw. On a GPU, you would call it a *render target*. We call it the *color* target, because later in the series a rasterizer can have more targets. Part 11 draws into a larger surface and scales it down for smoother edges, and then the color target is that larger surface.

The `Rasterizer` doesn't own the surface. It only stores a pointer to it, and the surface must stay alive as long as the `Rasterizer` draws into it. The constructor takes a reference, because a rasterizer always needs a color target. A reference can't be null, so the caller can't forget it. But the member is a pointer, because `SetColorTarget` must be able to point it to another surface, and a reference member can never change what it refers to.

Next are the scissor rectangle and the clip rectangle:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Rasterizer.hpp#L24-L30"
```

We look at the scissor rectangle in its own section below. `ClipRect` returns the rectangle that drawing is limited to. That's the part of the color target that is inside the scissor rectangle.

Then come the functions that draw, and the members:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Rasterizer.hpp#L32-L48"
```

`Clear` sets every pixel that may be drawn to one color, `SetPixel` sets one pixel, and `FillRect` and `DrawRect` draw a filled rectangle and the outline of one. The scissor rectangle is a `std::optional<Rect>` [\[17\]](#cite-17), because there doesn't have to be one.

### Rasterizer.cpp

The constructor and `SetColorTarget` only store the pointer:

```cpp source="2d-software-rasterizer-part-02/rast/src/Rasterizer.cpp#L1-L15"
```

`&colorTarget` takes the address of the surface that the reference refers to.

The clip rectangle comes from the scissor rectangle and the color target:

```cpp source="2d-software-rasterizer-part-02/rast/src/Rasterizer.cpp#L17-L31"
```

`SetScissor` stores the scissor rectangle, and `ResetScissor` removes it again. `ClipRect` intersects the scissor rectangle with the bounds of the color target, or returns the bounds when there is no scissor rectangle. So the clip rectangle is always inside the color target, even when the scissor rectangle is partly outside it. That's why the rest of the class only has to clip to one rectangle.

`ClipRect` computes the rectangle every time it's called, instead of storing it. That costs a few comparisons, but it can never be out of date, for example when the color target changes to a surface with a different size.

`Clear` uses the clip rectangle too:

```cpp source="2d-software-rasterizer-part-02/rast/src/Rasterizer.cpp#L33-L44"
```

When the clip rectangle covers the whole color target, `Surface::Clear` clears it with one `std::ranges::fill`. Otherwise, only the clip rectangle is cleared, with `FillRect`. This is also how a GPU clears a render target with a scissor rectangle in OpenGL [\[18\]](#cite-18).

`SetPixel` clips the easy way, by checking the pixel:

```cpp source="2d-software-rasterizer-part-02/rast/src/Rasterizer.cpp#L46-L52"
```

When the point isn't inside the clip rectangle, nothing happens. For a single pixel, that's the right way to clip.

### Filling Rectangles

For a rectangle, the first version that comes to mind uses `SetPixel` for every pixel:

```cpp
void Rasterizer::FillRect(const Rect& rect, Color color)
{
    for (int y = rect.top; y < rect.bottom; ++y)
    {
        for (int x = rect.left; x < rect.right; ++x)
        {
            SetPixel({ x, y }, color);
        }
    }
}
```

That's correct, because `SetPixel` never writes outside the clip rectangle. But it computes the clip rectangle and checks a pixel against it for every single pixel, even though the answer is the same for whole rows. Worse, it visits pixels that are never drawn. A rectangle that is 10,000 pixels to the left of the surface still loops over all of its pixels, only to skip every one of them.

The other way is to clip the rectangle before we draw it:

```cpp source="2d-software-rasterizer-part-02/rast/src/Rasterizer.cpp#L54-L69"
```

The intersection of the rectangle and the clip rectangle is exactly the set of pixels that we may draw. Every pixel inside it is inside the surface, so the loops don't need to check anything. When the intersection is empty, the function returns before it touches a single pixel, however large the rectangle is.

The pixels of one row of the rectangle are next to each other in memory, so we fill each row with `std::fill_n` [\[19\]](#cite-19), which writes `Width()` copies of the color starting at the first pixel of the row. That's the same loop as in `Surface::Clear`, and compilers turn it into the same fast instructions.

This is the most important idea of this part. Clip a shape once, before you draw it, so that the loops that visit the pixels don't have to check anything. The loops are where the time goes, because they run once for every pixel. Every shape in this series is drawn this way.

The outline of a rectangle is made of four thin rectangles:

```cpp source="2d-software-rasterizer-part-02/rast/src/Rasterizer.cpp#L71-L85"
```

The top and bottom rows are as wide as the rectangle. The left and right columns go from the row below the top row to the row above the bottom row, so the corners aren't drawn twice. (For a rectangle that's one or two pixels high, those columns are empty, which `FillRect` handles.) Each of the four calls clips itself, so an outline that's partly outside the surface needs no extra code. The outline is inside the rectangle, so `DrawRect` and `FillRect` cover the same pixels.

## The Scissor Rectangle

The clip rectangle limits drawing to the color target, but sometimes you want to limit it to a smaller area. A user interface may have a panel with a list that scrolls, and the items that scroll out of the panel must be cut off at its edge. A split-screen game draws each player's view into its own half of the screen. GPUs have a *scissor rectangle* for this [\[20\]](#cite-20) [\[21\]](#cite-21), and so does our rasterizer. It's set with `SetScissor`, and from then on, `ClipRect` is the part of the color target that's inside the scissor rectangle.

Because every function clips to `ClipRect`, the scissor rectangle works for everything at once. `FillRect`, `DrawRect`, `SetPixel` and `Clear` don't have any code for the scissor rectangle, and the lines, circles and triangles of the coming parts won't either. They clip to `ClipRect`, and that's all.

## The Mouse and the Function Keys

The example uses the mouse as a spotlight, so the program needs to know where the mouse is. It's also the first program with a *debug overlay*, something that we draw on top of the scene to see what the rasterizer is doing. Debug overlays are switched on and off with the function keys <kbd>F1</kbd> to <kbd>F4</kbd>. `Window.hpp` names those keys:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Window.hpp#L21-L29"
```

The values of the `Key` enumerators are SDL's *key codes* for the same keys [\[22\]](#cite-22) [\[23\]](#cite-23). That way, the `Window` can turn an SDL key code into a `Key` with a `static_cast`, and the program never needs to include SDL. For now, `Key` only has the four keys that the debug overlays use. We add more keys when an example needs them.

`Window.hpp` also includes `Rect.hpp` (for `Point`) and `<vector>`. The window gets two new functions:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Window.hpp#L58-L66" ins={4-9}
```

`WasKeyPressed` tells whether a key went down since the previous frame. That's what you need for a key that toggles something. A key that's held down for a second would otherwise toggle the overlay 60 times. `MousePosition` returns the position of the mouse in the coordinate space of the surface.

The window remembers the keys that were pressed during the last frame:

```cpp source="2d-software-rasterizer-part-02/rast/include/rast/Window.hpp#L89-L93" ins={4-5}
```

A `std::vector` is plenty for this. A frame usually has no key presses at all, and rarely more than one or two.

### Window.cpp

`Window.cpp` checks that the values of `Key` are correct:

```cpp source="2d-software-rasterizer-part-02/rast/src/Window.cpp#L9-L20" ins={1-2,8-12}
```

If a value is wrong, the program doesn't compile. `<algorithm>` and `<cmath>` are for the two new functions below.

`PollEvents` collects the keys:

```cpp source="2d-software-rasterizer-part-02/rast/src/Window.cpp#L76-L95" ins={3,19-20}
```

At the start of every frame, the list of pressed keys is cleared. Then every key press is added to it, except the repeats of a key that's held down, which are skipped by the check above it. Keys that the window uses itself, like <kbd>V</kbd>, are added too, but no example will ask for them.

`WasKeyPressed` looks for the key in the list:

```cpp source="2d-software-rasterizer-part-02/rast/src/Window.cpp#L210-L213"
```

`std::ranges::contains` is new in C++23 [\[24\]](#cite-24). It returns `true` if the range has an element that's equal to the value.

`MousePosition` converts the mouse from window coordinates to surface coordinates:

```cpp source="2d-software-rasterizer-part-02/rast/src/Window.cpp#L215-L230"
```

`SDL_GetMouseState` returns the position of the mouse in window coordinates [\[25\]](#cite-25), as `float`s. In part 1, we used `SDL_SetRenderLogicalPresentation` to scale the surface to the window and add the black bars. `SDL_RenderCoordinatesFromWindow` undoes exactly that conversion [\[26\]](#cite-26), so it gives us the position in surface pixels, even when the window has been resized or is fullscreen.

The result is a position like (130.75, 85.5), inside pixel (130, 85). To find that pixel, we round down with `std::floor` [\[27\]](#cite-27). A `static_cast<int>` would round *towards zero*, which is the same for positive numbers, but not for negative ones. The mouse at $x = -0.5$, over the black bar just left of the surface, is in the pixel column $-1$, but a `static_cast` would say column 0. The position can be outside the surface, and that's fine. Everything that we draw with it is clipped.

## The Example

The example draws a few rectangles that move back and forth, partly off the surface. It starts with the includes:

```cpp source="2d-software-rasterizer-part-02/examples/part-02-clipping/main.cpp#L1-L15"
```

The example includes `Rasterizer.hpp` now, because it doesn't write the pixels of the surface itself anymore.

Each rectangle follows a path that's described by a few numbers:

```cpp source="2d-software-rasterizer-part-02/examples/part-02-clipping/main.cpp#L17-L37"
```

A rectangle moves around the center of its path, `rangeX` pixels to the left and right and `rangeY` pixels up and down. Most of the ranges are large enough to move the rectangles partly or completely off the surface. The red rectangle, for example, is centered at $x = 320$ and moves 330 pixels to each side, so at the far left, its center is at $x = -10$. Their colors are named constants of `Color`, from [part 1](/2d-software-rasterizer-1-framebuffer/#named-colors). `std::array` deduces the number of rectangles from the list [\[28\]](#cite-28) [\[29\]](#cite-29).

`RectAt` computes where a rectangle is at a moment in time:

```cpp source="2d-software-rasterizer-part-02/examples/part-02-clipping/main.cpp#L39-L50"
```

The sine and the cosine both go from -1 to 1, so `x` and `y` stay within the range around the center. Because the horizontal and the vertical speeds are different, the rectangles don't move in a straight line, but in a curve that keeps changing.

Then the position is rounded to a whole pixel. `std::lround` rounds to the nearest whole number [\[30\]](#cite-30), also for negative numbers, while a `static_cast<int>` rounds towards zero. With a `static_cast`, every position from $x = -0.99$ to $x = 0.99$ would end up at pixel 0, a range twice as wide as for any other pixel. The rectangle would stop for a moment every time it crosses 0, which only happens near the left and top edges of the surface, exactly where we're testing the clipping.

The example draws the scene twice, once in darker colors:

```cpp source="2d-software-rasterizer-part-02/examples/part-02-clipping/main.cpp#L52-L57"
```

`Dim` divides each color channel by 3. `color.R() / 3` is an `int` (the `std::uint8_t` is promoted before the division), so we have to convert it back to a `std::uint8_t`.

`DrawScene` draws every rectangle with a white outline:

```cpp source="2d-software-rasterizer-part-02/examples/part-02-clipping/main.cpp#L59-L69"
```

Each rectangle is filled first, and then its outline is drawn on top of it. When `dimmed` is `true`, both colors are dimmed. Nothing in this function knows about clipping or scissor rectangles. That's all up to the rasterizer.

`main` creates the window and the surface, as in part 1, and now also the rasterizer:

```cpp source="2d-software-rasterizer-part-02/examples/part-02-clipping/main.cpp#L71-L87"
```

The rasterizer draws into `surface`. `showScissor` remembers whether the debug overlay is on.

The first half of the main loop draws the whole scene, dimmed:

```cpp source="2d-software-rasterizer-part-02/examples/part-02-clipping/main.cpp#L89-L101"
```

<kbd>F1</kbd> switches the overlay on and off. Then the scissor rectangle is removed, so that the clip rectangle is the whole surface. `Clear` clears the whole surface to a dimmed `DarkSlateGray` with `Surface::Clear`, and `DrawScene` draws the dimmed rectangles over it.

The second half draws the spotlight:

```cpp source="2d-software-rasterizer-part-02/examples/part-02-clipping/main.cpp#L103-L121"
```

The spotlight is a 200 by 150 pixel rectangle, centered on the mouse. It becomes the scissor rectangle, so from now on, only the pixels inside the spotlight change. `Clear` fills only the spotlight with `DarkSlateGray`, the same background without the dimming, because the clip rectangle is smaller than the surface now. Then `DrawScene` draws the same scene again, in full color. Every rectangle is clipped to the spotlight, so you only see the bright rectangles inside it.

When the overlay is on, the scissor rectangle is removed again, and `DrawRect` draws the outline of the spotlight in yellow. It has to be drawn without the scissor rectangle. The outline is inside the spotlight, so it would be drawn anyway, but an overlay should never depend on the state that it's showing.

Move the mouse to the edge of the surface, and the spotlight is clipped to the surface too. Move it over the black bars, and the spotlight disappears, because the intersection with the surface is empty.

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-2-clipping/index.html" title="Colored rectangles slide over the edges of a dark surface. Around the mouse, a rectangular spotlight shows them in full color." loading="lazy" allowfullscreen></iframe>

<div class="caption">

The example of part 2. Press <kbd>F1</kbd> to show the scissor rectangle (click the demo first, so that it gets the keys).

</div>

## What's Next

Every pixel that we draw now goes through the clip rectangle, so we can't write outside the surface anymore. In [part 3](/2d-software-rasterizer-3-lines/), we draw our first shape that isn't a rectangle, the line. We compare two classic algorithms, the *DDA* with floating-point numbers and *Bresenham's algorithm* with only integers, and make them work in all eight directions that a line can go.

## References

<span id="cite-1"></span>\[1\] Microsoft, “Coordinate Systems (Direct3D 10)”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-coordinates](https://learn.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-coordinates). \[Accessed: 28- Nov- 2026\].

<span id="cite-2"></span>\[2\] Microsoft, “Rasterization Rules”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules). \[Accessed: 28- Nov- 2026\].

<span id="cite-3"></span>\[3\] SDL, “SDL_SetRenderLogicalPresentation”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_SetRenderLogicalPresentation](https://wiki.libsdl.org/SDL3/SDL_SetRenderLogicalPresentation). \[Accessed: 28- Nov- 2026\].

<span id="cite-4"></span>\[4\] cppreference.com, “Default comparisons”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/default_comparisons](https://en.cppreference.com/w/cpp/language/default_comparisons). \[Accessed: 28- Nov- 2026\].

<span id="cite-5"></span>\[5\] E. W. Dijkstra, “Why numbering should start at zero (EWD831)”, 1982. \[Online\]. Available: [https://www.cs.utexas.edu/~EWD/transcriptions/EWD08xx/EWD831.html](https://www.cs.utexas.edu/~EWD/transcriptions/EWD08xx/EWD831.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-6"></span>\[6\] cppreference.com, “std::max”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/algorithm/max](https://en.cppreference.com/w/cpp/algorithm/max). \[Accessed: 28- Nov- 2026\].

<span id="cite-7"></span>\[7\] cppreference.com, “std::min”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/algorithm/min](https://en.cppreference.com/w/cpp/algorithm/min). \[Accessed: 28- Nov- 2026\].

<span id="cite-8"></span>\[8\] cppreference.com, “constexpr specifier”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/constexpr](https://en.cppreference.com/w/cpp/language/constexpr). \[Accessed: 28- Nov- 2026\].

<span id="cite-9"></span>\[9\] cppreference.com, “static_assert declaration”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/static_assert](https://en.cppreference.com/w/cpp/language/static_assert). \[Accessed: 28- Nov- 2026\].

<span id="cite-10"></span>\[10\] cppreference.com, “std::ranges::fill”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/algorithm/ranges/fill](https://en.cppreference.com/w/cpp/algorithm/ranges/fill). \[Accessed: 28- Nov- 2026\].

<span id="cite-11"></span>\[11\] Microsoft, “Auto-Parallelization and Auto-Vectorization”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/cpp/parallel/auto-parallelization-and-auto-vectorization](https://learn.microsoft.com/en-us/cpp/parallel/auto-parallelization-and-auto-vectorization). \[Accessed: 28- Nov- 2026\].

<span id="cite-12"></span>\[12\] cppreference.com, “std::memset”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/string/byte/memset](https://en.cppreference.com/w/cpp/string/byte/memset). \[Accessed: 28- Nov- 2026\].

<span id="cite-13"></span>\[13\] cppreference.com, “std::mdspan”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/mdspan](https://en.cppreference.com/w/cpp/container/mdspan). \[Accessed: 28- Nov- 2026\].

<span id="cite-14"></span>\[14\] cppreference.com, “std::vector”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/vector](https://en.cppreference.com/w/cpp/container/vector). \[Accessed: 28- Nov- 2026\].

<span id="cite-15"></span>\[15\] cppreference.com, “Undefined behavior”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/ub](https://en.cppreference.com/w/cpp/language/ub). \[Accessed: 28- Nov- 2026\].

<span id="cite-16"></span>\[16\] J. D. Foley, A. van Dam, S. K. Feiner and J. F. Hughes, Computer Graphics: Principles and Practice, 2nd ed. in C. Reading, MA: Addison-Wesley, 1996.

<span id="cite-17"></span>\[17\] cppreference.com, “std::optional”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/utility/optional](https://en.cppreference.com/w/cpp/utility/optional). \[Accessed: 28- Nov- 2026\].

<span id="cite-18"></span>\[18\] Khronos Group, “glClear”, OpenGL 4 Reference Pages, 2026. \[Online\]. Available: [https://registry.khronos.org/OpenGL-Refpages/gl4/html/glClear.xhtml](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glClear.xhtml). \[Accessed: 28- Nov- 2026\].

<span id="cite-19"></span>\[19\] cppreference.com, “std::fill_n”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/algorithm/fill_n](https://en.cppreference.com/w/cpp/algorithm/fill_n). \[Accessed: 28- Nov- 2026\].

<span id="cite-20"></span>\[20\] Khronos Group, “glScissor”, OpenGL 4 Reference Pages, 2026. \[Online\]. Available: [https://registry.khronos.org/OpenGL-Refpages/gl4/html/glScissor.xhtml](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glScissor.xhtml). \[Accessed: 28- Nov- 2026\].

<span id="cite-21"></span>\[21\] Microsoft, “ID3D12GraphicsCommandList::RSSetScissorRects method”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-rssetscissorrects](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-rssetscissorrects). \[Accessed: 28- Nov- 2026\].

<span id="cite-22"></span>\[22\] cppreference.com, “Enumeration declaration”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/enum](https://en.cppreference.com/w/cpp/language/enum). \[Accessed: 28- Nov- 2026\].

<span id="cite-23"></span>\[23\] SDL, “SDL_Keycode”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_Keycode](https://wiki.libsdl.org/SDL3/SDL_Keycode). \[Accessed: 28- Nov- 2026\].

<span id="cite-24"></span>\[24\] cppreference.com, “std::ranges::contains”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/algorithm/ranges/contains](https://en.cppreference.com/w/cpp/algorithm/ranges/contains). \[Accessed: 28- Nov- 2026\].

<span id="cite-25"></span>\[25\] SDL, “SDL_GetMouseState”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_GetMouseState](https://wiki.libsdl.org/SDL3/SDL_GetMouseState). \[Accessed: 28- Nov- 2026\].

<span id="cite-26"></span>\[26\] SDL, “SDL_RenderCoordinatesFromWindow”, SDL3 Wiki, 2026. \[Online\]. Available: [https://wiki.libsdl.org/SDL3/SDL_RenderCoordinatesFromWindow](https://wiki.libsdl.org/SDL3/SDL_RenderCoordinatesFromWindow). \[Accessed: 28- Nov- 2026\].

<span id="cite-27"></span>\[27\] cppreference.com, “std::floor”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/numeric/math/floor](https://en.cppreference.com/w/cpp/numeric/math/floor). \[Accessed: 28- Nov- 2026\].

<span id="cite-28"></span>\[28\] cppreference.com, “std::array”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/array](https://en.cppreference.com/w/cpp/container/array). \[Accessed: 28- Nov- 2026\].

<span id="cite-29"></span>\[29\] cppreference.com, “Class template argument deduction (CTAD)”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/class_template_argument_deduction](https://en.cppreference.com/w/cpp/language/class_template_argument_deduction). \[Accessed: 28- Nov- 2026\].

<span id="cite-30"></span>\[30\] cppreference.com, “std::round, std::lround, std::llround”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/numeric/math/round](https://en.cppreference.com/w/cpp/numeric/math/round). \[Accessed: 28- Nov- 2026\].
