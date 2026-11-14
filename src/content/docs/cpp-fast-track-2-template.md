---
title: "C++ Fast Track for Games Programming Part 2: The Template"
description: "Download the tmpl8 template, explore the project, and draw lines and text on the screen."
slug: cpp-fast-track-2-template
lastUpdated: 2026-11-14
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

As you noticed in the [first article](/cpp-fast-track-1-getting-started/), setting up a project in Visual Studio can be quite a task. And we didn't (nearly) touch all the settings that you can adjust for a project either. To make your life a bit easier, we will use a project template from now on. This template is simply a folder that contains all the files that you need, with all the settings tuned just right for the kind of programs that we will be building in this series. The template also contains a bit of code that you need for most projects, so that you don't have to type it yourself. This code aims to take away the *platform specific* things from you; i.e. it opens a window, lets you draw to it, and updates it for you. Sounds simple, but really it isn't. Dealing with the operating system properly can be quite a nightmare, and since that's just not the core of game development, we felt it's best to take care of that once and for all. The result is the template.

## Getting the Stuff You Need

Get the latest template package:

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

You'll make a new project for each part of this series, so let's keep them together in one folder: `C:\Projects\C++ Fast Track`. Right-click the zip file, select *Extract All...*, type `C:\Projects\C++ Fast Track` as the destination, and click *Extract*. This gives you the folder `C:\Projects\C++ Fast Track\tmpl8-main`. Rename it to `Part 2` (right-click it and select *Rename*, or select it and press F2). Open it, and double-click the `tmpl8.slnx` file to open the template in Visual Studio.

<img src="/images/cpp-fast-track-2/01-folder.png" alt="File Explorer showing the files of the template, with tmpl8.slnx selected" width="976" height="634" class="screenshot" />

<div class="caption">

The template folder. `tmpl8.slnx` is the Visual Studio solution.

</div>

The first time you open the template, Visual Studio asks you whether you trust it:

<img src="/images/cpp-fast-track-2/02-trust.png" alt="The This project was downloaded from the web dialog of Visual Studio, with Trust parent folder C++ Fast Track selected" width="631" height="395" class="screenshot" />

<div class="caption">

Visual Studio warns you about projects that you downloaded.

</div>

Visual Studio shows this warning because the project was downloaded from the internet: building a project can run commands on your computer, so you should only open projects from sources you trust. The template is fine. Select *Trust parent folder "C++ Fast Track"* in the *Trust level* list, and click *Trust and Continue*. Now Visual Studio trusts all your projects in `C:\Projects\C++ Fast Track`, so it won't ask again when you open the template for the next parts.

You should see the project in the *Solution Explorer* now. Expand the *tmpl8* project and the *template code* folder, and double-click `game.cpp` to open it:

<img src="/images/cpp-fast-track-2/03-solution.png" alt="Visual Studio with the tmpl8 solution, and game.cpp open" width="1151" height="655" class="screenshot" />

<div class="caption">

The tmpl8 solution in Visual Studio.

</div>

The files that you will work on are `game.hpp` and `game.cpp`. The template itself is in the *template code* folder: `surface.hpp` and `surface.cpp` (images, sprites and text), `template.hpp` and `template.cpp` (the window, the keyboard and the mouse, and some math), and `web-shell.html` (for web builds, more about that later).

If you click the *Local Windows Debugger* button (<img src="/images/cpp-fast-track-2/local-windows-debugger.png" alt="Local Windows Debugger" width="183" height="25" class="inline-ui" />) or press F5, you should see a beautiful window with the text "HELLO WORLD" printed in the top-left corner. To emphasise the magnitude of this accomplishment, there is a spinning gun turret drawn on the screen as well.

<iframe class="embed embed-tmpl8" src="/demos/cpp-fast-track-2/index.html" title="The template: HELLO WORLD and a spinning gun turret" loading="lazy" allowfullscreen></iframe>

<div class="caption">

You should see a beautiful window with a spinning gun turret. (The template also runs in a web browser: this is the template itself, running right on this page. Click it, and press F11 or the button in the top-right corner for fullscreen.)

</div>

Press Escape to close the window (or click the close button). You can also switch between a window and fullscreen with Alt+Enter or F11.

The code that you should be interested in right now is in `game.cpp`. Here are its contents:

```cpp source="tmpl8-part-02/game.cpp"
```

This code will be the starting point for all episodes of the Fast Track tutorial.

## Nuts and Bolts

Even though this is just a small program, there's a lot to say about it. Most (all, actually) of it will be explained in this tutorial series, but for now, let's limit ourselves to some observations:

-   The `#include` lines at the beginning of the program basically paste some source code from another file into the current file. `game.hpp` describes the `Game` (more about that later), and `<iostream>` is part of C++ itself: it gives you `std::cout`.
-   Whenever something needs to be drawn, a line starts with `screen.`: `screen` is the window that you draw on. The things we can make `screen` do are listed in `surface.hpp`, starting at line 44 (`class Surface`).
-   There are some things that happen just once. We specify those in the `Game::Init` function.
-   This particular program does nothing in the `Init` function.
-   Other things are done for each frame: these are listed in `Game::Tick`.
-   There's also a `Shutdown` function, which is executed only once: when the program ends (e.g. when you press Escape).
-   `rotatingGun` (the spinning gun turret) and `frame` (which of its 36 images is shown) are used in `Tick`, but you won't find them in `game.cpp`: they are part of the `Game`, so they are in `game.hpp`, together with `screen`:

    ```cpp source="tmpl8-part-02/game.hpp#L40-L45"
    ```

    `rotatingGun` is a *sprite*, loaded from the image `assets/aagun.tga`, which contains 36 frames (the turret in 36 directions). `frame` starts at `0`. You'll learn all about classes like `Game` in a later part; for now, remember that this is where they live.
-   Lines that start with `//` are merely comments. The program still works if you remove or alter them.

There's quite a bit more, and even more when you explore the other source files, such as `template.cpp`. Feel free to do so; if you mess anything up, just extract the template from the zip file you downloaded at the beginning of this tutorial again.

## Experiments

Let's try some random things with this program:

1.  Change the `0` on line 27 to some other number. Make it a big number.
2.  Change the `0xffffff` on line 29 to `0xff0000`. Then, change it to `0x00ff00`, and finally to `0x0000ff`. What happens?
3.  Change the `2, 2` on line 29 to `100, 20`. Now change it to `780, 20`. What happens?
4.  Change `Clear` into `clear` (lower case) on line 27. Is there a problem now?
5.  Append line 37 to the end of line 36 (don't remove the semicolon at the end of line 36). Line 36 should now look like this:

    ```cpp title="game.cpp" showLineNumbers startLineNumber=36
    rotatingGun.SetFrame(frame); rotatingGun.Draw(screen, 100, 100);
    ```

6.  Move lines 26-38 from the `Tick` function to the `Init` function (they do the same thing for each frame anyway, right?). What is the result of doing this?

An important concept is the screen coordinate system. Every pixel on the screen has a position: the horizontal position is called `X`, the vertical position is called `Y`. `X` starts at `0` on the left side of the screen, and `Y` starts at `0` at the top. The screen of the template is `800` by `512` pixels, so `X` goes from `0` to `799`, and `Y` from `0` to `511`. If you print text at `X = 780`, it doesn't fit: the letters that don't fit on the screen are cut off.

And finally, there are numbers: `0xffffff` (or capitals: `0xFFFFFF`) is a number too. In fact, `0x0000FF` equals `255`. We will discuss this in more detail at a later time.

## Console Window

You probably noticed that a console window opens too, behind the window of the game. This seems quite useless now that we can produce proper graphics, but it may come in handy later. We will use it for printing information that is not supposed to be seen by the player. This will be useful for debugging.

The template prints a line to the console when you press the space bar (lines 31-34): `std::cout` prints the text between the quotes, and `\n` starts a new line. Press the space bar a few times while the game window has the keyboard focus, and look at the console:

<img src="/images/cpp-fast-track-2/05-console.png" alt="The console window, with You pressed the space bar printed twice" width="976" height="268" class="screenshot" />

<div class="caption">

`std::cout` prints to the console window.

</div>

The console is only there in the *Debug* version of your program (the one you build when the toolbar says *Debug*). The *Release* version, which you share with your players, doesn't have one.

## Assignment

Take a single A4 sheet of paper (turned sideways) and draw a grid of 16 columns and 8 rows on it. Label the vertical lines from left to right: 0, 50, 100, 150, and so on, up to 800. Label the horizontal lines from top to bottom: 0, 64, 128, 192, 256, 320, 384, 448, 512. Now draw the word "CODE" on this sheet, using straight lines that pass through grid intersection points. Use as few lines as you can; you'll be translating each line to a line in your program, so don't make it too difficult for yourself.

Then, for each straight line:

1.  Determine the `X` and `Y` coordinate of the start of the line (`X1`, `Y1`).
2.  Determine the `X` and `Y` coordinate of the end of the line (`X2`, `Y2`).
3.  Add a line to the `Tick` function using `screen.Line( X1, Y1, X2, Y2, 0xffffff );`.

The lines on the edge of the grid are on the edge of the screen: use `799` instead of `800` and `511` instead of `512` for those, or they fall just outside the screen.

For bonus points: store the `X` and `Y` coordinates in an array and loop through the array to draw the lines.

And finally:

-   Make the **C** red;
-   Make the **O** green;
-   Make the **D** blue, and
-   Make the **E** yellow.

Your result could look something like this (your letters will probably have a different shape, and that's fine):

<img src="/images/cpp-fast-track-2/06-assignment.png" alt="The word CODE drawn with lines: a red C, a green O, a blue D and a yellow E" width="800" height="512" class="screenshot" />

<div class="caption">

CODE, drawn with 15 lines.

</div>

Once you have completed this assignment, you may continue with the next part.
