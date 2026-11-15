---
title: "C++ Fast Track for Games Programming Part 13: Data Structures"
description: "Data structures: a snake game that shows where plain arrays fall short, and how a queue class and a grid make the code faster and simpler."
slug: cpp-fast-track-13-data-structures
lastUpdated: 2026-11-15
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

You are quite far into the C++ Fast Track for Games Programming series. If you got here in good shape, you learned a lot: you went through the basics of C++ programming, and got a taste of classes as well. In the meantime, you experimented with quite a few game-related concepts. In the upcoming parts, you will expand your knowledge further, with file I/O, physics and AI. But first: let's get acquainted with the wonderful world of *data structures*. We will do that with a classic: a game of snake.

## Getting Started

For this tutorial, you will be using the same template as in the [2nd tutorial](/cpp-fast-track-2-template/):

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

As usual, we will start with a fresh copy of the template: extract the zip file to `C:\Projects\C++ Fast Track`, and rename the `tmpl8-main` folder to `Part 13`. You will also need the sprites for the snake:

[<img src="/images/zip1.png" alt="Download the snake sprites as a zip file" width="120" class="thumb" />cpp-fast-track-13-assets.zip](/downloads/cpp-fast-track-13-assets.zip)

Extract this zip file to the `assets` folder in `C:\Projects\C++ Fast Track\Part 13`. It contains `snake.png`, with all the tiles for the game:

<img src="/images/cpp-fast-track-13/snake-numbered.png" alt="The snake sprite sheet: 16 tiles of 32 by 32 pixels: grass, grass with flowers, a wall, an apple, two body tiles, the head in four directions, and the head of a dead snake in four directions" width="512" height="128" class="screenshot" />

<div class="caption">

`snake.png` (shown twice as large): grass (0 and 1), a wall (2), an apple (3), the body of the snake (4 and 5), its head (8 to 11: up, right, down, left), and the head of a dead snake (12 to 15). You may use these tiles in your own games too: they are free (CC0).

</div>

Then open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 13` folder.

## A Snake

The board is a grid of 25 × 16 cells of 32 × 32 pixels: exactly the 800 × 512 pixels of the screen. Replace the code in `game.cpp` by this:

```cpp source="tmpl8-part-13/1-array/game.cpp" {7-39,60-83}
```

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-13-array/index.html" title="A snake of eight cells that moves to the right, and turns with the arrow keys or W, A, S and D" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The snake: steer with the arrow keys, or with <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd> and <kbd>D</kbd>. The walls don't do anything yet: the snake drives through them, and when it leaves the board on one side, it comes back on the other side. (Click the demo first, so that it gets the keys.)

</div>

That's pretty short, isn't it? It does need some explanation though.

-   A cell of the board has two numbers: a column `x` and a row `y`. Instead of two arrays (one for the `x`'s and one for the `y`'s), we keep them together in a `struct`. A `struct` is a class whose members are public by default: a handy way to group a few variables that belong together. `snake` is an array of 8 cells: element 0 is the tail, and element 7 is the head.
-   `heading` is the direction of the snake: 0 is up, 1 is right, 2 is down and 3 is left (clockwise, like the tank frames in [part 11](/cpp-fast-track-11-tiles/)). `directions` holds the step in `x` and `y` for each heading: `directions[1]` is `{ 1, 0 }`, one cell to the right.
-   The keys choose a direction, but a snake can't turn around: when it moves to the right, left does nothing (and right does nothing either: it already goes that way). Only a turn to the side is allowed. Up and down (0 and 2) are even numbers, and left and right (1 and 3) are odd, so `wanted % 2 != lastStep % 2` means "a turn to the side". Why `lastStep`, and not `heading`? Press up and then left quickly, before the snake takes its next step: compared with `heading`, both turns are allowed, and the snake would turn around into itself. `lastStep` is the direction in which the snake really moved.
-   The snake moves one cell every 100 milliseconds: `timer` adds up the time of the frames, and every time it reaches 100, the snake takes a step. This way, the game keeps drawing at full speed, and the snake moves at the same speed on every computer. And because a step is a whole cell, you have time to press a key before the next step.
-   To take a step, every cell of the snake moves to the next one: the tail takes the place of the cell before it, and so on, until the head, which moves one cell in the direction of `heading`. When the head leaves the board, the `%` brings it back on the other side. The `+ BoardWidth` keeps the number from being negative: in C++, `-1 % 25` is `-1`, not 24 (remember [part 10](/cpp-fast-track-10-arrays/)?). We will replace that with walls later.
-   `DrawCell` draws one tile of `snake.png` (a sprite with 8 columns and 2 rows of frames, like the objects in part 11) in a cell of the board.

## The Problem with Arrays

This code uses an array to store the snake. At first, this seems like a logical choice. But think big: what happens when the snake is hundreds of cells long? Every step, the program copies every cell to the next one. That's a lot of work, and it's work for nothing: most of the snake doesn't change at all. Only the head moves to a new cell, and the tail leaves one. The rest of the body only *seems* to slide.

## A Ring Buffer

Imagine this: instead of always keeping the tail in element 0, we let the snake start anywhere in the array. We keep track of where it starts (the *front*), and how many cells it has (the *count*). When the snake reaches the end of the array, it simply continues at element 0 again: the array is used as a ring. That's why this is called a *ring buffer*.

Now a step is easy: put the new head after the last cell, and move the front one place, so that the old tail is no longer part of the snake. The cells in between stay exactly where they are:

<svg class="memory-diagram" viewBox="0 0 700 442" role="img" aria-label="A ring buffer of 8 places in three steps: the snake in places 2 to 6; after a step in places 3 to 7; and after one more step wrapped around, in places 4, 5, 6, 7 and 0, with an arrow from place 7 to place 0">
<defs><marker id="ring-arrow" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse"><path d="M0 0 10 5 0 10z" class="arrow-head"/></marker></defs>
<text class="dim mono" x="153.0" y="24" text-anchor="middle">0</text>
<text class="dim mono" x="219.0" y="24" text-anchor="middle">1</text>
<text class="dim mono" x="285.0" y="24" text-anchor="middle">2</text>
<text class="dim mono" x="351.0" y="24" text-anchor="middle">3</text>
<text class="dim mono" x="417.0" y="24" text-anchor="middle">4</text>
<text class="dim mono" x="483.0" y="24" text-anchor="middle">5</text>
<text class="dim mono" x="549.0" y="24" text-anchor="middle">6</text>
<text class="dim mono" x="615.0" y="24" text-anchor="middle">7</text>
<text class="mono" x="20" y="72">front = 2</text>
<text class="mono" x="20" y="94">count = 5</text>
<rect class="ptr" x="123" y="54" width="60" height="50" rx="4"/>
<rect class="ptr" x="189" y="54" width="60" height="50" rx="4"/>
<rect class="var" x="255" y="54" width="60" height="50" rx="4"/>
<text class="value" x="285.0" y="85" text-anchor="middle">tail</text>
<rect class="var" x="321" y="54" width="60" height="50" rx="4"/>
<text class="value" x="351.0" y="85" text-anchor="middle">body</text>
<rect class="var" x="387" y="54" width="60" height="50" rx="4"/>
<text class="value" x="417.0" y="85" text-anchor="middle">body</text>
<rect class="var" x="453" y="54" width="60" height="50" rx="4"/>
<text class="value" x="483.0" y="85" text-anchor="middle">body</text>
<rect class="var" x="519" y="54" width="60" height="50" rx="4"/>
<text class="value" x="549.0" y="85" text-anchor="middle">head</text>
<rect class="ptr" x="585" y="54" width="60" height="50" rx="4"/>
<text class="dim" x="285.0" y="124" text-anchor="middle">↑ front</text>
<text class="dim" x="120" y="144">The snake: the tail in place 2, the head in place 6.</text>
<text class="mono" x="20" y="204">front = 3</text>
<text class="mono" x="20" y="226">count = 5</text>
<rect class="ptr" x="123" y="186" width="60" height="50" rx="4"/>
<rect class="ptr" x="189" y="186" width="60" height="50" rx="4"/>
<rect class="ptr" x="255" y="186" width="60" height="50" rx="4"/>
<rect class="var" x="321" y="186" width="60" height="50" rx="4"/>
<text class="value" x="351.0" y="217" text-anchor="middle">tail</text>
<rect class="var" x="387" y="186" width="60" height="50" rx="4"/>
<text class="value" x="417.0" y="217" text-anchor="middle">body</text>
<rect class="var" x="453" y="186" width="60" height="50" rx="4"/>
<text class="value" x="483.0" y="217" text-anchor="middle">body</text>
<rect class="var" x="519" y="186" width="60" height="50" rx="4"/>
<text class="value" x="549.0" y="217" text-anchor="middle">body</text>
<rect class="var" x="585" y="186" width="60" height="50" rx="4"/>
<text class="value" x="615.0" y="217" text-anchor="middle">head</text>
<text class="dim" x="351.0" y="256" text-anchor="middle">↑ front</text>
<text class="dim" x="120" y="276">One step later: PushBack fills place 7, PopFront frees place 2.</text>
<text class="mono" x="20" y="336">front = 4</text>
<text class="mono" x="20" y="358">count = 5</text>
<rect class="var" x="123" y="318" width="60" height="50" rx="4"/>
<text class="value" x="153.0" y="349" text-anchor="middle">head</text>
<rect class="ptr" x="189" y="318" width="60" height="50" rx="4"/>
<rect class="ptr" x="255" y="318" width="60" height="50" rx="4"/>
<rect class="ptr" x="321" y="318" width="60" height="50" rx="4"/>
<rect class="var" x="387" y="318" width="60" height="50" rx="4"/>
<text class="value" x="417.0" y="349" text-anchor="middle">tail</text>
<rect class="var" x="453" y="318" width="60" height="50" rx="4"/>
<text class="value" x="483.0" y="349" text-anchor="middle">body</text>
<rect class="var" x="519" y="318" width="60" height="50" rx="4"/>
<text class="value" x="549.0" y="349" text-anchor="middle">body</text>
<rect class="var" x="585" y="318" width="60" height="50" rx="4"/>
<text class="value" x="615.0" y="349" text-anchor="middle">body</text>
<text class="dim" x="417.0" y="388" text-anchor="middle">↑ front</text>
<text class="dim" x="120" y="408">One more step: after place 7, the head goes on in place 0.</text>
<path class="arrow" d="M 615 316 Q 384 278 157 315" marker-end="url(#ring-arrow)"/>
<g class="made-by" transform="translate(679 421) scale(0.7)" opacity="0.35"><title>Made with Claude</title><path d="M4.709 15.955l4.72-2.647.08-.23-.08-.128H9.2l-.79-.048-2.698-.073-2.339-.097-2.266-.122-.571-.121L0 11.784l.055-.352.48-.321.686.06 1.52.103 2.278.158 1.652.097 2.449.255h.389l.055-.157-.134-.098-.103-.097-2.358-1.596-2.552-1.688-1.336-.972-.724-.491-.364-.462-.158-1.008.656-.722.881.06.225.061.893.686 1.908 1.476 2.491 1.833.365.304.145-.103.019-.073-.164-.274-1.355-2.446-1.446-2.49-.644-1.032-.17-.619a2.97 2.97 0 01-.104-.729L6.283.134 6.696 0l.996.134.42.364.62 1.414 1.002 2.229 1.555 3.03.456.898.243.832.091.255h.158V9.01l.128-1.706.237-2.095.23-2.695.08-.76.376-.91.747-.492.584.28.48.685-.067.444-.286 1.851-.559 2.903-.364 1.942h.212l.243-.242.985-1.306 1.652-2.064.73-.82.85-.904.547-.431h1.033l.76 1.129-.34 1.166-1.064 1.347-.881 1.142-1.264 1.7-.79 1.36.073.11.188-.02 2.856-.606 1.543-.28 1.841-.315.833.388.091.395-.328.807-1.969.486-2.309.462-3.439.813-.042.03.049.061 1.549.146.662.036h1.622l3.02.225.79.522.474.638-.079.485-1.215.62-1.64-.389-3.829-.91-1.312-.329h-.182v.11l1.093 1.068 2.006 1.81 2.509 2.33.127.578-.322.455-.34-.049-2.205-1.657-.851-.747-1.926-1.62h-.128v.17l.444.649 2.345 3.521.122 1.08-.17.353-.608.213-.668-.122-1.374-1.925-1.415-2.167-1.143-1.943-.14.08-.674 7.254-.316.37-.729.28-.607-.461-.322-.747.322-1.476.389-1.924.315-1.53.286-1.9.17-.632-.012-.042-.14.018-1.434 1.967-2.18 2.945-1.726 1.845-.414.164-.717-.37.067-.662.401-.589 2.388-3.036 1.44-1.882.93-1.086-.006-.158h-.055L4.132 18.56l-1.13.146-.487-.456.061-.746.231-.243 1.908-1.312-.006.006z" fill="#D97757"/></g>
</svg>

<div class="caption">

A ring buffer with 8 places, holding a snake of 5 cells, in three steps. In every step, the head goes in the next place, and the front moves one place. After place 7 (the end of the array), the head goes on in place 0.

</div>

The arithmetic for this is a bit tricky, with the `% Capacity` to wrap around. That's exactly the kind of code that you don't want in the middle of your game. So let's write it once, in a class, and then never think about it again: a *queue* of cells. A queue is like the line at a cash register: you join it at the back, and you leave it at the front. That's exactly what a snake does: the head joins at the back of the queue, and the tail leaves at the front.

:::note
This may feel backwards: the *front* of the queue is the *tail* of the snake, and the head of the snake is at the *back* of the queue. That's because "front" and "back" are the usual names for the ends of a queue (the C++ standard library uses them too): the front is where things leave, the back is where they join. A snake grows at its head and shrinks at its tail, so its head is where cells join the queue: at the back.
:::

```cpp source="tmpl8-part-13/2-queue/game.cpp#L19-L54"
```

A few things are new here:

-   `private:` In [part 12](/cpp-fast-track-12-classes/), all the members of the `Tank` were `public`: the rest of the program could use them. The members after `private:` can only be used by the methods of the class itself. Nobody else can change `front` or `count`, and break the ring. To use the queue, you *have to* use its methods, and they always keep the ring correct. This is called *encapsulation*.
-   `int front = 0;` gives a member variable its first value right where it's declared. Every queue starts empty, without the need for a constructor.
-   `static constexpr int Capacity` is a constant that belongs to the class: the same for every queue. The queue can never hold more cells than the board has, so `BoardWidth * BoardHeight` is enough.
-   `const` after a method (like `At`, `Back` and `Size`) promises that the method doesn't change the object: it only looks. The compiler checks that promise, and it lets you call these methods on a queue that may not be changed (a `const` queue, for example a `const CellQueue&` parameter). Give every method that only looks a `const`.
-   `At(i)` hides the ring: `At(0)` is always the front (the tail of the snake), and `At(Size() - 1)` is always the back (the head), wherever they are in the array.
-   The methods guard the ring. `PushBack` on a full queue would overwrite the front, so it adds nothing, and returns `false` to tell you so. `PopFront` on an empty queue would make `count` negative, so it does nothing. And `At` only gives you cells that are in the queue: asking for another cell is a bug in the code that calls it, and the `assert` (remember [part 7](/cpp-fast-track-7-debugging/)?) stops the program right there, in a Debug build. (Add `#include <cassert>` at the top of `game.cpp`.) `Back()` uses `At`, so it's checked too: an empty queue has no back.

## A Growing Snake

Now let's use the queue: replace the `snake` array, `Step`, and the code that starts the game:

```cpp source="tmpl8-part-13/2-queue/game.cpp#L56-L110"
```

Read `Step` out loud: take the cell after the head. If there's a wall or a part of the snake, the snake dies. Otherwise, the tail leaves (`PopFront`), and the head moves on (`PushBack`). Nothing else moves, however long the snake is.

And look at the apple: when the snake eats it, we simply *don't* take off the tail. The snake grows by one cell, for free. With the array, that would have been a lot harder: the array has a fixed size.

`Init` starts the game, and `Tick` uses the queue to draw the snake, and restarts the game with <kbd>Space</kbd> after the snake died:

```cpp source="tmpl8-part-13/2-queue/game.cpp#L121-L124" {3}
```

```cpp source="tmpl8-part-13/2-queue/game.cpp#L138-L166" {1-17,26-29}
```

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-13-queue/index.html" title="The snake game: the snake eats apples and grows, and dies when it hits a wall or itself" loading="lazy" allowfullscreen></iframe>

<div class="caption">

A real game of snake: eat the apples, and don't hit the walls or yourself. Press <kbd>Space</kbd> to play again. (Click the demo first, so that it gets the keys.)

</div>

## The Right Structure for the Question

There's one piece of slow code left: `OnSnake`. To find out whether the head runs into the body, it looks at every cell of the snake, every step. And when a new apple has to be placed, it does that again, for every place it tries.

The queue is great for the question "where are the head and the tail?", but bad for the question "is there a snake in this cell?". For that question, a different data structure is much better: a grid with a `bool` for every cell of the board, which is `true` when the cell is taken. Then the question is a single lookup: `occupied[y][x]`. And while we're at it, the walls go in the grid too: then a single lookup answers "can the snake go here?".

Add the grid:

```cpp source="tmpl8-part-13/game.cpp#L56-L57" {2}
```

Remove `OnSnake`, and let `PlaceApple` use the grid (the walls are taken too, so the apple can be anywhere):

```cpp source="tmpl8-part-13/game.cpp#L65-L71" {5-6}
```

`Start` fills the grid with the walls, and marks the cells of the snake:

```cpp source="tmpl8-part-13/game.cpp#L73-L88" {3-5,7-11}
```

And `Step` keeps the grid up to date: a cell is free when the tail leaves it, and taken when the head arrives:

```cpp source="tmpl8-part-13/game.cpp#L90-L113" {11,16-20,22}
```

Note that the snake now has its data in *two* data structures: the queue and the grid both know where the snake is. That's not a mistake: each one answers a different question quickly. The price is that you have to keep them in sync: every `PushBack` comes with an `occupied[...] = true`, and every `PopFront` with an `occupied[...] = false`.

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-13-grid/index.html" title="The snake game, which now uses a grid to check for walls and its own body" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The same game: you can't see the difference, but the computer does much less work for a long snake. (Click the demo first, so that it gets the keys.)

</div>

## Data Structures

This was a small example, but it shows what data structures are about:

-   A data structure stores data in a way that fits *what you do with it*. A snake adds cells at one end and removes them at the other: a queue. You ask "is this cell taken?": a grid.
-   A class can hide the tricky details (the ring, the wrap-around), so that the rest of the game reads like what it does: `snake.PushBack(next)`.
-   The right data structure often makes the code both faster *and* simpler.

There are data structures for many different things: for huge arrays that have just a few useful values, for searching through endless piles of values, for stacking values so that you can retrieve them in the opposite order, and so on. The C++ standard library already has many of them, ready to use: `std::queue` and `std::deque` can do what our `CellQueue` does, for example. You will meet them later; it helps to have built one yourself first, so that you know what happens inside.

## Assignment

### Basic

1.  Show the score (the number of apples eaten) in the top-left corner of the screen. (Hint: the snake starts with 8 cells.)
2.  Make the snake a little faster after every apple. (Hint: `StepTime` can't stay a constant.)

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-13-assignment-basic/index.html" title="The snake game with a score, in which the snake gets faster after every apple" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: a score, and a snake that gets a bit faster with every apple. (Click the demo first, so that it gets the keys.)

</div>

### Intermediate

1.  Press up and then left, quickly (while the snake moves to the right), and the snake only goes up: the second key press comes before the next step, and it's not a turn to the side yet, so it's lost. Store the key presses in a second queue (a queue of directions), and use one every step. Keep at most a few.
2.  Add walls inside the board. (Hint: describe the board as a grid of numbers, like the tile map in [part 11](/cpp-fast-track-11-tiles/), and use it for the drawing and for the `occupied` grid.)

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-13-assignment-intermediate/index.html" title="The snake game with walls inside the board; quick key presses are not lost" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: walls inside the board, and quick key presses aren't lost. (Click the demo first, so that it gets the keys.)

</div>

### Hard

1.  The queue of cells and the queue of directions are the same code, for a different type. Make one class *template*, `Queue<T, Capacity>`, and use it for both: `Queue<Cell, 400>` and `Queue<int, 4>`. (Look up *class templates*: it's a small step from the class you have.)
2.  Make a game for two players: player 1 steers with the arrow keys, player 2 with <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd> and <kbd>D</kbd>. (Hint: make a `Snake` class, and two instances. If both snakes use the same `occupied` grid, they run into each other without any extra code.)

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-13-assignment-hard/index.html" title="A snake game for two players, one with the arrow keys and one with W, A, S and D" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: two snakes, two players. (Click the demo first, so that it gets the keys.)

</div>

Once you have completed these, you may continue with the next part.
