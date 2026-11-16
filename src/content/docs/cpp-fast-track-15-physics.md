---
title: "C++ Fast Track for Games Programming Part 15: Physics"
description: "Physics: give the tanks a velocity, detect collisions between circles and boxes, push objects apart, and build a tow cable with position-based dynamics (Verlet, PBD and XPBD)."
slug: cpp-fast-track-15-physics
lastUpdated: 2026-11-16
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

Many games use a *physics engine*: a piece of software that moves the objects of the game in a natural way, with forces, collisions, mass and friction. Writing a good physics engine is hard, but the basic ideas are not. In this part, you will build a small one yourself, in the tank game of [part 12](/cpp-fast-track-12-classes/):

1. The tanks get a *velocity*: they speed up and slow down, instead of starting and stopping at once.
2. Every obstacle gets a *collider*: a circle or a box that fits it. You will learn how to test whether two circles, two boxes, or a circle and a box overlap.
3. When the tank drives into an obstacle, it is pushed out, so that it slides along it instead of getting stuck.
4. Some objects become loose: the tank can push drums around, and a heavy tank pushes harder than a light drum.
5. Finally, the tank gets a tow cable, with *position-based dynamics*: the same technique that many modern games use for ropes, cloth and ragdolls.

This is a long part, with a lot of new ideas. Take your time, and try the demos as you go.

## Getting Started

For this tutorial, you will start where [part 12](/cpp-fast-track-12-classes/) ended: the tile map with the green tank of the player, and the red tank that drives by itself. Download it here:

[<img src="/images/zip1.png" alt="Download the starting point for part 15 as a zip file" width="120" class="thumb" />part-15.zip](/downloads/part-15.zip)

Extract the zip file to `C:\Projects\C++ Fast Track`: it contains a `Part 15` folder. Open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 15` folder.

The code is the code from the end of part 12. The `assets` folder has one new image, `drums.png`, which you will need later: the four barrels of `objects.png` (tile 7) as separate sprites of 24 × 24 pixels.

## Vectors

Physics is full of pairs of numbers: a position has an `x` and a `y`, and so does a velocity. The template has a small struct for that: `vec2`, in `template.hpp`. A `vec2` has an `x` and a `y`, and you can calculate with it as if it were one number:

```cpp
vec2 position = { 100, 50 };
vec2 velocity = { 0.1f, 0 };
position += velocity * 16; // both x and y: position is now { 101.6, 50 }
float speed = velocity.length(); // the length of the vector: sqrt(x * x + y * y)
vec2 direction = velocity.normalized(); // the same direction, with length 1
float d = velocity.dot(direction); // the dot product: x1 * x2 + y1 * y2
```

`length` uses Pythagoras: a vector $(x, y)$ has length $\sqrt{x^2 + y^2}$. A vector with length 1 is called a *unit vector*: it only says which way, not how far. And the *dot product* of a vector with a unit vector tells you how much of the vector points in that direction. We will use all of these. (Have a look at `vec2` in `template.hpp`: it's a short struct, with operators like the ones you know from [part 12](/cpp-fast-track-12-classes/).)

## Velocity

So far, the tank moved at a fixed speed when you pressed a key, and stopped dead when you released it. Real vehicles don't do that: they *accelerate*. Let's give the tank a velocity. The engine changes the velocity, and the velocity changes the position.

First, add `<algorithm>` at the top of `game.cpp` (we need `std::min` from it):

```cpp source="tmpl8-part-15/1-velocity/game.cpp#L1-L4" {3}
```

Then replace the `Tank` class, and its `Move` method:

```cpp source="tmpl8-part-15/1-velocity/game.cpp#L74-L93" {1-3,7-9,13,16-17}
```

```cpp source="tmpl8-part-15/1-velocity/game.cpp#L95-L115"
```

The enemy and `Draw` use `position` now, instead of `x` and `y`:

```cpp source="tmpl8-part-15/1-velocity/game.cpp#L117-L131" {4-6,13-15}
```

```cpp source="tmpl8-part-15/1-velocity/game.cpp#L133-L134"
```

Finally, the keys now make a *direction*, and `Tick` passes that to `Move`:

```cpp source="tmpl8-part-15/1-velocity/game.cpp#L153-L156" {3}
```

```cpp source="tmpl8-part-15/1-velocity/game.cpp#L168-L180" {1-12}
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-15-velocity/index.html" title="The green tank speeds up and slows down smoothly, and turns towards the direction in which it drives" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The tank with a velocity: it speeds up and slows down (quickly), drives diagonally, and faces the way it drives. (Click the demo first, so that it gets the keys.)

</div>

Here's what happens:

- The keys make a `direction`: up and right gives `{ 1, -1 }`. That vector is longer than 1 (its length is $\sqrt{2} \approx 1.41$), so the tank would drive faster diagonally. `normalize` gives it length 1.
- `Move` calculates the *acceleration*: how much the velocity changes every millisecond. The engine pushes in `direction`, and *drag* (the friction of the tracks and the ground) slows the tank down: the faster it goes, the more it loses. When you let go of the keys, only the drag is left, and the tank stops.
- At some speed, the drag is exactly as large as the push of the engine, and the tank can't go any faster. That's the top speed: `Acceleration / Drag` = 0.003 / 0.02 = 0.15 pixels per millisecond, just like the fixed speed of part 11. A large drag makes the tank stop quickly, so it doesn't feel slippery: with a drag of 0.02, it loses 2% of its speed every millisecond.
- Then the velocity changes the position. Note the order: *first* the velocity, *then* the position, with the new velocity. More about that order below.
- A tank that drives diagonally needs the frames in between: `std::atan2` (from `<cmath>`) gives the angle of the velocity, and that angle becomes one of the 16 frames of `tank.png`. (In part 11, the tank had only 4 directions, because it could only drive in 4.)
- `TankFits` still works as before: it checks the four corners of a box around the tank, one axis at a time. When the tank can't move, its velocity in that direction becomes 0.
- `std::min(deltaTime, 33.0f)` limits the time step. When you drag the window, a frame can take a second, and a step of a second would make the tank jump far (or even make the drag larger than the velocity, so that the tank drives backwards). With this line, a frame counts as at most 33 milliseconds.

### Euler Integration

Calculating the new velocity and position from the old ones, step by step, is called *integration*. The simplest way is *Euler integration*, named after the mathematician Leonhard Euler. There are two versions, and the only difference is the order of the two lines. *Explicit* Euler moves first, with the old velocity:

$$
\begin{aligned}
x_{n+1} &= x_n + v_n \, \Delta t \\
v_{n+1} &= v_n + a_n \, \Delta t
\end{aligned}
$$

*Semi-implicit* Euler (also called *symplectic* Euler) changes the velocity first, and then moves with the new velocity:

$$
\begin{aligned}
v_{n+1} &= v_n + a_n \, \Delta t \\
x_{n+1} &= x_n + v_{n+1} \, \Delta t
\end{aligned}
$$

Here, $x$ is the position, $v$ the velocity, $a$ the acceleration, $\Delta t$ the time step (`deltaTime`), and $n$ the number of the step.

The two versions only differ in which velocity moves the object: the old one ($v_n$) or the new one ($v_{n+1}$). Swapping two lines of code may look like a detail, but it makes a big difference. Here are both versions, with a moon in orbit around a planet:

<iframe class="embed embed-demo embed-euler-orbit" src="/demos/euler-orbit.html" title="Two moons orbit a planet. The moon moved with explicit Euler spirals outwards; the moon moved with semi-implicit Euler stays in its orbit." loading="lazy" allowfullscreen></iframe>

<div class="caption">

Two moons start at the same place with the same velocity. With explicit Euler (red), the moon gains a bit of energy with every step, and spirals away. With semi-implicit Euler (blue), the orbit stays the same, orbit after orbit. Try the large steps: the difference gets much bigger.

</div>

Explicit Euler always overshoots a little: it moves with the velocity of the *start* of the step, while the velocity has already changed by the end of it. In a game, that means that a bouncing ball bounces higher and higher, and a spring swings further and further. Semi-implicit Euler doesn't have that problem, and it isn't more work: it's the same two lines, the other way around. That's why most game physics engines use it. Whenever you write physics code, change the velocity first.

## Colliders

The tank still uses `TankFits`: four points, and whole tiles. A tree is round, but its tile is square, so the tank stops at the empty corners of the tile. Most games give every object a *collider*: a simple shape that fits the object, which is used for the collisions only. Circles and boxes are the simplest shapes, and they are good enough for many games.

The boxes that we use are *axis-aligned bounding boxes*, or *AABBs*: their sides are horizontal and vertical (aligned with the x and y axes). A box that can rotate is called an *oriented bounding box* (*OBB*), and it is much harder to test against (it needs a technique called the *separating axis theorem*). An AABB is described by two corners: the top-left one (`min`), and the bottom-right one (`max`).

Replace the `solid` array by these structs, and the colliders of the 16 objects:

```cpp source="tmpl8-part-15/2-colliders/game.cpp#L38-L71"
```

- First, the shapes get a name: `BigTree`, `Square`, and so on, in pixels within a tile of 64 × 64. `objectCircles` and `objectBoxes` then give each object its collider, by the number of the object in `objects.png`. Several objects are the same shape in another colour (the trees 0 and 1, the wooden and the metal crates 3 and 4, two colours of sandbags), so the same name appears more than once. Trees (0, 1 and 2) get a circle; crates, sandbags, barrels and barricades get a box that fits their picture. The arrays have 16 elements, but the lists are shorter: the rest of the elements are 0, so oil and twigs (12 to 15) get a circle with radius 0 and an empty box. That means: no collider.

<img src="/images/cpp-fast-track-15/object-colliders.png" alt="The 16 objects of objects.png, numbered, with their colliders drawn in yellow: circles around the trees, boxes around the crates, sandbags, barrels and barricades, and nothing around oil and twigs" width="1024" height="256" class="screenshot" />

<div class="caption">

The colliders of the objects (shown twice as large). Most of them fit well, but look at tile 2: those are *two* small trees, and one circle around both also covers the empty corners. You will fix that in the assignment.

</div>

- `circles` and `boxes` will hold the colliders of the objects in the map: at most one per tile. `Init` will fill these in.

:::note[Why not `std::vector`?]
The C++ standard library has `std::vector` (from `<vector>`): an array that grows by itself. With a vector, `circles.push_back(circle)` adds a collider, and `circles.size()` tells you how many there are, so you wouldn't need `MaxColliders` and `circleCount`. Most C++ programmers would use one here. We use a plain array with a count, for two reasons:

- You can see exactly what happens: a fixed block of memory, and a number that says how much of it is in use. That's what a `std::vector` does on the inside too, plus getting a bigger block when the old one is full.
- An array never moves. Later in this part, we will store *pointers* to bodies. When a `std::vector` grows, it moves its elements to a new, bigger block of memory, and every pointer to the old place becomes invalid. With a fixed array, that can't happen.

Here, we know the maximum in advance (at most one collider per tile), so a fixed array is all we need.
:::

Add this to `Init`:

```cpp source="tmpl8-part-15/2-colliders/game.cpp#L205-L221" {3-16}
```

For every object in the map, `Init` takes its collider, moves it to the tile (by adding the position of the tile), and adds it to `circles` or `boxes`. `circleCount++` uses the count as the index, and then adds 1 to it: the next collider goes into the next element.

### Overlap Tests

Now the interesting part: do two shapes overlap? There is a test for each pair of shapes. Add them below the colliders, one by one.

#### Circle - Circle

```cpp source="tmpl8-part-15/2-colliders/game.cpp#L73-L77"
```

Two circles overlap when the distance between their centres is less than the sum of their radii:

$$
|c_a - c_b| < r_a + r_b
$$

where $c$ is the centre of a circle, and $r$ its radius. Think of it like this: two circles that just touch, have their centres exactly $r_a + r_b$ apart. Any closer, and they overlap.

`length` calculates $\sqrt{x^2 + y^2}$, and a square root is a fairly slow calculation. But we don't need the distance itself, only whether it is smaller than the sum of the radii. Both sides are positive numbers, so we can square them: $d < r$ means exactly the same as $d^2 < r^2$. And the squared length is just $x^2 + y^2$, without the square root. `vec2` has `sqrLength` for that:

```cpp
bool Overlaps(const Circle& a, const Circle& b)
{
    const float sum = a.radius + b.radius;
    return (a.center - b.center).sqrLength() < sum * sum; // no square root
}
```

This version gives exactly the same answer. When a game tests thousands of pairs of circles every frame, it's worth it. Our game only has a few dozen colliders, so in this part we keep the first version: it reads like the formula.

<iframe class="embed embed-demo embed-collision-tests" src="/demos/collision-tests.html?test=cc" title="Drag a circle over another circle: it turns red when the distance between the centres is less than the sum of the radii" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Drag the blue circle: it turns red when it overlaps the grey one. The dashed line is the distance between the centres. The yellow arrow is the *minimum translation vector*: how far the blue shape must move to stop overlapping. You will need it later, in Collision Response.

</div>

#### Box - Box

```cpp source="tmpl8-part-15/2-colliders/game.cpp#L79-L82"
```

Two boxes overlap when they overlap on the x axis *and* on the y axis. On one axis, a box is just an interval: from `min.x` to `max.x`. Two intervals overlap when each one starts before the other one ends: `a.min.x < b.max.x` (a starts before b ends) and `b.min.x < a.max.x` (b starts before a ends). The same goes for the y axis.

<div id="less-than"></div>

:::note[Why `<` and not `<=`?]
With `<`, two boxes that only *touch* don't overlap; with `<=`, they would. We use `<` because:

- `max` is the edge *after* the last pixel of a box: a box from 0 to 64 covers the pixels 0 to 63. The next tile starts at 64, so with `<=`, every two neighbouring tiles would overlap.
- The circle test uses `<` as well: two circles that just touch don't overlap either.

There is one more reason, which you will see in the next section.
:::

<iframe class="embed embed-demo embed-collision-tests" src="/demos/collision-tests.html?test=bb" title="Drag a box over another box: the intervals of both boxes on the x and the y axis are shown next to the axes; the boxes overlap when both pairs of intervals overlap" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Drag the blue box. Next to the axes, you see the intervals of both boxes: at the bottom on the x axis, and at the left on the y axis. An interval of the blue box turns red when it overlaps the grey one, and the boxes only overlap when both pairs do.

</div>

#### Circle - Box

This one needs a trick: first, find the point of the box that is *closest* to the centre of the circle. Add both functions:

```cpp source="tmpl8-part-15/2-colliders/game.cpp#L84-L94"
```

`std::clamp` (from `<algorithm>`) keeps a value between a minimum and a maximum. Clamping the centre of the circle to the box, on both axes, gives the point of the box closest to it. Then the circle overlaps the box when that point is closer to the centre than the radius. And when the centre of the circle is inside the box, the closest point is the centre itself: the distance is 0, so the circle certainly overlaps.

<iframe class="embed embed-demo embed-collision-tests" src="/demos/collision-tests.html?test=cb" title="Drag a circle over a box: the closest point of the box is shown, and the circle overlaps when that point is closer than the radius" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Drag the blue circle. The white dot is the point of the box that is closest to the centre of the circle. Move the circle along a side of the box, and around a corner, to see how the closest point follows it.

</div>

:::tip
The square root trick of circle - circle works here too: compare the squared distance to the closest point with the squared radius.
:::

### The Tank as a Circle

Our tank is a circle (with a radius of 20 pixels). Replace `IsFree` and `TankFits` by this version of `TankFits`, which tests the circle of the tank against every collider:

```cpp source="tmpl8-part-15/2-colliders/game.cpp#L121-L137"
```

And change the two calls in `Move`: `TankFits` now takes a `vec2`:

```cpp source="tmpl8-part-15/2-colliders/game.cpp#L166-L173" {1,5}
```

To see the colliders, add a function that draws a circle (as 32 short lines; the template has no circle function) below the overlap tests:

```cpp source="tmpl8-part-15/2-colliders/game.cpp#L96-L105"
```

And let <kbd>Tab</kbd> show them. Add a variable below the tanks:

```cpp source="tmpl8-part-15/2-colliders/game.cpp#L200-L200"
```

And draw the colliders at the end of `Tick`:

```cpp source="tmpl8-part-15/2-colliders/game.cpp#L265-L277"
```

`screen.Box` draws the outline from the first corner up to *and including* the second one. `max` is the edge just after the last pixel of a box (see [the note in Box - Box](#less-than)), so the last pixel of the box is `max - 1`.

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-15-colliders/index.html" title="The tile map with the colliders drawn as white circles and boxes; the tanks have a yellow circle" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Press <kbd>Tab</kbd> to see the colliders. The tank now gets much closer to the trees. (Click the demo first, so that it gets the keys.)

</div>

The tank now stops at the shape of a tree instead of its tile. But it still *stops*: `TankFits` only lets the tank move along an axis that is completely free, so against a round tree, it jerks or gets stuck instead of sliding around it. Time to do it properly.

## Collision Response

Testing *whether* two shapes overlap is called *collision detection*. Doing something about it is called *collision response*. For a tank that drives into a wall, the most natural response is: move the tank out of the wall, the shortest way. Then, when you drive diagonally into a wall, the tank slides along it.

### The Minimum Translation Vector

For that, we need to know *how far*, and *in which direction*, the tank must move to stop overlapping: the shortest vector that moves it out. That vector is called the *minimum translation vector*, or *MTV*. It's the yellow arrow in the demos above, and it starts at the centre of the blue shape. Like the overlap tests, there is a `PushOut` function for each pair of shapes, which calculates the MTV. The tank is a circle, so it needs two of these functions: one for a circle against a circle, and one for a circle against a box. The third one, box against box, completes the set (you will need it in the assignment). Add these functions below the overlap tests.

#### Circle - Circle

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L96-L108"
```

The shortest way out of another circle is straight away from its centre: along the line through both centres. So the MTV has:

- a *direction*: from the centre of b to the centre of a, as a unit vector. That's `difference / distance`.
- a *length*: the overlap. The circles touch when their centres are `a.radius + b.radius` apart, so they overlap by the sum of the radii minus the distance.

$$
\text{MTV} = \frac{c_a - c_b}{|c_a - c_b|} \, \big( r_a + r_b - |c_a - c_b| \big)
$$

When the circles don't overlap, the overlap is 0 or negative, and the function returns a zero vector: then the push doesn't change anything. And when the centres are exactly on top of each other, there is no direction (and `difference / distance` would divide by zero), so we just pick one: to the right.

#### Box - Box

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L110-L123"
```

Two overlapping boxes can be separated in four ways: move box a to the left, to the right, up, or down, until it just touches box b. The MTV is the shortest of the four:

- To the left, a moves until its right side (`a.max.x`) is at the left side of b (`b.min.x`): that's `b.min.x - a.max.x`, a negative number. To the right, a moves until its left side is at the right side of b: `b.max.x - a.min.x`, a positive number. `x` is the shorter of the two (compared without the minus sign).
- The same goes for up and down: `y` is the shorter way out on the y axis.
- Then the shorter of `x` and `y` wins (`std::abs` gives the value without its sign), and the MTV moves the box along that axis only.

So an MTV between two boxes is always horizontal or vertical. Drag the blue box in the box - box demo above, and see the arrow switch between the axes. The tank doesn't use this function (it's a circle), but you will need it in the assignment.

#### Circle - Box

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L125-L145"
```

This one starts like the overlap test: with the closest point of the box. The shortest way out of the box is straight away from that point:

- the *direction* is from the closest point to the centre of the circle: `difference / distance`.
- the *length* is how far the circle reaches past that point: the radius minus the distance.

Again, the function returns a zero vector when there is no overlap. But there is a special case: when the centre of the circle is *inside* the box, the closest point is the centre itself. The distance is 0, and there is no direction. Then we look at the four sides of the box instead: `left` is the distance from the centre to the left side, `right` to the right side, and so on. The nearest side is the shortest way out: the circle moves out through that side, far enough that its centre passes the side (`left`), and then its radius too (`circle.radius`). In the circle - box demo above, drag the circle into the box, past its centre, and see how the arrow jumps from one side to another.

### Sliding Along Obstacles

Now the tank can use the MTV. Remove `TankFits`, and add a new method `Push` to the `Tank` class:

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L186-L188" {2}
```

`Push` moves the tank out of a collider. Add it below `Move`:

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L216-L227"
```

`Push` moves the tank, and it also changes the velocity. The *normal* is the direction of the push, as a unit vector: it points out of the collider. `velocity.dot(normal)` is the part of the velocity along the normal. When it's negative, the tank drives *into* the collider, and that part is removed. What's left of the velocity points along the collider, and that's why the tank slides.

<iframe class="embed embed-demo embed-velocity-projection" src="/demos/velocity-projection.html" title="A tank touches a wall. Its velocity is split into the part along the normal of the wall and the part along the wall. When the velocity points into the wall, the part along the normal is removed, and the tank slides along the wall." loading="lazy" allowfullscreen></iframe>

<div class="caption">

Drag the blue circle at the tip of the velocity: when the velocity points into the wall, `velocity.dot(normal)` is negative. The red part (along the normal) is removed, and the green part (along the wall) is what's left: the new velocity. Drag the yellow circle on the wall to turn the wall, and *reset* to start over.

</div>

:::note[`<` and the MTV]
Here is the other reason to use `<` in the overlap tests. With `<=`, two shapes that just touch would overlap, but by nothing: their MTV would have length 0. Then every piece of code that reacts to an overlap needs a special case for that. `Push`, for example, divides the push by its length to get the normal: for a push of length 0, that's a division by zero. With `<`, an overlap always comes with a push longer than 0. (`Push` still checks for a length of 0, because `Move` also passes it the pushes of colliders that the tank doesn't overlap at all.)
:::

Then replace `Move` by this version:

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L197-L214" {1,6-11}
```

`Move` moves the tank first, without checking anything. Then it pushes the tank out of every collider that it overlaps. Every push moves the tank, so the circle of the tank is made again for each collider.

### Walls

The tanks also shouldn't leave the map. Instead of a special check, we put four boxes around it: walls, just outside the screen. First, make room for them in the arrays of colliders:

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L68-L68"
```

Then add them to the end of `Init`:

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L268-L273"
```

`W` and `H` are the width and the height of the map in pixels: 768 and 512. Each wall is a box, described by its top-left and its bottom-right corner, like the colliders of the objects:

- The wall *above* the map goes from y = -100 to 0: it ends exactly where the map starts. The wall *below* it starts exactly where the map ends, at y = `H`, and goes 100 pixels further.
- The walls to the *left* and to the *right* do the same on the x axis, and they are exactly as high as the map (from 0 to `H`).
- The walls above and below the map stick out 100 pixels on both sides (from x = -100 to `W + 100`), so that they also cover the corners: together, the four walls leave no gap anywhere around the map.

The tank needs no special code for the walls: they are boxes like any other collider, so `Move` pushes the tank out of them, and it slides along them.

<svg class="memory-diagram walls-diagram" viewBox="-56 -56 880 624" role="img" aria-label="The map of 768 by 512 pixels, with four wall boxes around it, just outside the screen: above the map from y = -100 to 0, below it from y = 512 to 612, to the left from x = -100 to 0, and to the right from x = 768 to 868. Only the part of each wall near the map is shown.">
<image href="/images/cpp-fast-track-15/map.png" x="0" y="0" width="768" height="512"/>
<rect class="wall across" x="-100" y="-100" width="968" height="100"/>
<rect class="wall across" x="-100" y="512" width="968" height="100"/>
<rect class="wall down" x="-100" y="0" width="100" height="512"/>
<rect class="wall down" x="768" y="0" width="100" height="512"/>
<text x="384" y="-22" text-anchor="middle">above the map: y from -100 to 0</text>
<text x="384" y="540" text-anchor="middle">below the map: y from 512 to 612</text>
<text transform="translate(-22 256) rotate(-90)" text-anchor="middle">left: x from -100 to 0</text>
<text transform="translate(790 256) rotate(90)" text-anchor="middle">right: x from 768 to 868</text>
<text class="dim mono" x="4" y="-6">(0, 0)</text>
<text class="dim mono" x="764" y="528" text-anchor="end">(768, 512)</text>
<image class="made-by" href="/images/claude-logo.svg" x="803" y="547" width="16.8" height="16.8" opacity="0.35"><title>Made with Claude</title></image>
</svg>

<div class="caption">

The four walls around the map, each with its own outline. Each wall is a box of 100 pixels thick, just outside the screen: only the part near the map is shown here. The boxes above and below the map are wider than the map, so that they also cover the corners; the boxes to the left and to the right are exactly as high as the map.

</div>

:::note[Why 100 pixels?]
The number 100 is arbitrary: any number that is large enough works, and the walls are outside the screen, so you never see how thick they are. Large enough means: a tank can never get halfway through a wall in a single frame. If it did, the shortest way out (the MTV) would be on the *other* side of the wall, and the tank would be pushed out there: outside the map. A tank moves at most about 5 pixels per frame (0.15 pixels per millisecond, in a frame of at most 33 milliseconds), so 100 pixels is plenty. A faster object, like a bullet, may need thicker walls, or other tricks.
:::

### The Enemy

So far, the enemy drives to the right, and when it leaves the map, it comes back on the left. Its `Update` changes its position directly, so it ignores the colliders: it drives right through the walls. Let's make it drive with `Move`, like the player, so that it bumps into things too. Then it can't leave the map anymore, so it will drive back and forth instead, and turn around when it can't go further. For that, it needs to remember which way it's going: add a variable `patrol` to the `Tank` class (the player's tank doesn't use it):

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L193-L194" {2}
```

Then replace its `Update`:

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L229-L235" {1,4-6}
```

And give the enemy a new starting point: on the map, instead of just outside it:

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L245-L246" {2}
```

Finally, the tanks push each other. Both move half of the overlap, in opposite directions:

```cpp source="tmpl8-part-15/3-sliding/game.cpp#L312-L317" {3-6}
```

:::note[Pushing without an overlap]
These lines run every frame, also when the tanks are far apart. That's safe: when the circles don't overlap, `PushOut` returns a zero vector, and `Push` returns right away when the push has a length of 0. So there is no need to check with `Overlaps` first: `PushOut` already does that. (`Move` uses the same trick: it calls `Push` for *every* collider in the map, and only the ones that overlap the tank actually do something.)
:::

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-15-sliding/index.html" title="The green tank slides along trees, crates and sandbags, and pushes the red tank, which drives back and forth" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The tank slides along obstacles, and the tanks push each other. Drive diagonally into a tree or the sandbags, and try to block the red tank. (Click the demo first, so that it gets the keys.)

</div>

## Bodies

In the last section, the push changed two things: the position, and the velocity. For a tank against a wall, that was simple enough. But when the tank pushes a drum, which pushes another drum into a wall, it gets complicated: which velocity changes, and by how much?

There is a neat way around that. Remember the position from the previous step. After all the pushing, the velocity is simply how far the object *really* moved, divided by the time:

$$
v = \frac{x_{\text{now}} - x_{\text{previous}}}{\Delta t}
$$

Now a push only has to move positions. The velocity follows automatically: a tank that is pushed back by a wall, didn't move (much), so it has no velocity (in that direction). This idea is called *Verlet integration*, after the physicist Loup Verlet, who used it in the 1960s to simulate molecules [\[1\]](#cite-1). In its classic form, it doesn't store a velocity at all:

$$
x_{n+1} = x_n + (x_n - x_{n-1}) + a \, \Delta t^2
$$

The distance of the last step, $x_n - x_{n-1}$, *is* the velocity (times $\Delta t$). We will keep a velocity variable, because the engine and the drag need it, but it's the same thing: when the velocity is calculated from the positions after every step, the result is exactly the formula above.

Moving objects by moving their positions, and calculating the velocity afterwards, is called *position-based dynamics* (*PBD*) [\[2\]](#cite-2). Let's apply it to everything that moves: the tanks, and some drums for them to push around.

### Body

A *body* is a circle that moves. Add this struct below the overlap tests:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L173-L182"
```

- `position` and `velocity` are what the tank had before. `previous` is new: the position before the last step, so that we can calculate the velocity from how far the body moved.
- `drive` and `drag` turn the acceleration of the tank into something every body can have: an engine (`drive`) and friction (`drag`). The drums have no engine, and a smaller drag than the tanks: they slide further.
- `radius` makes the body a circle, for the collisions.
- `inverseMass` is 1 divided by the mass: $w = 1 / m$. A heavy body has a small inverse mass, and a light body a large one.

:::note[Why the inverse mass?]
Why store $1 / m$ instead of the mass itself? Because the physics code only ever needs $1 / m$ (you will see that in `Separate`). And an inverse mass of 0 is handy: it means an *infinite* mass, a body that never moves, like a wall or a post. With the mass itself, that would be a division by zero.
:::

- Every member gets a starting value (`= 0`, and `vec2` starts at 0 by itself), like the queue of [part 13](/cpp-fast-track-13-data-structures/): a new body is at rest, at the origin.

### Collide

A body bumps into the colliders of the map, just like the tank did. Add this function below `Body`:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L184-L191"
```

`Collide` pushes the body out of every collider that it overlaps, with the same `PushOut` functions as before. The colliders don't move, so the body moves the whole way out. Compare it with the `Push` method of the last section: the velocity isn't changed here. That's the point of position-based dynamics: the velocity will be calculated from the positions afterwards.

### Separate

When two bodies overlap, *both* should move. Add this function below `Collide`:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L193-L203"
```

How far each body moves depends on its mass. When a tank (10 times as heavy) bumps into a drum, the drum should move 10 times as far as the tank. With inverse masses $w_a$ and $w_b$, and the MTV `push` (which moves a out of b), body a moves a part $\frac{w_a}{w_a + w_b}$ of the push, and body b the rest, in the other direction:

$$
\Delta x_a = \frac{w_a}{w_a + w_b} \, \text{push}, \qquad \Delta x_b = -\frac{w_b}{w_a + w_b} \, \text{push}
$$

The tank ($w = 0.1$) moves $0.1 / 1.1$ of the overlap, and the drum ($w = 1$) the other $1 / 1.1$. Two equally heavy bodies each move half, like the two tanks did in the last section. And a body with an inverse mass of 0 doesn't move at all: the other body moves the whole way, as if it hit a wall. Only when *both* inverse masses are 0, there is nothing to divide by: then neither body can move, and `Separate` returns right away.

### Tanks and Drums

The tank now gets a `Body`, instead of its own position and velocity. Start with the constants: `Drag` becomes `TankDrag` (the drums get a drag of their own), and a tank gets a mass:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L205-L207" {2-3}
```

The drums will have a mass of 1, so a tank is 10 times as heavy as a drum. Then the class:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L209-L228" {6-9,13-14,16}
```

- `body` replaces `position` and `velocity`. The constructor sets up the body: its position, its drag, its radius (20 pixels, like before) and its inverse mass.
- `Move` becomes `Drive`: the tank no longer moves itself. And `Push` is gone (remove its definition too): the physics will move the tank, and the velocity takes care of itself.
- `Update` no longer needs `deltaTime`: the enemy only decides which way to drive.

`Drive` only sets the `drive` of the body: the `direction` times `Acceleration` (the tank's acceleration). Replace the `Move` method by `Drive`:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L230-L240" {1-4,6-8}
```

The frame of the tank still follows the velocity, but now it's the velocity of the body.

The enemy uses `Drive` too. Change the `Drive` method to this:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L242-L248"
```

The check comes *before* `Drive` now: `body.velocity` is the velocity that the physics calculated in the last frame, after all collisions. When the enemy (almost) didn't move in its patrol direction, it was blocked, and it turns around.

And `Draw` uses the position of the body:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L250-L256" {5-6}
```

Then the drums. Each tile of barrels in the map (tile 7) will become 4 loose drums; `Init` will make them (see [Putting It Together](#putting-it-together)):

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L261-L265"
```

`drumColors` remembers which of the 4 frames of `drums.png` each drum shows:

<img src="/images/cpp-fast-track-15/drums-numbered.png" alt="The 4 frames of drums.png, numbered 0 to 3: a green drum, a red drum, a grey (rusty) drum and a black drum with a red top" width="512" height="128" class="screenshot" />

<div class="caption">

`drums.png` (shown 4 times as large, on grass): 4 frames of 24 × 24 pixels. These are the same 4 barrels as tile 7 of `objects.png`, in the same order: frame 0 is the top-left barrel of the tile, 1 the top-right one, 2 the bottom-left one, and 3 the bottom-right one.

</div>

Finally, an array of *pointers* to all bodies (see [part 8](/cpp-fast-track-8-memory-addresses/)): the tanks, and the drums. With that, one loop can handle the tanks and the drums alike:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L267-L269"
```

### Simulate

And now the physics itself. Add the `Simulate` function below the bodies:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L271-L292"
```

- First, every body remembers where it is (the `previous` position): we'll need that at the end, to calculate the `velocity`. 
- Then it moves: its drive and drag change its velocity, and the velocity moves it (semi-implicit Euler again).
- Then the overlaps are resolved between all pairs of bodies (`j` starts at `i + 1`, so every pair is handled once, and a body is never compared with itself), and between every body and the colliders.

:::caution
Solving one overlap can create another one: a drum that is pushed out of the tank can end up in a wall. So the overlap test is repeated 4 times. Each round, the overlaps get smaller. These rounds are called *iterations*.
:::

- Finally, each velocity is calculated from how far the body actually moved.

### Putting It Together

First, add the sprite of the drums at the top of `game.cpp`, below the other sprites:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L15-L15"
```

Then `Init` turns the barrels of the map (tile 7) into drums, and fills the array of bodies:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L298-L339" {3,7-20,37-41}
```

- When `Init` finds a tile of barrels (tile 7), it makes 4 drums: one for each barrel in the picture.
- `offsets` holds the middle of each barrel within its tile, in the same order as the frames of `drums.png`, so drum `i` gets frame `i`.
- Each drum gets a smaller drag than a tank (so it slides further), a radius of 12 pixels (half of the 24 pixels of its frame), and an inverse mass of 1.
- `drums[drumCount]` is the next free drum, and `drumColors[drumCount++]` stores its frame and then moves on to the next one.
- Then the tile is made empty (-1): the barrels are drums now, so the tile shouldn't be drawn anymore, and it shouldn't get a collider either. That's why this happens *before* the `if (objects[y][x] == -1) continue;`.
- At the end, the array `bodies` gets the addresses of the two tanks and all drums. From now on, the physics only works with these pointers.

And in `Tick`, the tanks drive, the physics moves everything, and the drums are drawn:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L377-L386" {1-9}
```

Show the bodies with the colliders too:

```cpp source="tmpl8-part-15/4-bodies/game.cpp#L398-L399"
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-15-bodies/index.html" title="The green tank pushes drums around; the drums bump into each other and into the obstacles" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Push the drums around: they are light, so the tank barely notices them, but the red tank (just as heavy as yours) is hard to push. (Click the demo first, so that it gets the keys.)

</div>

## Constraints and XPBD

In the last section, we moved bodies to solve overlaps. That's a *constraint*: a rule that the positions must follow. "These two circles may not overlap" is a constraint, and so is "this body stays out of that box". PBD can handle any constraint that you can write as a rule about positions. For example: "these two bodies stay exactly 10 pixels apart". Chain a number of bodies together with that constraint, and you have a rope.

That's what we will build in this section: a *tow cable* for the player's tank. One end of the cable is attached to the tank, and the other end to a drum, so that the tank can drag the drum around the map. In between, the cable is a chain of small, light bodies (the *links*), and each link stays at a fixed distance from its neighbours. But first, we need a better way to solve constraints like that.

Plain PBD has a problem though: how stiff a constraint feels depends on the time step and the number of iterations. Do more iterations, and the rope gets stiffer. In 2016, Miles Macklin, Matthias Müller and Nuttapong Chentanez fixed that with *XPBD* (*extended position-based dynamics*) [\[3\]](#cite-3). XPBD adds one number to every constraint: its *compliance* $\alpha$, the opposite of stiffness. With a compliance of 0, the constraint is solved completely, like before. With a larger compliance, it is solved only partly, so the bodies can get a bit further apart than the rule says: the constraint stretches, like a spring. And it behaves the same, whatever the time step.

For a distance constraint between two bodies $a$ and $b$, with rest length $L$, the *error* is how far the distance is off:

$$
C = |x_b - x_a| - L
$$

XPBD moves both bodies along the line between them, by:

$$
\Delta x_a = w_a \, \frac{C}{w_a + w_b + \alpha / \Delta t^2} \, \mathbf{n}, \qquad
\Delta x_b = -w_b \, \frac{C}{w_a + w_b + \alpha / \Delta t^2} \, \mathbf{n}
$$

where $\mathbf{n}$ is the unit vector from $a$ to $b$, $w$ the inverse masses, and $\Delta t$ the time step. With $\alpha = 0$, the two bodies together move exactly far enough to make $C$ zero: afterwards, their distance is exactly $L$. How that movement is shared between them depends on their masses, just like in `Separate`: body $a$ moves the part $\frac{w_a}{w_a + w_b}$ of the error, and body $b$ the rest. So the lighter body moves the most, and a body with an inverse mass of 0 doesn't move at all. The term $\alpha / \Delta t^2$ makes the correction smaller: the bodies don't move all the way back to a distance of $L$, so a cable can stretch a little when you pull it, like a spring. The larger the compliance, the more it stretches. (The full XPBD method keeps a running total of the corrections over the iterations, called $\lambda$. With the next trick, we don't need it.)

That trick is *substepping*: instead of one step with 4 iterations, the physics does 8 small steps, with one iteration each. In 2019, Miles Macklin, Matthias Müller and their colleagues showed that many small steps are much more accurate than many iterations, for the same amount of work [\[4\]](#cite-4). The time step of one substep is the time of the frame, divided by the number of substeps. In the code, it's called `h` (as in that paper): `h = deltaTime / Substeps`. That's the $\Delta t$ of the formula above.

First the tow cable mentioned at the start of this section. It's a chain of 16 links: light bodies, each held at `linkLength` from its neighbours by a distance constraint: the first one from the player's tank, and the last one from a drum. Add these variables below `bodies` and `bodyCount`:

```cpp source="tmpl8-part-15/game.cpp#L272-L278"
```

Then two new functions, which the new version of `Simulate` (at the end of this section) will use: `KeepDistance` and `Integrate`. Let's start with `KeepDistance`, the function that *solves* a distance constraint: it moves two bodies, so that the rule "stay `restLength` apart" holds (or holds better, when the compliance isn't 0):

```cpp source="tmpl8-part-15/game.cpp#L280-L294"
```

`KeepDistance` is the formula shown above, in code:

- `difference` is the vector from `a` to `b`, and `distance` its length. `direction` is the same vector with length 1: that's $\mathbf{n}$.
- `error` is $C$: how much too long (positive) or too short (negative) the distance is.
- `correction` is the fraction of the formula, without the inverse mass: the error, divided by the sum of the inverse masses plus $\alpha / \Delta t^2$. Here, `h` is $\Delta t$.
- Each body then moves along `direction`, by the correction times its own inverse mass: `a` towards `b` when the distance is too long, and `b` towards `a`.
- When the two bodies are at the same spot, there is no direction (and `difference / distance` would divide by zero); and when both inverse masses are 0, neither body can move. In both cases, the function does nothing.

Next, a function that moves one body during a substep:

```cpp source="tmpl8-part-15/game.cpp#L296-L303"
```

`Integrate` is the first loop of the old `Simulate`, for a single body: it remembers where the body is, and then moves it, with a time step of `h` instead of `deltaTime`. Because it's a function now, the same code moves the tanks, the drums, and the links of the cable.

And finally, `Simulate` itself, with the number of substeps:

```cpp source="tmpl8-part-15/game.cpp#L305-L336" {1-4,7-9,11-14,18-21,24-32}
```

- `h` is the time of one substep: the frame divided into `Substeps` (8) equal parts.
- Each substep does the same as the old `Simulate` did for a whole frame: everything moves (`Integrate`), then the constraints are solved once (the overlaps between the bodies, the links of the cable, and the colliders), and then the velocities follow from the positions.
- The cable is solved link by link: the player's tank with the first link, every link with the next one, and the last link with the drum.
- There are no *iterations* anymore: the substeps take their place.

Add a function to draw the cable (3 pixels wide, by drawing each line a few times, shifted by a pixel):

```cpp source="tmpl8-part-15/game.cpp#L338-L357"
```

The colour of the cable shows its type, so that you can see which one you picked: grey for a steel cable (a compliance of 0), brown for a rope, and red for a bungee cord (a compliance above 1000). The cable is a 3-pixel-wide line through the links: each piece is drawn 3 times next to each other, shifted by one pixel horizontally, and 3 times shifted vertically.

Set up the cable at the end of `Init`: the links start on a straight line between the tank and the drum, and `linkLength` makes the cable exactly long enough:

```cpp source="tmpl8-part-15/game.cpp#L405-L416"
```

And in `Tick`, the keys <kbd>1</kbd>, <kbd>2</kbd> and <kbd>3</kbd> set the compliance, and the cable is drawn before the drums and the tanks:

```cpp source="tmpl8-part-15/game.cpp#L455-L465" {3-8,11}
```

At the end of `Tick`, the links are shown with the colliders, and a line at the top of the screen tells you which keys to use (add `#include <format>` at the top for `std::format`):

```cpp source="tmpl8-part-15/game.cpp#L485-L489"
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-15-cable/index.html" title="The green tank tows a drum on a cable; the keys 1, 2 and 3 make the cable a steel cable, a rope or a bungee cord" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The tow cable. Drive away from the drum, and it follows. Press <kbd>3</kbd> for a bungee cord: the cable stretches, and the drum catches up with a swing. Press <kbd>1</kbd> for a steel cable again. Drive around a tree: the cable wraps around it. (Click the demo first, so that it gets the keys.)

</div>

The cable links are bodies too, so they bump into the trees and boxes (`Collide`): that's why the cable wraps around obstacles. And notice how little code the cable needed: one function for the constraint, and a loop. That's the strength of position-based dynamics: everything is a constraint on positions, and the velocities follow. Cloth is a grid of these distance constraints, and a ragdoll is a few bodies with distance constraints for the bones.

The compliance values (0, 1000 and 5000) have no special meaning. Time is measured in milliseconds, and the masses are numbers we chose ourselves: a drum has a mass of 1, a tank 10, and a link of the cable only 0.1. The links are that light on purpose: a heavy cable would hold the tank back, while a light one is still strong enough to pull the drum along. With made-up masses like these, the compliance has no unit that you could look up, so we simply tried values until the difference was easy to see. While the tank pulls the drum away, the rope stretches about 13%, and the bungee cord about 50%. (Even the steel cable stretches a little, about 1%: with one iteration per substep, the constraints are never solved *exactly*.) Tweaking values like this to get the desired result is very common in game physics.

:::caution[A stretched cable can pass through walls]
Only the links bump into the colliders: each link is a small circle, with a radius of 3 pixels. The pieces of cable *between* the links don't collide with anything. A steel cable keeps its links close together, so that hardly matters. But a bungee cord stretches by about half its length, and then the links are far apart: thin obstacles, like a row of sandbags, fits between two links, and the cable passes right through it.

There are several ways to fix that, from simple to more work:

- Use more links (with a shorter `linkLength`), or links with a larger radius, so that the gaps stay small.
- Limit how far the cable can stretch: when two links get much further apart than `linkLength` (say, twice as far), solve their distance constraint with a compliance of 0 for that step. The cable then stretches like a bungee cord, up to a maximum, and stops there like a steel cable.
- Test the pieces of cable themselves: the line between two links against the circles and boxes, and push both links out when a piece crosses a collider. That's the most work, but it never lets anything through.
:::

:::note[Further reading]
Matthias Müller's [Ten Minute Physics](https://matthias-research.github.io/pages/tenMinutePhysics/) is a series of short videos with small physics simulations, many of them with XPBD. And if you want to know more, the original papers (see the [references](#references) below) are surprisingly readable, especially the ones about XPBD [\[3\]](#cite-3) and substepping [\[4\]](#cite-4).
:::

## Assignment

### Basic

1. Hook the cable to the enemy tank instead of a drum: a tug of war! Then give the enemy a stronger engine, so that it wins.

:::tip[Hint]
For the stronger engine, give `Tank` a member that multiplies its drive.
:::

2. Make oil (tile 12) slippery: a tank on oil has hardly any drag, and its tracks have little grip, so its engine is weak there too.

:::tip[Hint]
Find the tile under the tank, like `IsFree` did in [part 11](/cpp-fast-track-11-tiles/), and change the drag and the drive of its body.
:::

3. Give the two small trees of tile 2 each their own circle, so that the tank can drive into the empty corners of the tile.

:::tip[Hint]
An object can have more than one collider: `objectCircles` could hold two or more circles per object, for example.
:::

If you completed the basic assignment, the solution should look like this:

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-15-assignment-basic/index.html" title="The green tank and the red tank pull at the two ends of the cable; on the oil, the green tank slides" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: a tug of war with the enemy, slippery oil, and a circle for each small tree (press <kbd>Tab</kbd>). (Click the demo first, so that it gets the keys.)

</div>

### Intermediate

1. Fire shells with <kbd>Space</kbd>: a shell is a small, fast body that flies in the direction that the tank faces, knocks drums away, and disappears after a second.

:::tip[Hint]
Make an array of shells, and add them all to `bodies` in `Init`, like the drums. Then *hide* the shells that aren't flying off-screen: set their position (and previous position) to a spot beyond the walls, a different spot for each shell, so that they don't overlap and push each other around. For shell `i`, for example: `{ -(i + 1) * 500.0f, -(i + 1) * 500.0f }`. Out there, they don't touch anything and stand still. To fire a shell, put it in front of the tank and give it a velocity; when its time is up, hide it again. This way, the array of bodies never has to change. Frame `f` of the tank faces the angle `f * 2 * PI / 16`, and $(\sin a, -\cos a)$ is the unit vector for angle $a$ (0 is up). 
:::

2. Press <kbd>C</kbd> to unhook the cable: the cable stays on the tank, and its loose end drags behind it. Press <kbd>C</kbd> again to hook the loose end to the drum that is nearest to the tank (when it is close enough: within the length of the cable).

:::tip[Hint]
When you hook a drum, don't change `linkLength`: the cable should keep its length. Simply move the last link onto the drum, and let the solver pull the rest of the cable along. (Try it with a drum close to the tank, and with the loose end far away. Hold on to your hat.) For the cable, `Separate` already does the work, but be careful: the first link is much closer to the player's tank than its radius.
:::

3. The cable goes right through the tanks and the drums: the links only bump into the static colliders of the map. Make the links bump into the dynamic bodies too, so that the cable wraps around the red tank and the other drums.

:::tip[Hint]
If the links also bumped into the bodies at the *ends* of the cable, the collision would push the first link away, and the cable would pull it back, every substep. So skip those two bodies. Leaving out certain pairs of bodies like this is called *collision filtering*.
:::

If you completed the intermediate assignment, the solution should look like this:

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-15-assignment-intermediate/index.html" title="The green tank fires shells that knock drums around, hooks and unhooks the cable with C, and the cable wraps around the red tank and the drums" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: shells, a cable that you can hook to another drum, and a cable that wraps around the red tank and the drums (press <kbd>Tab</kbd> to see the links). (Click the demo first, so that it gets the keys.)

</div>

### Hard

1. Make the crates movable: each tile of crates (3 and 4) becomes one heavy box that slides, but doesn't rotate. Use the `PushOut` for two boxes for a crate against the boxes of the map and against other crates, and write a version of `Separate` for a body and a crate, by their inverse masses.

2. Put a chain fence across the sandy road: two posts that never move, with a chain of links between them. A tank can push the fence aside, but the fence springs back.

:::tip[Hint]
The fence is just like the cable: bodies with distance constraints, but with a compliance, and fixed at both ends. For the posts, use bodies with an inverse mass of 0. And like the cable in the intermediate assignment, the links must bump into the tanks and the drums (with `Separate`): otherwise they drive right through the fence.
:::

3. In the intermediate assignment, hooking a drum close to the tank makes the drum fly away. Find out why, and fix it: after hooking, the cable should hang slack, and only start pulling the drum when it is straight.

:::tip[Hint]
Why does the drum fly away? The last link jumps onto the drum, so in the next substep, its distance to its neighbour is far too large. The solver fixes that at once: it moves the links and the drum a long way. And then the velocity is calculated from how far they moved, divided by the tiny time step `h`: a correction of 100 pixels becomes a speed of 50 pixels per millisecond. To avoid that, don't make the constraints jump: when you hook a drum, put all the links between the tank and the drum yourself, with every link `linkLength` from its neighbours. The drum is usually closer than the length of the cable, so they don't fit on a straight line: lay them out in a zigzag. (Squeezing them together on a straight line doesn't work either: then the constraints push them apart, and shoot the drum away.)
:::

If you completed the hard assignment, the solution should look like this:

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-15-assignment-hard/index.html" title="The green tank pushes heavy crates, drives through a chain fence that springs back, and hooks a drum with a slack cable" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: heavy crates that slide, a chain fence across the road, and a cable that hooks a drum without making it fly (<kbd>C</kbd> to unhook and hook). (Click the demo first, so that it gets the keys.)

</div>

Once you have completed these, you may continue with the next part.

## References

<span id="cite-1"></span>\[1\] L. Verlet, ‘Computer "Experiments" on Classical Fluids. I. Thermodynamical Properties of Lennard-Jones Molecules’, Physical Review, vol. 159, no. 1, p. 98, 1967. [doi:10.1103/PhysRev.159.98](https://doi.org/10.1103/PhysRev.159.98)

<span id="cite-2"></span>\[2\] M. Müller, B. Heidelberger, M. Hennix and J. Ratcliff, ‘Position Based Dynamics’, in Virtual Reality Interactions and Physical Simulations (VRIPhys), 2006. [PDF](https://matthias-research.github.io/pages/publications/posBasedDyn.pdf)

<span id="cite-3"></span>\[3\] M. Macklin, M. Müller and N. Chentanez, ‘XPBD: Position-Based Simulation of Compliant Constrained Dynamics’, in Motion in Games (MIG), 2016. [PDF](https://matthias-research.github.io/pages/publications/XPBD.pdf)

<span id="cite-4"></span>\[4\] M. Macklin, K. Storey, M. Lu, P. Terdiman, N. Chentanez, S. Jeschke and M. Müller, ‘Small Steps in Physics Simulation’, in Symposium on Computer Animation (SCA), 2019. [PDF](https://matthias-research.github.io/pages/publications/smallsteps.pdf)
