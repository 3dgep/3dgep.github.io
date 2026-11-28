---
title: "2D Software Rasterizer Part 5: Triangles"
description: "Fill triangles with edge functions, with fixed-point vertices, the winding order, front and back faces and culling, the bounding box, and the top-left rule that keeps meshes free of gaps and double pixels."
slug: 2d-software-rasterizer-5-triangles
lastUpdated: 2026-11-28
---

In the previous parts, we drew rectangles, lines and circles, each with its own algorithm. In this part, we draw the shape that almost everything else is built from, the triangle. Any polygon can be split into triangles, a sprite is two triangles, and every 3D model that a GPU draws is a mesh of triangles. So it's worth getting triangles exactly right.

In this part, you will:

1. use *edge functions* to decide whether a pixel is inside a triangle,
2. learn what the *winding order* of a triangle is, how it decides whether a triangle is *front-facing* or *back-facing*, and how to *cull* triangles,
3. snap the vertices to *fixed-point* numbers, so that all the math is exact,
4. visit only the pixels in the triangle's *bounding box*, clipped once to the clip rectangle,
5. and apply the *top-left rule*, so that triangles that share an edge never leave a gap between them, and never draw a pixel twice.

At the end of this part, your program shows this:

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-5-triangles/index.html" title="A wheel of red and gold triangles that slowly turns, and a wavy strip of blue triangles that follows the mouse, on a dark blue surface of 320 by 180 pixels that is scaled up four times" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Move the mouse over the demo to move the strip. Click the demo first, so that it gets the keys. <kbd>F1</kbd> shows the bounding boxes, <kbd>F2</kbd> shows how often every pixel is drawn, and <kbd>F3</kbd> switches between culling nothing, the back faces, and the front faces.

</div>

The code for this part is at the [`part-05-triangles`](https://github.com/3dgep/2d-software-rasterizer/tree/part-05-triangles) tag of the [2d-software-rasterizer](https://github.com/3dgep/2d-software-rasterizer) repository. You can also see [everything that changed since part 4](https://github.com/3dgep/2d-software-rasterizer/compare/part-04-circles...part-05-triangles).

## Edge Functions

A triangle has three vertices, $v_0$, $v_1$ and $v_2$, and three edges between them. For circles, we decided that a pixel belongs to the disc when its center is inside the circle, and we use the same rule for triangles. So for every pixel, we need to know whether its center is inside the triangle.

An edge from a point $a$ to a point $b$ divides the plane in two halves. The *edge function* tells us in which half a point $p$ is [\[1\]](#cite-1) [\[2\]](#cite-2):

$$
E_{ab}(p) = (b_x - a_x)(p_y - a_y) - (b_y - a_y)(p_x - a_x)
$$

This is the 2D *cross product* of the vector from $a$ to $b$ and the vector from $a$ to $p$. It's zero when $p$ is on the line through $a$ and $b$, and its sign tells on which side of the line $p$ is. On our screen, where the y-axis points down, it's positive when $p$ is on the right side of the edge, as you look from $a$ to $b$, and negative on the left side. Its value is twice the area of the triangle $a, b, p$, so it also grows with the distance of $p$ from the line.

Try it below. Drag `a` and `b` to move the edge, and `p` to move the point. The line through `a` and `b` splits the plane into the positive half (green) and the negative half (red), and every pixel center is colored by the half it's in. The parallelogram is spanned by the edge, from `a` to `b`, and by the vector from `a` to `p`. Its area is the value of the edge function, and the formula below the drawing shows it with the numbers filled in. Notice that "right" means right as you look along the edge. Turn the edge so that it points down, and its positive half is on the left side of the screen.

<iframe class="embed embed-demo embed-edge-function" src="/demos/edge-function.html" title="A line through two draggable points a and b splits the plane into a positive half and a negative half. A third point p can be dragged too; the parallelogram spanned by b minus a and p minus a shows the value of the edge function at p, which is written out below the drawing." loading="lazy" allowfullscreen></iframe>

A point is inside a triangle when it's on the same side of all three edges. That's the whole test. It was described by Juan Pineda in 1988 [\[1\]](#cite-1), who pointed out that it's perfect for hardware, because every pixel can be tested on its own, in parallel with all the others. GPUs still work this way today [\[3\]](#cite-3) [\[4\]](#cite-4).

There's a second reason why edge functions are so popular. An edge function is *linear* in $x$ and $y$. When $p$ moves one pixel to the right, $p_x$ goes up by 1, so the edge function changes by $-(b_y - a_y)$. When $p$ moves one pixel down, it changes by $b_x - a_x$. Both changes are the same for every pixel, so we can compute the edge function once, for the first pixel, and find all the others with one addition each.

## Winding Order

The *winding order* (or just *winding*) of a triangle is the direction in which its vertices go around it: $v_0$, then $v_1$, then $v_2$, and back to $v_0$. That's either clockwise or counter-clockwise. Our rasterizer defines it *as you see it on the screen*, in pixel coordinates, where the y-axis points down.

We can find the winding with the edge function, too. $E_{v_0 v_1}(v_2)$ is twice the signed area of the triangle (the same formula is known as the *shoelace formula* for the area of a polygon [\[5\]](#cite-5)). On our screen, it's positive when the vertices go clockwise, and negative when they go counter-clockwise. When it's zero, the three vertices are on one line, and the triangle has no area at all.

Take the triangle with the vertices $(0, 0)$, $(1, 0)$ and $(0, 1)$. On the screen, the second vertex is to the right of the first, and the third one is below the first. Going from the first to the second and then to the third is a turn to the right, which is clockwise, and the area is $(1 - 0)(1 - 0) - (0 - 0)(0 - 0) = 1$, which is positive.

The same three numbers on a sheet of graph paper, where the y-axis points *up*, put the third vertex *above* the first one. Then the same triangle goes counter-clockwise. Flipping the y-axis flips the winding of every triangle, and that's a classic source of confusion. Whenever you read about winding, check in which coordinates it's measured. OpenGL, for example, measures the winding in window coordinates where the y-axis points up, and its default front face is counter-clockwise [\[6\]](#cite-6). Direct3D measures it in render target coordinates where the y-axis points down, like ours, and its default front face is clockwise [\[7\]](#cite-7).

### Front and Back Faces

In 3D, a closed object like a sphere has triangles that face the camera and triangles on the far side that face away from it. When the object's triangles are all defined in the same winding (seen from the outside), the triangles that face the camera show that winding on the screen, and the ones that face away show the other winding. So the winding on the screen tells whether a triangle is *front-facing* or *back-facing*. The back-facing triangles of a closed object are always hidden behind its front-facing triangles, so a 3D renderer can skip them. That's called *back-face culling*, and it saves about half the work [\[8\]](#cite-8).

A 2D rasterizer doesn't have a far side, so it normally draws every triangle, whatever its winding. But culling is cheap (we compute the area anyway), so our rasterizer supports it, with the same names that 3D APIs like Direct3D, OpenGL and Vulkan use [\[7\]](#cite-7) [\[9\]](#cite-9) [\[10\]](#cite-10) [\[11\]](#cite-11). Two enumerations at the top of `Rasterizer.hpp` define them:

```cpp source="2d-software-rasterizer-part-05/rast/include/rast/Rasterizer.hpp#L13-L28"
```

`FrontFace` says which winding makes a triangle front-facing. The default is `Clockwise`, as in Direct3D, which measures the winding in the same y-down coordinates as we do. Triangles with the other winding are back-facing. `CullMode` says which of them to skip. The default is `None`, so every triangle is drawn. `Back` skips the back-facing triangles, which is what a 3D renderer usually does, and `Front` skips the front-facing ones.

The rasterizer gets functions to set and get both:

```cpp source="2d-software-rasterizer-part-05/rast/include/rast/Rasterizer.hpp#L47-L56" ins={4-10}
```

They're short enough to be defined in the class. The members that store the settings are at the end of the class, with their defaults:

```cpp source="2d-software-rasterizer-part-05/rast/include/rast/Rasterizer.hpp#L88-L93" ins={4-5}
```

Leaving culling off by default is important for a 2D library. In part 9, we transform shapes with matrices, and a transform that mirrors a shape (a negative scale) also flips the winding of all its triangles. With culling on, a mirrored sprite would silently disappear.

## Fixed-Point Vertices

The vertices of a triangle don't have to be on pixel corners or pixel centers. A triangle that moves or turns slowly has vertices anywhere between them. So `FillTriangle` takes its vertices as a new type, `Vec2`:

```cpp source="2d-software-rasterizer-part-05/rast/include/rast/Vec2.hpp"
```

A `Vec2` is a position (or a direction) with `float` coordinates, in pixels. It can be added and subtracted, and multiplied by a number, which is all we need for now. The `operator*` outside the struct allows `2.0f * v` as well as `v * 2.0f`. Part 9 builds its transforms on this type.

If we computed the edge functions with `float`s, we would have the same problem as the DDA in part 3, and worse [\[12\]](#cite-12). Every `float` operation rounds a little, and a pixel center that's exactly on an edge between two triangles might be computed as a tiny bit inside both triangles, or a tiny bit outside both. Then the pixel would be drawn twice or not at all, and we'll see below that that's exactly what we must avoid.

So `FillTriangle` first *snaps* the vertices to a grid of $1/256$ of a pixel, and stores them as integers in *fixed-point* format [\[13\]](#cite-13). A fixed-point number is an integer that counts fractions of a unit, here 256ths of a pixel. The position 10.5 becomes $10.5 \cdot 256 = 2688$. After snapping, everything is computed with integers, so every edge function is exact. 256 positions per pixel is plenty for smooth movement, and many GPUs use the same precision for the same reason [\[4\]](#cite-4).

`Rasterizer.cpp` includes two more headers for the triangle code:

```cpp source="2d-software-rasterizer-part-05/rast/src/Rasterizer.cpp#L3-L8" ins={2,6}
```

`<cmath>` is for `std::llround` [\[14\]](#cite-14), and `<utility>` for `std::swap` [\[15\]](#cite-15).

The fixed-point code starts with a few constants and the snapping:

```cpp source="2d-software-rasterizer-part-05/rast/src/Rasterizer.cpp#L321-L339"
```

`SubpixelBits` is the number of bits for the fraction, so `SubpixelScale` is $2^8 = 256$, one pixel in fixed-point units, and `HalfPixel` is half a pixel, which we need for pixel centers. `Snap` multiplies both coordinates by 256 and rounds them to the nearest integer with `std::llround`, which returns a `long long`. The coordinates are `std::int64_t`, because the edge functions multiply two of them, and the products are too large for 32 bits. A coordinate of 2,000 pixels is already 512,000 in fixed point, and the product of two of those doesn't fit in an `int`.

Two more helper functions follow:

```cpp source="2d-software-rasterizer-part-05/rast/src/Rasterizer.cpp#L341-L352"
```

`FloorDiv` divides and rounds down, also for negative numbers, using the `CeilDiv` from part 3. `EdgeFunction` is the formula from above, on fixed-point coordinates. Its result is in units of $1/256^2$ of a square pixel.

## The Top-Left Rule

Two triangles that share an edge, like the two halves of a rectangle, have that edge in common. A pixel center that's exactly on the shared edge is on the edge of both triangles, so its edge function is 0 for both. Should it be drawn?

If both triangles include it, the pixel is drawn twice. With opaque colors that's only wasted work, but once we blend in part 8, it shows up as a line of darker pixels along every shared edge of a mesh. If both triangles exclude it, the pixel isn't drawn at all, and there's a gap in the mesh. Neither is acceptable, so the pixel must belong to exactly one of the two triangles.

The *top-left rule* decides which one [\[16\]](#cite-16). A pixel center exactly on an edge belongs to the triangle only when that edge is a *top edge* or a *left edge* of the triangle. A top edge is exactly horizontal, with the rest of the triangle below it. A left edge is any other edge with the rest of the triangle on its right. Two triangles that share an edge are on opposite sides of it. So when the edge is a left edge for one triangle, it's a right edge for the other one, and when it's the top edge of one, it's the bottom edge of the other. The pixel goes to exactly one of them. Direct3D uses this rule, and so does almost every GPU [\[16\]](#cite-16) [\[4\]](#cite-4).

You've seen this rule before. A `Rect` from part 2 includes its left column and its top row, but not its right column or its bottom row. Two rectangles that touch share an edge value without sharing a pixel. The top-left rule does the same for triangles, whose edges can be at any angle.

Try it below. The two triangles `a` and `b` share the edge between `v1` and `v2`, and that edge starts on a diagonal that goes exactly through a row of pixel centers. With the top-left rule, all of those pixels go to the triangle for which the shared edge is a left edge, and none is drawn twice. Switch to *include all* to see the pixels that are drawn twice (red), or to *exclude all* to see the gaps (red rings). The edges are labeled when they're a top or a left edge, and hovering over a pixel shows the three edge functions of both triangles at its center. The other buttons switch the front face and the cull mode, and the text below the grid shows the winding of both triangles.

<iframe class="embed embed-demo embed-triangle-rasterization" src="/demos/triangle-rasterization.html" title="Two triangles that share an edge on a grid of pixels, with draggable vertices. Buttons switch between the top-left rule, including all edges and excluding all edges; pixels drawn twice are red, and pixel centers on the shared edge that neither triangle covers get a red ring. Other buttons switch the front face and the cull mode." loading="lazy" allowfullscreen></iframe>

### Edges in Code

To apply the rule, we first make all triangles clockwise on the screen, by swapping two vertices of a counter-clockwise triangle. (Swapping two vertices reverses the winding, but the triangle covers the same pixels.) Then the inside of the triangle is always on the right side of each edge, where the edge function is positive. And in a clockwise triangle, a top edge always goes exactly to the right, and a left edge always goes up.

The code steps through the edge functions with an `Edge`:

```cpp source="2d-software-rasterizer-part-05/rast/src/Rasterizer.cpp#L354-L377"
```

`value` is the edge function at the center of the first pixel, and `stepX` and `stepY` are the changes per pixel to the right and per pixel down. A pixel is one `SubpixelScale` in fixed point, so the steps are the changes from the edge functions section, times 256.

`MakeEdge` also applies the top-left rule. A pixel center is inside when all three edge functions are 0 or more. For a pixel center exactly on a top or left edge, the edge function is 0, so it's inside. For any other edge, we want it outside. The edge functions are integers, so "more than 0" is the same as "0 or more, after subtracting 1". So `MakeEdge` subtracts 1 from the edge functions of all edges that aren't top or left edges, and the loop only needs one kind of test for all three edges.

## FillTriangle

`FillTriangle` is declared at the end of the drawing functions:

```cpp source="2d-software-rasterizer-part-05/rast/include/rast/Rasterizer.hpp#L80-L86" ins={4-7}
```

The comment sums up the rules from this article. The vertices are snapped to $1/256$ of a pixel, pixel centers decide, and the top-left rule handles the pixel centers exactly on an edge.

The function starts by snapping the vertices and computing the area:

```cpp source="2d-software-rasterizer-part-05/rast/src/Rasterizer.cpp#L381-L392"
```

The area is $E_{v_0 v_1}(v_2)$, twice the signed area of the triangle in fixed-point units. A triangle without an area covers no pixel centers, so it's skipped. That also avoids every division by zero, if a later part ever needs to divide by the area. (It will, in part 6.)

Next come the winding, culling and the swap:

```cpp source="2d-software-rasterizer-part-05/rast/src/Rasterizer.cpp#L394-L406"
```

A positive area means clockwise. The triangle is front-facing when its winding is the front face's winding, so `frontFacing` compares the two. If the cull mode skips triangles that face this way, we're done. Otherwise, a counter-clockwise triangle gets its last two vertices swapped with `std::swap`, so that the rest of the function only has to handle clockwise triangles.

Then the bounding box:

```cpp source="2d-software-rasterizer-part-05/rast/src/Rasterizer.cpp#L408-L417"
```

The bounding box goes from the smallest to the largest coordinate of the three vertices, rounded outwards to whole pixels with `FloorDiv` and `CeilDiv`. Every pixel center inside the triangle is inside this box, so the loop only has to visit the pixels of the box. Intersecting it with the clip rectangle is all the clipping that a triangle needs [\[4\]](#cite-4). The loop below only visits pixels inside the clip rectangle, so it can never write outside the surface, and a pixel that's clipped away was never part of the box in the first place.

The box is a little *conservative*. It may contain a row or a column of pixels whose centers are outside the triangle, but the edge functions reject those.

Finally, the loop:

```cpp source="2d-software-rasterizer-part-05/rast/src/Rasterizer.cpp#L419-L450"
```

Edge $i$ is the edge *opposite* vertex $i$, so `e0` goes from $v_1$ to $v_2$, `e1` from $v_2$ to $v_0$, and `e2` from $v_0$ to $v_1$. That's not needed yet, but in part 6 we'll see that the edge function opposite a vertex is exactly what we need to blend that vertex's color across the triangle.

All three edges start at the center of the top-left pixel of the box. For every row, `w0`, `w1` and `w2` start at the values for the first pixel of the row. For every pixel, the test is three comparisons, and moving to the next pixel is three additions. After each row, the start values move down a row.

This loop is simple, and it's already quite fast, but it visits every pixel of the bounding box. For a thin diagonal triangle, most of those are outside [\[17\]](#cite-17). In the second half of the series, we test whole blocks of pixels at once, and skip the blocks that are completely outside, and in part 17, we test several pixels with one instruction.

## The Example

The example draws a fan of triangles that turns, and a strip of triangles that follows the mouse, on the same 320 by 180 pixel surface. It describes a triangle with a small struct:

```cpp source="2d-software-rasterizer-part-05/examples/part-05-triangles/main.cpp#L20-L26"
```

A `Triangle` has three vertices and a color. The example collects all triangles of a frame in a `std::vector<Triangle>` [\[18\]](#cite-18), so that it can draw them in different ways.

The fan is a wheel of 12 triangles:

```cpp source="2d-software-rasterizer-part-05/examples/part-05-triangles/main.cpp#L28-L41"
```

The `rim` lambda returns point `i` on a circle around the center. Every triangle goes from the center to one rim point and then to the next one. The angle grows from one rim point to the next, and with the y-axis pointing down, a growing angle turns clockwise on the screen, so every triangle of the fan is clockwise. That makes the fan front-facing with the default front face. Neighboring triangles share an edge from the center to a rim point, and all 12 share the center.

The strip is made of triangles too:

```cpp source="2d-software-rasterizer-part-05/examples/part-05-triangles/main.cpp#L43-L58"
```

The strip is a row of 10 quads along a wave. The `top` lambda returns a point on the top of the strip, and `bottom` returns the point 24 pixels below it. Each quad is split into two triangles along its diagonal. The first triangle goes from the top-left corner down to the bottom-left corner and then to the top-right corner, which is counter-clockwise on the screen, and so is the second one. So the strip is back-facing, and with culling off, it's drawn anyway.

For the debug overlay, the example computes the bounding boxes too:

```cpp source="2d-software-rasterizer-part-05/examples/part-05-triangles/main.cpp#L60-L67"
```

`Bounds` rounds the smallest coordinates down and the largest ones up, with `std::floor` and `std::ceil` [\[19\]](#cite-19), like `FillTriangle` does in fixed point.

`main` sets up the window, the surface, the rasterizer and the state of the overlays:

```cpp source="2d-software-rasterizer-part-05/examples/part-05-triangles/main.cpp#L69-L91"
```

`single` is a second surface for the overdraw view, and `drawCount` has a counter for every pixel. `triangles` is created once, and reused in every frame.

The main loop starts with the keys:

```cpp source="2d-software-rasterizer-part-05/examples/part-05-triangles/main.cpp#L93-L112"
```

<kbd>F1</kbd> and <kbd>F2</kbd> switch the overlays. <kbd>F3</kbd> moves the cull mode to the next one, with a `switch` on the current mode. From `None` it goes to `Back`, which skips the strip, then to `Front`, which skips the fan, and then back to `None`.

Then the triangles of this frame are collected:

```cpp source="2d-software-rasterizer-part-05/examples/part-05-triangles/main.cpp#L114-L126"
```

The strip follows the mouse, which is in whole pixels, so its center is converted to a `Vec2`. The fan turns slowly with the time. After clearing the surface, the triangles are drawn.

Normally, every triangle is filled with its own color:

```cpp source="2d-software-rasterizer-part-05/examples/part-05-triangles/main.cpp#L128-L134"
```

The rasterizer applies the cull mode, so this loop doesn't need to know about it.

The overdraw view counts how often every pixel is drawn:

```cpp source="2d-software-rasterizer-part-05/examples/part-05-triangles/main.cpp#L135-L164"
```

Every triangle is drawn on its own into `single`, in white, after clearing `single` to transparent. Then every white pixel adds one to its counter. Finally, every pixel that was drawn once becomes gray, and every pixel that was drawn more than once becomes red. With the top-left rule, the fan and the strip are evenly gray, without any red pixels or holes along the shared edges. Red only shows up where the strip overlaps the fan, because those are two different shapes on top of each other.

The bounding boxes are drawn last, on top of everything:

```cpp source="2d-software-rasterizer-part-05/examples/part-05-triangles/main.cpp#L166-L178"
```

Each box is clipped to the clip rectangle before it's drawn, so you see exactly the pixels that the loop of `FillTriangle` visits.

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-5-triangles/index.html" title="A wheel of red and gold triangles that slowly turns, and a wavy strip of blue triangles that follows the mouse, on a dark blue surface of 320 by 180 pixels that is scaled up four times" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The example of part 5. Click the demo first, so that it gets the keys. <kbd>F2</kbd> shows the overdraw, and <kbd>F3</kbd> switches the cull mode.

</div>

## What's Next

Our triangles have one color. In [part 6](/2d-software-rasterizer-6-barycentric/), we give each vertex its own color, and blend the colors smoothly across the triangle with *barycentric coordinates*. As it turns out, they're the three edge functions that we already compute, divided by the area.

## References

<span id="cite-1"></span>\[1\] J. Pineda, “A parallel algorithm for polygon rasterization”, ACM SIGGRAPH Computer Graphics, vol. 22, no. 4, pp. 17–20, 1988. \[Online\]. Available: [https://doi.org/10.1145/378456.378457](https://doi.org/10.1145/378456.378457).

<span id="cite-2"></span>\[2\] F. Giesen, “The barycentric conspiracy”, The ryg blog, 2013. \[Online\]. Available: [https://fgiesen.wordpress.com/2013/02/06/the-barycentric-conspirac/](https://fgiesen.wordpress.com/2013/02/06/the-barycentric-conspirac/). \[Accessed: 28- Nov- 2026\].

<span id="cite-3"></span>\[3\] M. Olano and T. Greer, “Triangle scan conversion using 2D homogeneous coordinates”, in Proceedings of the ACM SIGGRAPH/EUROGRAPHICS Workshop on Graphics Hardware (HWWS ’97), pp. 89–95, 1997. \[Online\]. Available: [https://doi.org/10.1145/258694.258723](https://doi.org/10.1145/258694.258723).

<span id="cite-4"></span>\[4\] F. Giesen, “Triangle rasterization in practice”, The ryg blog, 2013. \[Online\]. Available: [https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/](https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/). \[Accessed: 28- Nov- 2026\].

<span id="cite-5"></span>\[5\] Wikipedia, “Shoelace formula”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Shoelace_formula](https://en.wikipedia.org/wiki/Shoelace_formula). \[Accessed: 28- Nov- 2026\].

<span id="cite-6"></span>\[6\] Khronos Group, “glFrontFace”, OpenGL 4 Reference Pages, 2026. \[Online\]. Available: [https://registry.khronos.org/OpenGL-Refpages/gl4/html/glFrontFace.xhtml](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glFrontFace.xhtml). \[Accessed: 28- Nov- 2026\].

<span id="cite-7"></span>\[7\] Microsoft, “D3D12_RASTERIZER_DESC structure”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_rasterizer_desc](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_rasterizer_desc). \[Accessed: 28- Nov- 2026\].

<span id="cite-8"></span>\[8\] Wikipedia, “Back-face culling”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Back-face_culling](https://en.wikipedia.org/wiki/Back-face_culling). \[Accessed: 28- Nov- 2026\].

<span id="cite-9"></span>\[9\] Khronos Group, “glCullFace”, OpenGL 4 Reference Pages, 2026. \[Online\]. Available: [https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCullFace.xhtml](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCullFace.xhtml). \[Accessed: 28- Nov- 2026\].

<span id="cite-10"></span>\[10\] Khronos Group, “VkFrontFace”, Vulkan Specification, 2026. \[Online\]. Available: [https://registry.khronos.org/vulkan/specs/latest/man/html/VkFrontFace.html](https://registry.khronos.org/vulkan/specs/latest/man/html/VkFrontFace.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-11"></span>\[11\] Khronos Group, “VkCullModeFlagBits”, Vulkan Specification, 2026. \[Online\]. Available: [https://registry.khronos.org/vulkan/specs/latest/man/html/VkCullModeFlagBits.html](https://registry.khronos.org/vulkan/specs/latest/man/html/VkCullModeFlagBits.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-12"></span>\[12\] D. Goldberg, “What every computer scientist should know about floating-point arithmetic”, ACM Computing Surveys, vol. 23, no. 1, pp. 5–48, 1991. \[Online\]. Available: [https://doi.org/10.1145/103162.103163](https://doi.org/10.1145/103162.103163).

<span id="cite-13"></span>\[13\] Wikipedia, “Fixed-point arithmetic”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Fixed-point_arithmetic](https://en.wikipedia.org/wiki/Fixed-point_arithmetic). \[Accessed: 28- Nov- 2026\].

<span id="cite-14"></span>\[14\] cppreference.com, “std::round, std::lround, std::llround”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/numeric/math/round](https://en.cppreference.com/w/cpp/numeric/math/round). \[Accessed: 28- Nov- 2026\].

<span id="cite-15"></span>\[15\] cppreference.com, “std::swap”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/algorithm/swap](https://en.cppreference.com/w/cpp/algorithm/swap). \[Accessed: 28- Nov- 2026\].

<span id="cite-16"></span>\[16\] Microsoft, “Rasterization Rules”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules). \[Accessed: 28- Nov- 2026\].

<span id="cite-17"></span>\[17\] F. Giesen, “Optimizing the basic rasterizer”, The ryg blog, 2013. \[Online\]. Available: [https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/](https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/). \[Accessed: 28- Nov- 2026\].

<span id="cite-18"></span>\[18\] cppreference.com, “std::vector”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/vector](https://en.cppreference.com/w/cpp/container/vector). \[Accessed: 28- Nov- 2026\].

<span id="cite-19"></span>\[19\] cppreference.com, “std::floor”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/numeric/math/floor](https://en.cppreference.com/w/cpp/numeric/math/floor). \[Accessed: 28- Nov- 2026\].
