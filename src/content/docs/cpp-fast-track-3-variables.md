---
title: "C++ Fast Track for Games Programming Part 3: Variables and Functions"
description: "Variables and functions: store values, compute with them, and organise code into reusable functions."
slug: cpp-fast-track-3-variables
lastUpdated: 2026-11-14
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

The previous tutorial introduced you to the template. You can now draw some lines and print some text in all kinds of colours, but that's clearly far from the goal of making actual games, where bullets whizz past in glorious 3D and enemies flank you in the smartest ways. To get a bit closer to that, we'll take the fixed coordinates from last time and make them a bit more flexible by using C++ variables and functions. We'll need that for the next part, which is all about loops, sprites and other bouncy things.

## Getting the Stuff You Need

For this tutorial, you will be using the same template as in the [2nd tutorial](/cpp-fast-track-2-template/):

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

Start with a fresh copy of the template: extract the zip file again (it's probably still in your *Downloads* folder) to `C:\Projects\C++ Fast Track`, and rename the `tmpl8-main` folder to `Part 3`. Then open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 3` folder.

## Variables

Change the `Tick` function in `game.cpp` to this:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        screen.Clear(80);
        screen.Line(100, 50, 200, 50, 0xffffff);
        screen.Line(150, 50, 150, 300, 0xffffff);
        screen.Line(100, 300, 200, 300, 0xffffff);
    }
```

<img src="/images/cpp-fast-track-3/01-i.png" alt="A white I on a dark blue background" width="800" height="512" class="screenshot" />

This paints the *I* of *IMPRESSIVE* in glorious white on a vibrant blue backdrop. Now imagine we would like thicker lines. We take the three line commands, copy them, and draw them one pixel to the right:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        screen.Clear(80);
        screen.Line(100, 50, 200, 50, 0xffffff);
        screen.Line(150, 50, 150, 300, 0xffffff);
        screen.Line(100, 300, 200, 300, 0xffffff);
        screen.Line(101, 50, 201, 50, 0xffffff);
        screen.Line(151, 50, 151, 300, 0xffffff);
        screen.Line(101, 300, 201, 300, 0xffffff);
    }
```

<img src="/images/cpp-fast-track-3/02-thick-vertical.png" alt="The I, with a vertical line that is two pixels wide" width="800" height="512" class="screenshot" />

Now, that's a bit disappointing: obviously, only the vertical line got thick now. Let's draw all six lines one more time, this time one pixel lower:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        screen.Clear(80);
        screen.Line(100, 50, 200, 50, 0xffffff);
        screen.Line(150, 50, 150, 300, 0xffffff);
        screen.Line(100, 300, 200, 300, 0xffffff);
        screen.Line(101, 50, 201, 50, 0xffffff);
        screen.Line(151, 50, 151, 300, 0xffffff);
        screen.Line(101, 300, 201, 300, 0xffffff);
        screen.Line(100, 51, 200, 51, 0xffffff);
        screen.Line(150, 51, 150, 301, 0xffffff);
        screen.Line(100, 301, 200, 301, 0xffffff);
        screen.Line(101, 51, 201, 51, 0xffffff);
        screen.Line(151, 51, 151, 301, 0xffffff);
        screen.Line(101, 301, 201, 301, 0xffffff);
    }
```

<img src="/images/cpp-fast-track-3/03-thick-i.png" alt="The I, with all its lines two pixels thick" width="800" height="512" class="screenshot" />

This time the result is satisfying (click the image to see it larger). However, this is not something you want to spend a career on: we need to find a smarter way to do this. In C++ you would use a function for this. But before we get to that, let's try something else:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        screen.Clear(80);
        int x = 100;
        int y = 0;
        screen.Line(100 + x, 50 + y, 200 + x, 50 + y, 0xffffff);
        screen.Line(150 + x, 50 + y, 150 + x, 300 + y, 0xffffff);
        screen.Line(100 + x, 300 + y, 200 + x, 300 + y, 0xffffff);
    }
```

<img src="/images/cpp-fast-track-3/04-i-moved.png" alt="The I, 100 pixels further to the right" width="800" height="512" class="screenshot" />

Here we have the original three lines again, but this time, each coordinate has either `+ x` or `+ y` attached to it. `x` and `y` are *variables*: names for a value that the program keeps in its memory. `int x = 100;` creates the variable `x`, says that it holds a whole number (`int`, short for *integer*), and gives it the value `100`. You may give variables any name you like, as long as it starts with a letter (or `_`) and has no spaces. The value of a variable is simply added to the coordinate, so the I is now drawn 100 pixels further to the right.

This is useful: we can now make the I move. Move the two lines with `int` to just above the `Tick` function, and add a line that changes `x` at the end of `Tick`:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    int x = 100;
    int y = 0;
    void Game::Tick(float deltaTime)
    {
        screen.Clear(80);
        screen.Line(100 + x, 50 + y, 200 + x, 50 + y, 0xffffff);
        screen.Line(150 + x, 50 + y, 150 + x, 300 + y, 0xffffff);
        screen.Line(100 + x, 300 + y, 200 + x, 300 + y, 0xffffff);
        x = x + 1;
    }
```

`x = x + 1;` takes the value of `x`, adds `1` to it, and stores the result in `x` again. Because `Tick` is executed for every frame, the I moves one pixel to the right in every frame:

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-3-moving-i/index.html" title="The I moving to the right" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The I moves one pixel per frame. With the code above, it leaves the screen after a few seconds and never comes back; in this demo, it starts again on the left side (you'll learn how to do that in part 5).

</div>

How fast the I moves depends on your monitor: the template draws as many frames per second as your monitor shows (usually 60, but some monitors show 144 or more). In a later part, you will learn how to make things move at the same speed on every computer.

Now, if we could draw those three lines multiple times with different variable values, we would be all set: we could draw super-fat characters; heck, we could even move them across the screen! It's not a bullet yet, but it's a start.

:::note
By the way, why did the `int x = 100;` line have to move outside the `Tick` function? When it was inside the `Tick` function, `x` and `y` were reset for every frame, because the lines `int x = 100;` and `int y = 0;` are executed every time `Tick` is executed. Since they are supposed to be set to their starting values only once, you would expect that they should go to the `Init` function. However, one function cannot access the variables in another function, and even if two variables in different functions have the same name, they are still two different variables. Variables created outside a function, however, are available in all the functions below them. And they are initialized only once, when the program starts. In C++ terminology, the variables `x` and `y` were *declared in the scope of the `Tick` function*, and now they are *declared in the global scope*.
:::

## Functions

A function is a piece of code with a name. You have seen three of them so far: `Game::Init`, `Game::Tick`, and `Game::Shutdown`. We can create our own too. Add the highlighted line to the `game.hpp` header file:

```cpp source="tmpl8-part-03/game.hpp#L14-L17" {3}
```

You can put it in many correct places, but between the `Shutdown` and `Tick` functions is probably a good place. This line *declares* the function: it tells the compiler that the `Game` has a function called `DrawI`, which needs two whole numbers, `x` and `y`. Everything after `//` is a comment.

Then, return to `game.cpp`, and add the following function above the `Game::Tick` function. You can remove the two lines with `int x` and `int y` above `Tick` now:

```cpp source="tmpl8-part-03/game.cpp#L21-L29"
```

This *defines* the function: it says what the function does. And change the `Game::Tick` function to this:

```cpp title="game.cpp" showLineNumbers startLineNumber=34
    void Game::Tick(float deltaTime)
    {
        screen.Clear(80);
        DrawI(0, 0);
    }
```

<img src="/images/cpp-fast-track-3/05-drawi.png" alt="The I, drawn by the DrawI function" width="800" height="512" class="screenshot" />

You get the same I as at the start of this tutorial, but now it's drawn by your own function.

In other words: when we type `DrawI(0, 0)`, we actually execute the code in the `Game::DrawI` function. And whatever we type between the parentheses after `DrawI` (in this case: `0, 0`) gets passed to the function: inside the function, `x` and `y` will contain the values `0` and `0`. So, our fat character code becomes:

```cpp source="tmpl8-part-03/game.cpp#L34-L41"
```

<img src="/images/cpp-fast-track-3/06-fat-i.png" alt="The fat I, drawn with four calls to DrawI" width="800" height="512" class="screenshot" />

The same fat I as before, and that's quite a bit more reasonable.

By the way, the `screen.Clear` and `screen.Line` that you have been using all along are functions too! Have a look at `surface.cpp`, line 232:

```cpp source="tmpl8-part-03/tmpl8/surface.cpp#L232-L265"
```

Not everything in this function will make sense right now, but rest assured: this is how you draw a proper line in C++. The first part (up to `if (!accept) return;`) cuts off the parts of the line that are outside the screen, so that you can't draw outside the screen by accident. The second part draws the line, one pixel at a time.

There are actually two `Line` functions: this one takes `float` coordinates (numbers with a fraction, more about those in part 6), and the one at line 272 takes `int` coordinates, like the ones in `DrawI`. It turns them into `float` values and calls this one. C++ picks the right one for you, depending on the kind of numbers you pass.

## Assignment

Here's your task for today:

1.  Create a function that draws a fat I. Call it `DrawFatI`. This function will use `DrawI` in turn, of course.
2.  Modify `Game::Tick` in such a way that the fat I moves across the screen.

When it works, it looks like this:

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-3-assignment/index.html" title="A fat I moving across the screen" loading="lazy" allowfullscreen></iframe>

<div class="caption">

A fat I moving across the screen. (In this demo, it starts again on the left side when it has left the screen.)

</div>

Once you have completed this assignment you may continue with the next part.
