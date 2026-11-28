---
title: "2D Software Rasterizer Part 6: Barycentric Coordinates"
description: "Give every vertex of a triangle its own color, and blend the colors across the triangle with barycentric coordinates, which turn out to be the edge functions that the rasterizer already computes."
slug: 2d-software-rasterizer-6-barycentric
lastUpdated: 2026-11-28
---

In [part 5](/2d-software-rasterizer-5-triangles/), we filled triangles with a single color. In this part, every vertex of a triangle gets its own color, and the colors blend smoothly from one vertex to the next. This is called *Gouraud shading*, and it's the first time [\[1\]](#cite-1) that our rasterizer computes a different value for every pixel of a shape. The same technique works for any value that is defined at the vertices, like the texture coordinates of part 7.

In this part, you will:

1. learn what *barycentric coordinates* are, and why they always add up to 1,
2. see that they are the edge functions from part 5, divided by the area of the triangle,
3. add a `Vertex` with a position and a color,
4. split the triangle code in a setup and a loop that both kinds of triangles share,
5. and blend the vertex colors across the triangle.

At the end of this part, your program shows this:

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-6-barycentric/index.html" title="A turning triangle with a red, a green and a blue corner, blended smoothly, and a hexagon with a white center and a rainbow rim that follows the mouse, on a black surface of 320 by 180 pixels that is scaled up four times" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Move the mouse over the demo to move the hexagon. Click the demo first, so that it gets the keys. <kbd>F1</kbd> switches to flat shading, and <kbd>F2</kbd> shows the barycentric coordinates.

</div>

The code for this part is at the [`part-06-barycentric`](https://github.com/3dgep/2d-software-rasterizer/tree/part-06-barycentric) tag of the [2d-software-rasterizer](https://github.com/3dgep/2d-software-rasterizer) repository. You can also see [everything that changed since part 5](https://github.com/3dgep/2d-software-rasterizer/compare/part-05-triangles...part-06-barycentric).

## Barycentric Coordinates

Imagine three weights, one at each vertex of a triangle. Together, they balance at one point of the triangle, their *center of mass*. With equal weights, that's the middle of the triangle. Make the weight at $v_0$ heavier, and the balance point moves towards $v_0$. Every point inside the triangle is the balance point of exactly one set of weights, if we agree that the weights add up to 1. Those three weights are the *barycentric coordinates* of the point [\[2\]](#cite-2) ("barycentric" means "center of mass" in Greek). August Möbius introduced them in 1827 [\[3\]](#cite-3).

So a point $p$ with the barycentric coordinates $\lambda_0$, $\lambda_1$ and $\lambda_2$ is

$$
p = \lambda_0 v_0 + \lambda_1 v_1 + \lambda_2 v_2
\qquad\text{with}\qquad
\lambda_0 + \lambda_1 + \lambda_2 = 1
$$

At $v_0$ itself, $\lambda_0 = 1$ and the other two are 0. Halfway between $v_0$ and $v_1$, $\lambda_0$ and $\lambda_1$ are both $\frac{1}{2}$, and $\lambda_2 = 0$. On the edge opposite a vertex, the coordinate of that vertex is 0. Inside the triangle, all three are between 0 and 1. And outside the triangle, at least one of them is negative.

That last property should sound familiar. In part 5, a point was inside the triangle when all three edge functions were positive. Barycentric coordinates are positive inside the triangle too, and they're 0 on the edges, exactly like the edge functions. That's no coincidence.

### Areas and Edge Functions

The point $p$ splits the triangle into three smaller triangles. Each one is made of $p$ and one of the edges. The *sub-triangle opposite* vertex $v_0$ is the triangle $p, v_1, v_2$, which doesn't include $v_0$. The closer $p$ gets to $v_0$, the larger that sub-triangle becomes, until it's the whole triangle when $p$ reaches $v_0$. So the barycentric coordinate of a vertex is the area of the sub-triangle opposite it, divided by the area of the whole triangle [\[4\]](#cite-4):

$$
\lambda_0 = \frac{\text{area}(p, v_1, v_2)}{\text{area}(v_0, v_1, v_2)}
$$

and the same for $\lambda_1$ and $\lambda_2$. The three sub-triangles together make up the whole triangle, so their areas add up to the whole area, and the coordinates add up to 1.

In part 5, we saw that an edge function is twice the signed area of the triangle made by the edge and the point. The edge function $E_0$ of the edge from $v_1$ to $v_2$ is twice the area of the sub-triangle $p, v_1, v_2$, and $E_{v_0 v_1}(v_2)$ is twice the area of the whole triangle. The factors 2 cancel out, so with $A = E_{v_0 v_1}(v_2)$:

$$
\lambda_0 = \frac{E_0(p)}{A}
\qquad
\lambda_1 = \frac{E_1(p)}{A}
\qquad
\lambda_2 = \frac{E_2(p)}{A}
$$

Edge $i$ is the edge opposite vertex $i$. That's why we numbered the edges that way in part 5. For every pixel inside a triangle, the rasterizer already has $E_0$, $E_1$ and $E_2$. The barycentric coordinates only cost three multiplications by $1 / A$, which we compute once per triangle.

Try it below. Drag the vertices, and drag $p$ around. Each sub-triangle is labeled with the coordinate of the vertex opposite it, and the text below the drawing shows the coordinates, their sum, and the color at $p$. Drag $p$ outside the triangle, and the coordinate of the vertex across the edge that $p$ crossed becomes negative. The sum stays 1.

<iframe class="embed embed-demo embed-barycentric" src="/demos/barycentric.html" title="A triangle with a red, a green and a blue vertex, filled with the blend of their colors, and a draggable point p that splits it into three sub-triangles. Each sub-triangle is labeled with the barycentric coordinate of the vertex opposite it; the coordinates, their sum and the color at p are written below the drawing." loading="lazy" allowfullscreen></iframe>

## Interpolating Colors

If a vertex has a value, like a color, then the barycentric coordinates tell us how much each vertex contributes to the value at a point. Each channel of the color at $p$ is the weighted sum of that channel at the three vertices:

$$
c(p) = \lambda_0 c_0 + \lambda_1 c_1 + \lambda_2 c_2
$$

At a vertex, this is exactly that vertex's color. Along an edge, it blends the colors of the two vertices of that edge, and the third vertex has no influence. Two triangles that share an edge, with the same colors at the shared vertices, have exactly the same colors along that edge, so there's no visible seam between them. This is *Gouraud shading*, after Henri Gouraud, who used it in 1971 to shade curved surfaces that are made of flat triangles [\[5\]](#cite-5).

The barycentric coordinates are *linear* functions of $x$ and $y$, because the edge functions are. So the color of a pixel changes by the same amount for every step to the right, and for every step down. A linear interpolation like this is called *affine* [\[6\]](#cite-6). In a 2D rasterizer, it's exactly what we want. In 3D, where a triangle is seen in perspective, it isn't, because a step of one pixel on the screen covers a longer distance on the part of the triangle that is farther away. A 3D rasterizer corrects for that with *perspective-correct interpolation* [\[7\]](#cite-7), which divides by the depth of every vertex.

:::note[Blending colors in sRGB]
The numbers in our colors are *sRGB* values [\[8\]](#cite-8), which aren't proportional to the amount of light. An sRGB value of 128 is only about 22% as bright as 255, not 50%. So when we blend red and green in sRGB numbers, the middle of the gradient looks darker and muddier than an even mix of red and green light would. Most 2D libraries blend sRGB values directly anyway, and so do we for now. In part 8, transparent colors run into the same problem, and there we convert colors to linear light and back.
:::

## Vertices

A vertex has a position and a color now, so it gets a struct of its own, in `Vertex.hpp`:

```cpp source="2d-software-rasterizer-part-06/rast/include/rast/Vertex.hpp"
```

The values that are interpolated across a triangle are called the *attributes* of its vertices. For now, a vertex has one attribute, its color. In part 7, it gets a texture coordinate too.

`Rasterizer.hpp` includes the new header:

```cpp source="2d-software-rasterizer-part-06/rast/include/rast/Rasterizer.hpp#L3-L7" ins={5}
```

`Vertex.hpp` includes `Color.hpp` and `Vec2.hpp` itself, so the order of these includes doesn't matter.

The rasterizer also declares a new `FillTriangle` that takes three vertices:

```cpp source="2d-software-rasterizer-part-06/rast/include/rast/Rasterizer.hpp#L84-L92" ins={6-9}
```

It's an overload of the `FillTriangle` from part 5. Both fill the same pixels, but the new one blends the vertex colors instead of using one color for the whole triangle.

## Setup and Loop

Both `FillTriangle`s need the same steps before they visit the pixels. They snap the vertices, cull the triangle, make it clockwise, clip its bounding box and set up the edges. And both visit the pixels in the same way. The only difference is the color they write. So in this part, the code of part 5 is split into a *setup* and a *loop* that both functions share.

`Rasterizer.cpp` now also includes `<optional>`:

```cpp source="2d-software-rasterizer-part-06/rast/src/Rasterizer.cpp#L3-L9" ins={6}
```

The setup returns an `std::optional`, because sometimes there's nothing to draw [\[9\]](#cite-9).

### The Edges

The `Edge` struct gets one more member:

```cpp source="2d-software-rasterizer-part-06/rast/src/Rasterizer.cpp#L355-L379" ins={7,24}
```

`bias` remembers the top-left rule's bias, which is already included in `value`. The bias decides which pixels are inside, but it must not change the barycentric coordinates. Without it, a pixel whose edge function is exactly 0 on a right edge would get a coordinate of $-1 / A$, a tiny bit outside the triangle. So the loop will subtract the bias again before it computes the coordinates.

### The Setup

The setup stores what the loop needs to know:

```cpp source="2d-software-rasterizer-part-06/rast/src/Rasterizer.cpp#L381-L390"
```

`box` is the clipped bounding box, and `e0`, `e1` and `e2` are the edges, each opposite the vertex with the same number. `area` is $A$, twice the area of the triangle, which we need for the barycentric coordinates. Because the setup makes every triangle clockwise, `area` is always positive. `swapped` remembers whether the setup swapped `v1` and `v2` to make the triangle clockwise. The vertices' colors must be swapped too, so that each color stays with its own vertex.

`SetupTriangle` is the first half of part 5's `FillTriangle`:

```cpp source="2d-software-rasterizer-part-06/rast/src/Rasterizer.cpp#L392-L421" ins={29}
```

It's a free function in the anonymous namespace, so it gets the clip rectangle, the front face and the cull mode as parameters. A triangle without an area, or a culled one, gives `std::nullopt`, an empty `std::optional`. When it swaps `p1` and `p2`, it also negates the area, which used to be thrown away after the winding test, and now has to stay positive.

The rest of the setup is the bounding box and the edges:

```cpp source="2d-software-rasterizer-part-06/rast/src/Rasterizer.cpp#L423-L438"
```

This is the same code as in part 5. At the end, it returns all of it in a `TriangleSetup`, with `swapped` set when the vertices were counter-clockwise.

### The Loop

The loop is a function template that takes a *shading function* [\[10\]](#cite-10) [\[11\]](#cite-11):

```cpp source="2d-software-rasterizer-part-06/rast/src/Rasterizer.cpp#L440-L474"
```

The loop is the same as in part 5, except for the line that writes a pixel. Instead of a fixed color, it writes whatever `shade` returns. `shade` gets the three edge functions at the pixel center, with the bias subtracted again, so that they're exactly proportional to the barycentric coordinates. Like `WalkCircleOutline` in part 4, `Shade` is a template parameter, so the compiler can inline the lambda that each `FillTriangle` passes, and there's no function call per pixel.

A function that computes the color of a pixel is called a *pixel shader*, or a *fragment shader*, on a GPU [\[12\]](#cite-12). Ours are just small lambdas, but the idea is the same. The rasterizer decides which pixels a triangle covers, and the shader decides their colors.

The setup is passed by value, because the loop steps the edges' values, and it shouldn't change the caller's copy.

### Interpolate

The color of a pixel comes from a small helper:

```cpp source="2d-software-rasterizer-part-06/rast/src/Rasterizer.cpp#L476-L486"
```

`Interpolate` computes the weighted sum of the three colors with `float`s, one channel at a time. Alpha is interpolated too, which we'll need in part 8. The result is clamped to the range of a byte, because rounding errors can push a weighted sum a tiny bit below 0 or above 255, and then rounded to the nearest whole number.

## FillTriangle

The `FillTriangle` from part 5 is now short:

```cpp source="2d-software-rasterizer-part-06/rast/src/Rasterizer.cpp#L490-L500"
```

It sets up the triangle, and passes a lambda that returns the same color for every pixel. The lambda ignores the edge functions, so its parameters don't have names. After inlining, the compiler sees that the edge functions aren't used for anything but the test, so this is as fast as the code of part 5.

The new `FillTriangle` interpolates the colors:

```cpp source="2d-software-rasterizer-part-06/rast/src/Rasterizer.cpp#L502-L523"
```

After the setup, the colors are put in the same order as the setup's vertices. If the setup swapped `v1` and `v2`, `c1` is the color of `v2` and `c2` is the color of `v1`. Then `inverseArea` is computed once per triangle. For every pixel, the lambda multiplies the three edge functions by `inverseArea` to get the barycentric coordinates, and interpolates the colors with them.

We could save one multiplication per pixel, because the coordinates add up to 1, so $\lambda_2 = 1 - \lambda_0 - \lambda_1$. We could save even more. Every color channel is a linear function of $x$ and $y$, so it changes by a fixed amount per pixel, like the edge functions do. A rasterizer can compute those amounts once per triangle (they're called the *plane equation* of the attribute [\[13\]](#cite-13)), and find the color of the next pixel with one addition per channel. We keep the version that is easiest to follow for now, and come back to speed in the second half of the series.

## The Example

The example draws a triangle with a red, a green and a blue vertex, and a hexagon, on the 320 by 180 pixel surface. A triangle is three vertices:

```cpp source="2d-software-rasterizer-part-06/examples/part-06-barycentric/main.cpp#L20-L25"
```

Each vertex of a `Triangle` has its own position and color.

A small helper finds points on a circle:

```cpp source="2d-software-rasterizer-part-06/examples/part-06-barycentric/main.cpp#L27-L31"
```

`OnCircle` returns the point at `angle` on a circle with the given center and radius.

The first shape is the classic RGB triangle:

```cpp source="2d-software-rasterizer-part-06/examples/part-06-barycentric/main.cpp#L33-L40"
```

Its vertices are a third of a circle apart, and they're red, green (`Lime`, from part 1) and blue. Because each vertex has exactly one of the three channels, the color of a pixel shows its barycentric coordinates directly. The amount of red is $\lambda_0$, the amount of green $\lambda_1$, and the amount of blue $\lambda_2$.

The second shape is a hexagon:

```cpp source="2d-software-rasterizer-part-06/examples/part-06-barycentric/main.cpp#L42-L57"
```

The hexagon is a fan of six triangles around a white center, with the six colors of a rainbow on its rim. Neighboring triangles share the vertex between them, with the same color, so the colors flow from one triangle into the next without a seam. `i` is a `std::size_t`, because it indexes a `std::array` [\[14\]](#cite-14).

For flat shading, a triangle gets one color:

```cpp source="2d-software-rasterizer-part-06/examples/part-06-barycentric/main.cpp#L59-L66"
```

`Average` adds up the channels of the three vertex colors and divides them by 3, with `+ 1` to round to the nearest whole number.

The example has three ways to draw the triangles:

```cpp source="2d-software-rasterizer-part-06/examples/part-06-barycentric/main.cpp#L68-L73"
```

`Gouraud` blends the vertex colors, `Flat` fills each triangle with its average color, and `Weights` shows the barycentric coordinates.

`main` sets up everything, as before:

```cpp source="2d-software-rasterizer-part-06/examples/part-06-barycentric/main.cpp#L75-L92"
```

The example starts with Gouraud shading.

The main loop starts with the keys:

```cpp source="2d-software-rasterizer-part-06/examples/part-06-barycentric/main.cpp#L94-L104"
```

<kbd>F1</kbd> switches between flat and Gouraud shading, and <kbd>F2</kbd> between the barycentric coordinates and Gouraud shading.

Then the triangles of this frame are collected:

```cpp source="2d-software-rasterizer-part-06/examples/part-06-barycentric/main.cpp#L106-L118"
```

The RGB triangle turns slowly, and the hexagon follows the mouse. After clearing the surface to black, the triangles are drawn.

Each triangle is drawn in the chosen way:

```cpp source="2d-software-rasterizer-part-06/examples/part-06-barycentric/main.cpp#L120-L142"
```

Gouraud shading passes the vertices to the new `FillTriangle`. Flat shading passes the positions and the average color to the `FillTriangle` of part 5. The barycentric view replaces the colors of every triangle's vertices with red, green and blue, so every triangle looks like the RGB triangle. In the hexagon, you can see that every triangle has its red corner (`v0`) in the white center, because each triangle of the fan starts there.

Switch to flat shading with <kbd>F1</kbd> to see what the interpolation adds. The RGB triangle becomes a dull gray (the average of red, green and blue), and the hexagon becomes six flat wedges.

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-6-barycentric/index.html" title="A turning triangle with a red, a green and a blue corner, blended smoothly, and a hexagon with a white center and a rainbow rim that follows the mouse, on a black surface of 320 by 180 pixels that is scaled up four times" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The example of part 6. Click the demo first, so that it gets the keys. <kbd>F1</kbd> switches to flat shading, and <kbd>F2</kbd> shows the barycentric coordinates.

</div>

## What's Next

Every pixel of a triangle can get its own color now. In part 7, the colors come from an image. We load images with `stb_image`, give every vertex a *texture coordinate*, interpolate the texture coordinates with barycentric coordinates, and look up the color of every pixel in the image.

## References

<span id="cite-1"></span>\[1\] Wikipedia, “Gouraud shading”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Gouraud_shading](https://en.wikipedia.org/wiki/Gouraud_shading). \[Accessed: 28- Nov- 2026\].

<span id="cite-2"></span>\[2\] Wikipedia, “Barycentric coordinate system”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Barycentric_coordinate_system](https://en.wikipedia.org/wiki/Barycentric_coordinate_system). \[Accessed: 28- Nov- 2026\].

<span id="cite-3"></span>\[3\] A. F. Möbius, Der barycentrische Calcul. Leipzig: J. A. Barth, 1827.

<span id="cite-4"></span>\[4\] F. Giesen, “The barycentric conspiracy”, The ryg blog, 2013. \[Online\]. Available: [https://fgiesen.wordpress.com/2013/02/06/the-barycentric-conspirac/](https://fgiesen.wordpress.com/2013/02/06/the-barycentric-conspirac/). \[Accessed: 28- Nov- 2026\].

<span id="cite-5"></span>\[5\] H. Gouraud, “Continuous Shading of Curved Surfaces”, IEEE Transactions on Computers, vol. C-20, no. 6, pp. 623–629, 1971. \[Online\]. Available: [https://doi.org/10.1109/T-C.1971.223313](https://doi.org/10.1109/T-C.1971.223313).

<span id="cite-6"></span>\[6\] Wikipedia, “Affine transformation”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Affine_transformation](https://en.wikipedia.org/wiki/Affine_transformation). \[Accessed: 28- Nov- 2026\].

<span id="cite-7"></span>\[7\] Scratchapixel, “Perspective Correct Interpolation and Vertex Attributes”, scratchapixel.com, 2026. \[Online\]. Available: [https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes.html](https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes.html). \[Accessed: 28- Nov- 2026\].

<span id="cite-8"></span>\[8\] Wikipedia, “sRGB”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/SRGB](https://en.wikipedia.org/wiki/SRGB). \[Accessed: 28- Nov- 2026\].

<span id="cite-9"></span>\[9\] cppreference.com, “std::optional”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/utility/optional](https://en.cppreference.com/w/cpp/utility/optional). \[Accessed: 28- Nov- 2026\].

<span id="cite-10"></span>\[10\] cppreference.com, “Function template”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/function_template](https://en.cppreference.com/w/cpp/language/function_template). \[Accessed: 28- Nov- 2026\].

<span id="cite-11"></span>\[11\] cppreference.com, “Lambda expressions”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/lambda](https://en.cppreference.com/w/cpp/language/lambda). \[Accessed: 28- Nov- 2026\].

<span id="cite-12"></span>\[12\] Wikipedia, “Shader”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Shader](https://en.wikipedia.org/wiki/Shader). \[Accessed: 28- Nov- 2026\].

<span id="cite-13"></span>\[13\] F. Giesen, “Optimizing the basic rasterizer”, The ryg blog, 2013. \[Online\]. Available: [https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/](https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/). \[Accessed: 28- Nov- 2026\].

<span id="cite-14"></span>\[14\] cppreference.com, “std::array”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/array](https://en.cppreference.com/w/cpp/container/array). \[Accessed: 28- Nov- 2026\].
