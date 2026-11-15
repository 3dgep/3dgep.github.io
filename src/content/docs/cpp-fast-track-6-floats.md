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

### How Precise Is a Float?

Floats can store fractions, but not every fraction. A `float` is 32 bits, just like an `int`, so it can store about the same number of different values: some four billion. But it spreads them out over a much larger range, from tiny numbers close to zero up to about 3.4 × 10³⁸. The trick is that the values are close together near zero, and further and further apart for larger numbers. A `double` uses 64 bits, so it has many more values, and they are much closer together: it takes much larger numbers before the gaps between them become large.

Zoom into the number line below to see this for yourself. Each lane has a tick for every value that a type can store; where the values are too close together to draw separately, you see a solid band. Scroll (or pinch) to zoom, drag to move, or hold the **+** button to keep zooming in:

<iframe class="embed embed-demo embed-number-line" src="/demos/number-precision.html" title="A zoomable number line that shows which values an int, an unsigned int, a float and a double can store" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Every value of `int`, `unsigned int`, `float` and `double`. Point at the number line to see which value each type stores for the number under the cursor, how far apart the values are there, and the bits that store it (see below).

</div>

Some places to visit with the buttons below the number line:

-   **0**: this is where the float values are closest together.
-   **0.1**: there is no `float` with the value 0.1. The closest one is 0.100000001… That's why adding `0.1f` ten times gives 1.0000001, and not 1. (A `double` can't store 0.1 exactly either, but it gets much closer.)
-   **2²⁴** (16,777,216): below this number, a `float` can store every whole number. Above it, the step between two floats is 2, so 16,777,217 can't be stored in a `float`: it becomes 16,777,216. An `int` has no problem with it.
-   **1 day in ms**: a day has 86,400,000 milliseconds. At that size, the step between two floats is 8: if you count time in milliseconds in a `float`, it can only change in steps of 8 ms after a day. That's half a frame at 60 frames per second. So a `float` is fine for a short time, such as the time of a single frame, but not for a timer that runs for hours.
-   **10²⁰**: too large for an `int`. A `float` can store it, but the step between two floats is about 8.8 × 10¹².
-   **FLT_EPSILON**: zoomed in on 1. Just above 1, two floats are 2⁻²³ ≈ 0.000000119 apart: this is the value of `FLT_EPSILON` (or `std::numeric_limits<float>::epsilon()`), the distance between 1.0f and the next float. The next float after 1 is 1.00000012. Just below 1, the floats are twice as close together.
-   **DBL_EPSILON**: the same for `double`. Just above 1, two doubles are 2⁻⁵² ≈ 0.000000000000000222 apart: this is `DBL_EPSILON` (or `std::numeric_limits<double>::epsilon()`), and the next double after 1 is 1.0000000000000002. That's about 16 decimal digits of precision, against about 7 for a `float`. At this zoom level, the `float` lane has just one value, 1: the floats around 1 are 2²⁹ (more than 500 million) times further apart.
-   **Radius**: the radius of the Earth, Jupiter, the Sun and the Solar system, in metres. Positions in a game are often stored as floats in metres. On the surface of the Earth (6,371 km from its centre), two floats are 0.5 m apart; at the radius of Jupiter it's 8 m, and at the radius of the Sun 64 m. At the orbit of Neptune, the edge of the Solar system (about 4.5 × 10¹² m), the step is 524,288 m: over 500 km. That's why games with very large worlds use doubles for positions, or keep the numbers small by moving the origin of the world along with the player. A `double` still has a step of about 1 mm at Neptune.

Point at the number line, and each lane also shows the *bits* that store the value under the cursor, with the *sign bit* in red:

-   An `int` stores a whole number in binary, in 32 bits. The leftmost bit is the sign bit: when it is 1, the number is negative. Negative numbers are stored in a way called *two's complement*: −1 is 32 ones, and −2 is `1111 … 1110`. That's why an `int` goes from −2,147,483,648 up to 2,147,483,647. An `unsigned int` has no sign bit: it uses all 32 bits for the numbers from 0 to 4,294,967,295.
-   A `float` splits its 32 bits into three parts: the sign bit, 8 bits for the *exponent* (in the colour of the lane) and 23 bits for the *mantissa*. The exponent determines a range between two powers of two: from ½ to 1, from 1 to 2, from 2 to 4, from 1,024 to 2,048, and so on. The mantissa picks one of the 2²³ (8,388,608) evenly spaced values in that range. Every range has the same number of values, so the larger the range, the further apart the values are: that's why the step doubles at every power of two. As a formula, the value is $(-1)^{\text{sign}} \times \left(1 + \frac{\text{mantissa}}{2^{23}}\right) \times 2^{\text{exponent} - 127}$. The mantissa is a whole number from 0 to 8,388,607 ($2^{23} - 1$): divided by 2²³, it becomes a fraction from 0 up to (but not including) 1. So $1 + \frac{\text{mantissa}}{2^{23}}$ is always at least 1 and less than 2, and the exponent moves it to the right range. For example, 3 is stored with sign 0, exponent 128 and mantissa 4,194,304 (2²²): $\left(1 + \frac{4194304}{8388608}\right) \times 2^{128 - 127} = 1.5 \times 2 = 3$.

    Exponents below 127 give the ranges below 1: 126 gives ½ to 1, 125 gives ¼ to ½, and so on. Two exponents are special. With exponent 0, the "1 +" is left out (the value is $\frac{\text{mantissa}}{2^{23}} \times 2^{-126}$): these are the smallest floats, evenly spaced all the way down to 0, and 0 itself is stored as exponent 0 and mantissa 0. Exponent 255 is used for infinity and for NaN ("not a number").

    <svg class="bits-diagram" viewBox="0 0 700 118" role="img" aria-label="The bits of 3.0f: sign 0, exponent 128, mantissa 4,194,304">
    <text class="title float" x="30" y="20">float 3.0f</text>
    <text class="label sign" x="30.0" y="43">sign: 0</text>
    <text class="count" x="30.0" y="95">1 bit</text>
    <rect class="field sign" x="30.0" y="52" width="20.0" height="26" rx="3"/>
    <text class="bit sign" x="40.0" y="69.5" text-anchor="middle">0</text>
    <text class="label float" x="91.8" y="43">exponent: 128</text>
    <text class="count" x="130.0" y="95" text-anchor="middle">8 bits</text>
    <rect class="field float" x="50.0" y="52" width="160.0" height="26" rx="3"/>
    <text class="bit float" x="60.0" y="69.5" text-anchor="middle">1</text>
    <line class="cell float" x1="70.0" y1="52" x2="70.0" y2="78"/>
    <text class="bit float" x="80.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell float" x1="90.0" y1="52" x2="90.0" y2="78"/>
    <text class="bit float" x="100.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell float" x1="110.0" y1="52" x2="110.0" y2="78"/>
    <text class="bit float" x="120.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell float" x1="130.0" y1="52" x2="130.0" y2="78"/>
    <text class="bit float" x="140.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell float" x1="150.0" y1="52" x2="150.0" y2="78"/>
    <text class="bit float" x="160.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell float" x1="170.0" y1="52" x2="170.0" y2="78"/>
    <text class="bit float" x="180.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell float" x1="190.0" y1="52" x2="190.0" y2="78"/>
    <text class="bit float" x="200.0" y="69.5" text-anchor="middle">0</text>
    <text class="label mant" x="347.5" y="43">mantissa: 4,194,304 (2²²)</text>
    <text class="count" x="440.0" y="95" text-anchor="middle">23 bits</text>
    <rect class="field mant" x="210.0" y="52" width="460.0" height="26" rx="3"/>
    <text class="bit mant" x="220.0" y="69.5" text-anchor="middle">1</text>
    <line class="cell mant" x1="230.0" y1="52" x2="230.0" y2="78"/>
    <text class="bit mant" x="240.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="250.0" y1="52" x2="250.0" y2="78"/>
    <text class="bit mant" x="260.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="270.0" y1="52" x2="270.0" y2="78"/>
    <text class="bit mant" x="280.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="290.0" y1="52" x2="290.0" y2="78"/>
    <text class="bit mant" x="300.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="310.0" y1="52" x2="310.0" y2="78"/>
    <text class="bit mant" x="320.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="330.0" y1="52" x2="330.0" y2="78"/>
    <text class="bit mant" x="340.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="350.0" y1="52" x2="350.0" y2="78"/>
    <text class="bit mant" x="360.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="370.0" y1="52" x2="370.0" y2="78"/>
    <text class="bit mant" x="380.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="390.0" y1="52" x2="390.0" y2="78"/>
    <text class="bit mant" x="400.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="410.0" y1="52" x2="410.0" y2="78"/>
    <text class="bit mant" x="420.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="430.0" y1="52" x2="430.0" y2="78"/>
    <text class="bit mant" x="440.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="450.0" y1="52" x2="450.0" y2="78"/>
    <text class="bit mant" x="460.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="470.0" y1="52" x2="470.0" y2="78"/>
    <text class="bit mant" x="480.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="490.0" y1="52" x2="490.0" y2="78"/>
    <text class="bit mant" x="500.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="510.0" y1="52" x2="510.0" y2="78"/>
    <text class="bit mant" x="520.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="530.0" y1="52" x2="530.0" y2="78"/>
    <text class="bit mant" x="540.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="550.0" y1="52" x2="550.0" y2="78"/>
    <text class="bit mant" x="560.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="570.0" y1="52" x2="570.0" y2="78"/>
    <text class="bit mant" x="580.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="590.0" y1="52" x2="590.0" y2="78"/>
    <text class="bit mant" x="600.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="610.0" y1="52" x2="610.0" y2="78"/>
    <text class="bit mant" x="620.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="630.0" y1="52" x2="630.0" y2="78"/>
    <text class="bit mant" x="640.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="650.0" y1="52" x2="650.0" y2="78"/>
    <text class="bit mant" x="660.0" y="69.5" text-anchor="middle">0</text>
    <g transform="translate(679 97) scale(0.7)" opacity="0.35"><title>Made with Claude</title><path d="M4.709 15.955l4.72-2.647.08-.23-.08-.128H9.2l-.79-.048-2.698-.073-2.339-.097-2.266-.122-.571-.121L0 11.784l.055-.352.48-.321.686.06 1.52.103 2.278.158 1.652.097 2.449.255h.389l.055-.157-.134-.098-.103-.097-2.358-1.596-2.552-1.688-1.336-.972-.724-.491-.364-.462-.158-1.008.656-.722.881.06.225.061.893.686 1.908 1.476 2.491 1.833.365.304.145-.103.019-.073-.164-.274-1.355-2.446-1.446-2.49-.644-1.032-.17-.619a2.97 2.97 0 01-.104-.729L6.283.134 6.696 0l.996.134.42.364.62 1.414 1.002 2.229 1.555 3.03.456.898.243.832.091.255h.158V9.01l.128-1.706.237-2.095.23-2.695.08-.76.376-.91.747-.492.584.28.48.685-.067.444-.286 1.851-.559 2.903-.364 1.942h.212l.243-.242.985-1.306 1.652-2.064.73-.82.85-.904.547-.431h1.033l.76 1.129-.34 1.166-1.064 1.347-.881 1.142-1.264 1.7-.79 1.36.073.11.188-.02 2.856-.606 1.543-.28 1.841-.315.833.388.091.395-.328.807-1.969.486-2.309.462-3.439.813-.042.03.049.061 1.549.146.662.036h1.622l3.02.225.79.522.474.638-.079.485-1.215.62-1.64-.389-3.829-.91-1.312-.329h-.182v.11l1.093 1.068 2.006 1.81 2.509 2.33.127.578-.322.455-.34-.049-2.205-1.657-.851-.747-1.926-1.62h-.128v.17l.444.649 2.345 3.521.122 1.08-.17.353-.608.213-.668-.122-1.374-1.925-1.415-2.167-1.143-1.943-.14.08-.674 7.254-.316.37-.729.28-.607-.461-.322-.747.322-1.476.389-1.924.315-1.53.286-1.9.17-.632-.012-.042-.14.018-1.434 1.967-2.18 2.945-1.726 1.845-.414.164-.717-.37.067-.662.401-.589 2.388-3.036 1.44-1.882.93-1.086-.006-.158h-.055L4.132 18.56l-1.13.146-.487-.456.061-.746.231-.243 1.908-1.312-.006.006z" fill="#D97757"/></g>
    </svg>

    <div class="caption">

    The bits of 3.0f: the sign bit (red), the exponent (orange) and the mantissa (grey). The value is $\left(1 + \frac{4194304}{8388608}\right) \times 2^{128 - 127} = 1.5 \times 2 = 3$.

    </div>
-   A `double` works the same way, with 11 bits for the exponent and 52 bits for the mantissa: it has 2⁵² values in every range.

    <svg class="bits-diagram" viewBox="0 0 700 118" role="img" aria-label="The bits of 3.0 as a double: sign 0, exponent 1024, mantissa 2,251,799,813,685,248">
    <text class="title double" x="30" y="20">double 3.0</text>
    <text class="label sign" x="30.0" y="43">sign: 0</text>
    <text class="count" x="30.0" y="95">1 bit</text>
    <rect class="field sign" x="30.0" y="52" width="10.0" height="26" rx="3"/>
    <text class="bit sign small" x="35.0" y="69.5" text-anchor="middle">0</text>
    <text class="label double" x="91.8" y="43">exponent: 1,024</text>
    <text class="count" x="95.0" y="95" text-anchor="middle">11 bits</text>
    <rect class="field double" x="40.0" y="52" width="110.0" height="26" rx="3"/>
    <text class="bit double small" x="45.0" y="69.5" text-anchor="middle">1</text>
    <line class="cell double" x1="50.0" y1="52" x2="50.0" y2="78"/>
    <text class="bit double small" x="55.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell double" x1="60.0" y1="52" x2="60.0" y2="78"/>
    <text class="bit double small" x="65.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell double" x1="70.0" y1="52" x2="70.0" y2="78"/>
    <text class="bit double small" x="75.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell double" x1="80.0" y1="52" x2="80.0" y2="78"/>
    <text class="bit double small" x="85.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell double" x1="90.0" y1="52" x2="90.0" y2="78"/>
    <text class="bit double small" x="95.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell double" x1="100.0" y1="52" x2="100.0" y2="78"/>
    <text class="bit double small" x="105.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell double" x1="110.0" y1="52" x2="110.0" y2="78"/>
    <text class="bit double small" x="115.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell double" x1="120.0" y1="52" x2="120.0" y2="78"/>
    <text class="bit double small" x="125.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell double" x1="130.0" y1="52" x2="130.0" y2="78"/>
    <text class="bit double small" x="135.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell double" x1="140.0" y1="52" x2="140.0" y2="78"/>
    <text class="bit double small" x="145.0" y="69.5" text-anchor="middle">0</text>
    <text class="label mant" x="273.1" y="43">mantissa: 2,251,799,813,685,248 (2⁵¹)</text>
    <text class="count" x="410.0" y="95" text-anchor="middle">52 bits</text>
    <rect class="field mant" x="150.0" y="52" width="520.0" height="26" rx="3"/>
    <text class="bit mant small" x="155.0" y="69.5" text-anchor="middle">1</text>
    <line class="cell mant" x1="160.0" y1="52" x2="160.0" y2="78"/>
    <text class="bit mant small" x="165.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="170.0" y1="52" x2="170.0" y2="78"/>
    <text class="bit mant small" x="175.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="180.0" y1="52" x2="180.0" y2="78"/>
    <text class="bit mant small" x="185.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="190.0" y1="52" x2="190.0" y2="78"/>
    <text class="bit mant small" x="195.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="200.0" y1="52" x2="200.0" y2="78"/>
    <text class="bit mant small" x="205.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="210.0" y1="52" x2="210.0" y2="78"/>
    <text class="bit mant small" x="215.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="220.0" y1="52" x2="220.0" y2="78"/>
    <text class="bit mant small" x="225.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="230.0" y1="52" x2="230.0" y2="78"/>
    <text class="bit mant small" x="235.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="240.0" y1="52" x2="240.0" y2="78"/>
    <text class="bit mant small" x="245.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="250.0" y1="52" x2="250.0" y2="78"/>
    <text class="bit mant small" x="255.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="260.0" y1="52" x2="260.0" y2="78"/>
    <text class="bit mant small" x="265.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="270.0" y1="52" x2="270.0" y2="78"/>
    <text class="bit mant small" x="275.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="280.0" y1="52" x2="280.0" y2="78"/>
    <text class="bit mant small" x="285.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="290.0" y1="52" x2="290.0" y2="78"/>
    <text class="bit mant small" x="295.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="300.0" y1="52" x2="300.0" y2="78"/>
    <text class="bit mant small" x="305.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="310.0" y1="52" x2="310.0" y2="78"/>
    <text class="bit mant small" x="315.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="320.0" y1="52" x2="320.0" y2="78"/>
    <text class="bit mant small" x="325.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="330.0" y1="52" x2="330.0" y2="78"/>
    <text class="bit mant small" x="335.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="340.0" y1="52" x2="340.0" y2="78"/>
    <text class="bit mant small" x="345.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="350.0" y1="52" x2="350.0" y2="78"/>
    <text class="bit mant small" x="355.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="360.0" y1="52" x2="360.0" y2="78"/>
    <text class="bit mant small" x="365.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="370.0" y1="52" x2="370.0" y2="78"/>
    <text class="bit mant small" x="375.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="380.0" y1="52" x2="380.0" y2="78"/>
    <text class="bit mant small" x="385.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="390.0" y1="52" x2="390.0" y2="78"/>
    <text class="bit mant small" x="395.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="400.0" y1="52" x2="400.0" y2="78"/>
    <text class="bit mant small" x="405.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="410.0" y1="52" x2="410.0" y2="78"/>
    <text class="bit mant small" x="415.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="420.0" y1="52" x2="420.0" y2="78"/>
    <text class="bit mant small" x="425.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="430.0" y1="52" x2="430.0" y2="78"/>
    <text class="bit mant small" x="435.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="440.0" y1="52" x2="440.0" y2="78"/>
    <text class="bit mant small" x="445.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="450.0" y1="52" x2="450.0" y2="78"/>
    <text class="bit mant small" x="455.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="460.0" y1="52" x2="460.0" y2="78"/>
    <text class="bit mant small" x="465.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="470.0" y1="52" x2="470.0" y2="78"/>
    <text class="bit mant small" x="475.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="480.0" y1="52" x2="480.0" y2="78"/>
    <text class="bit mant small" x="485.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="490.0" y1="52" x2="490.0" y2="78"/>
    <text class="bit mant small" x="495.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="500.0" y1="52" x2="500.0" y2="78"/>
    <text class="bit mant small" x="505.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="510.0" y1="52" x2="510.0" y2="78"/>
    <text class="bit mant small" x="515.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="520.0" y1="52" x2="520.0" y2="78"/>
    <text class="bit mant small" x="525.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="530.0" y1="52" x2="530.0" y2="78"/>
    <text class="bit mant small" x="535.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="540.0" y1="52" x2="540.0" y2="78"/>
    <text class="bit mant small" x="545.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="550.0" y1="52" x2="550.0" y2="78"/>
    <text class="bit mant small" x="555.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="560.0" y1="52" x2="560.0" y2="78"/>
    <text class="bit mant small" x="565.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="570.0" y1="52" x2="570.0" y2="78"/>
    <text class="bit mant small" x="575.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="580.0" y1="52" x2="580.0" y2="78"/>
    <text class="bit mant small" x="585.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="590.0" y1="52" x2="590.0" y2="78"/>
    <text class="bit mant small" x="595.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="600.0" y1="52" x2="600.0" y2="78"/>
    <text class="bit mant small" x="605.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="610.0" y1="52" x2="610.0" y2="78"/>
    <text class="bit mant small" x="615.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="620.0" y1="52" x2="620.0" y2="78"/>
    <text class="bit mant small" x="625.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="630.0" y1="52" x2="630.0" y2="78"/>
    <text class="bit mant small" x="635.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="640.0" y1="52" x2="640.0" y2="78"/>
    <text class="bit mant small" x="645.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="650.0" y1="52" x2="650.0" y2="78"/>
    <text class="bit mant small" x="655.0" y="69.5" text-anchor="middle">0</text>
    <line class="cell mant" x1="660.0" y1="52" x2="660.0" y2="78"/>
    <text class="bit mant small" x="665.0" y="69.5" text-anchor="middle">0</text>
    <g transform="translate(679 97) scale(0.7)" opacity="0.35"><title>Made with Claude</title><path d="M4.709 15.955l4.72-2.647.08-.23-.08-.128H9.2l-.79-.048-2.698-.073-2.339-.097-2.266-.122-.571-.121L0 11.784l.055-.352.48-.321.686.06 1.52.103 2.278.158 1.652.097 2.449.255h.389l.055-.157-.134-.098-.103-.097-2.358-1.596-2.552-1.688-1.336-.972-.724-.491-.364-.462-.158-1.008.656-.722.881.06.225.061.893.686 1.908 1.476 2.491 1.833.365.304.145-.103.019-.073-.164-.274-1.355-2.446-1.446-2.49-.644-1.032-.17-.619a2.97 2.97 0 01-.104-.729L6.283.134 6.696 0l.996.134.42.364.62 1.414 1.002 2.229 1.555 3.03.456.898.243.832.091.255h.158V9.01l.128-1.706.237-2.095.23-2.695.08-.76.376-.91.747-.492.584.28.48.685-.067.444-.286 1.851-.559 2.903-.364 1.942h.212l.243-.242.985-1.306 1.652-2.064.73-.82.85-.904.547-.431h1.033l.76 1.129-.34 1.166-1.064 1.347-.881 1.142-1.264 1.7-.79 1.36.073.11.188-.02 2.856-.606 1.543-.28 1.841-.315.833.388.091.395-.328.807-1.969.486-2.309.462-3.439.813-.042.03.049.061 1.549.146.662.036h1.622l3.02.225.79.522.474.638-.079.485-1.215.62-1.64-.389-3.829-.91-1.312-.329h-.182v.11l1.093 1.068 2.006 1.81 2.509 2.33.127.578-.322.455-.34-.049-2.205-1.657-.851-.747-1.926-1.62h-.128v.17l.444.649 2.345 3.521.122 1.08-.17.353-.608.213-.668-.122-1.374-1.925-1.415-2.167-1.143-1.943-.14.08-.674 7.254-.316.37-.729.28-.607-.461-.322-.747.322-1.476.389-1.924.315-1.53.286-1.9.17-.632-.012-.042-.14.018-1.434 1.967-2.18 2.945-1.726 1.845-.414.164-.717-.37.067-.662.401-.589 2.388-3.036 1.44-1.882.93-1.086-.006-.158h-.055L4.132 18.56l-1.13.146-.487-.456.061-.746.231-.243 1.908-1.312-.006.006z" fill="#D97757"/></g>
    </svg>

    <div class="caption">

    The bits of 3.0 as a `double`: the sign bit (red), the exponent (purple) and the mantissa (grey). For a `double`, 1023 is subtracted from the exponent: $\left(1 + \frac{2^{51}}{2^{52}}\right) \times 2^{1024 - 1023} = 1.5 \times 2 = 3$.

    </div>

Try it at **2²⁴**: just below 16,777,216, the exponent is `10010110` (the range from 2²³ to 2²⁴, with one float for every whole number). Just above it, the exponent is `10010111` (from 2²⁴ to 2²⁵: the same number of floats in a range that is twice as large, so one float for every two whole numbers). And at **0**, the exponent is all zeros: these are the smallest floats, without the "1 +". You will learn more about binary numbers in [part 9](/cpp-fast-track-9-colours/).

The graph below puts all of this together. It shows the gap between two neighbouring values (the precision) against the value (the range), for `float` and `double`. Both axes are *logarithmic*: every grid line is ten times larger than the one before it, or else the whole range of a `double` wouldn't fit. Point at the graph to read the gaps, and scroll (or pinch) to zoom:

<iframe class="embed embed-demo embed-precision-graph" src="/demos/float-precision-graph.html" title="A log-log graph of the gap between two neighbouring values against the value, for float and double" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The precision of `float` and `double`. The higher the line, the larger the gaps between the values.

</div>

-   The `float` and `double` lines are staircases: the gap doubles at every power of two. Zoomed out, the steps form a straight diagonal line: the gap is always about the number divided by 2²³ for a `float`, and divided by 2⁵² for a `double`. So a `float` always has about 7 significant digits, whether the number is tiny or huge, and a `double` about 16. The `double` line is the same line, 29 powers of two lower, and much longer.
-   The points on the lines are the limits of each type, which you can find in `<cfloat>`. Each line ends at the largest value of its type (`FLT_MAX` and `DBL_MAX`): beyond it, the result is infinity.
-   Below the smallest *normal* value (`FLT_MIN` and `DBL_MIN`), the line goes flat: these are the *subnormal* numbers (exponent 0), which all have the same gap. That gap is also the smallest value above 0 (`FLT_TRUE_MIN` and `DBL_TRUE_MIN`).
-   `FLT_EPSILON` and `DBL_EPSILON` are the gaps at 1.
-   The other dots are the places from the number line above, on both lines. Read off the gap at the size of your numbers to see whether a `float` is precise enough: a position in metres near the edge of the Solar system is only precise to about 500 km in a `float`, and to about 1 mm in a `double`.

### Infinity and NaN

The largest exponent, 255 (all ones), is not used for numbers. A `float` with exponent 255 is one of the *special values*:

-   With mantissa 0, it is *infinity*: `inf`, or `-inf` when the sign bit is set. You get infinity when a result is too large for a `float`: $(3 \times 10^{38}) \times 10$ is not a number that is a bit off, but `inf`. Dividing a number (other than 0) by zero also gives infinity. Infinity behaves the way you would expect: it is larger than every other `float`, and $\infty + 1 = \infty$.
-   With any other mantissa, it is *NaN*: "not a number". You get NaN from a calculation that has no sensible answer: `0.0f / 0.0f`, $\infty - \infty$, or the square root of a negative number (`std::sqrt(-1.0f)`).

<svg class="bits-diagram" viewBox="0 0 700 320" role="img" aria-label="The bits of plus infinity, minus infinity and NaN as a float: the exponent is 255 (all ones); the mantissa is 0 for infinity and not 0 for NaN">
<text class="title float" x="30" y="20">+infinity</text>
<text class="label sign" x="30.0" y="41">sign: 0</text>
<rect class="field sign" x="30.0" y="50" width="20.0" height="24" rx="3"/>
<text class="bit sign" x="40.0" y="66.5" text-anchor="middle">0</text>
<text class="label float" x="91.8" y="41">exponent: 255</text>
<rect class="field float" x="50.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit float" x="60.0" y="66.5" text-anchor="middle">1</text>
<line class="cell float" x1="70.0" y1="50" x2="70.0" y2="74"/>
<text class="bit float" x="80.0" y="66.5" text-anchor="middle">1</text>
<line class="cell float" x1="90.0" y1="50" x2="90.0" y2="74"/>
<text class="bit float" x="100.0" y="66.5" text-anchor="middle">1</text>
<line class="cell float" x1="110.0" y1="50" x2="110.0" y2="74"/>
<text class="bit float" x="120.0" y="66.5" text-anchor="middle">1</text>
<line class="cell float" x1="130.0" y1="50" x2="130.0" y2="74"/>
<text class="bit float" x="140.0" y="66.5" text-anchor="middle">1</text>
<line class="cell float" x1="150.0" y1="50" x2="150.0" y2="74"/>
<text class="bit float" x="160.0" y="66.5" text-anchor="middle">1</text>
<line class="cell float" x1="170.0" y1="50" x2="170.0" y2="74"/>
<text class="bit float" x="180.0" y="66.5" text-anchor="middle">1</text>
<line class="cell float" x1="190.0" y1="50" x2="190.0" y2="74"/>
<text class="bit float" x="200.0" y="66.5" text-anchor="middle">1</text>
<text class="label mant" x="399.3" y="41">mantissa: 0</text>
<rect class="field mant" x="210.0" y="50" width="460.0" height="24" rx="3"/>
<text class="bit mant" x="220.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="230.0" y1="50" x2="230.0" y2="74"/>
<text class="bit mant" x="240.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="250.0" y1="50" x2="250.0" y2="74"/>
<text class="bit mant" x="260.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="270.0" y1="50" x2="270.0" y2="74"/>
<text class="bit mant" x="280.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="290.0" y1="50" x2="290.0" y2="74"/>
<text class="bit mant" x="300.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="310.0" y1="50" x2="310.0" y2="74"/>
<text class="bit mant" x="320.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="330.0" y1="50" x2="330.0" y2="74"/>
<text class="bit mant" x="340.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="350.0" y1="50" x2="350.0" y2="74"/>
<text class="bit mant" x="360.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="370.0" y1="50" x2="370.0" y2="74"/>
<text class="bit mant" x="380.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="390.0" y1="50" x2="390.0" y2="74"/>
<text class="bit mant" x="400.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="410.0" y1="50" x2="410.0" y2="74"/>
<text class="bit mant" x="420.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="430.0" y1="50" x2="430.0" y2="74"/>
<text class="bit mant" x="440.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="450.0" y1="50" x2="450.0" y2="74"/>
<text class="bit mant" x="460.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="470.0" y1="50" x2="470.0" y2="74"/>
<text class="bit mant" x="480.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="490.0" y1="50" x2="490.0" y2="74"/>
<text class="bit mant" x="500.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="510.0" y1="50" x2="510.0" y2="74"/>
<text class="bit mant" x="520.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="530.0" y1="50" x2="530.0" y2="74"/>
<text class="bit mant" x="540.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="550.0" y1="50" x2="550.0" y2="74"/>
<text class="bit mant" x="560.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="570.0" y1="50" x2="570.0" y2="74"/>
<text class="bit mant" x="580.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="590.0" y1="50" x2="590.0" y2="74"/>
<text class="bit mant" x="600.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="610.0" y1="50" x2="610.0" y2="74"/>
<text class="bit mant" x="620.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="630.0" y1="50" x2="630.0" y2="74"/>
<text class="bit mant" x="640.0" y="66.5" text-anchor="middle">0</text>
<line class="cell mant" x1="650.0" y1="50" x2="650.0" y2="74"/>
<text class="bit mant" x="660.0" y="66.5" text-anchor="middle">0</text>
<text class="title float" x="30" y="124">−infinity</text>
<text class="label sign" x="30.0" y="145">sign: 1</text>
<rect class="field sign" x="30.0" y="154" width="20.0" height="24" rx="3"/>
<text class="bit sign" x="40.0" y="170.5" text-anchor="middle">1</text>
<text class="label float" x="91.8" y="145">exponent: 255</text>
<rect class="field float" x="50.0" y="154" width="160.0" height="24" rx="3"/>
<text class="bit float" x="60.0" y="170.5" text-anchor="middle">1</text>
<line class="cell float" x1="70.0" y1="154" x2="70.0" y2="178"/>
<text class="bit float" x="80.0" y="170.5" text-anchor="middle">1</text>
<line class="cell float" x1="90.0" y1="154" x2="90.0" y2="178"/>
<text class="bit float" x="100.0" y="170.5" text-anchor="middle">1</text>
<line class="cell float" x1="110.0" y1="154" x2="110.0" y2="178"/>
<text class="bit float" x="120.0" y="170.5" text-anchor="middle">1</text>
<line class="cell float" x1="130.0" y1="154" x2="130.0" y2="178"/>
<text class="bit float" x="140.0" y="170.5" text-anchor="middle">1</text>
<line class="cell float" x1="150.0" y1="154" x2="150.0" y2="178"/>
<text class="bit float" x="160.0" y="170.5" text-anchor="middle">1</text>
<line class="cell float" x1="170.0" y1="154" x2="170.0" y2="178"/>
<text class="bit float" x="180.0" y="170.5" text-anchor="middle">1</text>
<line class="cell float" x1="190.0" y1="154" x2="190.0" y2="178"/>
<text class="bit float" x="200.0" y="170.5" text-anchor="middle">1</text>
<text class="label mant" x="399.3" y="145">mantissa: 0</text>
<rect class="field mant" x="210.0" y="154" width="460.0" height="24" rx="3"/>
<text class="bit mant" x="220.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="230.0" y1="154" x2="230.0" y2="178"/>
<text class="bit mant" x="240.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="250.0" y1="154" x2="250.0" y2="178"/>
<text class="bit mant" x="260.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="270.0" y1="154" x2="270.0" y2="178"/>
<text class="bit mant" x="280.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="290.0" y1="154" x2="290.0" y2="178"/>
<text class="bit mant" x="300.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="310.0" y1="154" x2="310.0" y2="178"/>
<text class="bit mant" x="320.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="330.0" y1="154" x2="330.0" y2="178"/>
<text class="bit mant" x="340.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="350.0" y1="154" x2="350.0" y2="178"/>
<text class="bit mant" x="360.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="370.0" y1="154" x2="370.0" y2="178"/>
<text class="bit mant" x="380.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="390.0" y1="154" x2="390.0" y2="178"/>
<text class="bit mant" x="400.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="410.0" y1="154" x2="410.0" y2="178"/>
<text class="bit mant" x="420.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="430.0" y1="154" x2="430.0" y2="178"/>
<text class="bit mant" x="440.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="450.0" y1="154" x2="450.0" y2="178"/>
<text class="bit mant" x="460.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="470.0" y1="154" x2="470.0" y2="178"/>
<text class="bit mant" x="480.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="490.0" y1="154" x2="490.0" y2="178"/>
<text class="bit mant" x="500.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="510.0" y1="154" x2="510.0" y2="178"/>
<text class="bit mant" x="520.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="530.0" y1="154" x2="530.0" y2="178"/>
<text class="bit mant" x="540.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="550.0" y1="154" x2="550.0" y2="178"/>
<text class="bit mant" x="560.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="570.0" y1="154" x2="570.0" y2="178"/>
<text class="bit mant" x="580.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="590.0" y1="154" x2="590.0" y2="178"/>
<text class="bit mant" x="600.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="610.0" y1="154" x2="610.0" y2="178"/>
<text class="bit mant" x="620.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="630.0" y1="154" x2="630.0" y2="178"/>
<text class="bit mant" x="640.0" y="170.5" text-anchor="middle">0</text>
<line class="cell mant" x1="650.0" y1="154" x2="650.0" y2="178"/>
<text class="bit mant" x="660.0" y="170.5" text-anchor="middle">0</text>
<text class="title float" x="30" y="228">NaN (from 0.0f / 0.0f)</text>
<text class="label sign" x="30.0" y="249">sign: 1</text>
<text class="count" x="30.0" y="299">1 bit</text>
<rect class="field sign" x="30.0" y="258" width="20.0" height="24" rx="3"/>
<text class="bit sign" x="40.0" y="274.5" text-anchor="middle">1</text>
<text class="label float" x="91.8" y="249">exponent: 255</text>
<text class="count" x="130.0" y="299" text-anchor="middle">8 bits</text>
<rect class="field float" x="50.0" y="258" width="160.0" height="24" rx="3"/>
<text class="bit float" x="60.0" y="274.5" text-anchor="middle">1</text>
<line class="cell float" x1="70.0" y1="258" x2="70.0" y2="282"/>
<text class="bit float" x="80.0" y="274.5" text-anchor="middle">1</text>
<line class="cell float" x1="90.0" y1="258" x2="90.0" y2="282"/>
<text class="bit float" x="100.0" y="274.5" text-anchor="middle">1</text>
<line class="cell float" x1="110.0" y1="258" x2="110.0" y2="282"/>
<text class="bit float" x="120.0" y="274.5" text-anchor="middle">1</text>
<line class="cell float" x1="130.0" y1="258" x2="130.0" y2="282"/>
<text class="bit float" x="140.0" y="274.5" text-anchor="middle">1</text>
<line class="cell float" x1="150.0" y1="258" x2="150.0" y2="282"/>
<text class="bit float" x="160.0" y="274.5" text-anchor="middle">1</text>
<line class="cell float" x1="170.0" y1="258" x2="170.0" y2="282"/>
<text class="bit float" x="180.0" y="274.5" text-anchor="middle">1</text>
<line class="cell float" x1="190.0" y1="258" x2="190.0" y2="282"/>
<text class="bit float" x="200.0" y="274.5" text-anchor="middle">1</text>
<text class="label mant" x="340.1" y="249">mantissa: 4,194,304 (not 0)</text>
<text class="count" x="440.0" y="299" text-anchor="middle">23 bits</text>
<rect class="field mant" x="210.0" y="258" width="460.0" height="24" rx="3"/>
<text class="bit mant" x="220.0" y="274.5" text-anchor="middle">1</text>
<line class="cell mant" x1="230.0" y1="258" x2="230.0" y2="282"/>
<text class="bit mant" x="240.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="250.0" y1="258" x2="250.0" y2="282"/>
<text class="bit mant" x="260.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="270.0" y1="258" x2="270.0" y2="282"/>
<text class="bit mant" x="280.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="290.0" y1="258" x2="290.0" y2="282"/>
<text class="bit mant" x="300.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="310.0" y1="258" x2="310.0" y2="282"/>
<text class="bit mant" x="320.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="330.0" y1="258" x2="330.0" y2="282"/>
<text class="bit mant" x="340.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="350.0" y1="258" x2="350.0" y2="282"/>
<text class="bit mant" x="360.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="370.0" y1="258" x2="370.0" y2="282"/>
<text class="bit mant" x="380.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="390.0" y1="258" x2="390.0" y2="282"/>
<text class="bit mant" x="400.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="410.0" y1="258" x2="410.0" y2="282"/>
<text class="bit mant" x="420.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="430.0" y1="258" x2="430.0" y2="282"/>
<text class="bit mant" x="440.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="450.0" y1="258" x2="450.0" y2="282"/>
<text class="bit mant" x="460.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="470.0" y1="258" x2="470.0" y2="282"/>
<text class="bit mant" x="480.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="490.0" y1="258" x2="490.0" y2="282"/>
<text class="bit mant" x="500.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="510.0" y1="258" x2="510.0" y2="282"/>
<text class="bit mant" x="520.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="530.0" y1="258" x2="530.0" y2="282"/>
<text class="bit mant" x="540.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="550.0" y1="258" x2="550.0" y2="282"/>
<text class="bit mant" x="560.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="570.0" y1="258" x2="570.0" y2="282"/>
<text class="bit mant" x="580.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="590.0" y1="258" x2="590.0" y2="282"/>
<text class="bit mant" x="600.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="610.0" y1="258" x2="610.0" y2="282"/>
<text class="bit mant" x="620.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="630.0" y1="258" x2="630.0" y2="282"/>
<text class="bit mant" x="640.0" y="274.5" text-anchor="middle">0</text>
<line class="cell mant" x1="650.0" y1="258" x2="650.0" y2="282"/>
<text class="bit mant" x="660.0" y="274.5" text-anchor="middle">0</text>
<g transform="translate(679 299) scale(0.7)" opacity="0.35"><title>Made with Claude</title><path d="M4.709 15.955l4.72-2.647.08-.23-.08-.128H9.2l-.79-.048-2.698-.073-2.339-.097-2.266-.122-.571-.121L0 11.784l.055-.352.48-.321.686.06 1.52.103 2.278.158 1.652.097 2.449.255h.389l.055-.157-.134-.098-.103-.097-2.358-1.596-2.552-1.688-1.336-.972-.724-.491-.364-.462-.158-1.008.656-.722.881.06.225.061.893.686 1.908 1.476 2.491 1.833.365.304.145-.103.019-.073-.164-.274-1.355-2.446-1.446-2.49-.644-1.032-.17-.619a2.97 2.97 0 01-.104-.729L6.283.134 6.696 0l.996.134.42.364.62 1.414 1.002 2.229 1.555 3.03.456.898.243.832.091.255h.158V9.01l.128-1.706.237-2.095.23-2.695.08-.76.376-.91.747-.492.584.28.48.685-.067.444-.286 1.851-.559 2.903-.364 1.942h.212l.243-.242.985-1.306 1.652-2.064.73-.82.85-.904.547-.431h1.033l.76 1.129-.34 1.166-1.064 1.347-.881 1.142-1.264 1.7-.79 1.36.073.11.188-.02 2.856-.606 1.543-.28 1.841-.315.833.388.091.395-.328.807-1.969.486-2.309.462-3.439.813-.042.03.049.061 1.549.146.662.036h1.622l3.02.225.79.522.474.638-.079.485-1.215.62-1.64-.389-3.829-.91-1.312-.329h-.182v.11l1.093 1.068 2.006 1.81 2.509 2.33.127.578-.322.455-.34-.049-2.205-1.657-.851-.747-1.926-1.62h-.128v.17l.444.649 2.345 3.521.122 1.08-.17.353-.608.213-.668-.122-1.374-1.925-1.415-2.167-1.143-1.943-.14.08-.674 7.254-.316.37-.729.28-.607-.461-.322-.747.322-1.476.389-1.924.315-1.53.286-1.9.17-.632-.012-.042-.14.018-1.434 1.967-2.18 2.945-1.726 1.845-.414.164-.717-.37.067-.662.401-.589 2.388-3.036 1.44-1.882.93-1.086-.006-.158h-.055L4.132 18.56l-1.13.146-.487-.456.061-.746.231-.243 1.908-1.312-.006.006z" fill="#D97757"/></g>
</svg>

<div class="caption">

Infinity, minus infinity, and the NaN that you get from `0.0f / 0.0f`. The sign bit of that NaN is set, so `std::cout` prints it as `-nan(ind)`.

</div>

NaN is sneaky. Every calculation with a NaN gives NaN again, so a single NaN spreads: a NaN in the speed of the ball gives a NaN position in the next frame, and the ball disappears from the screen. And NaN is not equal to anything, not even to itself: `x == x` is `false` when `x` is NaN. To check for it, use `std::isnan(x)` (from `<cmath>`); for infinity, there is `std::isinf(x)`.

A `double` has the same special values, with exponent 2047 (all 11 bits ones). In the demo, the `float` lane ends at the largest `float`, about 3.4 × 10³⁸ (click **float** next to *Range of*): a `float` calculation that goes beyond it gives infinity.

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
