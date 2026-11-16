---
title: "C++ Fast Track for Games Programming Part 12: Classes"
description: "Classes: restructure the tank code into reusable, easier to understand objects."
slug: cpp-fast-track-12-classes
lastUpdated: 2026-11-15
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

In this part, we will build on the code from our work with [Tiles](/cpp-fast-track-11-tiles/). We aim to give it a more reusable and understandable structure: this is where *classes* come in.

## Getting Started

For this tutorial, you will start where [part 11](/cpp-fast-track-11-tiles/) ended: the template with the tile map and the tank. Download it here:

[<img src="/images/zip1.png" alt="Download the starting point for part 12 as a zip file" width="120" class="thumb" />part-12.zip](/downloads/part-12.zip)

Extract the zip file to `C:\Projects\C++ Fast Track`: it contains a `Part 12` folder. Open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 12` folder.

The code is the code from the end of part 11, with two small differences. Two trees on the right are gone (they made a dead end, in which a tank that drives by itself would get stuck). And `tank.png` and `barrel.png` now have a second row of frames, with a red tank. That's why the sprites have 16 columns and 2 rows of frames:

```cpp source="tmpl8-part-12/start/game.cpp#L11-L13" {3}
```

Run it, and you will see the map and the green tank of part 11, which you can drive with the arrow keys (or <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd> and <kbd>D</kbd>).

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-12-start/index.html" title="The tile map of part 11, with the green tank that drives with the arrow keys or W, A, S and D" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The starting point: the tile map and the tank from part 11. (Click the demo first, so that it gets the keys.)

</div>

## Objects

Your game world consists of *objects*. Take the last part for example: we have a *tank*, a *map* (which consists of *tiles*), and a *screen* that we draw to. Later on, we might want to add *bullets*, *enemies*, more *levels*, and so on. This is a list of *things*, not a list of operations.

You could argue that the most logical way to start thinking about a game is: what *objects* do I need? And, slightly more detailed: what do these objects *do*? What *properties* do they have? And how are they *related*?

Note that this is something you have already been using. Have a look at `Game::Tick`:

```cpp source="tmpl8-part-12/start/game.cpp#L93-L95" {3}
```

The game has a `screen` variable, which is an object: its type is `Surface`. We tell this surface to do things: `screen.Clear(0)` asks it to clear itself, and `screen.Print` asks it to print text (you printed the frame rate with it in [part 6](/cpp-fast-track-6-floats/)). A `Surface` can do more: open `surface.hpp` (in the *template code* folder) and look for `class Surface`. There, you can see that a surface can also draw lines (`Line`), boxes (`Box` and `Bar`), and more. A surface also has *properties*: its width, its height, and its pixels.

The things that an object can do are called *methods*. The properties of an object are called *member variables*.

## A Tank Class

The tank is an object too, but our code doesn't say so: it's just three loose variables, `tankX`, `tankY` and `tankFrame`. Let's change that. A *class* describes what a kind of object looks like. Replace those three variables:

```cpp source="tmpl8-part-12/start/game.cpp#L43-L44"
```

by this, *below* the function `TankFits` (you will see why in a moment):

```cpp source="tmpl8-part-12/1-class/game.cpp#L73-L86"
```

The class `Tank` describes what a tank is: it has a position (`x` and `y`) and a direction (`frame`). `public:` means that the rest of the program can use these: we will get back to that.

The function with the same name as the class is a *constructor*. A constructor has no return type (not even `void`), and it runs automatically when a tank is created. That makes it the `Init` of the class: it gives the member variables their first values. Our constructor takes *parameters* (the start position and direction), so creating a tank looks like calling a function:

```cpp
Tank player(3 * TileSize + 32, 2 * TileSize + 32, 0);
```

This creates an instance of a `Tank` called `player`, in the middle of the tile in column 3 and row 2, pointing up. Note the difference: `Tank` is the class, the description of a tank. `player` is an actual tank: an *instance* of the class. You can make as many instances as you like, and each one has its own `x`, `y` and `frame` variables.

Now use the player's member variables in `Tick`, with a dot: `player.x` is the `x` of the player.

```cpp source="tmpl8-part-12/1-class/game.cpp#L119-L131" {2-13}
```

Run the program: it does exactly the same as before. But the variables of the tank are now together, in one place.

## Methods

The tank still doesn't *do* anything: moving and drawing happens in `Tick`, which uses the tank's variables. That code belongs to the tank. Let's give the tank two methods: `Move` and `Draw`. Add them to the class:

```cpp source="tmpl8-part-12/2-methods/game.cpp#L82-L83"
```

This *declares* the methods: it tells C++ that a tank can `Move` and `Draw`, but not yet *how*. Now replace the movement code in `Tick`:

```cpp source="tmpl8-part-12/2-methods/game.cpp#L136-L144" {2-9}
```

When you build this, you will get a linker error, [LNK2019](https://learn.microsoft.com/cpp/error-messages/tool-errors/linker-tools-error-lnk2019):

```
error LNK2019: unresolved external symbol "public: void __cdecl Tmpl8::Tank::Move(float,float)" ... referenced in function "public: void __cdecl Tmpl8::Game::Tick(float)"
```

We told C++ that a tank can `Move`, and `Tick` calls it, but we never said what `Move` does. (In a *Release* build, the same error is called LNK2001.) Let's fix that. Add the *definitions* of the methods below the class:

```cpp source="tmpl8-part-12/2-methods/game.cpp#L88-L101"
```

`Tank::Move` means "the method `Move` of the class `Tank`". Inside a method, `x`, `y` and `frame` are the member variables of the tank that the method is called for: `player.Move(dx, dy)` moves the player. This is also why the class is below `TankFits`: `Move` uses it, and in C++, you can only use a function below the place where it's declared.

Note the `const` after `Draw`, in the declaration and in the definition. It promises that `Draw` doesn't change the tank: it only looks at `x`, `y` and `frame`. The compiler checks that promise: try `x = 0;` inside `Draw`, and you get an error. `Move` does change the tank, so it can't be `const`. Make every method that only looks at the object `const`: the compiler will then warn you when it accidentally changes something.

Now look at `Draw`. Why does it have a parameter `target`, instead of drawing to `screen`, like `Tick` does? Try it: change `Draw` to draw to `screen`, and you get a compiler error, [C2065](https://learn.microsoft.com/cpp/error-messages/compiler-errors-1/compiler-error-c2065): `'screen': undeclared identifier`. The variable `screen` belongs to *another* object: the `Game`. Only the methods of the `Game` (like `Tick`) can use it directly. That may seem annoying, but it's actually good: the member variables of an object belong to that object. That's why a `Tank` can have an `x` and a `y`, a `Bullet` can have an `x` and a `y` too, and they don't get mixed up.

Where a variable can be used is called the *scope* of the variable (see [Scope](https://en.cppreference.com/w/cpp/language/scope) for the details). The solution is simple: the game knows the `screen`, and the game asks the tank to draw itself, so the game passes the screen along: `player.Draw(screen)`. Inside `Draw`, it's called `target`: it's a parameter of the method.

Run the program, and again it does exactly the same. But the code of the tank is now in the tank, and `Tick` only says *what* happens: the player moves, and the player is drawn.

## A Second Tank

Now that we have a class, a second tank is easy. Let's add an enemy: a red tank that drives by itself. Give the class a `color` (the row in `tank.png`: 0 is green, 1 is red), and a method `Update` for the enemy's behaviour:

```cpp source="tmpl8-part-12/game.cpp#L73-L111" {4,9,12,16,26-31,35-36}
```

Then create the enemy next to the player (the player is green, the enemy red, and it starts just left of the map, pointing right):

```cpp source="tmpl8-part-12/game.cpp#L113-L114"
```

And update and draw it in `Tick`:

```cpp source="tmpl8-part-12/game.cpp#L153-L157" {2,5}
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-12-two-tanks/index.html" title="The green player tank, and a red enemy tank that drives from left to right along the edge of the grass" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The player (green) and an enemy (red), two instances of the same class. Drive the player with the arrow keys or <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd> and <kbd>D</kbd>. (Click the demo first, so that it gets the keys.)

</div>

Both tanks run the same code, but each has its own position, direction and colour. The enemy's `Update` is very simple: it drives to the right (without looking at the obstacles!), and when it has left the map, it starts again on the left. You will improve that in the assignment.

## Conclusion

A few final words before you start hacking away. You have been using classes without knowing it: there is a `Game` class, a `Surface` class and a `Sprite` class. And the template has a few more, which you didn't use yet.

Having a class means nothing by itself: it only tells C++ what something looks like. To actually make something, you create a variable of that type, such as `player`. This variable is an *instance* of the class, and each instance has its own set of member variables.

You use classes for many reasons:

- They allow you to think in high-level concepts (tanks, maps, bullets) before you get to the details;
- They group data per object, instead of in one big messy pool of variables;
- They group data and the code that works with that data.

We will dive deeper into this subject later. For now, you know enough for the assignment.

## Assignment

There are three challenge levels for this assignment. You are encouraged to tackle the basic task first. When you are up for a greater challenge, you can move on to the intermediate and hard challenges.

### Basic

1. Make the enemy drive with these three rules, using `TankFits` to see where it can go:
    1. If the tank *can* move to the right, it does.
    2. If the tank *can't* move to the right, it moves up when it's in the lower half of the map, and down when it's in the upper half. It keeps going that way until it can move to the right again (and turns around when that way is blocked too).
    3. At the right side of the map, the tank drives off the screen. When it's completely gone, it comes back in from the left side (starting completely off the screen too), at a random height where it isn't blocked.

    :::tip[Hint]
    The tank has to remember which way it is going around an obstacle: otherwise, a tank near the middle of the map goes up, is then in the upper half, goes down, is in the lower half again, and so on. Give the class a member variable for it.
    :::
2. Make an array of tanks (remember [part 10](/cpp-fast-track-10-arrays/)?). Each tank starts on a random tile without an obstacle, and follows the same rules.

    :::tip[Hint]
    An array of tanks needs a constructor without parameters, a *default constructor*. A class can have more than one constructor.
    :::

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-12-assignment-basic/index.html" title="Six red tanks that drive to the right, and go around obstacles" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: six enemies that follow the three rules.

</div>

### Intermediate

1. Make sure that no two tanks start on the same tile.
2. Move the `Tank` class to its own files: a `tank.hpp` for the class (with the declarations of its methods), and a `tank.cpp` for the definitions of the methods.

    :::tip[Hint]
    The methods use a few things from `game.cpp`. Move the `tank` and `barrel` sprites to `tank.cpp`: only `Draw` uses them. `Move` needs `TankFits`, which can stay in `game.cpp`: add a *forward declaration* of it to `tank.cpp`, `bool TankFits(float x, float y);` (inside `namespace Tmpl8`, like everything else), just like the declarations of the methods in the class. And `Update` needs the size of the map: use `ScreenWidth` from `template.hpp`, or move the constants to a header that both files include. Put the new files next to `game.cpp`.
    :::

### Hard

1. Make sure that no two tanks ever overlap: a tank must not drive into another tank (or into the player).
2. Make a class for the tile map: `TileMap`. It holds the layers, and has methods to draw the map and to check whether a place is free. Separating the map from the rest of the game lets you reuse it in another project.

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-12-assignment-hard/index.html" title="Six red tanks that drive around without ever overlapping each other or the player" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: tanks that never overlap. A tank that is blocked by another tank waits, or goes around it. (Click the demo first to drive the green tank.)

</div>

Once you have completed these, you may continue with the next part.
