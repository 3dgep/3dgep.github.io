---
title: "2D Software Rasterizer Part 3: Lines"
description: "Draw lines in all eight directions with the DDA and with Bresenham's integer algorithm, make them half-open, and clip them without changing a single pixel."
slug: 2d-software-rasterizer-3-lines
lastUpdated: 2026-11-28
---

In [part 2](/2d-software-rasterizer-2-clipping/), we drew rectangles, whose edges follow the rows and columns of the pixels. A line can go in any direction, and that raises a question that rectangles never had. Which pixels make up a line from one point to another, when the line passes between the pixel centers?

In this part, you will:

1. find out what makes a line of pixels look right,
2. draw lines with the *DDA*, which uses floating-point numbers,
3. derive *Bresenham's algorithm*, which draws the same lines with integers only,
4. make it work in all eight directions that a line can go,
5. make lines *half-open*, like the rectangles of part 2,
6. and clip lines to the clip rectangle once, without changing a single pixel of the line.

At the end of this part, your program shows this:

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-3-lines/index.html" title="A starburst of white lines around the mouse, over a dark grid, on a surface of 320 by 180 pixels that is scaled up four times" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Move the mouse over the demo to move the starburst. Click the demo first, so that it gets the keys. <kbd>F1</kbd> switches between Bresenham's algorithm and the DDA, and <kbd>F2</kbd> shows where they draw different pixels.

</div>

The code for this part is at the [`part-03-lines`](https://github.com/3dgep/2d-software-rasterizer/tree/part-03-lines) tag of the [2d-software-rasterizer](https://github.com/3dgep/2d-software-rasterizer) repository. You can also see [everything that changed since part 2](https://github.com/3dgep/2d-software-rasterizer/compare/part-02-clipping...part-03-lines).

## A Line of Pixels

A mathematical line is infinitely thin, so it doesn't cover any pixel completely. We have to choose pixels that look like the line. Two rules give good results.

The first rule is that the line has exactly one pixel for every step along its longest direction. Take a line that goes 12 pixels to the right and 5 pixels down. It's 12 pixels long horizontally, so it gets 12 pixels, one in every column. If it had two pixels in a column, it would look thicker there. If it skipped a column, it would have a gap. The direction in which the line moves the most is called the *major axis*, and the other one is the *minor axis*. For this line, `x` is the major axis and `y` is the minor axis.

The second rule is that in every column (or row, for a line whose major axis is `y`), we pick the pixel whose center is closest to the line [\[1\]](#cite-1). As in part 2, the pixel at `(x, y)` covers the square from $(x, y)$ to $(x + 1, y + 1)$, so its center is at $(x + 0.5, y + 0.5)$. If both end points are pixel centers, then the line goes through the centers too, and in every column, we only have to round the line's `y` to the nearest whole number.

Our lines go from one pixel to another, so both end points are whole numbers. Lines with end points between pixels come back in part 11, with *anti-aliasing*.

## The DDA

The most direct way to follow those rules is to walk along the major axis, one pixel per step, and move along the minor axis by the *slope* at every step. That's the *DDA*, short for *digital differential analyzer* [\[2\]](#cite-2), a name that comes from machines that solved differential equations step by step. For our line of 12 to the right and 5 down, every step moves 1 pixel in `x` and $5/12$ of a pixel in `y`.

The example of this part has a DDA, so that we can compare it with Bresenham's algorithm. It's not part of the library:

```cpp source="2d-software-rasterizer-part-03/examples/part-03-lines/main.cpp#L17-L41"
```

The number of steps is the larger of the two distances, so the DDA takes one step per pixel along the major axis. Dividing `dx` and `dy` by the number of steps gives the distance of one step on both axes. Along the major axis, that's exactly 1 or -1, and along the minor axis, it's the slope. Then the loop draws a pixel at the rounded position, and moves `x` and `y` one step further. Like a `Rect` in part 2, the line is *half-open*. It draws `p0`, but stops one step before `p1`. We come back to that below.

The DDA clips with `SetPixel`, which checks every pixel against the clip rectangle. That's fine for a comparison, but in part 2 we saw that checking every pixel is the slow way to clip.

The DDA works well, but it has two weaknesses. It converts two `float`s to integers for every pixel, and for a long time, that was slow. And `x` and `y` are sums of many small steps, and every `float` addition rounds the result a little [\[3\]](#cite-3). For long lines, those small errors add up, and the line can end up a pixel off. That's not a problem for our short lines, but it means that the pixels of a line depend on how `float` arithmetic rounds.

## Bresenham's Algorithm

In 1962, Jack Bresenham found a way to draw exactly the right pixels with integers only, for a pen plotter at IBM [\[4\]](#cite-4) [\[5\]](#cite-5). To derive it, we take a line whose major axis is `x`, that goes to the right and down, so that $d_x \ge d_y \ge 0$. The other directions follow later.

At step $i$, the line is at $x = x_0 + i$, and its exact `y` is $y_0 + i \cdot d_y / d_x$. We round that to the nearest pixel by adding $\frac{1}{2}$ and rounding down. So the pixel is $k$ pixels below $y_0$, where

$$
k = \left\lfloor i \frac{d_y}{d_x} + \frac{1}{2} \right\rfloor = \left\lfloor \frac{2 i d_y + d_x}{2 d_x} \right\rfloor
$$

Multiplying the top and the bottom by $2 d_x$ removed both fractions. Everything in the last formula is an integer, so we could compute it for every pixel with an integer division. Bresenham's idea was to avoid even that division.

When we take one step to the right, $i$ goes up by 1, so the top of the fraction goes up by $2 d_y$. And $k$ goes up by one exactly when the top reaches the next multiple of $2 d_x$, which is $2 d_x (k + 1)$. So instead of $k$ itself, we keep track of how far the top still is from that next multiple. That's the *error*:

$$
e = 2 i d_y + d_x - 2 d_x (k + 1)
$$

The error is always negative, because the top is always below the next multiple. At the first step, $i = 0$ and $k = 0$, so $e = -d_x$. Every step to the right adds $2 d_y$ to the error. When that makes it zero or more, the top has reached the next multiple, so the line moves down a pixel ($k$ goes up by 1), and the next multiple is $2 d_x$ further away, so we subtract $2 d_x$ from the error. Because $d_y \le d_x$, the line never moves down more than one pixel per step.

So for every pixel, Bresenham's algorithm needs one addition, one comparison, and sometimes one subtraction, all with integers. And because nothing is rounded, every pixel is exactly the one that the formula for $k$ gives, no matter how long the line is.

Try it below. Drag `p0` and `p1`, and hover over a pixel of the line to see its step and the error at that step. The buttons switch between Bresenham's algorithm, the DDA, and a comparison of the two.

<iframe class="embed embed-demo embed-line-rasterization" src="/demos/line-rasterization.html" title="A grid of pixels with a line between two points that can be dragged. The pixels of the line are filled, and hovering over one shows its step and Bresenham's error term at that step." loading="lazy" allowfullscreen></iframe>

### When the Two Disagree

Most of the time, the DDA and Bresenham's algorithm draw exactly the same pixels. They disagree when the line passes exactly halfway between two pixels, which happens a lot for lines with simple slopes like $\frac{1}{2}$. Then both algorithms have to round a value like 9.5, and they round it in different directions. Bresenham's algorithm rounds $k$ up, which is always towards `p1`. The DDA uses `std::lround`, which rounds half-way cases *away from zero* [\[6\]](#cite-6). For a line that goes down, that's also towards `p1`, but for a line that goes up, it's towards `p0`.

The demo below shows a line that goes up with a slope of exactly $\frac{1}{2}$. Every other step lands exactly halfway between two pixels, and every time, the two algorithms pick a different one. Gray pixels are drawn by both, green ones only by Bresenham's algorithm, and red ones only by the DDA.

<iframe class="embed embed-demo embed-line-rasterization" src="/demos/line-rasterization.html?mode=compare&p0=1,11&p1=21,1" title="A line that goes up and to the right with a slope of one half, compared: half of its pixels are drawn by both algorithms, and the other half alternates between a pixel only Bresenham's algorithm draws and a pixel only the DDA draws" loading="lazy" allowfullscreen></iframe>

Neither choice is wrong, because both pixels are equally close to the line. But Bresenham's algorithm always makes the same choice, in every direction. Press <kbd>F2</kbd> in the example at the top of this page to see how few pixels differ in a whole starburst.

## Eight Directions

So far, our line went to the right and down, with `x` as its major axis. A line can also go to the left or up, and its major axis can be `y`. Two signs and two possible major axes give eight combinations, which are called the eight *octants*, because each one covers an eighth of all directions.

Fortunately, the algorithm is the same in all of them. We describe the line along its major and its minor axis, instead of `x` and `y`. The distances along both axes are positive, so the derivation above works unchanged. Only the directions in which we step are different. `majorSign` is +1 or -1 depending on the direction along the major axis, and `minorSign` is +1 or -1 depending on the direction along the minor axis. At the end, we swap `major` and `minor` back to `x` and `y` when the major axis is `y`. That's all it takes to turn the algorithm for one octant into an algorithm for all eight.

In the demo below, `p0` is in the middle, and each colored wedge is one octant. Drag `p1` around `p0`, or press **sweep** to send it around in a circle. The label of each wedge names its major axis first, with the direction of the steps along it, and then the direction of the minor steps. So `x→ y↑` is a line whose major axis is `x`, that steps to the right, and moves up one pixel now and then. Below the grid, you see the values that `DrawLine` computes for the line.

<iframe class="embed embed-demo embed-line-octants" src="/demos/line-octants.html" title="A grid of pixels with p0 in the middle and eight colored wedges around it, one for each octant. Dragging p1 around p0 highlights the octant that p1 is in, and shows xMajor, majorSign and minorSign for the line." loading="lazy" allowfullscreen></iframe>

The borders between the octants are the axes and the diagonals. A line that's exactly diagonal, with `|dx|` equal to `|dy|`, could use either axis as its major axis. `DrawLine` picks `x`, because it compares with `>=`. Both choices give the same pixels, because the line moves one pixel along both axes in every step. A line that's exactly horizontal or vertical has a minor distance of 0. Its minor sign is +1, but that doesn't matter, because the line never takes a step along its minor axis.

## Half-Open Lines

The rectangles in part 2 are half-open, and so are our lines. A line from `p0` to `p1` draws `p0`, but not `p1`. It has exactly one pixel per step along the major axis, which is `dMajor` pixels for a line that is `dMajor` pixels long. That's the same as a `Rect` from `left` to `right`, which is `right - left` pixels wide.

That might look like a strange choice. Why would `DrawLine(a, b)` not draw `b`? The reason is what happens when lines share an end point. A polyline, or the outline of a triangle, is made of lines that each start where the previous one ended. If every line drew both of its end points, every corner would be drawn twice. As long as we only draw opaque pixels, nobody notices. But in part 8, we draw transparent shapes, and a corner that's drawn twice is blended twice, so it shows up as a darker dot. With half-open lines, every corner is drawn exactly once, by the line that starts there. GPUs draw lines the same way, and OpenGL and Direct3D leave out the last pixel of every line for the same reason [\[7\]](#cite-7) [\[8\]](#cite-8).

There's a bonus too. A vertical line from `{ x, 0 }` to `{ x, height }` covers exactly the pixels of one column of the surface, the same as `Rect{ x, 0, x + 1, height }`. The example uses that to draw a grid.

## DrawLine

`Rasterizer.hpp` gets a new function:

```cpp source="2d-software-rasterizer-part-03/rast/include/rast/Rasterizer.hpp#L40-L45" ins={4-6}
```

The comment says that the line is half-open, because that's the one thing about it that you can't guess from its name.

Without clipping, the function is the derivation above, in the major and minor axes of the line:

```cpp
void Rasterizer::DrawLine(Point p0, Point p1, Color color)
{
    // ... the major and minor axes of the line, as below ...

    std::int64_t error = -dMajor;
    int major = majorStart;
    int minor = minorStart;
    for (std::int64_t step = 0; step < dMajor; ++step)
    {
        SetPixel(xMajor ? Point{ major, minor } : Point{ minor, major }, color);

        major += majorSign;
        error += 2 * dMinor;
        if (error >= 0)
        {
            minor += minorSign;
            error -= 2 * dMajor;
        }
    }
}
```

It starts with an error of $-d_{major}$, draws `dMajor` pixels, and updates the error as we derived. But it clips with `SetPixel`, which we want to avoid. A line from the middle of the surface to a point that's 10,000 pixels to the left would visit 10,000 pixels, only to skip almost all of them. So the real `DrawLine` clips the line once, before the loop.

### Clipping a Line

The usual way to clip a line is to compute where it crosses the edges of the clip rectangle [\[9\]](#cite-9) [\[10\]](#cite-10), and to draw the line between those points instead. But the crossing points are usually between pixels, and rounding them to whole pixels gives a new line with a slightly different slope. That new line can draw different pixels than the original one, so a line that slides off the edge of the screen would *wobble*, as its pixels change from frame to frame.

We clip in a different way. We don't change the line. We only find out which of its steps fall inside the clip rectangle, and draw those [\[11\]](#cite-11). Then the pixels inside the clip rectangle are exactly the pixels of the whole line. Bresenham's formula for $k$ tells us where every step is, so we can compute the first and the last visible step directly.

`Rasterizer.cpp` includes a few more headers for this:

```cpp source="2d-software-rasterizer-part-03/rast/src/Rasterizer.cpp#L1-L6" ins={4-6}
```

`<cstdint>` is for `std::int64_t` [\[12\]](#cite-12), `<cstdlib>` for `std::abs` [\[13\]](#cite-13), and `<limits>` for `std::numeric_limits` [\[14\]](#cite-14).

The clipping needs a few small helper functions. They're only used in `Rasterizer.cpp`, so they're in an *anonymous namespace* [\[15\]](#cite-15), which makes them invisible to other files. The first one divides and rounds up:

```cpp source="2d-software-rasterizer-part-03/rast/src/Rasterizer.cpp#L88-L95"
```

C++'s integer division rounds towards zero [\[16\]](#cite-16). For a positive result that's rounding down, so adding `b - 1` first makes it round up. For a negative `a`, rounding towards zero already rounds up, so we only have to divide the positive value and negate it.

The second helper works out which steps keep one coordinate inside a range:

```cpp source="2d-software-rasterizer-part-03/rast/src/Rasterizer.cpp#L97-L112"
```

A coordinate of the line is `start + sign * offset`, where the offset is the number of steps (along the major axis) or the value of $k$ (along the minor axis). When the line goes in the positive direction, the coordinate is inside `[lo, hi)` when the offset is inside `[lo - start, hi - start)`. When it goes in the negative direction, the coordinate goes down as the offset goes up, so the range is flipped. The coordinate `start - offset` is less than `hi` when the offset is more than `start - hi`, and at least `lo` when the offset is at most `start - lo`. Both ends get a `+ 1` to make that a half-open range.

For the major axis, that's all we need, because the offset *is* the step. For the minor axis, it gives a range of values for $k$, and we have to find the steps where $k$ is in that range. That's the third helper:

```cpp source="2d-software-rasterizer-part-03/rast/src/Rasterizer.cpp#L114-L127"
```

$k$ never goes down as the line goes on, so the steps where $k$ is at least some value $m$ start at one particular step. Solving the formula for $k$ gives that step:

$$
\left\lfloor \frac{2 i d_{minor} + d_{major}}{2 d_{major}} \right\rfloor \ge m
\iff
2 i d_{minor} + d_{major} \ge 2 d_{major} m
\iff
i \ge \frac{2 d_{major} m - d_{major}}{2 d_{minor}}
$$

The first step is the smallest whole $i$ that satisfies that, which is the fraction rounded up with `CeilDiv`. There are two special cases. $k$ starts at 0, so for $m \le 0$ the answer is step 0. And when the line is perfectly horizontal or vertical, $d_{minor}$ is 0, $k$ is always 0, and it never reaches a positive $m$, so the answer is "never", the largest possible step.

The helpers use `std::int64_t`, because the products can be large. With coordinates of 50,000 (a line far outside the surface), $2 i d_{minor}$ is already 5 billion, which doesn't fit in an `int`.

### The Function

`DrawLine` starts by describing the line along its major and minor axes:

```cpp source="2d-software-rasterizer-part-03/rast/src/Rasterizer.cpp#L131-L150"
```

The major axis is `x` when the line moves at least as far horizontally as vertically. Then the start, the distance and the direction of the line along both axes are taken from `x` or `y`. The distances are always positive, and the signs remember the directions. A line from a point to itself has no steps at all, so it draws nothing. That's the half-open rule again, the same as an empty `Rect`.

Next, the function finds the visible steps:

```cpp source="2d-software-rasterizer-part-03/rast/src/Rasterizer.cpp#L152-L166"
```

`OffsetsInside` gives the steps that keep the major coordinate inside the clip rectangle, and the values of $k$ that keep the minor coordinate inside. `FirstStepWithOffset` turns that second range into steps. The first visible step is the latest of the three starts (step 0, the first step inside along the major axis, and the first step inside along the minor axis), and the end is the earliest of the three ends (`dMajor`, because `p1` isn't drawn, and the ends along both axes). If the first step isn't before the end, no part of the line is inside the clip rectangle.

`std::max` and `std::min` with braces take an `std::initializer_list` [\[17\]](#cite-17) [\[18\]](#cite-18) [\[19\]](#cite-19), so they can compare three values at once.

Finally, the loop draws the visible steps:

```cpp source="2d-software-rasterizer-part-03/rast/src/Rasterizer.cpp#L168-L199"
```

The loop doesn't start at step 0, but at the first visible step. So before the loop, we compute $k$ and the error at that step with the formulas from the derivation. From there on, the loop does exactly what Bresenham's algorithm does, so every pixel it draws is the pixel that the whole line would have had at that step. And because every step between `first` and `last` is inside the clip rectangle, the loop writes straight to `pixels`, without a single check.

The `if (xMajor)` inside the loop looks like it costs time, but `xMajor` never changes during the loop, so the CPU's branch predictor always guesses it right [\[20\]](#cite-20). Compilers often move it out of the loop altogether, and make two copies of the loop [\[21\]](#cite-21).

## The Example

The example draws a grid and a starburst of lines on a surface of 320 by 180 pixels, in a window that's four times larger, so you can see every pixel of every line. After the DDA that we saw above, it chooses between the two algorithms:

```cpp source="2d-software-rasterizer-part-03/examples/part-03-lines/main.cpp#L43-L71"
```

The starburst has 48 lines of 120 pixels, which start at `center` and point in every direction, so it has lines in all eight octants and with many different slopes. `angle` turns the whole starburst slowly as time goes on. The end point of each line is rounded to whole pixels with `std::lround` [\[6\]](#cite-6), and then the line is drawn with the chosen algorithm.

The grid is made of lines too:

```cpp source="2d-software-rasterizer-part-03/examples/part-03-lines/main.cpp#L73-L85"
```

Because lines are half-open, a line from `{ x, 0 }` to `{ x, bounds.bottom }` covers exactly the column `x` of the surface, from the top row to the bottom row. These are the simplest lines that `DrawLine` gets. Their minor distance is 0, so the error never reaches 0, and the loop just walks straight down (or to the right).

`main` creates a window that is four times as large as the surface, and two more surfaces:

```cpp source="2d-software-rasterizer-part-03/examples/part-03-lines/main.cpp#L87-L108"
```

`bresenhamLines` and `ddaLines` are for the comparison, where each algorithm draws the starburst into a surface of its own. `algorithm` is the algorithm for the normal view, and `showDifferences` switches the comparison on and off.

The main loop starts with the keys and the position of the starburst:

```cpp source="2d-software-rasterizer-part-03/examples/part-03-lines/main.cpp#L110-L131"
```

<kbd>F1</kbd> switches the algorithm, and <kbd>F2</kbd> switches the comparison. The starburst follows the mouse. But when the mouse isn't over the surface (or the page has just loaded, and the mouse has never been over the demo), it stays in the middle. Then the surface is cleared, and the grid is drawn.

Normally, the starburst is drawn on top of the grid with one algorithm:

```cpp source="2d-software-rasterizer-part-03/examples/part-03-lines/main.cpp#L133-L136"
```

That's the view that you see when the demo starts.

The comparison draws the starburst twice, and compares the results:

```cpp source="2d-software-rasterizer-part-03/examples/part-03-lines/main.cpp#L137-L171"
```

This is the first time we use `SetColorTarget`. The rasterizer draws the starburst with Bresenham's algorithm into `bresenhamLines`, and with the DDA into `ddaLines`. Each of them is cleared to `Transparent` first, so that every pixel that isn't transparent afterwards belongs to a line. Then the color target goes back to `surface`.

The loop compares the two surfaces pixel by pixel. The position of a pixel doesn't matter for this, so it uses `Data()`, which gives all pixels of a surface as one `std::span` [\[22\]](#cite-22), in the same order. Pixels that both algorithms drew become gray, pixels that only Bresenham's algorithm drew become lime, and pixels that only the DDA drew become red. Pixels that neither drew keep the grid that's already on `surface`.

Finally, the surface is shown:

```cpp source="2d-software-rasterizer-part-03/examples/part-03-lines/main.cpp#L173-L177"
```

Move the starburst to the edges of the surface, and watch the lines that leave it. They don't wobble, because the clipped lines have exactly the pixels of the whole lines.

<iframe class="embed embed-rast" src="/demos/2d-software-rasterizer-3-lines/index.html" title="A starburst of white lines around the mouse, over a dark grid, on a surface of 320 by 180 pixels that is scaled up four times" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The example of part 3. Click the demo first, so that it gets the keys. <kbd>F1</kbd> switches the algorithm, and <kbd>F2</kbd> shows the differences between the two.

</div>

## What's Next

We can draw lines in every direction now, and clip them without changing them. In [part 4](/2d-software-rasterizer-4-circles/), we draw circles. The *midpoint circle algorithm* uses the same idea as Bresenham's algorithm, an error term that decides between two pixels with integers only. And a circle is symmetric in eight ways, so we only have to compute an eighth of it.

## References

<span id="cite-1"></span>\[1\] J. D. Foley, A. van Dam, S. K. Feiner and J. F. Hughes, Computer Graphics: Principles and Practice, 2nd ed. in C. Reading, MA: Addison-Wesley, 1996.

<span id="cite-2"></span>\[2\] Wikipedia, “Digital differential analyzer (graphics algorithm)”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)](https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)). \[Accessed: 28- Nov- 2026\].

<span id="cite-3"></span>\[3\] D. Goldberg, “What every computer scientist should know about floating-point arithmetic”, ACM Computing Surveys, vol. 23, no. 1, pp. 5–48, 1991. \[Online\]. Available: [https://doi.org/10.1145/103162.103163](https://doi.org/10.1145/103162.103163).

<span id="cite-4"></span>\[4\] J. E. Bresenham, “Algorithm for computer control of a digital plotter”, IBM Systems Journal, vol. 4, no. 1, pp. 25–30, 1965. \[Online\]. Available: [https://doi.org/10.1147/sj.41.0025](https://doi.org/10.1147/sj.41.0025).

<span id="cite-5"></span>\[5\] Wikipedia, “Bresenham's line algorithm”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm](https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm). \[Accessed: 28- Nov- 2026\].

<span id="cite-6"></span>\[6\] cppreference.com, “std::round, std::lround, std::llround”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/numeric/math/round](https://en.cppreference.com/w/cpp/numeric/math/round). \[Accessed: 28- Nov- 2026\].

<span id="cite-7"></span>\[7\] M. Segal and K. Akeley, “The OpenGL Graphics System: A Specification (Version 4.6 (Core Profile)), Section 14.5, Line Segments”, Khronos Group, 2022. \[Online\]. Available: [https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf](https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf). \[Accessed: 28- Nov- 2026\].

<span id="cite-8"></span>\[8\] Microsoft, “Rasterization Rules”, Microsoft Learn, 2026. \[Online\]. Available: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules). \[Accessed: 28- Nov- 2026\].

<span id="cite-9"></span>\[9\] Wikipedia, “Cohen–Sutherland algorithm”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm](https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm). \[Accessed: 28- Nov- 2026\].

<span id="cite-10"></span>\[10\] Y.-D. Liang and B. A. Barsky, “A New Concept and Method for Line Clipping”, ACM Transactions on Graphics, vol. 3, no. 1, pp. 1–22, 1984. \[Online\]. Available: [https://doi.org/10.1145/357332.357333](https://doi.org/10.1145/357332.357333).

<span id="cite-11"></span>\[11\] Y. P. Kuzmin, “Bresenham’s Line Generation Algorithm with Built-in Clipping”, Computer Graphics Forum, vol. 14, no. 5, pp. 275–280, 1995. \[Online\]. Available: [https://doi.org/10.1111/1467-8659.1450275](https://doi.org/10.1111/1467-8659.1450275).

<span id="cite-12"></span>\[12\] cppreference.com, “Fixed width integer types”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/types/integer](https://en.cppreference.com/w/cpp/types/integer). \[Accessed: 28- Nov- 2026\].

<span id="cite-13"></span>\[13\] cppreference.com, “std::abs”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/numeric/math/abs](https://en.cppreference.com/w/cpp/numeric/math/abs). \[Accessed: 28- Nov- 2026\].

<span id="cite-14"></span>\[14\] cppreference.com, “std::numeric_limits”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/types/numeric_limits](https://en.cppreference.com/w/cpp/types/numeric_limits). \[Accessed: 28- Nov- 2026\].

<span id="cite-15"></span>\[15\] cppreference.com, “Namespaces”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/namespace](https://en.cppreference.com/w/cpp/language/namespace). \[Accessed: 28- Nov- 2026\].

<span id="cite-16"></span>\[16\] cppreference.com, “Arithmetic operators”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/language/operator_arithmetic](https://en.cppreference.com/w/cpp/language/operator_arithmetic). \[Accessed: 28- Nov- 2026\].

<span id="cite-17"></span>\[17\] cppreference.com, “std::max”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/algorithm/max](https://en.cppreference.com/w/cpp/algorithm/max). \[Accessed: 28- Nov- 2026\].

<span id="cite-18"></span>\[18\] cppreference.com, “std::min”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/algorithm/min](https://en.cppreference.com/w/cpp/algorithm/min). \[Accessed: 28- Nov- 2026\].

<span id="cite-19"></span>\[19\] cppreference.com, “std::initializer_list”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/utility/initializer_list](https://en.cppreference.com/w/cpp/utility/initializer_list). \[Accessed: 28- Nov- 2026\].

<span id="cite-20"></span>\[20\] Wikipedia, “Branch predictor”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Branch_predictor](https://en.wikipedia.org/wiki/Branch_predictor). \[Accessed: 28- Nov- 2026\].

<span id="cite-21"></span>\[21\] Wikipedia, “Loop unswitching”, en.wikipedia.org, 2026. \[Online\]. Available: [https://en.wikipedia.org/wiki/Loop_unswitching](https://en.wikipedia.org/wiki/Loop_unswitching). \[Accessed: 28- Nov- 2026\].

<span id="cite-22"></span>\[22\] cppreference.com, “std::span”, cppreference.com, 2026. \[Online\]. Available: [https://en.cppreference.com/w/cpp/container/span](https://en.cppreference.com/w/cpp/container/span). \[Accessed: 28- Nov- 2026\].
