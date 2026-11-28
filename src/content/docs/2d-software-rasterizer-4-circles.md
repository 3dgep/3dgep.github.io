---
title: "2D Software Rasterizer Part 4: Circles"
description: "Draw circles and discs with the midpoint circle algorithm, with integers only, one eighth of the circle mirrored eight ways, one span per row, and clipping with a bounding box."
slug: 2d-software-rasterizer-4-circles
lastUpdated: 2026-11-28
---

In [part 3](/2d-software-rasterizer-3-lines/), we drew lines with Bresenham's algorithm, which uses an integer error term to decide which pixel comes next. In this part, we use the same idea for circles. A circle is a curve, so the decision is a little different, but it still needs nothing but integer additions. And a circle is very symmetric, so we only have to compute an eighth of it.

In this part, you will:

1. decide which pixels belong to a circle, and where its center is,
2. derive the *midpoint circle algorithm*, which walks along the edge of a circle with integers only,
3. mirror one eighth of the circle to draw the whole outline,
4. fill a circle with one horizontal span per row,
5. and clip circles with their *bounding box*.

At the end of this part, your program shows this:

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-4-circles/index.html" title="Gold discs and rings with a radius of 1 to 8 pixels, blue rings that grow from one point, and a red disc with a white outline that follows the mouse, on a dark blue surface of 320 by 180 pixels that is scaled up four times" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Move the mouse over the demo to move the red disc. Click the demo first, so that it gets the keys, and press <kbd>F1</kbd> to see the bounding box of every circle.

</div>

The code for this part is at the [`part-04-circles`](https://github.com/3dgep/2d-software-rasterizer/tree/part-04-circles) tag of the [2d-software-rasterizer](https://github.com/3dgep/2d-software-rasterizer) repository. You can also see [everything that changed since part 3](https://github.com/3dgep/2d-software-rasterizer/compare/part-03-lines...part-04-circles).

## Which Pixels Make a Circle?

A circle is the set of points at a distance $r$ (the *radius*) from its center. As with lines, no pixel lies exactly on that curve, so we have to decide which pixels make up a circle. For a filled circle, a *disc*, the rule is the same as it will be for triangles in part 5. A pixel belongs to the disc when its center is inside the circle [\[1\]](#cite-1).

That leaves the question where the circle's center is. Our pixel coordinates are whole numbers, so the center could be the center of a pixel, or the corner between four pixels. We put it on the corner. Then the circle is symmetric around that corner, and a circle with radius $r$ is exactly $2r$ pixels wide and $2r$ pixels high. It fits exactly in the half-open rectangle `Rect::FromSize(cx - r, cy - r, 2 * r, 2 * r)`, like everything else that we have drawn. (With the center on a pixel center, a circle would always be an odd number of pixels wide, $2r + 1$, and it would never quite fit a `Rect`.)

The pixel that is $a$ columns to the right of the center and $b$ rows below it covers the square from $(a, b)$ to $(a + 1, b + 1)$, so its center is at $(a + \frac{1}{2}, b + \frac{1}{2})$, measured from the center of the circle. It's inside the circle when its distance to the center is less than $r$. Multiplying both sides by 4 removes the halves:

$$
\left(a + \tfrac{1}{2}\right)^2 + \left(b + \tfrac{1}{2}\right)^2 < r^2
\iff
(2a + 1)^2 + (2b + 1)^2 < 4r^2
$$

The left side of the second inequality is always the sum of two odd squares, so it's an even number that isn't a multiple of 4, while $4r^2$ is a multiple of 4. They can never be equal, so a pixel center can never be exactly on the circle, and we never have to break a tie. That's a nice bonus of putting the center on a corner.

## The Midpoint Algorithm

We could draw a disc by testing every pixel of its bounding box with the inequality above. That works, but it does two multiplications per pixel, and it doesn't help with the outline. The *midpoint circle algorithm* walks along the edge of the circle instead [\[2\]](#cite-2) [\[3\]](#cite-3), the same way Bresenham's algorithm walks along a line [\[4\]](#cite-4).

Let's look at the part of the circle below the center and to the right, from the bottom of the circle to the diagonal where $a = b$. That's one eighth of the circle. Along that eighth, the circle is never steeper than the diagonal, so `a` is its major axis. It has one pixel in every column, and from one column to the next, the edge moves up by at most one pixel. So for every column `a`, we look for the outermost pixel that's still inside the circle, and that pixel is either in the same row `b` as in the previous column, or one row higher.

To decide, we keep track of the *decision variable*

$$
d = (2a + 1)^2 + (2b + 1)^2 - 4r^2
$$

which is negative when the pixel $(a, b)$ is inside the circle. We start in the bottom pixel of the first column, at $a = 0$ and $b = r - 1$, where $d = 1 + (2r - 1)^2 - 4r^2 = 2 - 4r$. Then, for every step:

1. We draw the pixel $(a, b)$.
2. We move one column to the right. $a$ goes up by one, and $(2a + 3)^2 - (2a + 1)^2 = 8(a + 1)$, so $d$ goes up by $8(a + 1)$.
3. If $d$ is not negative anymore, the pixel in the new column is outside the circle, so we take the one above it. $b$ goes down by one, and $(2b - 1)^2 - (2b + 1)^2 = -8b$, so $d$ goes down by $8b$.

We stop when $a$ passes $b$, at the diagonal. Every step costs one or two additions, and there's no multiplication by a variable left (the multiplications by 8 are shifts).

The algorithm has its name from the classic version, which puts the center of the circle on a pixel center [\[5\]](#cite-5) [\[6\]](#cite-6). Then the edge of the circle runs between two candidate pixels, and the algorithm tests the *midpoint* between them, which lies halfway between two whole numbers. In our version, the pixel centers themselves lie halfway between whole numbers, so we test them directly. The arithmetic is the same.

Try it below. Drag the blue dot to move the center, and the orange dot to change the radius. In *outline* mode, the eighth that the algorithm walks is drawn in the strongest color, and hovering over one of its pixels shows the step and the decision variable. The *fill* mode is for later in this article.

<iframe class="embed embed-demo embed-midpoint-circle" src="/demos/midpoint-circle.html" title="A grid of pixels with a circle whose center is on the corner between four pixels. The eighth of the circle that the midpoint algorithm walks is drawn in a strong color, and its seven mirror images in other colors. Hovering over a pixel shows its step and the decision variable." loading="lazy" allowfullscreen></iframe>

### Eight Mirror Images

A circle looks the same when you mirror it left to right, top to bottom, or across a diagonal. So once we know one eighth, we know the whole circle. The pixel $(a, b)$ has seven mirror images, which are drawn in the other colors in the demo.

Mirroring a pixel across the center takes a little care. Pixel $a$ to the right of the center has its center at $a + \frac{1}{2}$. Its mirror image has its center at $-(a + \frac{1}{2})$, which is the center of pixel $-1 - a$. So the pixel right next to the center on the right ($a = 0$) mirrors to the pixel right next to it on the left ($-1$), not to itself. That's the price of putting the center on a corner, but it's only a `- 1`.

Mirroring across the diagonal swaps $a$ and $b$. On the diagonal itself, where $a = b$, the pixel is its own mirror image, so we must not draw it twice. That doesn't matter for opaque pixels, but once we blend in part 8, a pixel that's drawn twice would be darker than its neighbors.

## DrawCircle

`Rasterizer.hpp` gets two new functions:

```cpp source="2d-software-rasterizer-part-04/rast/include/rast/Rasterizer.hpp#L43-L55" ins={5-13}
```

The comment before them says where the center of a circle is, because that decides how large every circle is. `DrawCircle` draws the outline and `FillCircle` draws the disc. The outline is the outermost ring of pixels of the disc, so it's inside the disc, like the outline of `DrawRect` is inside `FillRect`.

In `Rasterizer.cpp`, the circle functions start with a small helper in an anonymous namespace [\[7\]](#cite-7):

```cpp source="2d-software-rasterizer-part-04/rast/src/Rasterizer.cpp#L199-L206"
```

The bounding box of a circle is the smallest rectangle that contains all of its pixels [\[8\]](#cite-8). Because the center is on a corner, that's exactly $2r$ pixels wide and high, from $r$ pixels to the left of the center to $r$ pixels to the right of it.

### Walking the Outline

The midpoint algorithm itself is a function template [\[9\]](#cite-9):

```cpp source="2d-software-rasterizer-part-04/rast/src/Rasterizer.cpp#L208-L224"
```

`WalkCircleOutline` doesn't draw anything itself. It calls `plot` for every pixel of the outline, and the caller decides what that means. `Plot` is a template parameter, so `plot` can be any function or lambda that takes an `x` and a `y`. Because the compiler sees the lambda's code when it instantiates the template, it can inline it, so there's no function call per pixel.

`plotMirrored` is a lambda too [\[10\]](#cite-10). It plots a pixel $(a, b)$ and its mirror images left of the center (`center.x - 1 - a`), above it (`center.y - 1 - b`), and both. It captures `plot` and `center` by reference with `[&]`.

The walk follows the steps that we derived:

```cpp source="2d-software-rasterizer-part-04/rast/src/Rasterizer.cpp#L226-L253"
```

The walk starts at the bottom of the circle, in the column right of the center, with $d = 2 - 4r$. For every step, it plots the pixel and its three mirror images, and the four mirror images across the diagonal, unless the pixel is on the diagonal. Then it moves one column to the right, and one row up if the new pixel is outside the circle. The loop ends when it passes the diagonal.

`d` is an `std::int64_t` [\[11\]](#cite-11), because $4r^2$ doesn't fit in an `int` for a radius above 23,170. That's a large circle, but a circle that's mostly off the screen can easily be that large.

### Clipping the Outline

`DrawCircle` decides how to clip with the bounding box:

```cpp source="2d-software-rasterizer-part-04/rast/src/Rasterizer.cpp#L255-L285"
```

A radius of 0 or less gives an empty circle, so it draws nothing, like an empty `Rect`. Then the bounding box is compared with the clip rectangle, which gives three cases:

1. The bounding box is completely outside the clip rectangle. Then no pixel of the circle can be visible, and we're done.
2. The bounding box is completely inside the clip rectangle. Then every pixel of the circle is inside it too, and the lambda writes the pixels without any checks.
3. The bounding box is partly inside. Then some pixels are visible and some aren't, and the lambda checks every pixel.

The third case breaks our rule from part 2, which says to clip once and never check every pixel. But clipping a circle exactly is a lot of work, and the outline only has about $5.66 r$ pixels (eight eighths of about $r / \sqrt{2}$ pixels each), so checking them is cheap. The two lambdas are passed to two separate instantiations of `WalkCircleOutline`, so the decision is made once per circle, not once per pixel.

This test with the bounding box is an idea that comes back in the last part of the series. There, we split the screen into small blocks, and decide for every block whether a triangle covers it completely, partly, or not at all.

## FillCircle

You might expect `FillCircle` to use the same walk as `DrawCircle`, and fill a span between every pixel and its mirror image. That gives the right pixels, but it fills some rows more than once. Near the top and the bottom of the circle, several steps of the walk are in the same row, so that row would be filled for each of them. Again, that only becomes visible once we blend, but we would rather do it right now.

So `FillCircle` walks the rows instead, from the middle row down to the bottom row. In every row, it finds the outermost pixel inside the circle, and fills one span from its mirror image on the left to it:

```cpp source="2d-software-rasterizer-part-04/rast/src/Rasterizer.cpp#L287-L319"
```

The early test with the bounding box skips circles that are completely outside the clip rectangle. Then `w` is the offset of the outermost pixel in the row, which starts at $r - 1$ in the middle row, and `d` is the decision variable of the pixel $(w, b)$, which also starts at $2 - 4r$.

For every row, the `while` loop moves `w` inwards as long as the pixel $(w, b)$ is outside the circle. That's the same step as in the outline walk, but sideways. The span then goes from the mirror image of that pixel, at `center.x - 1 - w`, to the pixel itself, so its half-open right edge is `center.x + w + 1`. The same span is filled in the row below the center and in its mirror image above the center. Finally, `d` is updated for the next row.

Because `w` only moves inwards, the `while` loop runs at most $r$ times in total, for the whole circle. So a disc takes $r$ steps for its $2r$ rows, plus the spans themselves. Each span is filled with `FillRect`, which clips it to the clip rectangle and fills it with `std::fill_n` [\[12\]](#cite-12). That's one clip per row, not per pixel.

Switch the demo above to *fill* to see the spans. Every row of the disc is one span, in alternating colors, and hovering over a pixel shows its row and the offset `w` of its outermost pixel.

## The Example

The example draws circles of all sizes on the same 320 by 180 pixel surface as in part 3, scaled up four times. It starts with a function for the debug overlay:

```cpp source="2d-software-rasterizer-part-04/examples/part-04-circles/main.cpp#L16-L26"
```

`DrawBounds` draws the bounding box of a circle in the color of the clipping case that `DrawCircle` uses for it. A green box is completely inside the clip rectangle, so its circle is drawn without checks. A yellow box is partly inside, so its pixels are checked. A circle that's completely outside isn't drawn at all, so neither is its box.

`main` creates the window, the surface and the rasterizer, as before:

```cpp source="2d-software-rasterizer-part-04/examples/part-04-circles/main.cpp#L28-L44"
```

`showBounds` is the state of the overlay, which <kbd>F1</kbd> switches on and off.

The main loop starts with the key and the time, and clears the surface:

```cpp source="2d-software-rasterizer-part-04/examples/part-04-circles/main.cpp#L46-L55"
```

The surface is cleared to `MidnightBlue`, one of the named colors from part 1.

The first row of circles shows what small circles look like:

```cpp source="2d-software-rasterizer-part-04/examples/part-04-circles/main.cpp#L57-L69"
```

There's a disc and a ring for every radius from 1 to 8. A circle with a radius of 1 is a square of 2 by 2 pixels, because the center of all four pixels is inside the circle. Larger circles look rounder and rounder. `x` moves to the right by the width of the circle and a gap of 6 pixels.

Next, a few rings grow from the same center:

```cpp source="2d-software-rasterizer-part-04/examples/part-04-circles/main.cpp#L71-L82"
```

`std::fmod` takes the remainder of a division [\[13\]](#cite-13), so `grow` goes from 0 to 90 and then starts at 0 again. The six rings are 15 pixels apart. A ring whose radius is still 0 draws nothing. The larger rings go off the bottom and the right of the surface, so their bounding boxes are yellow.

Finally, a disc with an outline follows the mouse:

```cpp source="2d-software-rasterizer-part-04/examples/part-04-circles/main.cpp#L84-L103"
```

The radius pulses between 12 and 40 pixels, once every 4 seconds. The disc is drawn first, and its outline on top of it in white. Because the outline is the outermost ring of the disc's own pixels, the white ring covers the edge of the disc exactly. Move the disc to the edges of the surface to see both of them clipped.

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-4-circles/index.html" title="Gold discs and rings with a radius of 1 to 8 pixels, blue rings that grow from one point, and a red disc with a white outline that follows the mouse, on a dark blue surface of 320 by 180 pixels that is scaled up four times" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The example of part 4. Click the demo first, so that it gets the keys, and press <kbd>F1</kbd> to show the bounding boxes.

</div>

## What's Next

We can draw rectangles, lines and circles now, each with its own algorithm. In [part 5](/2d-software-rasterizer-5-triangles/), we draw the shape that everything else in this series is built from, the triangle. We decide which pixels it covers with *edge functions*, and make sure that triangles that share an edge never leave a gap between them, and never draw a pixel twice.

## References

<span id="cite-1"></span>\[1\] Microsoft, “Rasterization Rules”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules). \[Accessed: 28- Nov- 2026\].

<span id="cite-2"></span>\[2\] Wikipedia, “Midpoint circle algorithm”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Midpoint_circle_algorithm](https://en.wikipedia.org/wiki/Midpoint_circle_algorithm). \[Accessed: 28- Nov- 2026\].

<span id="cite-3"></span>\[3\] J. D. Foley, A. van Dam, S. K. Feiner and J. F. Hughes, Computer Graphics: Principles and Practice, 2nd ed. in C. Reading, MA: Addison-Wesley, 1996.

<span id="cite-4"></span>\[4\] J. E. Bresenham, “Algorithm for computer control of a digital plotter”, IBM Systems Journal, vol. 4, no. 1, pp. 25–30, 1965. \[Online\]. Available: [https://doi.org/10.1147/sj.41.0025](https://doi.org/10.1147/sj.41.0025).

<span id="cite-5"></span>\[5\] M. L. V. Pitteway, “Algorithm for drawing ellipses or hyperbolae with a digital plotter”, The Computer Journal, vol. 10, no. 3, pp. 282–289, 1967. \[Online\]. Available: [https://doi.org/10.1093/comjnl/10.3.282](https://doi.org/10.1093/comjnl/10.3.282).

<span id="cite-6"></span>\[6\] J. E. Bresenham, “A linear algorithm for incremental digital display of circular arcs”, Communications of the ACM, vol. 20, no. 2, pp. 100–106, 1977. \[Online\]. Available: [https://doi.org/10.1145/359423.359432](https://doi.org/10.1145/359423.359432).

<span id="cite-7"></span>\[7\] cppreference.com, “Namespaces”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/namespace](https://en.cppreference.com/w/cpp/language/namespace). \[Accessed: 28- Nov- 2026\].

<span id="cite-8"></span>\[8\] Wikipedia, “Bounding volume”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Bounding_volume](https://en.wikipedia.org/wiki/Bounding_volume). \[Accessed: 28- Nov- 2026\].

<span id="cite-9"></span>\[9\] cppreference.com, “Function template”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/function_template](https://en.cppreference.com/w/cpp/language/function_template). \[Accessed: 28- Nov- 2026\].

<span id="cite-10"></span>\[10\] cppreference.com, “Lambda expressions”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/lambda](https://en.cppreference.com/w/cpp/language/lambda). \[Accessed: 28- Nov- 2026\].

<span id="cite-11"></span>\[11\] cppreference.com, “Fixed width integer types”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/types/integer](https://en.cppreference.com/w/cpp/types/integer). \[Accessed: 28- Nov- 2026\].

<span id="cite-12"></span>\[12\] cppreference.com, “std::fill_n”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/algorithm/fill_n](https://en.cppreference.com/w/cpp/algorithm/fill_n). \[Accessed: 28- Nov- 2026\].

<span id="cite-13"></span>\[13\] cppreference.com, “std::fmod”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/numeric/math/fmod](https://en.cppreference.com/w/cpp/numeric/math/fmod). \[Accessed: 28- Nov- 2026\].
