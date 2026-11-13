---
title: "C++ Fast Track for Games Programming Part 3: Variables and Functions"
description: "Variables and functions: store values, compute with them, and organise code into reusable functions."
slug: cpp-fast-track-3-variables
lastUpdated: 2022-12-05
---

<span id="attachment-8103"></span><img src="/images/cpp-logo.png" alt="C++ Fast-track" width="150" class="thumb" />

The previous tutorial introduced you to the template. You can now draw some lines and print some text in all kinds of colours, but that’s clearly far from the goal of making actual games, where bullets wizz past in glorious 3D and enemies flank you in the smartest ways. To get a bit closer to that, we’ll take the static coordinates from last time and make them a bit more flexible by using C++ variables and functions. We’ll need that for the next part, which is all about loops, sprites and other bouncy things.

## Getting the stuff you need

For this tutorial, you will be using the same template from the [2nd tutorial](/cpp-fast-track-2-template/):

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

Extract the template zip file to a fresh directory (say, `C:\Projects\variables` or where ever you saved your project from the previous tutorial) and load up the `.sln` file.

Change the `Tick` function in `game.cpp` to this:

```cpp title="game.cpp" showLineNumbers startLineNumber=27
void Game::Tick(float deltaTime)
{
    screen->Clear(80);
    screen->Line(100, 50, 200, 50, 0xffffff);
    screen->Line(150, 50, 150, 300, 0xffffff);
    screen->Line(100, 300, 200, 300, 0xffffff);
}
```

This paints the *I* of *IMPRESSIVE* in glorious white on a vibrant blue backdrop. Now imagine we would like thicker lines. We take the three line commands, copy them, and draw them one pixel to the right:

```cpp title="game.cpp" showLineNumbers startLineNumber=27
void Game::Tick(float deltaTime)
{
    screen->Clear(80);
    screen->Line(100, 50, 200, 50, 0xffffff);
    screen->Line(150, 50, 150, 300, 0xffffff);
    screen->Line(100, 300, 200, 300, 0xffffff);
    screen->Line(101, 50, 201, 50, 0xffffff);
    screen->Line(151, 50, 151, 300, 0xffffff);
    screen->Line(101, 300, 201, 300, 0xffffff);
}
```

Now, that’s a bit disappointing: obviously, only the vertical line got thick now. Let’s draw all six lines one more time, this time one pixel lower:

```cpp title="game.cpp" showLineNumbers startLineNumber=27
void Game::Tick(float deltaTime)
{
    screen->Clear(80);
    screen->Line(100, 50, 200, 50, 0xffffff);
    screen->Line(150, 50, 150, 300, 0xffffff);
    screen->Line(100, 300, 200, 300, 0xffffff);
    screen->Line(101, 50, 201, 50, 0xffffff);
    screen->Line(151, 50, 151, 300, 0xffffff);
    screen->Line(101, 300, 201, 300, 0xffffff);
    screen->Line(100, 51, 200, 51, 0xffffff);
    screen->Line(150, 51, 150, 301, 0xffffff);
    screen->Line(100, 301, 200, 301, 0xffffff);
    screen->Line(101, 51, 201, 51, 0xffffff);
    screen->Line(151, 51, 151, 301, 0xffffff);
    screen->Line(101, 301, 201, 301, 0xffffff);
}
```

This time the result is satisfying. However, this is not something you want to spend a career on: we need to find a smarter way to do this. In C/C++ you would use a function for this. But before we get to that, let’s try something else.

```cpp title="game.cpp" showLineNumbers startLineNumber=27
void Game::Tick(float deltaTime)
{
    screen->Clear(80);
    int x = 100;
    int y = 0;
    screen->Line(100 + x, 50 + y, 200 + x, 50 + y, 0xffffff);
    screen->Line(150 + x, 50 + y, 150 + x, 300 + y, 0xffffff);
    screen->Line(100 + x, 300 + y, 200 + x, 300 + y, 0xffffff);
}
```

Here we have the original three lines again, but this time, each coordinate has either `+ x` or `+ y` attached to it. `X` and `Y` are variables here, and you may give them any name you like. The number they represent is simply added to the coordinate. This is useful; we can now make the ‘I’ move. Try this:

```cpp title="game.cpp" showLineNumbers startLineNumber=27
int x = 100;
int y = 0;
void Game::Tick(float deltaTime)
{
    screen->Clear(80);
    screen->Line(100 + x, 50 + y, 200 + x, 50 + y, 0xffffff);
    screen->Line(150 + x, 50 + y, 150 + x, 300 + y, 0xffffff);
    screen->Line(100 + x, 300 + y, 200 + x, 300 + y, 0xffffff);
    x = x + 1;
}
```

Now, if we could draw those three lines multiple times with different variable values, we would be all set: we could draw super-fat characters; heck, we could even move them across the screen! It’s not a bullet yet, but it’s a start.

:::note
By the way, something odd happened to that `int x = 100;` line: It was moved outside the `Tick` function. When it was inside the `Tick` function, `x` and `y` were reset for every image, because the line `int x = 100;`, `int y = 0;` is executed for everytime `Tick` is invoked. Since they are supposed to be set to their initial values only in the first frame, you would expect that they should go to the `Init` function. However, one function cannot access variables in another function in C/C++, and even if two variables in different functions have identical names, they will still be two different things. Variables created outside a function however are available in all functions. And, they are initialized only once. In C/C++ terminology, the variables `x` and `y` were *declared within the scope of the `Tick` method*, and now they are *declared in the global scope*.
:::

## Functions

A function is a piece of code. You have seen three of them so far: `Game::Init()`, `Game::Tick()`, and `Game::Shutdown()`. We can create our own too. Add the highlighted line to the `game.h` header file:

```cpp title="game.h" showLineNumbers startLineNumber=11 {2}
    void Shutdown();
    void DrawI(int x, int y);
    void Tick( float deltaTime );
```

You can put it in many correct locations, but between `Shutdown` and `Tick` functions is probably a good place.

Then, return to `game.cpp`, and add the following function above the `Game::Tick` function:

```cpp title="game.cpp" showLineNumbers startLineNumber=23
void Game::DrawI( int x, int y )
{
    screen->Line( 100 + x, 50 + y, 200 + x, 50 + y, 0xffffff );
    screen->Line( 150 + x, 50 + y, 150 + x, 300 + y, 0xffffff );
    screen->Line( 100 + x, 300 + y, 200 + x, 300 + y, 0xffffff );
}
```

And, change the `Game::Tick` function to this:

```cpp title="game.cpp" showLineNumbers startLineNumber=34
void Game::Tick(float deltaTime)
{
    screen->Clear(80);
    DrawI(0, 0);
}
```

In other words: When we type `DrawI( 0, 0 )`, we actually execute the code specified in `Game::DrawI()` method. And, whatever we type after `DrawI` (in this case: `0, 0`) gets passed to the function: Inside the function, `x` and `y` will contain the values `0` and `0`. So, our fat character code becomes:

```cpp title="game.cpp" showLineNumbers startLineNumber=34
void Game::Tick(float deltaTime)
{
    screen->Clear(80);
    DrawI(0, 0);
    DrawI(1, 0);
    DrawI(0, 1);
    DrawI(1, 1);
}
```

And that’s quite a bit more reasonable.

By the way, the `screen->Clear` and `screen->Line` that you have been using all along are functions too! Have a look at `surface.cpp`, line 151:

```cpp title="surface.cpp" showLineNumbers startLineNumber=151
void Surface::Line(float x1, float y1, float x2, float y2, Pixel c)
{
    ...
    float b = x2 - x1;
    float h = y2 - y1;
    float l = fabsf(b);
    if (fabsf(h) > l) l = fabsf(h);
    int il = (int)l;
    float dx = b / (float)l;
    float dy = h / (float)l;
    for (int i = 0; i <= il; i++)
    {
        *(m_Buffer + (int)x1 + (int)y1 * m_Pitch) = c;
        x1 += dx, y1 += dy;
    }
}
```

Not everything in this function will make sense right now, but rest assured: this is how you draw a proper line in C/C++.

## Assignment

Here’s your task for today:

1.  Create a function that draws a fat I. Call it `DrawFatI`. This function will use `DrawI` in turn, of course.
2.  Modify `Game::Tick()` in such a way that the fat I moves across the screen.

Once you have completed this assignment you may continue with the next part.
