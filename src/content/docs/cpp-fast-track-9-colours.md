---
title: "C++ Fast Track for Games Programming Part 9: Colours"
description: "Colours and bit manipulation: how colours are stored as numbers, and shifting and masking bits."
slug: cpp-fast-track-9-colours
lastUpdated: 2026-11-15
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

In this article we will return to the numbers that represent colours, which you have been passing to `Clear`, `Print` and `Bar` since the [second part](/cpp-fast-track-2-template/) of this series. To understand those numbers, we need to look at the *bits* that they are made of. Welcome to the wonderful world of bit magic, which is a very nice place to be, so we will explore it thoroughly.

## Getting the Stuff You Need

For this tutorial, you will be using the same template as in the [2nd tutorial](/cpp-fast-track-2-template/):

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

As usual, we will start with a fresh copy of the template: extract the zip file to `C:\Projects\C++ Fast Track`, and rename the `tmpl8-main` folder to `Part 9`. Open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 9` folder.

As an introduction, change the `Game::Tick` function to:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        screen.Clear(100);
    }
```

What you get is a dark blue screen. Change `100` to `255`, and it becomes bright blue. Increase it to `256`, and it's black. Well, not really black: it's actually very dark green, as we will see in a minute.

<div class="figure-row equal">
<figure>
<img src="/images/cpp-fast-track-9/01-clear-100.png" alt="A dark blue screen" width="800" height="512" class="screenshot" />
<figcaption><code>screen.Clear(100)</code></figcaption>
</figure>
<figure>
<img src="/images/cpp-fast-track-9/01-clear-255.png" alt="A bright blue screen" width="800" height="512" class="screenshot" />
<figcaption><code>screen.Clear(255)</code></figcaption>
</figure>
<figure>
<img src="/images/cpp-fast-track-9/01-clear-256.png" alt="An almost black screen" width="800" height="512" class="screenshot" />
<figcaption><code>screen.Clear(256)</code></figcaption>
</figure>
</div>

## Bits and Bytes

A computer stores every number as a series of *bits*: tiny switches that are either off (`0`) or on (`1`). That's called a *binary* number. Eight bits make a *byte*. An `int` is 32 bits (4 bytes), but there are smaller types as well: an `unsigned char` is a single byte (8 bits), which can store the numbers from 0 to 255. Let's see what that means. Change the `Tick` function to:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        int i = 255;
        unsigned char b = static_cast<unsigned char>(i);
        i++;
        b++;
        int w = 0;
    }
```

Put a breakpoint (<kbd>F9</kbd>) on line 28 (`i++;`), and run the program. When it stops, add `i` and `b` to the *Watch 1* window, just like in the [previous part](/cpp-fast-track-8-memory-addresses/):

<img src="/images/cpp-fast-track-9/02-watch-before.png" alt="Visual Studio halted at line 28, with i and b both 255 in the Watch 1 window" width="1141" height="651" class="screenshot" />

<div class="caption">

`i` and `b` are both `255`.

</div>

Both are `255`. (For an `unsigned char`, Visual Studio also shows which character it is: `char` is the type that C++ uses for characters, such as letters in a text, and character 255 is `ÿ`.) Now press <kbd>F10</kbd> twice, to execute lines 28 and 29:

<img src="/images/cpp-fast-track-9/03-watch-after.png" alt="Visual Studio halted at line 30, where i is 256 and b is 0" width="1141" height="651" class="screenshot" />

<div class="caption">

After `i++` and `b++`, `i` is `256`, but `b` is `0`.

</div>

`i` is `256`, as expected. But `b` is `0`! Eight bits can store $2^8 = 256$ different values: 0 to 255. There is no room for 256, so `b` starts at 0 again. Here are the bits of `i`, before and after:

| Binary | Decimal |
| --- | --- |
| `00000000 00000000 00000000 11111111` | `255` |
| `00000000 00000000 00000001 00000000` | `256` |

When you add 1 to `11111111`, all those ones become zeros, and the 1 *carries* to the next bit, just like 999 + 1 = 1000 in decimal. An `int` has room for that ninth bit. An `unsigned char` doesn't: it only keeps the lowest 8 bits, and those are all zero.

What does this have to do with colours? Hang in there, we'll get to that in a minute!

## Shifting Bits

There's another thing about binary numbers. Every bit is worth twice as much as the bit to the right of it: 1, 2, 4, 8, 16, and so on. So if you move all the bits of a number one place to the left, the number doubles. C++ has an operator for that: `<<` (*shift left*). `1 << 3` moves the bits of `1` three places to the left, which gives `8`. Let's print a few of those. Add the highlighted line to the top of `game.cpp`, and change the `Init` function to:

```cpp title="game.cpp" showLineNumbers {4}
#include "game.hpp"

#include <iostream>
#include <print>
```

```cpp title="game.cpp" showLineNumbers startLineNumber=11
    void Game::Init()
    {
        for (int i = 0; i <= 8; i++)
        {
            std::println("1 << {} = {:3} = {:016b}", i, 1 << i, 1 << i);
        }
    }
```

`Init` runs once, when the program starts, so the console window shows each line just once:

<img src="/images/cpp-fast-track-9/04-console-shift.png" alt="The console window with nine lines: 1 shifted left by 0 to 8 places, in decimal and in binary" width="655" height="314" class="screenshot" />

<div class="caption">

Every shift to the left doubles the number.

</div>

`std::println` (which you know from [part 1](/cpp-fast-track-1-getting-started/)) replaces each `{}` with the next value. You can say how: `{:3}` makes the number at least 3 characters wide, and `{:016b}` prints it in binary (`b`), 16 digits wide, with zeros in front (`0`).

There's also *shift right*: `>>`. It moves the bits to the right, which divides the number by 2 for every place. The bits that fall off on the right are gone: `5 >> 1` is `2`.

## Hexadecimal

Binary numbers are long: 32 digits for an `int`. That's why programmers often write them in *hexadecimal* instead: a way to write numbers that counts in sixteens instead of tens. It uses the digits 0 to 9, and then the letters A to F for 10 to 15. In C++, you write a hexadecimal number with `0x` in front of it: `0xFF` is 255, and `0x100` is 256.

The nice thing about hexadecimal is that every hexadecimal digit is exactly 4 bits:

| Hex | Binary | | Hex | Binary | | Hex | Binary | | Hex | Binary |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `0` | `0000` | | `4` | `0100` | | `8` | `1000` | | `C` | `1100` |
| `1` | `0001` | | `5` | `0101` | | `9` | `1001` | | `D` | `1101` |
| `2` | `0010` | | `6` | `0110` | | `A` | `1010` | | `E` | `1110` |
| `3` | `0011` | | `7` | `0111` | | `B` | `1011` | | `F` | `1111` |

So you can read the bits right from a hexadecimal number: `0xC8` is `1100 1000`, and a byte is always two hexadecimal digits (`0x00` to `0xFF`). The debugger can show values in hexadecimal too: right-click the *Watch 1* window and select *Hexadecimal Display*:

<img src="/images/cpp-fast-track-9/05-watch-hex.png" alt="The Watch 1 window in hexadecimal: i is 0x00000100 and b is 0x00" width="1141" height="651" class="screenshot" />

<div class="caption">

The same values in hexadecimal: 256 is `0x00000100`. (Turn *Hexadecimal Display* off again for the rest of this series.)

</div>

## Colours

Now let's return to colours. A colour on the screen is a mix of red, green and blue light, and the template stores each of them as a number from 0 to 255: 8 bits. Together with an *alpha* value (for transparency, which the screen ignores), that's 32 bits: exactly one `Pixel`, which is a 32-bit `unsigned int`. Blue goes in the lowest (rightmost) 8 bits, green in the next 8 bits, and red in the 8 bits after that:

<svg class="bits-diagram" viewBox="0 0 700 112" role="img" aria-label="The 32 bits of yellow, 0x00C8C800: alpha 0, red 200, green 200 and blue 0, 8 bits each">
<text class="title title" x="30" y="20">yellow: 0x00C8C800</text>
<text class="label alpha" x="80.4" y="41">alpha: 0</text>
<text class="count" x="110.0" y="91" text-anchor="middle">bits 31–24</text>
<rect class="field alpha" x="30.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit alpha" x="40.0" y="66.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="50.0" y1="50" x2="50.0" y2="74"/>
<text class="bit alpha" x="60.0" y="66.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="70.0" y1="50" x2="70.0" y2="74"/>
<text class="bit alpha" x="80.0" y="66.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="90.0" y1="50" x2="90.0" y2="74"/>
<text class="bit alpha" x="100.0" y="66.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="110.0" y1="50" x2="110.0" y2="74"/>
<text class="bit alpha" x="120.0" y="66.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="130.0" y1="50" x2="130.0" y2="74"/>
<text class="bit alpha" x="140.0" y="66.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="150.0" y1="50" x2="150.0" y2="74"/>
<text class="bit alpha" x="160.0" y="66.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="170.0" y1="50" x2="170.0" y2="74"/>
<text class="bit alpha" x="180.0" y="66.5" text-anchor="middle">0</text>
<text class="label red" x="214.5" y="41">red: 200 (0xC8)</text>
<text class="count" x="270.0" y="91" text-anchor="middle">bits 23–16</text>
<rect class="field red" x="190.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit red" x="200.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="210.0" y1="50" x2="210.0" y2="74"/>
<text class="bit red" x="220.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="230.0" y1="50" x2="230.0" y2="74"/>
<text class="bit red" x="240.0" y="66.5" text-anchor="middle">0</text>
<line class="cell red" x1="250.0" y1="50" x2="250.0" y2="74"/>
<text class="bit red" x="260.0" y="66.5" text-anchor="middle">0</text>
<line class="cell red" x1="270.0" y1="50" x2="270.0" y2="74"/>
<text class="bit red" x="280.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="290.0" y1="50" x2="290.0" y2="74"/>
<text class="bit red" x="300.0" y="66.5" text-anchor="middle">0</text>
<line class="cell red" x1="310.0" y1="50" x2="310.0" y2="74"/>
<text class="bit red" x="320.0" y="66.5" text-anchor="middle">0</text>
<line class="cell red" x1="330.0" y1="50" x2="330.0" y2="74"/>
<text class="bit red" x="340.0" y="66.5" text-anchor="middle">0</text>
<text class="label green" x="367.1" y="41">green: 200 (0xC8)</text>
<text class="count" x="430.0" y="91" text-anchor="middle">bits 15–8</text>
<rect class="field green" x="350.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit green" x="360.0" y="66.5" text-anchor="middle">1</text>
<line class="cell green" x1="370.0" y1="50" x2="370.0" y2="74"/>
<text class="bit green" x="380.0" y="66.5" text-anchor="middle">1</text>
<line class="cell green" x1="390.0" y1="50" x2="390.0" y2="74"/>
<text class="bit green" x="400.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="410.0" y1="50" x2="410.0" y2="74"/>
<text class="bit green" x="420.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="430.0" y1="50" x2="430.0" y2="74"/>
<text class="bit green" x="440.0" y="66.5" text-anchor="middle">1</text>
<line class="cell green" x1="450.0" y1="50" x2="450.0" y2="74"/>
<text class="bit green" x="460.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="470.0" y1="50" x2="470.0" y2="74"/>
<text class="bit green" x="480.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="490.0" y1="50" x2="490.0" y2="74"/>
<text class="bit green" x="500.0" y="66.5" text-anchor="middle">0</text>
<text class="label blue" x="564.1" y="41">blue: 0</text>
<text class="count" x="590.0" y="91" text-anchor="middle">bits 7–0</text>
<rect class="field blue" x="510.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit blue" x="520.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="530.0" y1="50" x2="530.0" y2="74"/>
<text class="bit blue" x="540.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="550.0" y1="50" x2="550.0" y2="74"/>
<text class="bit blue" x="560.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="570.0" y1="50" x2="570.0" y2="74"/>
<text class="bit blue" x="580.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="590.0" y1="50" x2="590.0" y2="74"/>
<text class="bit blue" x="600.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="610.0" y1="50" x2="610.0" y2="74"/>
<text class="bit blue" x="620.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="630.0" y1="50" x2="630.0" y2="74"/>
<text class="bit blue" x="640.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="650.0" y1="50" x2="650.0" y2="74"/>
<text class="bit blue" x="660.0" y="66.5" text-anchor="middle">0</text>
<g transform="translate(679 91) scale(0.7)" opacity="0.35"><title>Made with Claude</title><path d="M4.709 15.955l4.72-2.647.08-.23-.08-.128H9.2l-.79-.048-2.698-.073-2.339-.097-2.266-.122-.571-.121L0 11.784l.055-.352.48-.321.686.06 1.52.103 2.278.158 1.652.097 2.449.255h.389l.055-.157-.134-.098-.103-.097-2.358-1.596-2.552-1.688-1.336-.972-.724-.491-.364-.462-.158-1.008.656-.722.881.06.225.061.893.686 1.908 1.476 2.491 1.833.365.304.145-.103.019-.073-.164-.274-1.355-2.446-1.446-2.49-.644-1.032-.17-.619a2.97 2.97 0 01-.104-.729L6.283.134 6.696 0l.996.134.42.364.62 1.414 1.002 2.229 1.555 3.03.456.898.243.832.091.255h.158V9.01l.128-1.706.237-2.095.23-2.695.08-.76.376-.91.747-.492.584.28.48.685-.067.444-.286 1.851-.559 2.903-.364 1.942h.212l.243-.242.985-1.306 1.652-2.064.73-.82.85-.904.547-.431h1.033l.76 1.129-.34 1.166-1.064 1.347-.881 1.142-1.264 1.7-.79 1.36.073.11.188-.02 2.856-.606 1.543-.28 1.841-.315.833.388.091.395-.328.807-1.969.486-2.309.462-3.439.813-.042.03.049.061 1.549.146.662.036h1.622l3.02.225.79.522.474.638-.079.485-1.215.62-1.64-.389-3.829-.91-1.312-.329h-.182v.11l1.093 1.068 2.006 1.81 2.509 2.33.127.578-.322.455-.34-.049-2.205-1.657-.851-.747-1.926-1.62h-.128v.17l.444.649 2.345 3.521.122 1.08-.17.353-.608.213-.668-.122-1.374-1.925-1.415-2.167-1.143-1.943-.14.08-.674 7.254-.316.37-.729.28-.607-.461-.322-.747.322-1.476.389-1.924.315-1.53.286-1.9.17-.632-.012-.042-.14.018-1.434 1.967-2.18 2.945-1.726 1.845-.414.164-.717-.37.067-.662.401-.589 2.388-3.036 1.44-1.882.93-1.086-.006-.158h-.055L4.132 18.56l-1.13.146-.487-.456.061-.746.231-.243 1.908-1.312-.006.006z" fill="#D97757"/></g>
</svg>

<div class="caption">

The bits of a colour: alpha, red, green and blue, 8 bits each.

</div>

That's why any value from 0 to 255 gets you a shade of blue: it only fills the blue bits. And 256 is the lowest green bit (with all the blue bits 0): a very, very dark green. To get to the green bits, we need to shift 8 bits to the left, which is the same as multiplying by 256. So for shades of green, we take a number between 0 and 255, and multiply it by 256. And to get to red, we multiply by 256 twice. Try each of these lines in `Tick`:

```cpp title="game.cpp" showLineNumbers startLineNumber=31
        screen.Clear(100);             // dark blue
        screen.Clear(100 * 256);       // dark green
        screen.Clear(100 * 256 * 256); // dark red
```

<div class="figure-row equal">
<figure>
<img src="/images/cpp-fast-track-9/06-blue.png" alt="A dark blue screen" width="800" height="512" class="screenshot" />
<figcaption><code>100</code></figcaption>
</figure>
<figure>
<img src="/images/cpp-fast-track-9/06-green.png" alt="A dark green screen" width="800" height="512" class="screenshot" />
<figcaption><code>100 * 256</code></figcaption>
</figure>
<figure>
<img src="/images/cpp-fast-track-9/06-red.png" alt="A dark red screen" width="800" height="512" class="screenshot" />
<figcaption><code>100 * 256 * 256</code></figcaption>
</figure>
</div>

So the brightest red that you can get is `255 * 256 * 256`. You can also mix red, green and blue, by adding them:

```cpp title="game.cpp" showLineNumbers startLineNumber=31
        screen.Clear((200 * 256 * 256) + (200 * 256)); // yellow
```

<img src="/images/cpp-fast-track-9/07-yellow.png" alt="A yellow screen" width="800" height="512" class="screenshot" />

<div class="caption">

Red and green light make yellow.

</div>

That's a lot of multiplying. But you already know an easier way to move bits to the right place: shifting. `<< 8` multiplies by 256, and `<< 16` by 256 × 256:

```cpp title="game.cpp" showLineNumbers startLineNumber=31
        screen.Clear((200 << 16) + (200 << 8)); // yellow
```

Now it is clear what's happening: we take `200` for red, and shift it 16 bits to the left, to the red bits. Likewise, we put `200` for green in place by shifting it 8 bits to the left. And because every byte is two hexadecimal digits, you can also write the colour in hexadecimal: `0xRRGGBB`, with two digits for red, two for green and two for blue. 200 is `0xC8`, so this is the same yellow again:

```cpp title="game.cpp" showLineNumbers startLineNumber=31
        screen.Clear(0xC8C800); // yellow
```

That's why you will see colours like `0xffffff` (white) in the template. And, consider this a little teaser: you can also store something in the alpha bits, by shifting a value 24 bits to the left...

## Bitmasks

Now that we can put red, green and blue exactly where we want them, the next question is: how do we get them out again?

Suppose that we have an orange pixel, with some garbage in the alpha bits (the screen ignores those, so they can contain anything). And suppose that we want to know how much green this colour contains. The answer is *bitmasking*, with the `&` operator (pronounced as *and*). Change the `Tick` function to:

```cpp source="tmpl8-part-09/game.cpp#L29-L35"
```

In binary, this happens:

<svg class="bits-diagram" viewBox="0 0 700 320" role="img" aria-label="Orange with garbage in alpha, the mask for green, and the result of colour AND mask: only the green bits are left">
<text class="title title" x="30" y="20">orange, with garbage in alpha: 0xD7FFED00</text>
<text class="label alpha" x="58.2" y="41">alpha: garbage</text>
<rect class="field alpha" x="30.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit alpha" x="40.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="50.0" y1="50" x2="50.0" y2="74"/>
<text class="bit alpha" x="60.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="70.0" y1="50" x2="70.0" y2="74"/>
<text class="bit alpha" x="80.0" y="66.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="90.0" y1="50" x2="90.0" y2="74"/>
<text class="bit alpha" x="100.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="110.0" y1="50" x2="110.0" y2="74"/>
<text class="bit alpha" x="120.0" y="66.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="130.0" y1="50" x2="130.0" y2="74"/>
<text class="bit alpha" x="140.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="150.0" y1="50" x2="150.0" y2="74"/>
<text class="bit alpha" x="160.0" y="66.5" text-anchor="middle">1</text>
<line class="cell alpha" x1="170.0" y1="50" x2="170.0" y2="74"/>
<text class="bit alpha" x="180.0" y="66.5" text-anchor="middle">1</text>
<text class="label red" x="240.4" y="41">red: 255</text>
<rect class="field red" x="190.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit red" x="200.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="210.0" y1="50" x2="210.0" y2="74"/>
<text class="bit red" x="220.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="230.0" y1="50" x2="230.0" y2="74"/>
<text class="bit red" x="240.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="250.0" y1="50" x2="250.0" y2="74"/>
<text class="bit red" x="260.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="270.0" y1="50" x2="270.0" y2="74"/>
<text class="bit red" x="280.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="290.0" y1="50" x2="290.0" y2="74"/>
<text class="bit red" x="300.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="310.0" y1="50" x2="310.0" y2="74"/>
<text class="bit red" x="320.0" y="66.5" text-anchor="middle">1</text>
<line class="cell red" x1="330.0" y1="50" x2="330.0" y2="74"/>
<text class="bit red" x="340.0" y="66.5" text-anchor="middle">1</text>
<text class="label green" x="393.0" y="41">green: 237</text>
<rect class="field green" x="350.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit green" x="360.0" y="66.5" text-anchor="middle">1</text>
<line class="cell green" x1="370.0" y1="50" x2="370.0" y2="74"/>
<text class="bit green" x="380.0" y="66.5" text-anchor="middle">1</text>
<line class="cell green" x1="390.0" y1="50" x2="390.0" y2="74"/>
<text class="bit green" x="400.0" y="66.5" text-anchor="middle">1</text>
<line class="cell green" x1="410.0" y1="50" x2="410.0" y2="74"/>
<text class="bit green" x="420.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="430.0" y1="50" x2="430.0" y2="74"/>
<text class="bit green" x="440.0" y="66.5" text-anchor="middle">1</text>
<line class="cell green" x1="450.0" y1="50" x2="450.0" y2="74"/>
<text class="bit green" x="460.0" y="66.5" text-anchor="middle">1</text>
<line class="cell green" x1="470.0" y1="50" x2="470.0" y2="74"/>
<text class="bit green" x="480.0" y="66.5" text-anchor="middle">0</text>
<line class="cell green" x1="490.0" y1="50" x2="490.0" y2="74"/>
<text class="bit green" x="500.0" y="66.5" text-anchor="middle">1</text>
<text class="label blue" x="564.1" y="41">blue: 0</text>
<rect class="field blue" x="510.0" y="50" width="160.0" height="24" rx="3"/>
<text class="bit blue" x="520.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="530.0" y1="50" x2="530.0" y2="74"/>
<text class="bit blue" x="540.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="550.0" y1="50" x2="550.0" y2="74"/>
<text class="bit blue" x="560.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="570.0" y1="50" x2="570.0" y2="74"/>
<text class="bit blue" x="580.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="590.0" y1="50" x2="590.0" y2="74"/>
<text class="bit blue" x="600.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="610.0" y1="50" x2="610.0" y2="74"/>
<text class="bit blue" x="620.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="630.0" y1="50" x2="630.0" y2="74"/>
<text class="bit blue" x="640.0" y="66.5" text-anchor="middle">0</text>
<line class="cell blue" x1="650.0" y1="50" x2="650.0" y2="74"/>
<text class="bit blue" x="660.0" y="66.5" text-anchor="middle">0</text>
<text class="title title" x="30" y="124">the mask for green: 0x0000FF00</text>
<rect class="field alpha" x="30.0" y="154" width="160.0" height="24" rx="3"/>
<text class="bit alpha" x="40.0" y="170.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="50.0" y1="154" x2="50.0" y2="178"/>
<text class="bit alpha" x="60.0" y="170.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="70.0" y1="154" x2="70.0" y2="178"/>
<text class="bit alpha" x="80.0" y="170.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="90.0" y1="154" x2="90.0" y2="178"/>
<text class="bit alpha" x="100.0" y="170.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="110.0" y1="154" x2="110.0" y2="178"/>
<text class="bit alpha" x="120.0" y="170.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="130.0" y1="154" x2="130.0" y2="178"/>
<text class="bit alpha" x="140.0" y="170.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="150.0" y1="154" x2="150.0" y2="178"/>
<text class="bit alpha" x="160.0" y="170.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="170.0" y1="154" x2="170.0" y2="178"/>
<text class="bit alpha" x="180.0" y="170.5" text-anchor="middle">0</text>
<rect class="field red" x="190.0" y="154" width="160.0" height="24" rx="3"/>
<text class="bit red" x="200.0" y="170.5" text-anchor="middle">0</text>
<line class="cell red" x1="210.0" y1="154" x2="210.0" y2="178"/>
<text class="bit red" x="220.0" y="170.5" text-anchor="middle">0</text>
<line class="cell red" x1="230.0" y1="154" x2="230.0" y2="178"/>
<text class="bit red" x="240.0" y="170.5" text-anchor="middle">0</text>
<line class="cell red" x1="250.0" y1="154" x2="250.0" y2="178"/>
<text class="bit red" x="260.0" y="170.5" text-anchor="middle">0</text>
<line class="cell red" x1="270.0" y1="154" x2="270.0" y2="178"/>
<text class="bit red" x="280.0" y="170.5" text-anchor="middle">0</text>
<line class="cell red" x1="290.0" y1="154" x2="290.0" y2="178"/>
<text class="bit red" x="300.0" y="170.5" text-anchor="middle">0</text>
<line class="cell red" x1="310.0" y1="154" x2="310.0" y2="178"/>
<text class="bit red" x="320.0" y="170.5" text-anchor="middle">0</text>
<line class="cell red" x1="330.0" y1="154" x2="330.0" y2="178"/>
<text class="bit red" x="340.0" y="170.5" text-anchor="middle">0</text>
<text class="label green" x="400.4" y="145">255 << 8</text>
<rect class="field green" x="350.0" y="154" width="160.0" height="24" rx="3"/>
<text class="bit green" x="360.0" y="170.5" text-anchor="middle">1</text>
<line class="cell green" x1="370.0" y1="154" x2="370.0" y2="178"/>
<text class="bit green" x="380.0" y="170.5" text-anchor="middle">1</text>
<line class="cell green" x1="390.0" y1="154" x2="390.0" y2="178"/>
<text class="bit green" x="400.0" y="170.5" text-anchor="middle">1</text>
<line class="cell green" x1="410.0" y1="154" x2="410.0" y2="178"/>
<text class="bit green" x="420.0" y="170.5" text-anchor="middle">1</text>
<line class="cell green" x1="430.0" y1="154" x2="430.0" y2="178"/>
<text class="bit green" x="440.0" y="170.5" text-anchor="middle">1</text>
<line class="cell green" x1="450.0" y1="154" x2="450.0" y2="178"/>
<text class="bit green" x="460.0" y="170.5" text-anchor="middle">1</text>
<line class="cell green" x1="470.0" y1="154" x2="470.0" y2="178"/>
<text class="bit green" x="480.0" y="170.5" text-anchor="middle">1</text>
<line class="cell green" x1="490.0" y1="154" x2="490.0" y2="178"/>
<text class="bit green" x="500.0" y="170.5" text-anchor="middle">1</text>
<rect class="field blue" x="510.0" y="154" width="160.0" height="24" rx="3"/>
<text class="bit blue" x="520.0" y="170.5" text-anchor="middle">0</text>
<line class="cell blue" x1="530.0" y1="154" x2="530.0" y2="178"/>
<text class="bit blue" x="540.0" y="170.5" text-anchor="middle">0</text>
<line class="cell blue" x1="550.0" y1="154" x2="550.0" y2="178"/>
<text class="bit blue" x="560.0" y="170.5" text-anchor="middle">0</text>
<line class="cell blue" x1="570.0" y1="154" x2="570.0" y2="178"/>
<text class="bit blue" x="580.0" y="170.5" text-anchor="middle">0</text>
<line class="cell blue" x1="590.0" y1="154" x2="590.0" y2="178"/>
<text class="bit blue" x="600.0" y="170.5" text-anchor="middle">0</text>
<line class="cell blue" x1="610.0" y1="154" x2="610.0" y2="178"/>
<text class="bit blue" x="620.0" y="170.5" text-anchor="middle">0</text>
<line class="cell blue" x1="630.0" y1="154" x2="630.0" y2="178"/>
<text class="bit blue" x="640.0" y="170.5" text-anchor="middle">0</text>
<line class="cell blue" x1="650.0" y1="154" x2="650.0" y2="178"/>
<text class="bit blue" x="660.0" y="170.5" text-anchor="middle">0</text>
<text class="title title" x="30" y="228">colour & mask: 0x0000ED00</text>
<text class="count" x="110.0" y="299" text-anchor="middle">bits 31–24</text>
<rect class="field alpha" x="30.0" y="258" width="160.0" height="24" rx="3"/>
<text class="bit alpha" x="40.0" y="274.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="50.0" y1="258" x2="50.0" y2="282"/>
<text class="bit alpha" x="60.0" y="274.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="70.0" y1="258" x2="70.0" y2="282"/>
<text class="bit alpha" x="80.0" y="274.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="90.0" y1="258" x2="90.0" y2="282"/>
<text class="bit alpha" x="100.0" y="274.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="110.0" y1="258" x2="110.0" y2="282"/>
<text class="bit alpha" x="120.0" y="274.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="130.0" y1="258" x2="130.0" y2="282"/>
<text class="bit alpha" x="140.0" y="274.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="150.0" y1="258" x2="150.0" y2="282"/>
<text class="bit alpha" x="160.0" y="274.5" text-anchor="middle">0</text>
<line class="cell alpha" x1="170.0" y1="258" x2="170.0" y2="282"/>
<text class="bit alpha" x="180.0" y="274.5" text-anchor="middle">0</text>
<text class="count" x="270.0" y="299" text-anchor="middle">bits 23–16</text>
<rect class="field red" x="190.0" y="258" width="160.0" height="24" rx="3"/>
<text class="bit red" x="200.0" y="274.5" text-anchor="middle">0</text>
<line class="cell red" x1="210.0" y1="258" x2="210.0" y2="282"/>
<text class="bit red" x="220.0" y="274.5" text-anchor="middle">0</text>
<line class="cell red" x1="230.0" y1="258" x2="230.0" y2="282"/>
<text class="bit red" x="240.0" y="274.5" text-anchor="middle">0</text>
<line class="cell red" x1="250.0" y1="258" x2="250.0" y2="282"/>
<text class="bit red" x="260.0" y="274.5" text-anchor="middle">0</text>
<line class="cell red" x1="270.0" y1="258" x2="270.0" y2="282"/>
<text class="bit red" x="280.0" y="274.5" text-anchor="middle">0</text>
<line class="cell red" x1="290.0" y1="258" x2="290.0" y2="282"/>
<text class="bit red" x="300.0" y="274.5" text-anchor="middle">0</text>
<line class="cell red" x1="310.0" y1="258" x2="310.0" y2="282"/>
<text class="bit red" x="320.0" y="274.5" text-anchor="middle">0</text>
<line class="cell red" x1="330.0" y1="258" x2="330.0" y2="282"/>
<text class="bit red" x="340.0" y="274.5" text-anchor="middle">0</text>
<text class="label green" x="344.9" y="249">only green is left: 237</text>
<text class="count" x="430.0" y="299" text-anchor="middle">bits 15–8</text>
<rect class="field green" x="350.0" y="258" width="160.0" height="24" rx="3"/>
<text class="bit green" x="360.0" y="274.5" text-anchor="middle">1</text>
<line class="cell green" x1="370.0" y1="258" x2="370.0" y2="282"/>
<text class="bit green" x="380.0" y="274.5" text-anchor="middle">1</text>
<line class="cell green" x1="390.0" y1="258" x2="390.0" y2="282"/>
<text class="bit green" x="400.0" y="274.5" text-anchor="middle">1</text>
<line class="cell green" x1="410.0" y1="258" x2="410.0" y2="282"/>
<text class="bit green" x="420.0" y="274.5" text-anchor="middle">0</text>
<line class="cell green" x1="430.0" y1="258" x2="430.0" y2="282"/>
<text class="bit green" x="440.0" y="274.5" text-anchor="middle">1</text>
<line class="cell green" x1="450.0" y1="258" x2="450.0" y2="282"/>
<text class="bit green" x="460.0" y="274.5" text-anchor="middle">1</text>
<line class="cell green" x1="470.0" y1="258" x2="470.0" y2="282"/>
<text class="bit green" x="480.0" y="274.5" text-anchor="middle">0</text>
<line class="cell green" x1="490.0" y1="258" x2="490.0" y2="282"/>
<text class="bit green" x="500.0" y="274.5" text-anchor="middle">1</text>
<text class="count" x="590.0" y="299" text-anchor="middle">bits 7–0</text>
<rect class="field blue" x="510.0" y="258" width="160.0" height="24" rx="3"/>
<text class="bit blue" x="520.0" y="274.5" text-anchor="middle">0</text>
<line class="cell blue" x1="530.0" y1="258" x2="530.0" y2="282"/>
<text class="bit blue" x="540.0" y="274.5" text-anchor="middle">0</text>
<line class="cell blue" x1="550.0" y1="258" x2="550.0" y2="282"/>
<text class="bit blue" x="560.0" y="274.5" text-anchor="middle">0</text>
<line class="cell blue" x1="570.0" y1="258" x2="570.0" y2="282"/>
<text class="bit blue" x="580.0" y="274.5" text-anchor="middle">0</text>
<line class="cell blue" x1="590.0" y1="258" x2="590.0" y2="282"/>
<text class="bit blue" x="600.0" y="274.5" text-anchor="middle">0</text>
<line class="cell blue" x1="610.0" y1="258" x2="610.0" y2="282"/>
<text class="bit blue" x="620.0" y="274.5" text-anchor="middle">0</text>
<line class="cell blue" x1="630.0" y1="258" x2="630.0" y2="282"/>
<text class="bit blue" x="640.0" y="274.5" text-anchor="middle">0</text>
<line class="cell blue" x1="650.0" y1="258" x2="650.0" y2="282"/>
<text class="bit blue" x="660.0" y="274.5" text-anchor="middle">0</text>
<g transform="translate(679 299) scale(0.7)" opacity="0.35"><title>Made with Claude</title><path d="M4.709 15.955l4.72-2.647.08-.23-.08-.128H9.2l-.79-.048-2.698-.073-2.339-.097-2.266-.122-.571-.121L0 11.784l.055-.352.48-.321.686.06 1.52.103 2.278.158 1.652.097 2.449.255h.389l.055-.157-.134-.098-.103-.097-2.358-1.596-2.552-1.688-1.336-.972-.724-.491-.364-.462-.158-1.008.656-.722.881.06.225.061.893.686 1.908 1.476 2.491 1.833.365.304.145-.103.019-.073-.164-.274-1.355-2.446-1.446-2.49-.644-1.032-.17-.619a2.97 2.97 0 01-.104-.729L6.283.134 6.696 0l.996.134.42.364.62 1.414 1.002 2.229 1.555 3.03.456.898.243.832.091.255h.158V9.01l.128-1.706.237-2.095.23-2.695.08-.76.376-.91.747-.492.584.28.48.685-.067.444-.286 1.851-.559 2.903-.364 1.942h.212l.243-.242.985-1.306 1.652-2.064.73-.82.85-.904.547-.431h1.033l.76 1.129-.34 1.166-1.064 1.347-.881 1.142-1.264 1.7-.79 1.36.073.11.188-.02 2.856-.606 1.543-.28 1.841-.315.833.388.091.395-.328.807-1.969.486-2.309.462-3.439.813-.042.03.049.061 1.549.146.662.036h1.622l3.02.225.79.522.474.638-.079.485-1.215.62-1.64-.389-3.829-.91-1.312-.329h-.182v.11l1.093 1.068 2.006 1.81 2.509 2.33.127.578-.322.455-.34-.049-2.205-1.657-.851-.747-1.926-1.62h-.128v.17l.444.649 2.345 3.521.122 1.08-.17.353-.608.213-.668-.122-1.374-1.925-1.415-2.167-1.143-1.943-.14.08-.674 7.254-.316.37-.729.28-.607-.461-.322-.747.322-1.476.389-1.924.315-1.53.286-1.9.17-.632-.012-.042-.14.018-1.434 1.967-2.18 2.945-1.726 1.845-.414.164-.717-.37.067-.662.401-.589 2.388-3.036 1.44-1.882.93-1.086-.006-.158h-.055L4.132 18.56l-1.13.146-.487-.456.061-.746.231-.243 1.908-1.312-.006.006z" fill="#D97757"/></g>
</svg>

<div class="caption">

`&` keeps only the bits that are `1` in both values.

</div>

What `&` does is this: every bit that is `1` in the first value, and also `1` in the second value, is `1` in the result. All other bits are `0` in the result. The *mask* has ones for the green bits only, so only the green bits survive, and the screen shows just the green part of the colour:

<img src="/images/cpp-fast-track-9/08-mask.png" alt="A bright green screen" width="800" height="512" class="screenshot" />

<div class="caption">

The green part of the orange colour: `0x00ED00`.

</div>

We extracted the value of green. Well, almost: it's still shifted 8 bits to the left. To get the actual amount of green (`237`), we need to shift it back:

```cpp title="game.cpp" showLineNumbers startLineNumber=33
        Pixel green = (colour & mask) >> 8; // 237
```

Red, blue and alpha can be extracted in the same way, with their own masks and shifts. The template has masks for red, green and blue: `RedMask` (`0xff0000`), `GreenMask` (`0x00ff00`) and `BlueMask` (`0x0000ff`), in `surface.hpp`.

There's a lot more to explore when it comes to bit magic, and trust me, you will learn to love it. But now, let's make things practical, in the assignment for this part.

## Assignment

Here are your tasks for today:

1.  Load an image, display it, and fade it (slowly) to black. Note: there are two ways to do this. The easy way uses floating point operations; the slightly harder way uses integers only. Try both, and measure the speed difference (remember `deltaTime` from [part 6](/cpp-fast-track-6-floats/)?). You can use your own image, or this one: [kodim03.png](/downloads/kodim03.png), from the Kodak Lossless True Color Image Suite: a set of images that is often used to test image processing. It is 768 pixels wide, so it fits on the screen with a small border.

    <iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-9-assignment/index.html" title="A photo of colourful caps that fades to black" loading="lazy" allowfullscreen></iframe>

    <div class="caption">

    The image fades to black, in about three seconds. (This demo starts again every 6 seconds.)

    </div>

2.  Convert an image to greyscale. For every pixel, extract red, green and blue, and replace each of them with their average. Then try a weighted average instead: $0.299 \times \text{red} + 0.587 \times \text{green} + 0.114 \times \text{blue}$. Which of the two looks more natural? And why do you think green counts so much more than blue? A good image to test this with is [kodim23.png](/downloads/kodim23.png): the parrots from the same Kodak image suite.

    <div class="figure-row equal">
    <figure>
    <img src="/images/cpp-fast-track-9/assignment-kodim-colour.png" alt="Two parrots in colour: a blue and yellow one and a red one" width="800" height="512" class="screenshot" />
    <figcaption>The original</figcaption>
    </figure>
    <figure>
    <img src="/images/cpp-fast-track-9/assignment-grey-average.png" alt="The parrots in grey, using the average of red, green and blue" width="800" height="512" class="screenshot" />
    <figcaption>The average of red, green and blue</figcaption>
    </figure>
    <figure>
    <img src="/images/cpp-fast-track-9/assignment-grey-weighted.png" alt="The parrots in grey, using the weighted average" width="800" height="512" class="screenshot" />
    <figcaption>The weighted average</figcaption>
    </figure>
    </div>

3.  **EXTRA** / **HARD**: draw the colours of the rainbow in vertical lines. This will require some research; what you are looking for is a conversion from wavelength to red/green/blue.

    <img src="/images/cpp-fast-track-9/assignment-rainbow.png" alt="The colours of the rainbow in vertical lines, from violet on the left to dark red on the right" width="800" height="512" class="screenshot" />

    <div class="caption">

    The colours of light from 380 nm (violet) to 780 nm (dark red).

    </div>

Once you have completed these, you may continue with the next part.
