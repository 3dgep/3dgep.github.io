---
title: "C++ Fast Track for Games Programming Part 4: Sprites and Loops"
description: "Sprites and loops: make things move on the screen, and use loops to write less code that does more."
slug: cpp-fast-track-4-sprites-and-loops
lastUpdated: 2026-11-14
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

This tutorial is about two things: sprites and loops. Sprites will get you on your way to making interesting stuff move on the screen, whereas loops will help you write less code to do more. On with the show!

## Getting the Stuff You Need

For this tutorial, you will be using the same template as in the [2nd tutorial](/cpp-fast-track-2-template/):

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

Start with a fresh copy of the template: extract the zip file to `C:\Projects\C++ Fast Track`, and rename the `tmpl8-main` folder to `Part 4`. Then open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 4` folder.

Open the `game.cpp` file and remove the *hello world* stuff in the `Game::Tick` function. Your `Game::Tick` function should now look like this:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
    }
```

## Sprites

Let's see if we can draw something more interesting than a line this time. We'd like to start drawing 2D game entities, like cars, people, and so on. These 2D game elements are called *sprites*. You have already seen one: the spinning gun turret in the [2nd tutorial](/cpp-fast-track-2-template/) is a sprite.

Here's how you make your own. Open `game.hpp`, and add the highlighted lines below the `rotatingGun` sprite:

```cpp source="tmpl8-part-04/game.hpp#L42-L47" {5-6}
```

This creates a `Sprite` called `tank`, from the image `ctankbase.tga`, which you can find in the `assets` folder of the template. This is what that image looks like:

<img src="/images/cpp-fast-track-4/ctankbase.png" alt="The ctankbase.tga image: a tank in 16 directions, next to each other" width="800" height="50" class="screenshot" />

<div class="caption">

`ctankbase.tga`: 16 images of 50 by 50 pixels, next to each other.

</div>

:::note
Windows can't show `.tga` images, so you can't open `ctankbase.tga` by double-clicking it (unless you have installed an image editor like [GIMP](https://www.gimp.org) or [Paint.NET](https://www.getpaint.net)). The template can load `.tga`, `.png`, `.jpg`, `.bmp` and `.gif` images.
:::

Now make your `Tick` function like this:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        screen.Clear(0);
        tank.Draw(screen, 0, 0);
    }
```

Run the program and look at the pretty tank in the top-left corner of the window!

<img src="/images/cpp-fast-track-4/01-tank.png" alt="One tank in the top-left corner of a black screen" width="800" height="512" class="screenshot" />

Ok, so we can display an image, which is what a `Sprite` basically is. Except that sprites are usually animated. As you saw above, `ctankbase.tga` contains a row of similar tank images, each one rotated a bit more, so all the directions of the tank are in the image. These images are the *frames* of the sprite. But when we draw our tank in the `Tick` function, it only shows the first of the rotated tanks! That is what the `Sprite` does for us: it keeps track of which one of the frames to draw. We can tell the sprite to draw one of the other rotated tanks in the image (i.e. another frame of the animation) if we want to. That's what the `16` in `game.hpp` is for: it tells the sprite that the image contains 16 frames.

Change the `Game::Tick` function to make it look like this:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        screen.Clear(0);
        tank.SetFrame(2);
        tank.Draw(screen, 0, 0);
    }
```

<img src="/images/cpp-fast-track-4/02-tank-45.png" alt="The tank in the top-left corner, rotated 45 degrees" width="800" height="512" class="screenshot" />

The tank is now rotated 45°!

What we've done is that we told the sprite that its *active* or *current* frame is now 2. Each next call to `Sprite::Draw` will now draw frame 2 of the sprite, which is the third image: frame 0 is the first one. The 16 frames turn the tank all the way around (360°), so each frame turns it 360 / 16 = 22.5° more, and frame 2 turns it 2 × 22.5 = 45°.

## Loops

I like the tank so much, I want to see more of them on the screen. Let's try this. Change the `Tick` function to look like this:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        screen.Clear(0);
        tank.SetFrame(0);
        tank.Draw(screen, 0, 0);
        tank.SetFrame(1);
        tank.Draw(screen, 50, 0);
        tank.SetFrame(2);
        tank.Draw(screen, 100, 0);
        tank.SetFrame(3);
        tank.Draw(screen, 150, 0);
    }
```

<img src="/images/cpp-fast-track-4/03-four-tanks.png" alt="Four tanks in a row at the top of the screen, each one rotated a bit more" width="800" height="512" class="screenshot" />

Four tanks in a row, and with different frames shown. Nice, but it's a lot of typing, or copy-pasting. And what if we want to draw sixteen tanks in a row? That would become a lot of work to produce a long list of very similar code.

Luckily, C++ has loops to do this. Try the following:

```cpp source="tmpl8-part-04/game.cpp#L24-L32"
```

<img src="/images/cpp-fast-track-4/16-tanks.png" alt="Sixteen tanks in a row at the top of the screen, each one rotated a bit more" width="800" height="512" class="screenshot" />

<div class="caption">

Sixteen tanks in a row.

</div>

Sixteen tanks in a row, with only a few lines of code! And if the screen were wider, we could draw more without making the code any bigger: just change the number `16` in the `for` line to change the number of tanks that are drawn.

But how does this work, I hear you ask. Well, let's analyze the code, line by line:

```cpp source="tmpl8-part-04/game.cpp#L27"
```

This is the start of the loop, where you define what the loop will do and when it will stop. It has three important parts, each separated by a semicolon `;`.

The first part is `int i = 0`. This means we are declaring a variable with the name `i` and it will be an integer, and its initial value will be `0`.

:::caution
We encountered this concept in the [previous tutorial](/cpp-fast-track-3-variables/). Variables can be declared at global scope (i.e., outside any function), in which case they are available to all the functions below them in the current `.cpp` file.

We can also declare them inside a function, in which case they can be used in that function, but not outside it.

And we can declare a variable inside a block marked by curly brackets (`{` and `}`). It is not available outside that block, and if the name of the variable is the same as the name of a variable outside the block, the newly declared variable takes precedence.

The `for` statement is a special case: the `int i` in the first line is not declared inside the curly brackets, but it is in fact limited to the scope of the loop. That means that the variable `i` is not available outside the `for` loop.
:::

The second part is the condition that will be checked at the start of each loop. In our case, that check is `i < 16`. This means that, as long as `i` is smaller than `16`, the code below the `for` statement will run again and again and again.

The third and last part is what should happen at the end of the loop every time. In our case, that is `i++`. This is C++ shorthand for `i = i + 1`, and it means that, at the end of each loop, we want to add one to the current value of `i`, or *increment* it by one.

What this means in practice is that we are going to run the code below the `for` statement exactly 16 times: we start from zero, and add one each time we loop **as long as** we stay under 16. Once we reach 16, the loop stops: we skip over the code below the `for` statement and continue with whatever comes after it.

Now let's look at the line below the `for` statement:

```cpp source="tmpl8-part-04/game.cpp#L28"
```

Not much, is it? Just the opening bracket. In C++, anything between `{` and `}` is a *code block*, and it belongs together. In the case of the `for` loop, it means that all the code below the `for` statement, between `{` and `}`, is executed as many times as the loop wants.

The next line then:

```cpp source="tmpl8-part-04/game.cpp#L29"
```

You should recognise this line. It tells the sprite which animation frame is the active one. Except that this time, we don't tell it `0` or `1` or `2` directly, but we use the variable `i` that we declared in the `for` loop. Once the program runs, the actual value of `i` will be used. This means that the first time it runs, frame `0` will be set. The second time, frame `1`, the third time, frame `2` and so on.

The line after that one looks like this:

```cpp source="tmpl8-part-04/game.cpp#L30"
```

This is where we draw the sprite to the screen. This time, we are using `i` to change the `X` coordinate of where we are going to draw the sprite. Each frame of the sprite is 50 pixels wide, so by multiplying `i` by `50`, the first time around, `X` will be `0 * 50 = 0`, the second time it will be `1 * 50 = 50`, the third time `2 * 50 = 100`, and so on.

The last line of the `for` block looks like this:

```cpp source="tmpl8-part-04/game.cpp#L31"
```

It does nothing more than close the code block that is looped.

That's it, your first `for` loop! Note that you can put any kind of code inside the code block that follows a `for` statement, including other `for` statements. Try doing the assignment now, to find out why this could be useful.

## Assignment

Here's your task for today:

1.  Create a **nested** `for` loop (a loop inside a loop) that draws 10 rows of 16 tanks.

You should see something like this:

<img src="/images/cpp-fast-track-4/10-rows-of-16-tanks.png" alt="Ten rows of sixteen tanks, each tank in a row rotated a bit more" width="800" height="512" class="screenshot" />

<div class="caption">

This is what you should see if you draw 10 rows of 16 tanks.

</div>

Once you have completed this assignment you are ready for the next part.
