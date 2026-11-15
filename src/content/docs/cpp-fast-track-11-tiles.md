---
title: "C++ Fast Track for Games Programming Part 11: Tiles"
description: "Tile maps: build 2D game worlds from small tiles, a simple and efficient technique for 2D graphics."
slug: cpp-fast-track-11-tiles
lastUpdated: 2026-11-15
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

It is time to introduce you to a great concept for 2D graphics, which you will find very useful and easy to use: **tile maps**. In this part, you will build a small battlefield out of tiles, and drive a tank over it.

## Getting the Stuff You Need

For this tutorial, you will be using the same template as in the [2nd tutorial](/cpp-fast-track-2-template/):

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

As usual, we will start with a fresh copy of the template: extract the zip file to `C:\Projects\C++ Fast Track`, and rename the `tmpl8-main` folder to `Part 11`.

You will also need the images for this part:

[<img src="/images/zip1.png" alt="Download the images for part 11 as a zip file" width="120" class="thumb" />cpp-fast-track-11-assets.zip](/downloads/cpp-fast-track-11-assets.zip)

Extract this zip file to the `assets` folder in `C:\Projects\C++ Fast Track\Part 11`. It contains `terrain.png` (the tiles), `objects.png` (things to put on the tiles), and `tank.png` and `barrel.png` (a tank, and its barrel). The images are made from [Top-down Tanks Remastered](https://kenney.nl/assets/top-down-tanks-remastered) by Kenney, who shares them for free (under the [CC0](https://creativecommons.org/publicdomain/zero/1.0/) license, so you can use them in your own games too).

Then open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 11` folder.

## Tile Maps

Have a look at `terrain.png`:

<img src="/images/cpp-fast-track-11/terrain-numbered.png" alt="The terrain tile sheet: 40 tiles of grass, sand and roads, in 4 rows of 10, numbered 0 to 39" width="640" height="256" class="screenshot" />

<div class="caption">

`terrain.png`: 40 tiles of 64 × 64 pixels, with their numbers. (The numbers and the white lines are not in the image itself.)

</div>

This is a *tile sheet*: a single image with many small images (the *tiles*), all of the same size, in a grid. With these 40 tiles you can build a large world: put them next to each other, and the grass, the sand and the roads continue from one tile to the next.

To build a world out of tiles, we give each tile a number: count from the top-left, row by row. The first row has tiles 0 to 9, the second row 10 to 19, and so on. A *tile map* is then simply a grid of numbers: one number for each place in the world.

```cpp
{  0,  0, 10,  1,  0 },
{  2,  2,  2, 12,  2 },
{  0, 10,  0,  1,  0 },
```

This bit of map is grass (0 and 10), with a road from left to right (2), and a road from top to bottom (1) that crosses it (12).

This is how the tile editors that game developers use work too: in [Tiled](https://www.mapeditor.org/) and [LDtk](https://ldtk.io/) you paint a map with the mouse, but what they save is a grid of tile numbers for every layer (we will get to layers in a moment). Once you know how to draw a grid of numbers, you can draw the maps that those editors make.

## My First Tile Map

Replace the code in `game.cpp` by this:

```cpp source="tmpl8-part-11/1-ground/game.cpp" {7-36,57-60}
```

The map is 12 tiles of 64 pixels wide: 768 pixels. The screen of the template is 800 pixels wide, which would leave a black strip of 32 pixels on the right. For this part, let's make the screen exactly as large as the map. The size of the screen is set in `template.hpp`, in the *template code* folder (see [part 2](/cpp-fast-track-2-template/)). Change the width to 768:

```cpp source="tmpl8-part-11/tmpl8/template.hpp#L10-L12" {2}
```

The window and the screen surface follow these numbers, so this is all you need to change. Now run the program, and you will see a world of grass, sand and roads:

<img src="/images/cpp-fast-track-11/01-ground.png" alt="A map of grass at the top and sand at the bottom, with two grey roads that become sandy roads, and a sandy road from left to right" width="768" height="512" class="screenshot" />

<div class="caption">

The tile map: 12 × 8 tiles, on a screen of 768 × 512 pixels.

</div>

Let's go over the details.

-   `ground` is a *two-dimensional* array: an array of 8 rows, where each row is an array of 12 numbers. The first index is the row, the second index is the column, so the tile in column `x` of row `y` is `ground[y][x]`. That is the other way round from what you might expect (we usually write *x* first), but it makes the code look like the map: each line of numbers is a row of the map.
-   The constants `TileSize`, `SheetColumns`, `MapWidth` and `MapHeight` (remember `constexpr` from [part 10](/cpp-fast-track-10-arrays/)) give names to the numbers. You can make the map larger by changing `MapWidth` and `MapHeight` (and adding numbers to `ground`, of course).
-   `Tick` loops over the rows (`y`) and the columns (`x`) of the map, and draws tile `ground[y][x]` at `x * TileSize`, `y * TileSize`. The tile in column $x$ and row $y$ starts at pixel

    $$
    (p_x, p_y) = (x \times 64,\ y \times 64)
    $$

    For example, the tile in row 2 and column 3 starts at pixel

    $$
    (p_x, p_y) = (3 \times 64,\ 2 \times 64) = (192, 128)
    $$

    :::note
    Rows and columns are counted from 0, just like the elements of an array: the first row is row 0, and the first column is column 0. So row 2 is the third row, and column 3 is the fourth column.
    :::
-   `DrawTile` copies the pixels of a tile. First it finds the column and the row of the tile in the tile sheet: tile 23 is in row `23 / 10 = 2` (an integer division drops the fraction), and in column `23 % 10 = 3` (the remainder; you saw `%` in [part 10](/cpp-fast-track-10-arrays/)). Then it uses pointers (from [part 8](/cpp-fast-track-8-memory-addresses/)): `src` points to the top-left pixel of the tile in the tile sheet, and `dst` to the pixel on the screen where it goes. The loop copies one line of 64 pixels, and then moves both pointers down one line. `GetPitch()` is the number of pixels from one line to the next: 640 for the tile sheet, and 800 for the screen.

## Layers

Grass and roads alone make a rather empty world. Let's add trees, crates, sandbags and barrels. Have a look at `objects.png`:

<img src="/images/cpp-fast-track-11/objects-numbered.png" alt="The object sheet: 16 objects in 2 rows of 8, numbered 0 to 15: trees, crates, sandbags, barrels, barricades, oil and twigs" width="512" height="128" class="screenshot" />

<div class="caption">

`objects.png`: 16 objects, each in a square of 64 × 64 pixels. (Shown on grass: around the objects, the image is transparent.)

</div>

We could make tiles of a tree on grass, a tree on sand, a crate on grass, a crate on a road, and so on, but that would be a lot of tiles. Instead, we put the objects in a second map, on top of the first one: a second *layer*. Tile editors like Tiled and LDtk work with layers too: a map has a number of layers, and each layer is a grid of tile numbers.

There is one difference with the ground: most places don't have an object. We use the number `-1` for "nothing here". Add the highlighted lines:

```cpp source="tmpl8-part-11/2-objects/game.cpp#L11-L34" {2,15-24}
```

And draw the object on top of each tile, if there is one:

```cpp source="tmpl8-part-11/2-objects/game.cpp#L67-L80" {6,8-13}
```

<img src="/images/cpp-fast-track-11/02-objects.png" alt="The same map, now with trees, crates, sandbags, barrels, barricades, oil and twigs" width="768" height="512" class="screenshot" />

<div class="caption">

The map with two layers: the ground, and the objects on top of it.

</div>

The objects are drawn with a `Sprite` instead of `DrawTile`, for a good reason: around a tree, the image is transparent. `DrawTile` copies *every* pixel, which would give every tree a black square. A sprite skips the transparent pixels, so the ground stays visible around the tree. (Every pixel has an *alpha* value next to its red, green and blue: 0 is fully transparent, 255 is fully opaque. A sprite draws the pixels with an alpha of at least 128, and skips the others.)

Remember from [part 4](/cpp-fast-track-4-sprites-and-loops/) that a sprite can have several frames? `objects.png` is a sprite with 8 columns and 2 rows of frames, and the frames are numbered just like the tiles: row by row. So `SetFrame(objects[y][x])` picks the right object.

## Player

Now that we have a world, we need a player to explore it. `tank.png` has 16 frames of the tank, one for each direction: frame 0 points up, frame 4 points right, frame 8 down, and frame 12 left. `barrel.png` has the barrel, in the same 16 directions. Add the sprites, and a position and a frame for the tank:

```cpp source="tmpl8-part-11/3-tank/game.cpp#L13" {1}
```

```cpp source="tmpl8-part-11/3-tank/game.cpp#L37-L38" {1-2}
```

And let the arrow keys (or <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd> and <kbd>D</kbd>) drive it, at the end of `Tick`:

```cpp source="tmpl8-part-11/3-tank/game.cpp#L85-L94" {1-10}
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-11-tank/index.html" title="A green tank on the tile map, which drives with the arrow keys or W, A, S and D" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Drive the tank with the arrow keys, or with <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd> and <kbd>D</kbd>. (Click the demo first, so that it gets the keys.)

</div>

`GetKey` is `true` as long as a key is held down (you used `GetKeyPressed` in [part 6](/cpp-fast-track-6-floats/), which is only `true` in the first frame). `||` means *or*: the condition is true when the value on at least one side of it is true, so either key drives the tank. (It's the counterpart of `&&`, *and*, from [part 5](/cpp-fast-track-5-conditions/).) The speed is in pixels per millisecond, so the tank drives equally fast at any frame rate (remember `deltaTime` from [part 6](/cpp-fast-track-6-floats/)?): 0.15 pixels per millisecond is 150 pixels per second.

`tankX` and `tankY` are the *middle* of the tank. The frames of the tank are 64 × 64 pixels, with the tank in the middle, so we draw the sprite 32 pixels to the left of and above that point. The barrel is drawn at the same place, on top of the tank: the middle of its frames is the middle of the turret, so it fits in every direction.

:::note
Hold two keys at once, say up and right, and the tank moves `speed` pixels up *and* `speed` pixels to the right in the same frame. That is a diagonal step of

$$
\sqrt{\text{speed}^2 + \text{speed}^2} = \sqrt{2} \times \text{speed} \approx 1.41 \times \text{speed}
$$

so the tank drives about 41% faster diagonally than straight. Many games have this bug! To fix it, first work out the direction from the keys (for example `dx` and `dy`, each `-1`, `0` or `1`), and when both are not `0`, multiply them by $\frac{1}{\sqrt{2}} \approx 0.707$ before you multiply them by the speed. (More generally: divide the direction by its length, $\sqrt{dx^2 + dy^2}$, so that it always has a length of 1. This is called *normalizing* a vector.)
:::

## Obstacles

The tank drives right through the trees. One of the nicest things about a tile map is that it can tell you more than what a place looks like: it can also tell you whether the tank can go there.

Which objects block the tank? Trees, crates, sandbags, barrels and barricades do, but the tank can drive over the oil and the twigs. That is a property of the *kind* of object, not of a place in the map: every tree blocks the tank. So we store it once for each object, in an array with an element for each of the 16 objects (a tile editor lets you set *properties* for each tile of a tile sheet in the same way):

```cpp source="tmpl8-part-11/game.cpp#L37-L41" {1-5}
```

The tank is not a single point: we need to make sure that *no part* of the tank drives into an obstacle. An easy way to do that is to treat the tank (for collisions) as a square of 36 × 36 pixels around its middle, and check its four corners. Add these two functions, below `DrawTile`:

```cpp source="tmpl8-part-11/game.cpp#L60-L74" {1-15}
```

`IsFree` turns a position in pixels into a place in the map: divide by the size of a tile. A tank at pixel 200 is in column `200 / 64 = 3`. The place is free when there is no object (`-1`), or when the object is not solid. And outside the map, it's never free: that keeps the tank on the map.

Four corners are enough, because the square of the tank (36 pixels) is smaller than a tile (64 pixels): an obstacle can't fit between two corners. (For a larger object, you would also check points in between.)

Now change the movement code in `Tick` like this:

```cpp source="tmpl8-part-11/game.cpp#L107-L114" {2-8}
```

Instead of moving the tank right away, we first compute where it *would* go (`newX` and `newY`), and only move it there when it fits. We check the horizontal and the vertical movement separately: when you drive diagonally into a tree, the tank can't go up, but it can still go to the right, so it slides along the tree instead of getting stuck.

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-11-obstacles/index.html" title="The tank on the tile map, which stops at trees, crates, sandbags and barrels" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The tank stops at obstacles, but drives over the oil and the twigs. (Click the demo first, so that it gets the keys.)

</div>

:::note
Drive along the sandbags or the small trees, and you will see that the tank sometimes stops before it touches them. That is because the *whole* tile of a solid object is solid, also the transparent part around the object: the tank collides with the invisible part of the tile. Most games do this differently: each obstacle has a *collider*, a simple shape such as a box or a circle that fits the object (a small circle for the trunk of a tree, a long box for a row of sandbags), and the game checks whether the collider of the tank overlaps the collider of an obstacle. Collision detection between shapes is a topic of its own, so in this part we keep it simple. In [part 15](/cpp-fast-track-15-physics/), you will learn how to check whether two boxes, two circles, or a box and a circle overlap.
:::

Besides obstacles, a tile map can store all kinds of information:

-   tiles that hurt or destroy the tank, such as mines;
-   tiles with a bonus, such as fuel or ammunition;
-   tiles that move the tank in a certain direction, such as conveyor belts or ice;
-   tiles that slow the tank down, such as mud or sand (see the assignment).

## Further Improvements

There are many things that could improve the code we have so far:

-   Typing the numbers of a map by hand is fine for a small map, but not for a large one. That's what tile editors like [Tiled](https://www.mapeditor.org/) and [LDtk](https://ldtk.io/) are for. They can export the layers of a map as a text file of numbers (a *CSV* file), which your game could read: in [part 14](/cpp-fast-track-14-fileio/), you will learn how to read files.
-   The tile map is spread over a number of global variables and functions. It would be much nicer to keep the tiles, the map and its functions together, separated from the rest of the game. This is something we will deal with in the [next part](/cpp-fast-track-12-classes/).

## Assignment

The assignment for this tutorial comes in two difficulty settings. The regular difficulty tests your ability to make minor changes to the code. The hard difficulty will take more time and effort to complete. Which difficulty are you able to achieve?

### Regular

1.  The ground can have properties too. Make the tank drive at full speed on roads, a bit slower on grass, and even slower on sand. (Hint: look up the ground tile under the middle of the tank, and multiply the speed by a factor that depends on that tile.)
2.  Draw the collision box of the tank with `screen.Box`. Does the square fit the tank in every direction? Try a few sizes, and find the size that feels best when you drive along the obstacles.
3.  Make the tank drive diagonally just as fast as straight (see the [note](#player) at the end of the Player section).

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-11-assignment-regular/index.html" title="The tank with a yellow collision box, which drives fast on roads and slowly on sand, and equally fast in every direction" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: the tank drives fast on the roads and slowly on the sand, shows its collision box, and drives diagonally just as fast as straight.

</div>

### Hard

1.  Use all 16 frames of the tank: when it changes direction, let it turn one frame at a time, and only drive when it faces the right direction.
2.  Let the barrel point towards the mouse (store the mouse position like in [part 10](/cpp-fast-track-10-arrays/)), but let it follow the mouse with a slight delay: one frame at a time. (Hint: `std::atan2` from `<cmath>` gives the angle from the tank to the mouse.)
3.  Make the map larger than the screen, and scroll it: keep the tank in the middle of the screen, until it gets to an edge of the map. Make sure that tiles that are partly outside the screen are drawn correctly: `DrawTile` doesn't check that yet.

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-11-assignment-hard/index.html" title="A larger map that scrolls with the tank; the tank turns in steps, and its barrel follows the mouse" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: a larger map that scrolls with the tank. The tank turns in steps, and its barrel follows the mouse. (Click the demo first, so that it gets the keys.)

</div>

Once you have completed these, you may continue with the next part.
