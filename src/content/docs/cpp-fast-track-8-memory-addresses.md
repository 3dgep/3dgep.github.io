---
title: "C++ Fast Track for Games Programming Part 8: Memory Addresses"
description: "Memory addresses and pointers: how variables live in memory, and how C++ lets you work with that memory directly."
slug: cpp-fast-track-8-memory-addresses
lastUpdated: 2026-11-14
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

One of the harder concepts in C++ is the concept of *pointers*. Pointers have everything to do with the very nature of the language: C++ gets you pretty close to the hardware you are working with, and one of the most important parts of that hardware is memory. In this tutorial, you will learn to use pointers to refer to memory addresses.

## Getting the Stuff You Need

For this tutorial, you will be using the same template as in the [2nd tutorial](/cpp-fast-track-2-template/):

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

As usual, we will start with a fresh copy of the template: extract the zip file to `C:\Projects\C++ Fast Track`, and rename the `tmpl8-main` folder to `Part 8`. Open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 8` folder, and remove the *hello world* code from the `Game::Tick` function.

Your `Game::Tick` function should look like this now:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
    }
```

## Memory

Your computer probably has a few *GiB* (often written as *GB*, which is not quite the same thing) of memory, which is a couple of billion bytes:

<div class="centered-table">

| Symbol | Prefix | Name | Bytes | Size |
| --- | --- | --- | --- | --- |
| B |  | byte (B) | $2^{0}$ bytes | 1 B |
| Ki | kibi | kibibyte (KiB) | $2^{10}$ bytes | 1,024 B |
| Mi | mebi | mebibyte (MiB) | $2^{20}$ bytes | 1,024 KiB |
| Gi | gibi | gibibyte (GiB) | $2^{30}$ bytes | 1,024 MiB |
| Ti | tebi | tebibyte (TiB) | $2^{40}$ bytes | 1,024 GiB |
| Pi | pebi | pebibyte (PiB) | $2^{50}$ bytes | 1,024 TiB |
| Ei | exbi | exbibyte (EiB) | $2^{60}$ bytes | 1,024 PiB |

</div>

<div class="caption">

Binary units of information (see [multiple-byte units](https://en.wikipedia.org/wiki/Byte#Multiple-byte_units) on Wikipedia).

</div>

We use the Ki, Mi, Gi, Ti (and so on) symbols for binary units, to tell them apart from the metric symbols k, M, G and T, which are powers of 10. A kilobyte (kB) is 1,000 bytes, but a kibibyte (KiB) is 1,024 bytes: computers count in powers of 2.

Each of those billions of bytes has its own number: its memory *address*. A 32-bit processor uses 32-bit numbers for addresses, so it can use at most $2^{32}$ different addresses. Each address refers to one byte (memory is *byte addressable*), so that's 4,294,967,296 bytes, or 4 GiB: not enough for a modern game.

A 64-bit processor uses 64-bit addresses: that's $2^{64}$ or 18,446,744,073,709,551,616 bytes, or 16 EiB. Most computers today have between 8 and 64 GiB of memory, so that's more memory than you will see in a computer for quite a while. The template is a 64-bit program: that's what the *x64* in the toolbar of Visual Studio means.

## What Is a Pointer?

You can think of memory as one very long row of boxes. Each box holds one byte, and each box has a number: its address. It works like the houses in a street: if you know the house number, you can find the house.

When you create a variable, the compiler reserves some of those boxes for it. An `int` takes 4 bytes, so these two lines:

```cpp
int a = 100;
int b = 200;
```

might put `a` in the boxes `0x1000` to `0x1003`, and `b` in the boxes `0x1004` to `0x1007`. Addresses are usually written in *hexadecimal*: a way to write numbers that counts in sixteens instead of tens, with the digits 0 to 9 and then a to f, and `0x` in front. You'll learn more about it in the next part; for now, the `0x` tells you that a number is an address, and not an ordinary value like `100`. (Real addresses are much larger numbers, but small numbers are easier to read.) The address of a variable is the address of its first byte, so `a` lives at address `0x1000`.

In C++, `&` in front of a variable gives you its address: `&a` means *the address of `a`*, which is `0x1000` in this example. And you can store that address in a variable of its own:

```cpp
int* c = &a;
```

`c` is a *pointer*: a variable that holds an address. Its type, `int*` (say: *pointer to int*), tells the compiler what it will find at that address: an `int`. A pointer is a variable like any other, so it needs boxes of its own as well. In a 64-bit program, an address is 64 bits, so `c` takes 8 bytes:

<svg class="memory-diagram" viewBox="0 0 700 180" role="img" aria-label="Memory as a row of bytes: a (100) at address 0x1000, b (200) at 0x1004, and c (0x1000) at 0x1008, with an arrow from c to a">
<defs><marker id="memory-arrow" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse"><path d="M0 0 10 5 0 10z" class="arrow-head"/></marker></defs>
<rect class="var" x="30" y="50" width="160" height="50" rx="4"/>
<line class="byte" x1="70" y1="50" x2="70" y2="100"/>
<line class="byte" x1="110" y1="50" x2="110" y2="100"/>
<line class="byte" x1="150" y1="50" x2="150" y2="100"/>
<text x="110.0" y="36" text-anchor="middle"><tspan class="mono">a</tspan> <tspan class="dim">(int, 4 bytes)</tspan></text>
<text class="mono value" x="110.0" y="81.0" text-anchor="middle">100</text>
<rect class="var" x="190" y="50" width="160" height="50" rx="4"/>
<line class="byte" x1="230" y1="50" x2="230" y2="100"/>
<line class="byte" x1="270" y1="50" x2="270" y2="100"/>
<line class="byte" x1="310" y1="50" x2="310" y2="100"/>
<text x="270.0" y="36" text-anchor="middle"><tspan class="mono">b</tspan> <tspan class="dim">(int, 4 bytes)</tspan></text>
<text class="mono value" x="270.0" y="81.0" text-anchor="middle">200</text>
<rect class="ptr" x="350" y="50" width="320" height="50" rx="4"/>
<line class="byte" x1="390" y1="50" x2="390" y2="100"/>
<line class="byte" x1="430" y1="50" x2="430" y2="100"/>
<line class="byte" x1="470" y1="50" x2="470" y2="100"/>
<line class="byte" x1="510" y1="50" x2="510" y2="100"/>
<line class="byte" x1="550" y1="50" x2="550" y2="100"/>
<line class="byte" x1="590" y1="50" x2="590" y2="100"/>
<line class="byte" x1="630" y1="50" x2="630" y2="100"/>
<text x="510.0" y="36" text-anchor="middle"><tspan class="mono">c</tspan> <tspan class="dim">(int*, 8 bytes)</tspan></text>
<text class="mono value" x="510.0" y="81.0" text-anchor="middle">0x1000</text>
<text class="mono dim" x="30" y="122" text-anchor="middle">0x1000</text>
<text class="mono dim" x="190" y="122" text-anchor="middle">0x1004</text>
<text class="mono dim" x="350" y="122" text-anchor="middle">0x1008</text>
<text class="mono dim" x="670" y="122" text-anchor="middle">0x1010</text>
<text class="dim" x="12" y="80.0" text-anchor="middle">…</text><text class="dim" x="688" y="80.0" text-anchor="middle">…</text>
<path class="arrow" d="M510 106 V168 H110 V104" marker-end="url(#memory-arrow)"/>
</svg>

<div class="caption">

Three variables in memory. `c` holds the address of `a`: we say that `c` *points to* `a`.

</div>

Knowing the house number is nice, but sometimes you need to know who's home. An `*` in front of the pointer will get you the variable that lives there.

```cpp
int d = *c; // d is 100: the int at the address in c, and that's a
*c = 300;   // a is 300 now
```

You can read `*c` as *the `int` that `c` points to*. So `*c = 300;` doesn't change `c` (it still holds `0x1000`): it changes the value at address `0x1000`, and that's `a`.

:::note
The `*` has two jobs. In a declaration, like `int* c`, it makes `c` a pointer. In front of a pointer, like `*c`, it follows the pointer to the variable that it points to.
:::

There's one more thing that you'll use a lot: you can add a number to a pointer. `c + 1` is not the next byte, but the next `int`: 4 bytes further, at address `0x1004`. The compiler knows how large an `int` is from the type of the pointer. So `*(c + 1)` is the `int` right after the one that `c` points to. Because you need this all the time, there's a shorter way to write it: `c[1]` means exactly the same as `*(c + 1)`, and `c[0]` is the same as `*c`.

In the picture, `c[1]` would be `b`. In a real program, the compiler decides where your variables go, so don't count on that. But it's exactly the right tool for memory that you *know* is one long row of values of the same type. And the screen is such a row: 800 × 512 values of the type `Pixel` (a 32-bit `unsigned int`, so 4 bytes each). `screen.GetBuffer()` gives you a `Pixel*` that points to the first pixel, so `screen.GetBuffer()[10]` is the 11th pixel of the screen. That's the pixel itself (its colour), not its address: the address of that pixel is `screen.GetBuffer() + 10`. Just like `*c`, you can read the colour of the pixel, or write a new colour into it: `screen.GetBuffer()[10] = 0xff0000;` makes it red.

## Back to the Screen

Have a look at the `Surface::Bar` function in `surface.cpp` (in the `tmpl8` folder of your project):

```cpp source="tmpl8-part-08/tmpl8/surface.cpp#L285-L293" {3}
```

This function draws a bar. Line 287 creates a pointer `a` to the top-left pixel of the bar: it starts at the first pixel (`m_Pixels.data()`), goes `x1` pixels to the right, and `y1` lines down (`m_Pitch` is the number of pixels from one line to the next: 800 for the screen).

Then the function draws the pixels of the bar, one line at a time (from `y1` to `y2`). `std::fill_n` writes the colour `c` into `x2 - x1 + 1` pixels, starting at address `a`. After that, `a += m_Pitch` moves the pointer `m_Pitch` pixels further: to the same position on the next line. It doesn't use the `Plot` function at all: it writes directly to the memory that contains the screen pixels.

So, let's do some experiments. Change the `Game::Tick` function to:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        Pixel* address = screen.GetBuffer();
        for (int i = 0; i < 800 * 512; i++) address[i] = i;
    }
```

This code gets the memory address of the first pixel of the screen, and stores it in a pointer variable (`address`). Then, it uses a single for-loop to fill the whole screen: `address[i]` is the pixel `i` pixels after the first one.

<img src="/images/cpp-fast-track-8/01-fill.png" alt="The screen filled with horizontal bands of blue and green" width="800" height="512" class="screenshot" />

<div class="caption">

The result of writing directly to the screen buffer.

</div>

Every pixel gets its own number as its colour. The lowest 8 bits of a colour are the blue component, so the blue goes up from 0 to 255 in 256 pixels, and starts at 0 again; the next 8 bits are the green component, which goes up by one every 256 pixels. You will learn more about colours in the next part.

Now try this code:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        Pixel* address = screen.GetBuffer() + 100;
        for (int i = 0; i < 256; i++) address[i * 800] = i;
    }
```

This gets us a blue vertical line:

<img src="/images/cpp-fast-track-8/02-line.png" alt="A thin vertical line at the top-left of the screen, going from black to bright blue" width="800" height="512" class="screenshot" />

<div class="caption">

A vertical blue line. Click the image to see it at full size.

</div>

This demonstrates something important. The lines of the screen may look separate, but they are not: in memory, the screen is one long row of 800 × 512 pixels. To get to the pixel below the current one, you go 800 pixels further: that's exactly what `address[i * 800]` does. It also means that the screen has no right edge in memory: if you write past the end of a line, you end up at the start of the next line. That's why the drawing functions of the template have to check the coordinates that you give them (remember the crash in [part 7](/cpp-fast-track-7-debugging/)?).

## Pointers in the Debugger

Let's leave the screen pixels for a moment, and look at the pointer example from the start of this article in the debugger. Change the `Tick` function to:

```cpp source="tmpl8-part-08/game.cpp#L24-L31"
```

Put a breakpoint (<kbd>F9</kbd>) on line 28 (`int* c = &a;`), and run the program. It stops right before it executes line 28.

Now open a *watch* window (*Debug > Windows > Watch > Watch 1*), and add `a`, `&a`, `c` and `*c` to it: click *Add item to watch*, type the name, and press <kbd>Enter</kbd>.

<img src="/images/cpp-fast-track-8/03-watch-before.png" alt="Visual Studio halted at line 28, with a, &a, c and *c in the Watch 1 window" width="1141" height="651" class="screenshot" />

<div class="caption">

The program is halted at line 28: `c` doesn't have a value yet.

</div>

`a` is `100`, and `&a` is the address where that `100` is stored. It's a big number (it will be different for you), shown in hexadecimal: 16 hexadecimal digits, because the template is a 64-bit program. The type of `&a` is `int *`: a *pointer to an int*.

Line 28 creates the pointer `c`, but it hasn't been executed yet, so `c` still contains the value `0xcccccccccccccccc` that you saw in [part 7](/cpp-fast-track-7-debugging/): the Debug build fills variables that have no value yet with `0xcc` bytes. And that's not an address that the debugger can read, so `*c` gives an error.

Press <kbd>F10</kbd> twice, to execute lines 28 and 29:

<img src="/images/cpp-fast-track-8/04-watch-after.png" alt="Visual Studio halted at line 30, where a, *c are 300 and c is the address of a" width="1141" height="651" class="screenshot" />

<div class="caption">

After lines 28 and 29, `c` points to `a`, and `a` is `300`.

</div>

Now `c` contains the address of `a`: the same value as `&a`. And `*c = 300;` changed the variable that `c` points to, so `a` is now `300`, even though line 29 doesn't mention `a` at all. (Line 30 is only there so that the program has a line to stop at after line 29.)

## But… Why!?

So why are pointers so important in C++? The reason is: performance. When you plot a pixel at coordinate (`x`, `y`), in the end all that happens is that one value at one memory location changes. That's a simple operation. But when you only have `x` and `y`, you need to calculate that address first: it's on line `y`, so `800 * y` pixels after the first pixel of the screen. Add `x` to that, and don't forget the address of the screen itself. So the formula is: `screen.GetBuffer() + x + y * 800`. And then you still have to write the pixel.

That's an awful lot of work for a single pixel. If you already know the address, all you need to do is put a value right there, and the pixel next to it is just one address further. Pointers give you low-level control over memory. That gives you fine control over the work that your program does, and that in turn gives you raw performance (and thus, speedy games).

## Assignment

Here's your task for today:

1.  Draw a dotted line (skip every other dot) from (`0`, `0`) to (`400`, `400`). Don't use the `Plot` function: use a pointer variable instead.
2.  Figure out the distance in memory between two dots, to make this code super simple and fast.
3.  Create an application that does the following:
    1.  Load an `800x512` image.
    2.  Move a green dot from the top-center (`x = 400`, `y = 0`) to the bottom. On its way down, the dot avoids obstacles:
    3.  When the pixel below the dot is black, it goes down. Otherwise, it goes one pixel to the left if its `y` is an odd number, or to the right if its `y` is an even number.
    4.  When the dot reaches the bottom of the screen, it starts at the top again.

Make sure that the image that you use for step 3.1 is suitable for the other steps. You can use this noisy image: [noise.png](/downloads/noise.png). Right-click the image or the link, save it in the `assets` folder of your project, and load it into a `Surface`, just like `ball.png` in part 7.

<img src="/downloads/noise.png" alt="An image of black and white noise, 800 by 512 pixels" width="800" height="512" class="screenshot" />

<div class="caption">

You can use this noisy image for this assignment.

</div>

For this assignment, you need to use pointers: they are also a great way to *read* pixels, rather than writing them. And note that even though we're moving a single pixel now, this will become an avalanche soon. 🙂

The dotted line looks like this:

<img src="/images/cpp-fast-track-8/assignment-dots.png" alt="A dotted white line from the top-left corner to the middle of the screen" width="800" height="512" class="screenshot" />

<div class="caption">

A dotted line from (0, 0) to (400, 400). Click the image to see the dots.

</div>

And the dot that finds its way down looks like this:

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-8-assignment/index.html" title="A green dot that finds its way down through a noisy black and white image" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The green dot finds its way down through the noise. (In this demo, the dot is 5 × 5 pixels, so that it is easier to see; you only need to draw a single green pixel.)

</div>

Once you have completed this assignment, you may continue with the next part.
