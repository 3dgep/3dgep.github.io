---
title: "C++ Fast Track for Games Programming Part 5: Conditions"
description: "Conditions: let your program make decisions with if, else and comparison operators."
slug: cpp-fast-track-5-conditions
lastUpdated: 2026-11-14
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

This tutorial is about the conditions in C++, which allow your program to make decisions. You already used a condition when you wrote loops (perhaps without realising it), but in this episode we will look at more explicit conditions.

## Getting the Stuff You Need

For this tutorial, you will be using the same template as in the [2nd tutorial](/cpp-fast-track-2-template/):

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

As in the previous episodes, we'll use the template again. But you will need to know how to use sprites from our [last adventure](/cpp-fast-track-4-sprites-and-loops/), so if you have not done that one, please go back and do so now.

Start with a fresh copy of the template: extract the zip file to `C:\Projects\C++ Fast Track`, and rename the `tmpl8-main` folder to `Part 5`. Then open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 5` folder, remove the *hello world* code from the `Game::Tick` function, and we're ready to begin.

Your `Game::Tick` function should look like this now:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
    }
```

## To Bounce or not to Bounce

In [the last lesson](/cpp-fast-track-4-sprites-and-loops/), you put a sprite on the screen. Let's do that again, with a ball this time. Add the highlighted lines to `game.hpp`, below the `rotatingGun` sprite:

```cpp source="tmpl8-part-05/game.hpp#L43-L48" {5-6}
```

The image `ball.png` contains just one ball, so the sprite has just one frame. Make your `Game::Tick` function look like this:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        screen.Clear(0);
        ball.Draw(screen, 0, 0);
    }
```

Run this and make sure you get an image of a ball in the top-left corner of the screen.

<img src="/images/cpp-fast-track-5/01-ball.png" alt="A ball in the top-left corner of a black screen" width="800" height="512" class="screenshot" />

<div class="caption">

Ball in the top-left corner of the screen.

</div>

Let's move the ball down a little bit, and then move it around using your variable skills. Add a couple of global variables right above the `Tick` function, and use them in `Tick`:

```cpp title="game.cpp" showLineNumbers startLineNumber=24 {1-3,7}
    int spriteX = 0;
    int spriteY = 100;
    int speed = 1;
    void Game::Tick(float deltaTime)
    {
        screen.Clear(0);
        ball.Draw(screen, spriteX, spriteY);
    }
```

Hopefully these variable names explain what they are for. Notice also that each variable is set to an initial value.

Run this and you should see that the ball is now drawn a little lower on the screen.

<img src="/images/cpp-fast-track-5/02-ball-lower.png" alt="The ball on the left side of the screen, 100 pixels from the top" width="800" height="512" class="screenshot" />

<div class="caption">

Ball slightly lower on the screen.

</div>

Now let's make the ball move. Add two lines to the `Game::Tick` function:

```cpp title="game.cpp" showLineNumbers startLineNumber=27 {5-6}
    void Game::Tick(float deltaTime)
    {
        screen.Clear(0);
        ball.Draw(screen, spriteX, spriteY);
        speed += 1;
        spriteY += speed;
    }
```

`speed += 1;` is short for `speed = speed + 1;`: it makes `speed` 1 larger in every frame, like gravity pulling the ball down a bit harder every moment. `spriteY += speed;` then moves the ball down by `speed` pixels. So the ball falls faster and faster, just like a real ball.

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-5-falling-ball/index.html" title="The ball falling off the screen" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The ball falls off the screen. (In this demo, it starts again at the top after a while; with your code, it's gone for good.)

</div>

The ball quickly leaves the bottom of the screen. It would obviously be nice if it would bounce back at the bottom... That's where conditions come in. *If it has reached the bottom of the screen* is a condition, and we can actually use something very close to that in our code. Add these lines at the end of the `Tick` function:

```cpp title="game.cpp" showLineNumbers startLineNumber=33
        if (spriteY > (512 - 50))
        {
            spriteY = 512 - 50;
            speed = -speed;
        }
```

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-5-bounce/index.html" title="The ball bouncing on the bottom of the screen" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The ball bounces on the bottom of the screen.

</div>

That looks simple enough, but there's a lot of detail in there.

First, `512`: that's the bottom of the screen, but sprites are positioned by their top-left corner. The ball has a height of `50` pixels, so we make it bounce on the bottom of the screen when the ball's Y coordinate is greater than `(512 - 50)`.

Next, lines 35-36 are the code that is executed if the condition is true. On line 35, the Y coordinate of the ball is reset to be exactly at the bottom of the screen, in case the ball has moved past the bottom of the screen.

On line 36, the speed is inverted (a positive speed becomes negative), which makes the ball move in the opposite direction: up. Because the speed still increases by 1 in every frame, the ball slows down on its way up, stops, and falls down again.

:::note[First the speed, then the position]
The order of the two lines `speed += 1;` and `spriteY += speed;` matters more than you might think. Swap them, so that the ball moves first and the speed changes after that, and watch the ball for a while: it bounces a little higher every time, until it flies off the top of the screen! The speed is then increased one time too many before it is inverted, so the ball goes up a bit faster than it came down, and each bounce adds a little more.

Changing the speed first and then moving the ball with the new speed is called *semi-implicit Euler integration* (or *symplectic Euler*). It's simple, and it keeps the energy of things like bouncing balls, orbits and springs stable, which is why games use it a lot. You'll see it again in the later part about physics. If you want to know more: [Integration Basics](https://gafferongames.com/post/integration_basics/) by Glenn Fiedler explains it for games, and Wikipedia has the math: [Semi-implicit Euler method](https://en.wikipedia.org/wiki/Semi-implicit_Euler_method).
:::

:::tip
You may have noticed that there are parentheses around `(512 - 50)` on line 33. Why? Well, we want to compare `spriteY` with the result of `512 - 50`, instead of subtracting `50` from the result of `spriteY > 512`.

Read more about operator precedence here: [Operator Precedence in C++](https://en.cppreference.com/cpp/language/operator_precedence). Are the parentheses really needed in this case?
:::

## Or Else

To add a bit of drama to the bouncing, we can flash the background when the ball hits the bottom of the screen. Try this:

```cpp title="game.cpp" showLineNumbers startLineNumber=27 {9-10,12-16}
    void Game::Tick(float deltaTime)
    {
        speed++;
        spriteY += speed;
        if (spriteY > (512 - 50))
        {
            spriteY = 512 - 50;
            speed = -speed;
            screen.Clear(0xff0000);
            ball.Draw(screen, spriteX, spriteY);
        }
        else
        {
            screen.Clear(0);
            ball.Draw(screen, spriteX, spriteY);
        }
    }
```

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-5-flash/index.html" title="The ball bouncing, with a red flash every time it hits the bottom" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The screen flashes red when the ball hits the bottom.

</div>

This time, we have two code blocks that belong to the condition. The second block, after `else`, is executed when the condition is false. (`speed++;` is short for `speed += 1;`, just like `i++` in a `for` loop.)

## Booleans

In many programming languages, `true` and `false` are *boolean* values. We can store these in a `bool` variable:

```cpp
        bool hitBottom = spriteY > 512 - 50;
        if (hitBottom) { ... }
```

An interesting question is of course how the computer stores these values in memory. Let's try to gain some insight. Change your `Tick` function so that it uses a `hitBottom` boolean. Then, print the boolean to the console window:

```cpp source="tmpl8-part-05/game.cpp#L27-L45" {5-7}
```

Run the program, and bring the console window into focus. Things move fast, but if you watch carefully, you will see that `false` apparently equals `0`, and `true` equals `1`:

<img src="/images/cpp-fast-track-5/03-hitbottom-console.png" alt="The console window, with hitBottom: 0 printed many times, and once hitBottom: 1" width="976" height="268" class="screenshot" />

<div class="caption">

`hitBottom` is `1` in the frame in which the ball hits the bottom.

</div>

In C++, this works both ways: `0` is `false`, and anything that is not `0` is `true`. So:

```cpp
        int a = 1000, b = -1000;
        if (a && b)
        {
            std::cout << "both true.\n";
        }
```

will print `both true.` to the console window, which means that both `a` and `b` are `true`. (`&&` means *and*: the condition is true when the values on both sides of it are true.)

<img src="/images/cpp-fast-track-5/04-both-true-console.png" alt="The console window, with both true. printed many times" width="976" height="268" class="screenshot" />

<div class="caption">

`1000` and `-1000` are both `true`.

</div>

Not directly useful, but now you know.

## Assignment

Let's combine some things from the last couple of episodes.

Add a variable `speedX` to make the ball move horizontally as well. Use it to make the ball bounce from the left to the right side of the screen. When it reaches the right side, make it bounce off the right wall, so it starts moving back to the left. After, say, 50 bounces, stop testing for the bottom of the screen, so that the ball falls through and disappears.

When it works, it looks like this:

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-5-assignment/index.html" title="The ball bouncing around, and falling through the bottom after 50 bounces" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The ball bounces between the walls, and falls through the bottom after 50 bounces. (This demo also prints the number of bounces, and starts again after the ball has disappeared.)

</div>

Once you have completed this assignment, you may continue with the next part.
