---
title: "C++ Fast Track for Games Programming Part 1: Getting Started"
description: "Start the C++ Fast Track: install Visual Studio 2026, create a new C++23 project, and write Hello World."
slug: cpp-fast-track-1-getting-started
lastUpdated: 2026-11-14
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

Welcome to the first article in the C++ Fast Track for Games Programming tutorial series! These tutorials are designed to take you from zero to a decent entry level in a somewhat smooth fashion. We start at the absolute basics: all that you need to get started is a laptop or PC, a fair bit of time, and quite a bit of dedication.

Regarding dedication: These tutorials are supposed to take you anywhere between two and four hours each. Aim to complete a minimum of two tutorials per week. That means that you have an extra workload of 4-8 hours. No-one said this was going to be easy, right?

If you need support while going through these articles, join the Discord channel [https://discord.com/invite/gsxxaxc](https://discord.com/invite/gsxxaxc). You'll have no problems getting answers to your newbie questions there. Lecturers and experienced programmers also roam this channel.

## About the Authors

This course was originally created for the Games programme at Breda University of Applied Sciences (known as IGAD at the previously named NHTV); see [https://www.buas.nl/games](https://www.buas.nl/games).  
It was recently updated for the students of the Utrecht University as well; see [https://www.uu.nl/en/organisation/faculty-of-science/education/bachelors-programmes](https://www.uu.nl/en/organisation/faculty-of-science/education/bachelors-programmes).

This tutorial was written by Jacco Bikker, Brian Beuken, Nils Deslé, and Carlos Bott and updated for Visual Studio 2019 by Robbie Grigg and Phil de Groot and updated for Visual Studio 2026 by Jeremiah van Oosten.

## Getting the Stuff You Need

For the purpose of these tutorials, we assume you develop your games on a Windows PC. The development environment we use is Microsoft's Visual Studio 2026. The Community edition of Visual Studio is free, and you can download it from [https://visualstudio.microsoft.com/downloads/](https://visualstudio.microsoft.com/downloads/).

Checklist:

-   A PC or laptop: pretty much any CPU and GPU will do.
-   Windows 10 or Windows 11 (64-bit).
-   Visual Studio 2026 Community (Professional and Enterprise are more than welcome) with the **Desktop development with C++** workload.

On the download page, click *Free download* under *Visual Studio Community*:

<img src="/images/cpp-fast-track-1/00-download.png" alt="The Visual Studio download page, with the Free download button of Visual Studio Community" width="1160" height="523" class="screenshot" />

<div class="caption">

Visual Studio Community is free for students and individuals.

</div>

Run the file that you downloaded (`VisualStudioSetup.exe`): it starts the *Visual Studio Installer*. When it asks you what you want to install, check the *Desktop development with C++* workload on the *Workloads* tab (it's in the *Desktop & Mobile* section), and click *Install*. Without this workload, Visual Studio can't build C++ programs: you won't even find the C++ project templates.

<img src="/images/cpp-fast-track-1/00b-workloads.png" alt="The Visual Studio Installer with the Desktop development with C++ workload checked" width="1200" height="640" class="screenshot" />

<div class="caption">

Check *Desktop development with C++*. You don't need any of the other workloads for these articles.

</div>

The installer then downloads and installs Visual Studio. This takes a while (it's several gigabytes), so it's a good moment to get a drink.

<img src="/images/cpp-fast-track-1/00c-installing.png" alt="The Visual Studio Installer while it installs Visual Studio Community 2026" width="1200" height="640" class="screenshot" />

<div class="caption">

Visual Studio Community 2026 is being installed.

</div>

:::note
If you already have Visual Studio 2026, but not the C++ workload, start the *Visual Studio Installer* from the Start menu, click *Modify* next to your Visual Studio, check *Desktop development with C++*, and click *Modify* at the bottom right.
:::

:::tip
Install Visual Studio, and keep your programming projects, on your fastest drive: a Solid State Drive (SSD) is preferred. This might not matter now, but when your projects grow to tens of thousands of lines of code, it will save you a lot of time.
:::

## Starting Visual Studio 2026

The first time you start Visual Studio, it asks you to sign in with a Microsoft account. Signing in is optional: you can skip it and add an account later (or never). Next, you get to pick a theme. As the window says, you can always change this later. The screenshots in these articles use the *Dark* theme.

You are then greeted by the start window:

<img src="/images/cpp-fast-track-1/01-start-window.png" alt="The Visual Studio 2026 start window" width="881" height="571" class="screenshot" />

<div class="caption">

The Visual Studio 2026 start window. The list on the left shows the projects you opened recently: it's empty the first time.

</div>

## Create a New Project

Click *Create a new project* on the right side of the start window. You then need to select the type of project. We are building a very simple program, so select *Empty Project* (the one with the *C++*, *Windows* and *Console* tags) and click *Next*:

<img src="/images/cpp-fast-track-1/02-create-new-project.png" alt="The Create a new project dialog, with Empty Project selected" width="771" height="533" class="screenshot" />

<div class="caption">

Starting with an empty project means we don't have to delete as many things later.

</div>

:::note
If you don't see *Empty Project*, type `empty` in the search box at the top, or set the language filter to *C++*. If there are no C++ templates at all, the *Desktop development with C++* workload is missing: see [Getting the Stuff You Need](#getting-the-stuff-you-need).
:::

In the next dialog, you choose the name of your project and where your files go:

<img src="/images/cpp-fast-track-1/03-configure.png" alt="The Configure your new project dialog" width="771" height="533" class="screenshot" />

<div class="caption">

You may choose any folder you want for your new project.

</div>

1.  Set the *Project name* to `Getting Started`.
2.  Set the *Location* to `C:\Projects` (or whichever folder you keep all of your programming projects in).
3.  Set the *Solution name* to `C++ Fast Track for Games Programming`.
4.  Leave *Place solution and project in the same directory* unchecked.

:::danger
Don't keep your projects on the **Desktop** or in your **Documents** folder. Windows indexes these folders for search, and OneDrive often synchronizes them to the cloud. Both get in the way when Visual Studio writes hundreds of files during a build: builds become slower, and sometimes fail because a file is in use. A folder like `C:\Projects` avoids all of that.
:::

Click the *Create* button to create your new project! You should see something similar to the screenshot below.

<img src="/images/cpp-fast-track-1/04-new-project.png" alt="Visual Studio with the new, empty Getting Started project" width="1151" height="655" class="screenshot" />

<div class="caption">

An empty project: the *Solution Explorer* on the left shows the solution, the project and its (empty) folders.

</div>

The *Solution Explorer* on the left side of the window shows your *solution* (`C++ Fast Track for Games Programming`) and the *project* inside it (`Getting Started`). The project has a few folders (*filters*, really) that help you organize your files: *Header Files*, *Resource Files* and *Source Files*. They are all empty. If you don't see the Solution Explorer, open it with *View > Solution Explorer* in the main menu.

## Use the Latest C++

C++ is still evolving: a new version of the language appears every three years. The newest version that Visual Studio supports is C++23, and that is the version we use in these articles. A new project uses C++20 by default, so we have to tell Visual Studio to use C++23 instead.

Right-click the *Getting Started* project in the Solution Explorer and select *Properties* (at the bottom of the menu). In the *Property Pages* dialog that appears:

1.  Set *Configuration* (at the top) to *All Configurations*, so the setting applies to the Debug and the Release version of your program.
2.  Select *Configuration Properties > General* on the left.
3.  Click the value of *C++ Language Standard*, open the dropdown list, and select *Preview - ISO C++23 Standard (/std:c++23preview)*.
4.  Click *OK*.

<img src="/images/cpp-fast-track-1/06-language-standard.png" alt="The Property Pages dialog with the C++ Language Standard set to ISO C++23" width="789" height="541" class="screenshot" />

<div class="caption">

Select *Preview - ISO C++23 Standard* for *All Configurations*.

</div>

:::note
Don't let the word *Preview* scare you: Microsoft calls its C++23 support a preview because a few parts of the standard are still missing. Everything we use in these articles works.
:::

## Add a Source File

Before we go on:

:::caution
This is the first and only article in this series that is this boring. Starting with part 2, you will be coding, not downloading + installing + adjusting settings. Promise.
:::

That being said, let's get coding! Now it's finally time to add a C++ source file to play with. Right-click the *Source Files* folder of the *Getting Started* project in the Solution Explorer and select *Add > New Item*.

Type `main.cpp` as the name of the new file (or anything else really, as long as it ends with `.cpp`), and press Enter.

<img src="/images/cpp-fast-track-1/05-add-new-item.png" alt="The Add New Item dialog, with main.cpp as the name of the new file" width="441" height="147" class="screenshot" />

<div class="caption">

We just need the one `.cpp` file for now.

</div>

The new file is immediately opened, and we're ready for some coding.

## Hello World

A tutorial on C++ is not complete without a "Hello world" example, so here we go. Enter the following program:

```cpp title="main.cpp" showLineNumbers
#include <print>

int main()
{
    std::println("Hello world!");
}
```

<img src="/images/cpp-fast-track-1/07-main-cpp.png" alt="Visual Studio with the Hello World program in main.cpp" width="1151" height="655" class="screenshot" />

<div class="caption">

Your first C++ program. Check that the toolbar says *Debug* and *x64*.

</div>

So what does this program do?

-   `#include <print>` makes the `std::println` function available to your program. It comes with C++ (in the *standard library*), but you have to ask for it.
-   `int main()` is where every C++ program starts. The code between the curly braces `{` and `}` is executed when you run the program.
-   `std::println("Hello world!");` prints the text between the quotes, followed by a new line. Note the semicolon at the end: every *statement* in C++ ends with one.

Start the program by pressing **F5**, or by clicking the green *Local Windows Debugger* button on the toolbar. Visual Studio *builds* your program first: it turns your code into an executable file. Then it starts it.

You should see a console window with the text `Hello world!` printed in it.

<img src="/images/cpp-fast-track-1/08-hello-world.png" alt="A console window that shows Hello world!" width="1181" height="268" class="screenshot" />

<div class="caption">

Your program prints *Hello world!*. The lines below it are added by Visual Studio.

</div>

The text below `Hello world!` is not printed by your program: when your program ends, Visual Studio keeps the console window open, so you can read what your program printed. Press any key (while the console window has the keyboard focus) to close it.

:::note
If Visual Studio tells you that there were build errors, compare your code with the code above, character by character. C++ is very picky: `Println`, a missing `;` or a missing `"` are all errors. Also check that you selected C++23 in the previous section: `std::println` doesn't exist in C++20.
:::

## Discussion

Open the folder `C:\Projects\C++ Fast Track for Games Programming` (or wherever you saved your Visual Studio solution) in File Explorer. Visual Studio created quite a few files and folders for you:

<img src="/images/cpp-fast-track-1/09-explorer.png" alt="File Explorer showing the solution folder" width="976" height="495" class="screenshot" />

<div class="caption">

The solution folder.

</div>

-   `C++ Fast Track for Games Programming.slnx`: your *solution file*, which points to
-   `Getting Started\Getting Started.vcxproj`: your *project file*, which points to
-   `Getting Started\main.cpp`: the *source code file*.

A project can have multiple source files, and a solution can have multiple projects. The most interesting file is in the `x64\Debug` folder, and it's called `Getting Started.exe`. That's the program you just made. Double-click it, and you'll see that you created it yourself (if you are fast enough: the console window closes as soon as the program ends, because Visual Studio isn't there to keep it open).

The other files:

-   `Getting Started\Getting Started.vcxproj.filters`: the folders (filters) that you see in the Solution Explorer;
-   `Getting Started\Getting Started.vcxproj.user`: your own settings for the project, for example for debugging;
-   `Getting Started\x64\Debug`: files that the compiler creates while it builds your program, such as `.obj` files;
-   `x64\Debug`: your program (`.exe`) and its debug information (`.pdb`);
-   `.vs`: a hidden folder with information that Visual Studio keeps for itself, such as which files you had open.

Quite confusing perhaps, but all that matters right now is the `.cpp` file in the `Getting Started` folder and the executable that was built from it.

:::tip
It is always good to understand what each file is for, especially when it comes to working out which files belong in version control such as Git: never commit the files that Visual Studio or the compiler generates (`.vs`, `x64`, the `Debug` and `Release` folders, and `.vcxproj.user`).
:::

If you don't see the `.vs` folder, or if you don't see the file extensions (`.slnx`, `.cpp`, `.exe`), tell File Explorer to show them. In Windows 11, open the *View* menu, select *Show*, and check *File name extensions* and *Hidden items*. In Windows 10, check *File name extensions* and *Hidden items* on the *View* tab of the ribbon.

<img src="/images/cpp-fast-track-1/10-explorer-show.png" alt="The View > Show menu of File Explorer in Windows 11" width="400" height="169" class="screenshot" />

<div class="caption">

Programmers want to see everything: show the file name extensions and hidden items.

</div>

## Assignment

So, here's your task:

1.  Every article in this series ends with a small practical assignment. You need to complete this assignment. Feel free to share your solution on the 3dgep.com Discord server: [https://discord.com/invite/gsxxaxc](https://discord.com/invite/gsxxaxc).
2.  Create a program that prints your name 10 times, then waits for a key, then prints *Coding is awesome!* 10 times, then waits for a key again, then exits.

:::tip[Hint]
To wait for a key, add `#include <cstdio>` at the top of `main.cpp`, and call `std::getchar();` where the program has to wait. `std::getchar` waits until you press Enter. And yes, you may copy and paste lines for now: in part 4 you will learn a better way.
:::
