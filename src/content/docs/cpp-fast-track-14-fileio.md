---
title: "C++ Fast Track for Games Programming Part 14: File I/O"
description: "File I/O: save the tile map of part 11 to a text file, load it again, build a small map editor, and store the map in a binary file."
slug: cpp-fast-track-14-fileio
lastUpdated: 2026-11-15
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

Before we continue with physics and AI, there is one more general technique that every game needs: *file I/O*. I/O stands for *Input/Output*: reading data from files, and writing data to files. You have been reading files all along, every time you loaded an image, but the template did the work for you. Games store much more than images: levels, settings, high scores, saved games. In this part, you will write and read your own files.

Remember the tile map of [part 11](/cpp-fast-track-11-tiles/)? The map is written in the code, as two arrays of numbers. To change a single tree, you have to change the code and build the game again. A level designer would not be happy with that. In this part, we will move the map to a file, and then build a small editor in the game itself, so that you can change the map with the mouse, and save it.

## Getting Started

For this tutorial, you will start where [part 11](/cpp-fast-track-11-tiles/) ended: the template with the tile map and the tank. Download it here:

[<img src="/images/zip1.png" alt="Download the starting point for part 14 as a zip file" width="120" class="thumb" />part-14.zip](/downloads/part-14.zip)

Extract the zip file to `C:\Projects\C++ Fast Track`: it contains a `Part 14` folder. Open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 14` folder. Run it, and you will see the map and the green tank of part 11, which you can drive with the arrow keys (or <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd> and <kbd>D</kbd>). As in part 11, the screen is 768 × 512 pixels: exactly 12 × 8 tiles of 64 × 64 pixels.

## Writing a File

Let's start by writing the map to a file. We need a few more headers: `<fstream>` for files, `<format>` for `std::format`, and `<string>` for `std::string`. Add the highlighted lines at the top of `game.cpp`:

```cpp source="tmpl8-part-14/1-save/game.cpp#L1-L7" {3-4,6}
```

### SaveLayer

The map has two layers, and both are saved in the same way. So we start with a function that writes one layer. Add it below `TankFits`:

```cpp source="tmpl8-part-14/1-save/game.cpp#L79-L85"
```

`SaveLayer` writes the layer row by row: the numbers of one row on one line, separated by commas.

-   `file` is the file to write to (a *file stream*, see `SaveMap` below). `file << value` writes a value to the file, just like `std::cout << value` would print it in the console.
-   `std::format("{:3}", layer[y][x])` makes the number at least 3 characters wide (you saw that in [part 9](/cpp-fast-track-9-colours/)). That's not necessary, but it makes the columns line up, so that the file is easy to read for a human.
-   After each number, the code writes a comma, except after the last number of a row: then it writes `"\n"`, the end of a line (a *new line*). The `? :` picks one of the two (it's the *conditional operator*: `condition ? a : b` is `a` when the condition is true, and `b` otherwise).
-   The `&` in `std::ofstream& file` makes the parameter a *reference*: `SaveLayer` doesn't get a copy of the file, but the file of the code that called it. A reference is another name for an existing variable. (A file stream can't even be copied: try it without the `&`, and Visual Studio will complain.) The layer is an array, and an array parameter is always passed as a pointer to its first element (see [part 10](/cpp-fast-track-10-arrays/)), so `SaveLayer` doesn't copy the layer either.

### SaveMap

Next, the function that saves the whole map. Add it below `SaveLayer`:

```cpp source="tmpl8-part-14/1-save/game.cpp#L87-L97"
```

`SaveMap` opens the file, writes the size of the map, and then uses `SaveLayer` twice: once for the ground, and once for the objects, with an empty line in between. It returns `true` when the map was saved, and `false` when it wasn't.

-   `std::ofstream` (from `<fstream>`) is an *output file stream*: a file that you write to. `std::ofstream file(filename);` creates a variable `file`, and opens the file with that name. If the file already exists, it is replaced; if it doesn't, it is created.
-   Opening a file can fail: the folder may not exist, or the file may be read-only, or opened by another program. Then `file` is `false`, so `!file` (*not* `file`) is `true`: that's what `if (!file)` checks. In that case, `SaveMap` returns `false`, so that the code that called it knows that the map was not saved.
-   `file << MapWidth << " " << MapHeight` writes the size of the map, with a space in between. `"\n\n"` ends that line, and adds an empty line after it.
-   At the end, `static_cast<bool>(file)` turns the file into `true` if everything went fine, and `false` if anything went wrong while writing (a full disk, for example).
-   Where is the code that closes the file? There is none. `file` is an object, and when the function ends, the object is destroyed, and it closes the file by itself. (It's a class, like the classes of [part 12](/cpp-fast-track-12-classes/): a class can have a *destructor*, a method that runs automatically when the object is destroyed.)

### Saving the Map

Finally, call `SaveMap` in `Init`, so that the game saves the map when it starts:

```cpp source="tmpl8-part-14/1-save/game.cpp#L99-L105" {6}
```

Run the game. It looks exactly the same as before, but it wrote a new file: `map.txt`, in the `assets` folder of your project (`C:\Projects\C++ Fast Track\Part 14\assets`). Open it in Visual Studio (<kbd>Ctrl</kbd>+<kbd>O</kbd>, or drag it from Windows Explorer into Visual Studio):

```text title="assets/map.txt"
12 8

  0,  0, 10,  1,  0,  0, 10,  0,  0,  0, 10,  0
  2,  2,  2, 12,  2,  2,  2, 14,  0,  0,  0,  0
  0, 10,  0,  1,  0,  0,  0,  1,  0, 10,  0,  0
  0,  0,  0,  1,  0, 10,  0,  1,  0,  0,  0, 10
 18, 18, 18, 38, 18, 18, 18, 38, 18, 18, 18, 18
 20, 30, 20, 21, 20, 20, 20, 21, 20, 30, 20, 20
 22, 22, 22, 25, 22, 22, 22, 25, 22, 22, 22, 22
 20, 20, 30, 20, 20, 30, 20, 20, 20, 20, 30, 20

 -1,  0, -1, -1, -1,  1, -1, -1,  2, -1,  0, -1
 -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  1
  0, -1, 14, -1, -1,  3, -1, -1, -1, -1,  2, -1
 -1,  1, -1, -1, -1, -1, -1, -1,  5,  5, -1,  0
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
  7, -1, -1, -1, -1,  4, -1, -1, -1, 11, -1,  3
 -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, -1, -1
 -1, -1, 10, 10, -1, -1,  8, -1, -1, -1,  7, -1
```

The map, in a file: the size of the map (12 by 8 tiles), and then the two layers, the ground and the objects.

This kind of file, with values separated by commas, is called a *CSV* file (*comma-separated values*). It's a very common format: spreadsheets can open it, and tile map editors like [Tiled](https://www.mapeditor.org/) can save a layer as CSV. (Tiled counts the tiles from 1, and uses 0 for an empty spot, so you would subtract 1 from every number to use its maps here.)

## Reading a File

Now the other way around: reading the map from the file. Add the `<print>` header (for `std::println`):

```cpp source="tmpl8-part-14/2-load/game.cpp#L1-L8" {6}
```

### LoadLayer

Just like saving, loading starts with one layer. Add `LoadLayer` below `SaveMap`:

```cpp source="tmpl8-part-14/2-load/game.cpp#L100-L114"
```

`LoadLayer` reads the numbers of one layer, row by row, into the array `layer`. It returns `true` when it could read all the numbers, and `false` when it couldn't.

-   `file` is an `std::ifstream`, an *input file stream*: a file that you read from. `file >> layer[y][x]` reads a number from the file into the array: `>>` is the opposite of `<<`. Spaces and new lines are skipped automatically, so `>>` doesn't care how the numbers are spread over the lines.
-   The numbers are separated by commas, and a comma isn't part of a number. So after every number (except the last one of a row), `LoadLayer` reads one character into `comma`. (It doesn't check that it really is a comma: it simply skips it.)
-   A file can be broken: someone may have made a typo, or a file from an older version of the game has a different size. When `>>` can't read a number (for example, because it finds a letter), the file goes into a *fail state*: `file` is then `false` (and `!file` is `true`), and every read after that fails too. So `LoadLayer` doesn't need to check every number: at the end, `static_cast<bool>(file)` is `false` when any of the reads failed.

:::caution
`LoadLayer` checks that it could read the numbers, but not that they are real tiles: 0 to 39 for the ground (`terrain.png` has 40 tiles), and -1 to 15 for the objects (no object, or one of the 16 objects). A tile number of 50 in `map.txt` would make `DrawTile` copy pixels from far outside the tile sheet: you get garbage on the screen, or the game crashes. We trust `map.txt` here, to keep the code short. In a real game, check every number that comes from a file before you use it.
:::

### LoadMap

Then the function that loads the whole map. Add it below `LoadLayer`:

```cpp source="tmpl8-part-14/2-load/game.cpp#L116-L133"
```

`LoadMap` opens the file, checks the size of the map, and uses `LoadLayer` twice: once for the ground, and once for the objects. It returns `true` when the map was loaded, and `false` when it wasn't.

-   `std::ifstream file(filename);` opens the file for reading. When the file doesn't exist, `file` is `false` (and `!file` is `true`), and `LoadMap` returns `false`.
-   `file >> width >> height` reads the two numbers at the start of the file. A map of a different size doesn't fit in our arrays, so then `LoadMap` stops too.
-   Why are the width and the height in the file at all, if `LoadMap` only accepts 12 × 8? They protect the game against a file that was made for a different map. Suppose that you make the map 16 tiles wide later, and an old `map.txt` is still around (or the other way around). Without the size, `LoadMap` would just read numbers until the arrays are full: the rows of the file would end up in the wrong places, the end of the ground layer would end up in the object layer, and every read would succeed. You would get a scrambled map, without any error. With the size in the file, `LoadMap` sees right away that the file doesn't fit. The size also makes the file easier to understand for a human.
-   `LoadMap` reads the layers into two temporary arrays first. Only when the whole file is correct, it copies them to `ground` and `objects`. If the file is broken halfway, the game keeps the map that it had: a broken map would be worse than just keeping the old one.

:::note[Maps of any size]
With the size in the file, the map size could also be *dynamic*: `LoadMap` would read the width and the height first, and then make arrays of exactly that size, so that every level can have its own size. That needs memory that is allocated while the game runs (with `new` or `std::vector`), and that is beyond the scope of this part. To keep things simple, every map is 12 × 8 tiles.
:::

### Loading the Map

Finally, change `Init`: instead of saving the map, it now loads it:

```cpp source="tmpl8-part-14/2-load/game.cpp#L135-L141" {6}
```

When `LoadMap` fails, `Init` prints a message in the console window, and the game simply uses the map in the code.

:::caution
Make sure that `Init` doesn't call `SaveMap` anymore. Otherwise, the game would write the map from the code to `map.txt` every time it starts, and your changes to the file would be gone.
:::

Run it: you will see the same map again. But this time, it comes from the file. To prove that, change `map.txt`. Replace the fifth row of the objects (the row of `-1`'s, where the grass meets the sand) by a fence, with gaps for the roads:

```text title="assets/map.txt"
  8,  8, 10, -1, 10,  8,  9, -1,  9, 10,  8,  8
```

Save the file, and run the game again. The code didn't change, so Visual Studio doesn't even need to build it again:

<img src="/images/cpp-fast-track-14/02-edited-map.png" alt="The map of part 11, with a fence of barricades and sandbags between the grass and the sand, with gaps for the two roads" width="768" height="512" class="screenshot" />

<div class="caption">

The map with the fence from `map.txt`. Try to drive through it!

</div>

Now try to break it: change a number in `map.txt` into `x`, or delete a row, and run the game. The console window shows the message, and you get the map from the code. Then undo your change.

:::note[The C way: fopen, fprintf and fscanf]
In older code (and in older versions of this tutorial) you will see files handled with functions from C, the language that C++ grew out of:

```cpp
FILE* f = fopen("assets/map.txt", "w"); // "w": write, "r": read, "wb" and "rb": binary
fprintf(f, "%d %d\n", MapWidth, MapHeight);
fclose(f);
```

`fprintf` works like `printf`: `%d` is replaced by an `int`, `%f` by a `float`. For reading, there is `fscanf`, and for binary files `fread` and `fwrite`. These functions still work in C++, but they are easy to get wrong: nothing checks that `%d` gets an `int`, and if you forget `fclose`, the file stays open. File streams check the types for you, and close the file by themselves.
:::

## A Map Editor

Editing `map.txt` by hand works, but it's not much fun: which number was that tree again? Let's put an editor in the game itself. Press <kbd>E</kbd>, and you can paint tiles with the mouse; press <kbd>S</kbd>, and the map is saved.

`Tick` is getting long, so before we add the editor, let's split it up. `Tick` will keep drawing the map and the tank, and then call one of two new methods: `Update`, which drives the tank, or `Edit`, the map editor. They are methods of `Game` (like `Tick`), so they can use `GetKey` and `screen`, just like `Tick` does.

The editor also needs to know where the mouse is, and whether its buttons are down. The template tells the game about the mouse with `MouseMove`, `MouseDown` and `MouseUp`, but those don't do anything yet. Open `game.hpp`, add the two methods below `Tick`, and change the mouse methods:

```cpp source="tmpl8-part-14/game.hpp#L17-L23" {2-7}
```

And add these variables (in the `private` part of the class, below `frame`):

```cpp source="tmpl8-part-14/game.hpp#L50-L51"
```

The template calls `MouseMove` whenever the mouse moves, with the position of the mouse in the screen (in pixels), and `MouseDown` and `MouseUp` when a button goes down or up. Button 1 is the left mouse button, and button 3 the right one (button 2 is the middle one). Now `mousex`, `mousey`, `mouseLeft` and `mouseRight` always tell you what the mouse is doing.

Then, in `game.cpp`, add an include at the top:

```cpp source="tmpl8-part-14/3-editor/game.cpp#L1-L4" {3}
```

And add these variables below `tankFrame`:

```cpp source="tmpl8-part-14/3-editor/game.cpp#L49-L56" {4-8}
```

### Letter Keys

The editor uses letters as shortcuts: <kbd>E</kbd> for *edit*, <kbd>S</kbd> for *save*, and <kbd>L</kbd> for *load*. For those keys, we need one more small function. Add it below `Shutdown`:

```cpp source="tmpl8-part-14/3-editor/game.cpp#L158-L163"
```

:::note[Scancodes and keycodes]
SDL has two ways to name a key. A *scancode* (`SDL_SCANCODE_W`) is a *position* on the keyboard: the names come from a US keyboard, so `SDL_SCANCODE_W` is the key to the left of <kbd>E</kbd> on a US keyboard, whatever is printed on it. A *keycode* (`SDLK_W`) is the key that types a "w" on the player's keyboard, wherever that key is.

On a French (AZERTY) keyboard, the key to the left of <kbd>E</kbd> is <kbd>Z</kbd>, and <kbd>A</kbd> and <kbd>Q</kbd> have swapped places. For driving, that's exactly what we want: `SDL_SCANCODE_W`, `A`, `S` and `D` become <kbd>Z</kbd>, <kbd>Q</kbd>, <kbd>S</kbd> and <kbd>D</kbd> there, in the same spot under the left hand. The same goes for <kbd>Z</kbd> and <kbd>X</kbd> in the editor: they are two keys next to each other. But <kbd>S</kbd> for *save* is about the letter, not the position: on some keyboards, `SDL_SCANCODE_S` isn't the key with an "s" on it.

`GetKey` and `GetKeyPressed` take a scancode. `SDL_GetScancodeFromKey` (from `<SDL3/SDL_keyboard.h>`) finds the scancode of the key that types a letter on the player's keyboard, so `GetKeyPressed(KeyFor(SDLK_S))` means: the key with an "s" on it was pressed. (The `nullptr` means that we don't need to know whether <kbd>Shift</kbd> is needed for it.) The rule of thumb: **use a scancode when the position of the key matters, and a keycode when the letter matters**.
:::

### Update

The first new method is `Update`. Add it below `KeyFor`, and *move* the code that drives the tank from `Tick` into it:

```cpp source="tmpl8-part-14/3-editor/game.cpp#L165-L176"
```

This is exactly the code of part 11; only its place changed. Because `Update` is a method of `Game`, it is written as `Game::Update`, just like `Game::Tick` (remember the methods of the `Tank` class in [part 12](/cpp-fast-track-12-classes/)?).

### Edit

The second new method is the editor. Add it below `Update`:

```cpp source="tmpl8-part-14/3-editor/game.cpp#L178-L218"
```

-   `layer` is the layer that you edit, and `brush` holds the tile that you paint, for each of the two layers. `layer = 1 - layer` switches between 0 and 1.
-   <kbd>Z</kbd> and <kbd>X</kbd> choose the previous and the next tile. `lowest` and `highest` are the lowest and highest tile numbers of the layer; after the last tile, you get the first one again.
-   `mousex / TileSize` is the column of the tile under the mouse, and `mousey / TileSize` its row (an integer division, like in `IsFree`). The `if` checks that the mouse really is on the map.
-   `int& tile` is a reference again: `tile` is another name for the tile under the mouse, in the layer that you edit. `tile = brush[layer]` changes the map itself, not a copy. Without the reference, we would need the `layer == 0 ? ... : ...` for every use of the tile.
-   The tile under the mouse is drawn with the tile of the brush, with a white frame around it (`Box` draws a rectangle), so you can see what you are about to paint.
-   `SaveMap` and `LoadMap` return `true` or `false`, and the editor shows a message for 3 seconds (3000 milliseconds of `deltaTime`), so that you know whether it worked.
-   `Edit` draws on top of the map and the tank, so it must run after they are drawn. That's up to `Tick`.

### Tick

Finally, `Tick` itself. It still draws the map and the tank, and then lets <kbd>E</kbd> choose between driving and editing:

```cpp source="tmpl8-part-14/3-editor/game.cpp#L220-L246" {23-26}
```

Every frame, exactly one of the two methods runs: `Edit` when you are editing, and `Update` when you are driving. That's also why the editor can use <kbd>S</kbd> to save: while you edit, the tank doesn't drive. And `Tick` reads like a summary of the game: draw the world, then drive or edit.

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-14-editor/index.html" title="The tile map with a map editor: press E, paint tiles with the mouse, and save the map with S" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The map editor. Press <kbd>E</kbd> to edit, <kbd>Tab</kbd> to switch between the ground and the objects, <kbd>Z</kbd> and <kbd>X</kbd> to choose a tile, the left mouse button to paint, and the right mouse button to pick the tile under the mouse. <kbd>S</kbd> saves the map, <kbd>L</kbd> loads it again, and <kbd>E</kbd> takes you back to the tank. (Click the demo first, so that it gets the keys. In the browser, the file is only kept until you reload the page.)

</div>

Paint a few trees, press <kbd>S</kbd>, and close the game. Run it again: your trees are still there, because `Init` loads `map.txt`. You can use the editor and the game together: change the map, press <kbd>E</kbd>, and try it with the tank right away.

## Binary Files

`map.txt` is a *text file*: it contains characters, which you can read and change in any text editor. To store the number 38, the file contains the characters `3` and `8`, and `LoadLayer` has to turn those back into a number. That's not the only way. In memory, an `int` is 4 bytes, and a *binary file* simply contains those 4 bytes. Let's write the map as a binary file too.

### MapHeader

A binary file usually starts with a *header*: a small block of data that describes the rest of the file. Add this `struct` below `LoadMap`:

```cpp source="tmpl8-part-14/game.cpp#L143-L148"
```

The first 4 bytes of the file are the characters `TMAP`. Such a code is called a *magic number*: many file formats start with one (a PNG file starts with the characters `PNG`, after one special byte). It tells a program that reads the file what kind of file it is. After the magic number, the header holds the size of the map, just like the first line of `map.txt`, and for the same reason: so that `LoadMapBinary` can refuse a file that was made for a map of a different size.

### SaveMapBinary

Next, the function that saves the map as a binary file. Add it below `MapHeader`:

```cpp source="tmpl8-part-14/game.cpp#L150-L160"
```

`SaveMapBinary` opens the file in binary mode, and writes three blocks of bytes: the header, the ground layer and the object layer, exactly as they are in memory. Like `SaveMap`, it returns `true` when the map was saved, and `false` when it wasn't.

-   `std::ios::binary` opens the file in *binary mode*. In text mode, Windows writes the end of a line as two characters (a *carriage return* and a *line feed*), and turns them back into one `"\n"` when you read the file. That is fine for text, but in a binary file, any byte can be part of a number: the `int` 10 contains a byte with the value 10, which happens to be the code of a line feed. In binary mode, the bytes are written exactly as they are.
-   `file.write` writes a block of memory: it takes the address of the first byte, and the number of bytes. `sizeof(ground)` is the size of the whole array in bytes: 12 × 8 tiles × 4 bytes = 384 bytes. So one `write` writes the whole layer, and one `read` reads it back. No loops, no commas, no conversions.
-   `write` wants the address as a `const char*` (a `char` is one byte), but `&header` is a `MapHeader*`, and `ground` is an array of `int`'s. `reinterpret_cast` tells the compiler: treat this address as the address of some bytes. It doesn't change anything in memory; it only changes how the compiler looks at the address. It's a dangerous tool (it lets you treat anything as anything), so only use it for jobs like this one.

### LoadMapBinary

And the function that loads it again. Add it below `SaveMapBinary`:

```cpp source="tmpl8-part-14/game.cpp#L162-L183"
```

`LoadMapBinary` reads the same three blocks back: first the header, and then the two layers. It returns `true` when the map was loaded, and `false` when it wasn't.

-   `file.read` is the opposite of `file.write`: it reads a number of bytes from the file into memory, at the address that you give it. One `read` fills the whole header, and one `read` fills a whole layer.
-   When the header doesn't start with `TMAP`, it isn't a map file, and `LoadMapBinary` stops right there. When the size in the header isn't 12 × 8, the layers won't fit in our arrays, so it stops too.
-   Like `LoadMap`, `LoadMapBinary` reads the layers into temporary arrays, and only changes the map when the whole file could be read. And like `LoadLayer`, it doesn't check the tile numbers themselves: a broken `map.bin` can still contain anything.

### Saving and Loading

Finally, change the part of `Edit` that saves and loads the map (and the text at the top of the screen):

```cpp source="tmpl8-part-14/game.cpp#L244-L258" {1-2,4,6-9,14}
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-14-binary/index.html" title="The map editor, which can also save and load the map as a binary file with Shift+S and Shift+L" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The editor with binary files: <kbd>Shift</kbd>+<kbd>S</kbd> saves `assets/map.bin`, and <kbd>Shift</kbd>+<kbd>L</kbd> loads it. Save the map as a binary file, change a few tiles, and load it again. (Click the demo first, so that it gets the keys.)

</div>

This is what the start of `map.bin` looks like, byte by byte (in hexadecimal, see [part 9](/cpp-fast-track-9-colours/)):

```text title="assets/map.bin"
54 4D 41 50 0C 00 00 00 08 00 00 00 00 00 00 00
00 00 00 00 0A 00 00 00 01 00 00 00 00 00 00 00
```

`54 4D 41 50` are the character codes of `T`, `M`, `A` and `P`. Then comes `0C 00 00 00`: the `int` 12 (`0C` in hexadecimal), with the lowest byte first. That's how your processor stores an `int` in memory (it's called *little-endian*). Then 8, and then the tiles of the ground: 0, 0, 10 (`0A`), 1, and so on. You can't read this file in a text editor; Visual Studio can show it as bytes like this (open it with *Open With...* and choose the *Binary Editor*).

So is the binary file smaller? Not really: `map.bin` is 780 bytes (12 for the header, and 384 for each layer), and `map.txt` is 794 bytes. Every tile takes 4 bytes in the binary file, and 4 characters (`" 38,"`) in the text file. The tile numbers are small; a `float` like `3.14159274` takes 10 characters as text, but always 4 bytes in a binary file, so for other data, the difference can be much bigger. The real advantages of a binary file are that loading it is fast (one `read` for a whole array, without turning text into numbers), and that you don't lose anything: a `float` written as text may be rounded, but its 4 bytes are exact.

There are disadvantages too. You can't read or change the file in a text editor, and a mistake is hard to find. And the file depends on how *this* program stores its data in memory: a different compiler may add padding bytes to `MapHeader`, and a processor that stores an `int` the other way around reads 12 as 201326592. For files that you only use in your own game, that's fine. Many games use text files while the game is being made (they are easy to edit and to check), and binary files in the finished game (they are fast to load).

## Assignment

### Basic

Make a function that saves the screen as an image file, and call it when the player presses <kbd>P</kbd> (for *picture*: a letter key, so use `KeyFor`). Use the *TGA* format: it's a simple format, and image editors like Photoshop, GIMP and Paint.NET can open it.

A TGA file starts with a header of 18 bytes, followed by the pixels, row by row:

```cpp
struct TGAHeader
{
    unsigned char ID, colmapt;       // set both to 0
    unsigned char type;              // set to 2: an image without a palette, not compressed
    unsigned char colmap[5];         // set all elements to 0
    unsigned short xorigin, yorigin; // set both to 0
    unsigned short width, height;    // the size of the image
    unsigned char bpp;               // bits per pixel: set to 32
    unsigned char idesc;             // set to 0x20: the first row of pixels is the top row
};
```

The header must be exactly 18 bytes. Add `static_assert(sizeof(TGAHeader) == 18);` below the struct: the compiler will then check that for you.

:::tip[Hint]
The pixels of a surface are 4 bytes each, in exactly the order in which TGA wants them (blue, green, red, alpha), so you can write them with `write`, just like the layers of the map. `screen.GetBuffer()` is the address of the first pixel. Write the screen one row of `GetWidth()` pixels at a time, and move `GetPitch()` pixels forward for every row, like `DrawTile` does.
:::

<img src="/images/cpp-fast-track-14/screenshot001.png" alt="A screenshot of the tile map with the green tank, as saved by the game" width="768" height="512" class="screenshot" />

<div class="caption">

The result: a screenshot saved by the game, opened in an image editor.

</div>

### Intermediate

1.  Pressing <kbd>P</kbd> again overwrites the screenshot. Give every screenshot its own name: `screenshot001.tga`, `screenshot002.tga`, and so on. Don't overwrite the screenshots of an earlier run of the game either.
2.  Save the position of the tank in `map.txt` too, after the layers, and put the tank there when the map is loaded. A `map.txt` without a tank position (like the one you have now) must still load.

:::tip[Hint]
`std::format("screenshot{:03}.tga", 7)` gives `screenshot007.tga`: at least 3 digits, with zeros in front. `std::filesystem::exists(name)` (from `<filesystem>`) tells you whether a file already exists.
:::

The result: a new file every time you press <kbd>P</kbd>, and a tank that starts where you left it:

```text title="C:\Projects\C++ Fast Track\Part 14"
screenshot001.tga
screenshot002.tga
screenshot003.tga
```

### Hard

1.  Every tile number fits in a single byte: a `signed char` holds the numbers from -128 to 127. Make the binary file 4 times smaller, by writing one byte per tile instead of an `int`.
2.  Old `map.bin` files (with 4 bytes per tile) must still load. Add a *version number* to the header, and make `LoadMapBinary` read both versions. (But how do you recognize a file without a version number? Change the magic number too!)

The result: `map.bin` shrinks from 780 bytes to about 200, and the game still loads your old `map.bin`.

Once you have completed these, you may continue with the next part.
