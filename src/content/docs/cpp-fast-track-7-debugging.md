---
title: "C++ Fast Track for Games Programming Part 7: Debugging"
description: "Debugging basics: breakpoints, stepping through code, and inspecting variables in Visual Studio."
slug: cpp-fast-track-7-debugging
lastUpdated: 2026-11-14
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

This episode covers *debugging*. We will see how a program can be halted at any time, or at a specific line, and how we can inspect variables to verify that everything works as intended. Debugging can be a complex process, but we'll limit ourselves to the basics: breakpoints and following the program flow, while keeping an eye on variables.

## Getting the Stuff You Need

For this tutorial, you will be using the same template as in the [2nd tutorial](/cpp-fast-track-2-template/):

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

As usual, we will start with a fresh copy of the template: extract the zip file to `C:\Projects\C++ Fast Track`, and rename the `tmpl8-main` folder to `Part 7`. Open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 7` folder, and remove the *hello world* code from the `Game::Tick` function.

Your `Game::Tick` function should look like this now:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
    }
```

## Debug Mode

Visual Studio projects can be built in two *configurations*: *Debug* and *Release*. You select the configuration in the drop-down box in the toolbar:

<img src="/images/cpp-fast-track-7/01-configuration.png" alt="The Solution Configurations drop-down box, with Debug, Release and Configuration Manager" width="373" height="117" class="screenshot" />

<div class="caption">

The configuration drop-down box.

</div>

By default, *Debug* is selected.

### So What's the Difference?

Well, Debug builds produce larger, slower programs. To verify this, build your program: select *Build > Build Solution* from the main menu (or press <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>B</kbd>). Now select *Release* in the drop-down box, and build it again. Then go to the folder of your project (`C:\Projects\C++ Fast Track\Part 7`): you will find both versions of your program there, next to the `tmpl8.slnx` file. The Debug version is called `tmpl8_debug.exe`, and the Release version `tmpl8.exe` (yes, we set this up in the project settings, so that you can always tell them apart):

<img src="/images/cpp-fast-track-7/02-exe-sizes.png" alt="File Explorer with tmpl8.exe (312 KB) and tmpl8_debug.exe (1,201 KB) selected" width="976" height="661" class="screenshot" />

<div class="caption">

The Debug version is about four times as large as the Release version.

</div>

You can run both programs directly from this folder, by double-clicking them. They need the `SDL3.dll` file next to them (a *DLL* is a library that a program loads when it starts): the template copies it there when you build. The `.pdb` files contain the *debug information* for each version: Visual Studio uses them to show you which line of code is running, and the values of your variables.

### Why is it Larger?

The answer is: *debug information*, and checks. The Debug version doesn't optimize your code (so that every line of code that you wrote really exists in the program, and the debugger can show it to you), and it adds checks that catch common mistakes: for example, the C++ library checks every time you use an index into a `std::vector` whether it is valid. It also fills memory that you didn't initialize with special values (like `0xCCCCCCCC`), so that it's easier to notice when you use a variable that you forgot to give a value. The Release version doesn't have these checks, and the compiler optimizes your code as much as it can. As a result, it runs faster and is smaller (even though it contains the C++ runtime library, so that it also runs on computers that don't have Visual Studio).

Switch back to *Debug* in the drop-down box. Now add the highlighted lines to `game.hpp`, below the `rotatingGun` sprite:

```cpp source="tmpl8-part-07/game.hpp#L43-L48" {5-6}
```

This loads the image `ball.png` into a `Surface`. We don't want to draw it as a sprite this time: we only want to read its pixels. Then make the `Tick` function in `game.cpp` like this:

```cpp title="game.cpp" showLineNumbers startLineNumber=24
    void Game::Tick(float deltaTime)
    {
        // clear the graphics window
        screen.Clear(0);
        // draw a grid
        for (int x = 15; x < 800; x += 16)
        {
            for (int y = 6; y < 512; y += 12)
            {
                Pixel p = image.GetBuffer()[x / 16 + (y / 12) * 50];
                Pixel red = p & 0xff0000;
                Pixel green = p & 0x00ff00;
                Pixel blue = p & 0x0000ff;
                screen.Bar(x, y, x + 12, y + 2, red);
                screen.Bar(x, y + 4, x + 12, y + 6, green);
                screen.Bar(x, y + 8, x + 12, y + 10, blue);
            }
        }
    }
```

What it is supposed to do is a surprise, but for now one thing is clear: it doesn't do it. Run it (<kbd>F5</kbd>), and the program stops:

<img src="/images/cpp-fast-track-7/03-exception.png" alt="Visual Studio stopped at an exception: Access violation writing location" width="983" height="418" class="screenshot" />

<div class="caption">

An exception: *Access violation writing location*. Now what should we do?

</div>

The program tried to write to a piece of memory that it isn't allowed to write to (an *access violation*), and Visual Studio stopped it right there. You are now looking at code that you didn't write: a file called `xutility`, which is part of the C++ library, with a yellow arrow pointing at the line that caused the problem. Something in your program called this code with bad values. Time to figure out what the problem is.

## Gathering Intel

The first useful bit of information is the *call stack*. Click *Show Call Stack* in the exception popup (or select *Debug > Windows > Call Stack* in the main menu, or press <kbd>Ctrl</kbd>+<kbd>Alt</kbd>+<kbd>C</kbd>). The **Call Stack** window shows where we are, and how we got there: each line is a function, called by the function on the line below it. At the top is the code of the C++ library where the program stopped (`std::_Fill_zero_memset`, which is called by `std::fill_n`), then `Tmpl8::Surface::Bar` (a function of the template), and then `Tmpl8::Game::Tick`, at line 39: your code! Double-click on a line in the **Call Stack** window to see that code. Double-click the `Game::Tick` line to see which line of your code called `Surface::Bar`, and then double-click the `Surface::Bar` line.

Now let's look at the values of the variables in `Surface::Bar`. Visual Studio shows them next to the code already, but we can also *watch* them: select *Debug > Windows > Watch > Watch 1* (or click the *Watch 1* tab at the bottom of the window), click *Add item to watch*, type `x1`, and press <kbd>Enter</kbd>. You will immediately see the value of parameter `x1`, which was passed to `Surface::Bar` by the `Tick` function. Add `y1` as well:

<img src="/images/cpp-fast-track-7/04-call-stack.png" alt="Visual Studio in Surface::Bar, with x1 = 15 and y1 = 514 in the Watch 1 window, and the Call Stack" width="1280" height="672" class="screenshot" />

<div class="caption">

The Call Stack shows how we got here, and Watch 1 shows the values of `x1` and `y1`.

</div>

The value of `y1` is `514`... And that's outside of the screen, which is `800` by `512` pixels: `y` goes from `0` to `511`. Is that a problem? Yes it is: a block of memory was reserved for the pixels of the screen, and we're trying to write outside of that block. Writing to memory outside of the memory that your program has reserved is not allowed, and the program crashes.

:::note
Visual Studio may also show a **Diagnostic Tools** window while you debug, with graphs of the memory and the processor time that your program uses. We don't need it in this tutorial, so you can close it.
:::

Fixing this is easy: in the `Game::Tick` function, make sure that we don't loop all the way to `512` in the `y` direction. Let's loop to `480` instead:

```cpp title="game.cpp" showLineNumbers startLineNumber=29 {3}
        for (int x = 15; x < 800; x += 16)
        {
            for (int y = 6; y < 480; y += 12)
            {
```

Stop the program (*Debug > Stop Debugging*, or <kbd>Shift</kbd>+<kbd>F5</kbd>), and run it again. This time, it works, and here's the surprise:

<img src="/images/cpp-fast-track-7/leds.png" alt="The ball image, drawn as a grid of tiny red, green and blue LEDs" width="800" height="512" class="screenshot" />

<div class="caption">

The ball, drawn with red, green and blue "LEDs", like the pixels of a screen.

</div>

Each pixel of the ball image is drawn as three small bars: a red one, a green one and a blue one, each as bright as the red, green and blue part of the pixel. That's how the pixels of a real screen work. We only see the top of the ball, though: the image is 50 pixels high, and there's only room for 40 rows of LEDs on the screen.

## Tracing

Obviously, you don't have to wait for a crash to freeze your program mid-flight. Click in the grey margin to the left of line 33 in `game.cpp` (or put the cursor on line 33 and press <kbd>F9</kbd>). The result is a red circle in front of the line: a *breakpoint*. Now run the program again. It halts at the line you marked, and a yellow arrow shows which line will run next. You can have as many breakpoints in your program as you like.

<img src="/images/cpp-fast-track-7/05-breakpoint.png" alt="Visual Studio halted at a breakpoint on line 33, with the Locals and Breakpoints windows" width="1280" height="672" class="screenshot" />

<div class="caption">

The program halted at the breakpoint on line 33. The **Locals** window shows the values of the variables in the function.

</div>

While the program is halted, the **Locals** window (*Debug > Windows > Locals*) shows the values of all the variables in the current function: `x` is `15`, the first column, and `y` is `18`, the second row.

Once your program is halted, you can continue running it with <kbd>F5</kbd>, or step through it one line at a time by pressing <kbd>F10</kbd>. When you get to a function call (such as `screen.Bar(...)`), you can either step *over* it with <kbd>F10</kbd>, or *into* it with <kbd>F11</kbd>. And when you're in a function, <kbd>Shift</kbd>+<kbd>F11</kbd> steps *out* of it: it runs the rest of the function, and halts right after the function call. If you don't feel like remembering those function keys: there are buttons on the toolbar for the same functionality.

<img src="/images/cpp-fast-track-7/06-debug-controls.png" alt="The debug toolbar: Continue, Pause, Stop, Restart, Show Next Statement, Step Into, Step Over and Step Out" width="400" height="31" class="screenshot" />

<div class="caption">

The debug toolbar: Continue (<kbd>F5</kbd>), Stop (<kbd>Shift</kbd>+<kbd>F5</kbd>), and Step Into (<kbd>F11</kbd>), Step Over (<kbd>F10</kbd>) and Step Out (<kbd>Shift</kbd>+<kbd>F11</kbd>) at the right side.

</div>

## Conditional Breakpoints

Imagine you want to halt the program only when a certain condition is met, e.g. when the `x` coordinate is larger than `500`.

One way to do this is by setting a condition for the breakpoint. Select *Debug > Windows > Breakpoints* in the main menu. Right-click the breakpoint in the **Breakpoints** window, and select *Settings...*:

<img src="/images/cpp-fast-track-7/07-breakpoint-menu.png" alt="The Breakpoints window with the menu of a breakpoint, with Settings selected" width="496" height="267" class="screenshot" />

<div class="caption">

Right-click a breakpoint in the **Breakpoints** window and select *Settings...*

</div>

In the **Breakpoint Settings** window that appears, select the **Conditions** checkbox and type `x > 500` in the text box:

<img src="/images/cpp-fast-track-7/08-condition.png" alt="The Breakpoint Settings window, with the condition x > 500" width="791" height="316" class="screenshot" />

<div class="caption">

Add a condition to the breakpoint, so that it only halts the program when `x > 500` is `true`.

</div>

Click *OK*, and continue the program (<kbd>F5</kbd>). This time, it halts when `x` is `511`: the first column for which `x > 500` is `true`. The red circle of the breakpoint now has a small white `+` in it, which means that it has a condition.

<img src="/images/cpp-fast-track-7/09-condition-hit.png" alt="Visual Studio halted at the conditional breakpoint, with x = 511 in the Locals window" width="1280" height="672" class="screenshot" />

<div class="caption">

The conditional breakpoint halts the program when `x` is `511`.

</div>

Or, keep it simple, and add the condition directly to your code:

```cpp title="game.cpp" showLineNumbers startLineNumber=33
                if (x > 500)
                {
                    int w = 0;
                }
```

The `int w = 0;` statement is obviously irrelevant: it is just there to be able to put a breakpoint inside the `if`.

:::tip
You can safely leave bits of debugging code like these in your program: they don't affect its performance in the Release version. The reason for this is interesting: in the Release version, the compiler does its best to optimize your code. It notices that the value of `w` is never used, so it doesn't produce any code for it in your `.exe`. That means in turn that there is no code inside the `if`, so the `if` is removed as well. In the Debug version, this code remains in the program, but in Debug mode you shouldn't be concerned with performance anyway.
:::

## Trust No One

Some things are not supposed to happen. Perhaps you assume no one will draw boxes outside the screen. And still it happens. A great way to catch problems like that is with *asserts*.

Add the highlighted line inside the inner `for` loop:

```cpp source="tmpl8-part-07/game.cpp#L34-L41" {5}
```

Note that the program doesn't compile, and that `assert` has a red squiggly line under it. This is because we have not included the header file that `assert` comes from. If you are not sure which one it is, you can search for it online, or just add the highlighted line at the top of `game.cpp`:

```cpp source="tmpl8-part-07/game.cpp#L1-L4" {3}
```

`assert` checks that the condition between the parentheses is true. Obviously, this assertion is going to fail: `y` goes up to `474`. When it fails, the program prints a message in the console window, with the condition, the file and the line number:

<img src="/images/cpp-fast-track-7/10-assert-console.png" alt="The console window: Assertion failed: y < 400, file C:\Projects\C++ Fast Track\Part 7\game.cpp, line 38" width="1121" height="268" class="screenshot" />

<div class="caption">

The assertion failed on line 38: `y` wasn't smaller than `400`.

</div>

And then the program stops, with this message:

<img src="/images/cpp-fast-track-7/11-abort.png" alt="The Microsoft Visual C++ Runtime Library window: Debug Error, abort() has been called" width="387" height="204" class="screenshot" />

<div class="caption">

Press *Retry* to debug the program, or *Abort* to stop it.

</div>

Click *Retry* to look around in the debugger (the Call Stack shows you where the assertion failed), or *Abort* to stop the program.

It is a good habit to add assertions like these (well, slightly more sane assertions perhaps) to your program: they are only included in the Debug version, and they add a fair bit of safety for those unforeseen situations.

## Assignment

Debugging is something you need to get used to; once you are familiar with the process you will not be able to live without it. It's a bit hard to give you a debugging assignment, so instead:

-   Fix the `Surface::Bar` function (in `surface.cpp`) so that it doesn't crash when coordinates outside the screen are specified.
-   Replace the `ball.png` image with an image you found on the internet (i.e. not an image that is already in the `assets` folder).
-   Change the size of the *LEDs*, so that the full image fits on the screen.

With the ball image, it could look like this:

<img src="/images/cpp-fast-track-7/assignment.png" alt="The whole ball image, drawn as a grid of smaller red, green and blue LEDs" width="800" height="512" class="screenshot" />

<div class="caption">

All 50 rows of the ball image fit on the screen, with smaller LEDs.

</div>

Once you have completed this assignment, you may continue with the next part.
