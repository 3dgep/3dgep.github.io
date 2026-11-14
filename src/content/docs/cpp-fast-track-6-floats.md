---
title: "C++ Fast Track for Games Programming Part 6: Floats"
description: "Floats: calculate with decimal numbers instead of integers, and what that means for movement and precision."
slug: cpp-fast-track-6-floats
lastUpdated: 2026-11-14
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

In this tutorial, we're going to look at how you can get the computer to calculate with decimal values (numbers like 3.14, called *floats*), instead of only whole numbers (numbers like 3, called *integers*).

## Getting the Stuff You Need

If you don't have it yet, get the template package:

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

As usual, start with a fresh copy of the template: extract the zip file to `C:\Projects\C++ Fast Track`, and rename the `tmpl8-main` folder to `Part 6`. Open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 6` folder, and remove the *hello world* code from the `Game::Tick` function in `game.cpp`:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
    }
```

## Balls Again

Before we start working with floats, let's set things up so that we can show something interesting. Let's load the ball sprite again! Add the highlighted lines to `game.hpp`, below the `rotatingGun` sprite, just like in the [previous part](/cpp-fast-track-5-conditions/):

```cpp source="tmpl8-part-06/game.hpp#L43-L48" {5-6}
```

:::note
You may have noticed that some images are `.tga` files, while others are `.png` files. These are image file formats. Other formats are `.gif`, `.jpg` and `.bmp`, and many others exist. Each format has advantages and disadvantages: `.gif` is small, but can store only 256 different colours; `.jpg` is often even smaller, but it is *lossy*, which means that it is only an approximation of the original image; `.png` is not lossy, and is supported by almost every program; `.tga` has a really simple internal layout.

The template loads images with a library, [stb_image](https://github.com/nothings/stb). A library is a collection of useful code that we can include in our project. Have a look at `Surface::LoadImage` (in `surface.cpp`, line 147) to see how `stbi_load` is used to load an image file into a surface.
:::

Now make the `Tick` function like this, with two variables above it:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    int spriteY = 0, speed = 1;
    void Game::Tick(float deltaTime)
    {
        screen.Clear(0);
        spriteY += ++speed;
        if (spriteY > (512 - 50))
        {
            spriteY = 512 - 50;
            speed = (-speed * 40) / 50;
        }
        ball.Draw(screen, 20, spriteY);
    }
```

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-6-int-bounce/index.html" title="A ball that bounces lower and lower, until it lies still on the bottom" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The ball loses some of its speed at every bounce. (This demo starts again every 10 seconds.)

</div>

Some interesting things here. First, `int spriteY = 0, speed = 1;` declares two variables of the same type in one line.

Then, the two lines from the [previous part](/cpp-fast-track-5-conditions/) that increase `speed` and add it to `spriteY` are collapsed into a single line on line 28, with the *increment* operator: `++speed` first adds 1 to `speed`, and then uses its new value. Just like in the previous part: first the speed, then the position. You can also write `speed++`, but that means something different: use the current value of `speed`, and add 1 to it *after* that. So `spriteY += speed++;` would move the ball first, and increase the speed after that.

The bouncing also contains an interesting construct: the inverted speed is multiplied by `40`, and then divided by `50`. Why? To dampen it: the ball now loses some of its energy at every bounce, so every bounce is lower than the previous one, until the ball lies still. Alternatively, we could have written:

```cpp title="game.cpp" showLineNumbers startLineNumber=32
            speed = -speed * 0.8;
```

But that is not the same thing. Try that line, and instead of pressing <kbd>F5</kbd> to run the program, press <kbd>Ctrl</kbd>+<kbd>F7</kbd> (or select *Build > Compile* in the main menu) to compile just the source file that is open in the editor. The compiler gives you a warning:

<img src="/images/cpp-fast-track-6/01-warning.png" alt="Visual Studio with warning C4244 in the Error List: conversion from double to int, possible loss of data" width="1151" height="655" class="screenshot" />

<div class="caption">

Warning C4244: `'=': conversion from 'double' to 'int', possible loss of data`.

</div>

The `speed` variable is an `int` (an integer, $\mathbb{Z}$), which means that it can store whole numbers only. `0.8` is a *real* number ($\mathbb{R}$) with a fraction (in C++, a `double`), so `-speed * 0.8` has a fraction too. When that result is stored in `speed`, the fraction has to be dropped (the value is *truncated*: `-12.8` becomes `-12`), which is a potential loss of precision; hence the warning. If you don't see the warning in the *Error List* at the bottom of the window, click its *Warnings* button: the warnings may be hidden.

## Switching to Floats

Let's convert our program so that it uses `float` variables. Here is the code:

```cpp title="game.cpp" showLineNumbers startLineNumber=24 {1,5-6,10}
    float spriteY = 0, speed = 1;
    void Game::Tick(float deltaTime)
    {
        screen.Clear(0);
        speed += 1.0f;
        spriteY += speed;
        if (spriteY > (512 - 50))
        {
            spriteY = 512 - 50;
            speed = -speed * 0.8f;
        }
        ball.Draw(screen, 20, spriteY);
    }
```

Not a lot changed: `speed` gets larger with `+= 1.0f` on line 28 (the `f` makes `1.0f` a `float` number instead of a `double`), and the damping is now done by multiplying with `0.8f` on line 33. The code still does the same thing. So what's the big deal?

Well, in the previous version, `speed` could only be a whole number (1, 2, 3, ...), so making the ball fall slower than 1 pixel per frame would be a bit of a problem. With floats, we totally have that option. We can for instance have a low gravity bounce by adding `0.1f` to `speed`:

```cpp title="game.cpp" showLineNumbers startLineNumber=28
        speed += 0.1f;
```

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-6-low-gravity/index.html" title="A ball that falls and bounces slowly, as if on the moon" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Low gravity: the speed increases by only 0.1 pixels per frame in every frame. (This demo starts again every 30 seconds.)

</div>

Or, we can adapt the speed to the frame rate. More about that below.

:::caution[Another warning]
We fixed the warning on line 32, but now there is a warning on line 35, where the ball is drawn: `'argument': conversion from 'float' to 'int', possible loss of data`. Why?

`Sprite::Draw` expects integer coordinates (a pixel position on the screen). Giving it a `float` value where it expects an `int` makes the compiler drop the fraction. To prevent the warning, we can drop the fraction ourselves, so that it is clear that the loss of precision is intentional. We do this by *casting* the `float` value to an `int` with `static_cast<int>(...)`:

```cpp title="game.cpp" showLineNumbers startLineNumber=35
        ball.Draw(screen, 20, static_cast<int>(spriteY));
```

You will also see the older way of writing a cast: `(int)spriteY`. It does the same thing here, but `static_cast` is easier to spot in your code, and the compiler checks it more strictly.

Likewise, we can cast an `int` value to a `float`:

```cpp
int a = 300;
float b = static_cast<float>(a);
```

This seems useless: the compiler doesn't warn us about this conversion, even without the cast. However, you need to be aware that this conversion happens: conversions take time, and they can become a performance issue once we start writing complex applications.
:::

With the cast, the float version bounces like this:

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-6-float-bounce/index.html" title="A ball that bounces lower and lower, calculated with floats" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The same bounce, now with floats. (This demo starts again every 10 seconds.)

</div>

## Frames per Second

How fast does our program run? The ball moves by `speed` pixels in every frame, so how fast it moves on the screen depends on how many frames per second (*FPS*) your computer draws. Let's print that in the window. The `deltaTime` value that `Tick` gets is the time since the previous frame, in milliseconds, so `1000.0f / deltaTime` is the number of frames per second. Add `#include <format>` to the top of `game.cpp`:

```cpp source="tmpl8-part-06/game.cpp#L1-L4" {3}
```

and add this line right below `screen.Clear(0);` in the `Tick` function:

```cpp source="tmpl8-part-06/game.cpp#L31"
```

`std::format` makes a piece of text in which the `{}` is replaced by the value after it. The `:.0f` inside the `{}` means: show the `float` value with 0 digits after the decimal point.

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-6-fps/index.html" title="The bouncing ball, with the frame rate in the top-left corner" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The frame rate in the top-left corner. (This demo starts again every 10 seconds.)

</div>

You will probably see 60 FPS (or 120, or 144, if you have a fast monitor). This is not a limit of your computer: the template waits for your screen before it shows the next frame, and your screen shows a fixed number of frames per second (60 Hz, 120 Hz or 144 Hz). This is called *VSync*, and you can switch it off. Add this line at the top of the `Tick` function:

```cpp source="tmpl8-part-06/game.cpp#L29"
```

`GetKeyPressed` is `true` in the frame in which you press a key (you saw it in [part 2](/cpp-fast-track-2-template/), with <kbd>Space</kbd>), and `ToggleVSync()` switches VSync on when it's off, and off when it's on. Run the program, and press <kbd>V</kbd> to switch VSync off (and press <kbd>V</kbd> again to switch it back on).

The template has more functions like this: `SetVSync(false)` switches VSync off, and `IsVSync()` tells you whether it's on. If you want the template to start without VSync, set `StartVSync` to `false` in `template.hpp`:

```cpp source="tmpl8-part-06/tmpl8/template.hpp#L18-L20"
```

What is *VSync* anyway? Well, old-skool monitors produced images by aiming an electron beam at a glass plate (and then your eyes). This beam zig-zagged over the pixels to light them up. At the bottom of the screen, this beam stops and returns to the top-left corner. This is called the *vertical retrace*. This is the perfect moment to start showing a new image: only during the retrace can the image be swapped without *tearing* (showing half of the old image and half of the new one). So *synchronising* frames to this retrace is useful. Modern monitors don't have an electron beam, but they still show a new image at a fixed rate, typically 60 times per second (60 Hz). Synchronising to the refresh rate of your screen limits your frame rate to 60 frames per second (or 120 if you have a 120 Hz monitor).

Without VSync (press <kbd>V</kbd>), the template doesn't wait for your screen any more, so it can reach much higher frame rates: hundreds or even thousands of frames per second, depending on your computer.

<img src="/images/cpp-fast-track-6/02-vsync-off.png" alt="The game window with VSync off: thousands of frames per second in the top-left corner, and the ball already lying still on the bottom" width="800" height="512" class="screenshot" />

<div class="caption">

With VSync off, the ball has bounced to a stop before you know it. (Your frame rate will be different: it depends on your computer.)

</div>

:::note
This only works on your computer: in a web browser, the browser decides when a frame is shown, and that's always in step with your screen. That's why the demos on this page show your screen's refresh rate.
:::

It is important to keep an eye on the performance of your program, so that you know whether a change you made had a positive or a negative impact on the performance. But keep in mind that what works well on your PC may not work the same way on everyone's PC.

## Too Fast to be Fun

By now, the ball is bouncing frantically on your screen. We could easily reduce its speed, but by how much? One computer runs at a few hundred FPS, another at thousands, and perhaps an old laptop barely reaches 60 FPS...

We can solve that.

Add the following line at the top of your `Game::Tick` function:

```cpp source="tmpl8-part-06/game.cpp#L26-L29" {3}
```

Run the program, and press <kbd>V</kbd> to switch VSync off.

This prints the value of the mysterious `deltaTime` variable that the `Tick` function has had since the start of this course:

<img src="/images/cpp-fast-track-6/03-deltatime-console.png" alt="The console window, with numbers around 0.3 printed below each other" width="976" height="268" class="screenshot" />

<div class="caption">

The time per frame, in milliseconds. (Your numbers will be different: they depend on your computer.)

</div>

The numbers in the console window are the frame durations, in milliseconds. In other words: we know how much time has passed since the previous call to the `Tick` function.

And that is very useful if you want something to move over the screen at a constant speed, regardless of the frame rate. If the ball should move 0.06 pixels per millisecond, it should move `0.06f * deltaTime` pixels in this frame: few pixels in a short frame, more pixels in a long frame. The same goes for the speed: gravity should change it by a certain amount per millisecond. Which takes us directly to the...

## Assignment

Convert the bouncing ball program to floats, with the speed and the gravity per millisecond instead of per frame. Make sure that the ball moves at the same speed, regardless of the frame rate: try it with VSync on and off. And make sure that your code compiles without compiler warnings.

When it works, it looks like this, at any frame rate:

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-6-assignment/index.html" title="The bouncing ball, moving at the same speed at any frame rate" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The ball bounces at the same speed, whatever the frame rate. (This demo starts again every 10 seconds.)

</div>

Once you have completed this assignment, you may continue with the next part.
