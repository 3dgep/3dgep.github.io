---
title: "C++ Fast Track for Games Programming Part 14: File I/O"
description: "File I/O: read and write files, for example to load game settings from a text file."
slug: cpp-fast-track-14-fileio
lastUpdated: 2022-12-05
---

<span id="attachment-8103"></span><img src="/images/cpp-logo.png" alt="C++ Fast-track" width="150" class="thumb" />

Before we continue with typical game related topics, you need to know a few more general techniques that will help you when you write your games. One of these techniques is *file I/O*. *File I/O* stands for *Input/Output*. You are of course already performing file I/O operations, for example when you load an image. However, this is handled by some image library ([FreeImage](https://freeimage.sourceforge.io/)). Sometimes, you simply want to store your own data.

## Getting Started

For this tutorial, we’ll be using the same template from the [2nd tutorial](/cpp-fast-track-2-template/):

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

## Text Output

The very first article of this series made you write a *Hello World* application. This was also the last application that you had to print anything to a text window. There’s a reason for this: there are too many books already that make you write text-based applications, and this simply has little to do with game development.

There are however a few areas where [`printf`](https://cplusplus.com/reference/cstdio/printf/) comes in handy. One of these areas is debugging (see [Part 7: Debugging](/cpp-fast-track-7-debugging/)).

Try this piece of code:

```cpp title="game.cpp" showLineNumbers
#include "game.h"
#include "surface.h"
#include "template.h"
#include <cstdio> //printf

namespace Tmpl8
{
    float x = 200.0f, y = 0.0f, vx = 0.1f, vy = 0.0f;
    void Game::Init() { }
	
    void Game::Shutdown() { }

    static Sprite rotatingGun(new Surface("assets/aagun.tga"), 36);
    static int frame = 0;

    void Game::Tick(float deltaTime)
    {
        screen->Clear(0);
        screen->Box(x, y, x + 5, y + 5, 0xffffff);
        if ((vy += 0.02f, y += vy) > ScreenHeight) vy = -vy;
        if ((x += vx < 0) || (x >= ScreenWidth)) vx = -vx;
    }
};
```

This is probably the shortest (but also most horribly-written) bouncing cube code possible (send your code if you can think of something worse!). On top of that, it doesn’t work: The bouncing cube leaves the screen at the right side. Let’s do some debugging!

Add the following line to your `Game::Tick` method:

```cpp title="game.cpp" showLineNumbers startLineNumber=19
printf("X - position: % f\nY - position: % f\n", x, y);
```

## What Happens?

The output of the [`printf`](https://cplusplus.com/reference/cstdio/printf/) command is directed to the console window that is behind your game window. Left-click with your mouse inside the console window (to select some text) to pause the running application, this will block the main thread and let you examine the contents of the console window. Right-click to continue running the application.

## Printf

The [`printf`](https://cplusplus.com/reference/cstdio/printf/) function used in the previous section has a number of *interesting codes* in it. There’s the `%f`: which is replaced by the contents of variable `x`. Then, there is the `\n` character: this sequence emits a *new line*, so printing continues on the next line.

The [`printf`](https://cplusplus.com/reference/cstdio/printf/) command is pretty useful, especially if you know how to use it. Find out about the details of the [`printf`](https://cplusplus.com/reference/cstdio/printf/) function here: [printf – C++ Reference](https://cplusplus.com/reference/cstdio/printf/). After reading about the [`printf`](https://cplusplus.com/reference/cstdio/printf/) function, you will learn that you can replace `%f` by `%i` to print an integer. You can also write `%.2f` to print a float with `2` decimals.

## Printing to a String

Sadly, all this formatting goodness is not something we can use with the `Surface::Print` function, which just expects a plain string to print. We can however *print* to a string to bypass this issue:

```cpp title="game.cpp" showLineNumbers startLineNumber=16 {4,5,6}
    void Game::Tick(float deltaTime)
    {
        screen->Clear(0);
        char text[128];
        sprintf(text, "X - position: % f\nY - position: % f\n", x, y);
        screen->Print(text, 2, 2, 0xffffff);
        screen->Box(x, y, x + 5, y + 5, 0xffffff);
        if ((vy += 0.02f, y += vy) > ScreenHeight) vy = -vy;
        if ((x += vx > 0) || (x >= ScreenWidth)) vx = -vx;
    }
```

This time, we get all the benefits of [`printf`](https://cplusplus.com/reference/cstdio/printf/), and use this to print the current position of the box to the game window.

## Printing to a File

Once you know how to get text to the screen using the [`printf`](https://cplusplus.com/reference/cstdio/printf/) function and to a string using [`sprintf`](https://cplusplus.com/reference/cstdio/sprintf/) function, moving on to files is easy. Try the following:

```cpp title="game.cpp" showLineNumbers startLineNumber=16 {4,5,6}
    void Game::Tick(float deltaTime)
    {
        screen->Clear(0);
        FILE* f = fopen( "positions.txt", "a" );
        fprintf( f, "X-position: %f\nY-position: %f\n", x, y );
        fclose( f );
        screen->Box(x, y, x + 5, y + 5, 0xffffff);
        if ((vy += 0.02f, y += vy) > ScreenHeight) vy = -vy;
        if ((x += vx < 0) || (x >= ScreenWidth)) vx = -vx;
    }
```

:::caution
Always remember to close any file you open – think of this as closing any door you open.
:::

This time, the position of the cube will be written to a file. Note how [`printf`](https://cplusplus.com/reference/cstdio/printf/) was replaced by [`fprintf`](https://cplusplus.com/reference/cstdio/fprintf/). The first parameter of [`fprintf`](https://cplusplus.com/reference/cstdio/fprintf/) is a [`FILE`](https://cplusplus.com/reference/cstdio/FILE/), which you first need to open. In this case, the file is opened for *appending* (`"a"`). You can also open a file for *writing* (`"w"` – this will clear the file first) or *reading* (`"r"`). You can also open the file for *binary* reading and writing, using `"rb"` and `"wb"` respectively.

## Retrieving Data from a File

Before we continue, you need to make a file for testing purposes. Using notepad, create a file named `settings.txt` and place it in the directory where you unzipped template zip file. Put the following info in it:

```text title="settings.txt" showLineNumbers
xpos = 100
```

Now, in your `Game::Init` function, add the following code:

```cpp title="game.cpp" showLineNumbers startLineNumber=9
    void Game::Init()
    {
        FILE* f = fopen("settings.txt", "r");
        fscanf(f, "xpos = %f", &x);
        fclose(f);
    }
```

When you run the application, you will see that the cube starts bouncing at `x = 100` now, and you can change the start position using a configuration file, without recompiling your code. This is a very useful feature for games!

## Binary Data

The data that we have written to and read from a file so far has been *text-only*. The advantage of this is that the resulting files are *human-readable*; the disadvantage is that the files get somewhat large. A float variable, for instance, needs `4` bytes in memory, but in a file, `3.14159265358979` requires 16 bytes.

If you don’t need to be able to read the file yourself, you can use binary file i/o. This lets you write the contents of any variable, or even better, the contents of any area in memory, directly to a file. Here are some examples:

```cpp title="game.cpp" showLineNumbers startLineNumber=9 {3,4,5,6}
    void Game::Init()
    {
        FILE* f = fopen("bindat.bin", "wb");
        fwrite(&x, 4, 1, f);
        fwrite(&y, 4, 1, f);
        fclose(f);
    }
```

This code will create a file for binary writing. Then, it writes two blocks of 4 bytes (the size of a float). The first line (line 12) writes the value of `x` to the file, and the second line (line 13) writes the value of `y` to the file.

Note that you need to know how large a variable is to be able to store it to a file. When in doubt, use the [`sizeof`](https://en.cppreference.com/cpp/language/sizeof) operator:

```cpp title="game.cpp" showLineNumbers startLineNumber=12
fwrite( &x, sizeof(x), 1, f );
```

Compared to just writing `4` this will not make your code slower, it’s just more typing.

Reading the data from such a file is very similar:

```cpp title="game.cpp" showLineNumbers startLineNumber=11
    FILE* f = fopen( "bindat.bin", "rb" );
    fread( &x, 4, 1, f );
    fread( &y, 4, 1, f );
    fclose( f );
```

This time, the file is opened for binary reading, and [`fread`](https://cplusplus.com/reference/cstdio/fread/) is used instead of [`fwrite`](https://cplusplus.com/reference/cstdio/fwrite/).

## Assignment

Create a function that saves the contents of the screen to a `.tga` image file that can be loaded into your favourite image editing application.

A `.tga` file consists of a *header* and the actual image data. For the image data, you can simply save lines of pixels. For the header, you need the following data:

```cpp title="game.cpp" showLineNumbers startLineNumber=8
struct TGAHeader
{
    unsigned char ID, colmapt;      // set both to 0
    unsigned char type;             // set to 2
    unsigned char colmap[5];        // set all elements to 0
    unsigned short xorigin, yorigin;// set to 0
    unsigned short width, height;   // put image size here
    unsigned char bpp;              // set to 32
    unsigned char idesc;            // set to 0
};
```

You can now create a variable of type `TGAHeader`:

```cpp title="game.cpp" showLineNumbers startLineNumber=16
TGAHeader header;
```

The size of the header is (and absolutely must be) precisely 18 bytes. You can verify this again using `sizeof(TGAHeader)`.

Finally, you can set individual fields of this header:

```cpp title="game.cpp" showLineNumbers startLineNumber=17
header.ID = 0;
```

1.  Find one of your previous assignments and add a `Game::ScreenShot` method to it.
2.  Add code to your application that saves a screenshot whenever you press a specific key on the keyboard.

### Optional

Use [`sprintf`](https://cplusplus.com/reference/cstdio/sprintf/) to produce file names like `screenshot001.tga` where each new screenshot increments the last three digits of the filename.
