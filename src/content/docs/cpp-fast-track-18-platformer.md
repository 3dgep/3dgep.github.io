---
title: "C++ Fast Track for Games Programming Part 18: Platformer"
description: "Platformer: build a complete 2D side-scrolling platformer from a fresh template, with scenes, tile levels, a camera, jumping that feels good, enemies, sound, menus and settings."
slug: cpp-fast-track-18-platformer
lastUpdated: 2026-11-23
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

In the previous parts, you learned the pieces of a game one at a time, like sprites, tiles, classes, files, physics, AI and sound. In this part, you put them together, and build a complete game from a fresh copy of the template. It's a 2D side-scrolling platformer called *Alien Jump*. You will:

1. split the game into *scenes*, like a title screen and the game itself,
2. load levels from text files, and draw them with tiles that fit together,
3. follow the player with a camera, in front of a background that scrolls slower than the level,
4. make an alien that runs, falls and jumps, and collides with the level,
5. tune the jump until it feels good,
6. add coins, spikes, lives and a flag at the end of each level,
7. add enemies that walk and fly, and that you can stomp on,
8. add sound effects and music,
9. and add a main menu, a pause menu, a level picker and a settings screen.

This part doesn't continue the code of an earlier part. Everything the game needs is explained here, so you only need to know the basics of C++ and classes. It's a long part, but every section ends with a game that runs, so you can take a break after any of them.

This is what you will build:

<iframe class="embed embed-tmpl8 embed-480x270" src="/demos/cpp-fast-track-18-settings/index.html" title="Alien Jump: a platformer with a main menu, three levels with coins, spikes and enemies, sound and music" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Alien Jump. Click the demo first, so that it gets the keys and can play sound. Run with the arrows or <kbd>A</kbd> and <kbd>D</kbd>, jump with <kbd>Space</kbd>, and pause with <kbd>P</kbd>.

</div>

## Getting Started

As usual, we start with a fresh copy of the template. Download it here:

[<img src="/images/github-mark.svg" alt="GitHub" width="120" class="thumb light-only" /><img src="/images/github-mark-white.svg" alt="GitHub" width="120" class="thumb dark-only" />https://github.com/3dgep/tmpl8](https://github.com/3dgep/tmpl8)

[<img src="/images/zip1.png" alt="Download tmpl8 as a zip file" width="120" class="thumb" />tmpl8-main.zip](https://github.com/3dgep/tmpl8/archive/refs/heads/main.zip)

Extract the zip file to `C:\Projects\C++ Fast Track`, and rename the `tmpl8-main` folder to `Part 18`.

The game also needs images, levels, sounds and music. Download them here:

[<img src="/images/zip1.png" alt="Download the assets of part 18 as a zip file" width="120" class="thumb" />part-18-assets.zip](/downloads/part-18-assets.zip)

Extract this zip file to the `Part 18` folder. Its `assets` folder goes into the `assets` folder of the template, and adds these files to it:

```txt
assets
├── backgrounds   the scrolling background of each level, and of the menus
├── levels        level1.txt, level2.txt and level3.txt
├── music         the music of the menus and the levels
├── sounds        the sound effects
├── ui            the title, the buttons and the panel of the menus
├── characters.png
├── font.png
├── tiles.png
└── CREDITS.md
```

The images come from [Pixel Platformer](https://kenney.nl/assets/pixel-platformer), [UI Pack (Adventure)](https://kenney.nl/assets/ui-pack-adventure), [UI Pack (Pixel Adventure)](https://kenney.nl/assets/ui-pack-pixel-adventure) and [Kenney Fonts](https://kenney.nl/assets/kenney-fonts) by Kenney. The sound effects come from Kenney's [Digital Audio](https://kenney.nl/assets/digital-audio), [Interface Sounds](https://kenney.nl/assets/interface-sounds) and [Music Jingles](https://kenney.nl/assets/music-jingles), from [512 Sound Effects (8-bit style)](https://opengameart.org/content/512-sound-effects-8-bit-style) by Juhani Junkala, [Victory](https://opengameart.org/content/victory) by celestialghost8 and the [Chiptune SFX Pack](https://opengameart.org/content/chiptune-sfx-pack) by FrogPog. The music is from [4 Chiptunes (Adventure)](https://opengameart.org/content/4-chiptunes-adventure) by Juhani Junkala. They all share their work for free, under the [CC0](https://creativecommons.org/publicdomain/zero/1.0/) license. `CREDITS.md` lists which file came from where.

Open `tmpl8.slnx` in the `Part 18` folder.

### A Smaller Screen

The template draws on a screen of 800×512 pixels. That's a lot of pixels for pixel art. The tiles of this game are only 18×18 pixels, and the characters 24×24, so they would look tiny on an 800×512 screen. Instead, the game draws on a screen of 480×270 pixels, and the template shows every pixel as a block of 2×2 pixels in the window. 480×270 has the same shape (16:9) as most monitors, so the game also fills the screen nicely in fullscreen.

The size of the screen is set in the template. Open `template.hpp` in the `tmpl8` folder of the **Solution Explorer**, and change the size near the top of the file:

```cpp source="tmpl8-part-18/start/tmpl8/template.hpp#L10-L18" {1-5,7}
```

`ScreenScale` is new. It tells the template how much larger the window is than the screen. Now the template has to use it. Open `template.cpp`, and find `ScaleWindow`. It calculates the size of the window, so multiply the size of the screen by `ScreenScale` there:

```cpp source="tmpl8-part-18/start/tmpl8/template.cpp#L144-L148" {4}
```

Now the window is twice as wide and twice as high as the screen.

Then find `SDL_CreateWindowAndRenderer` in `SDL_AppInit`, and make the window `ScreenScale` times as large as the screen there too:

```cpp source="tmpl8-part-18/start/tmpl8/template.cpp#L201-L201"
```

The template already scales the screen up to the window without blurring the pixels, so this is all it needs. Press <kbd>F5</kbd>, and you see that the spinning gun turret and the text of the template are twice as large now.

### Adding Files

This game has more code than the earlier parts, so it doesn't all go in `game.cpp`. Every class gets its own header file (`.hpp`) and source file (`.cpp`), next to `game.hpp` and `game.cpp` in the `Part 18` folder.

To add a file, right-click the `tmpl8` project in the **Solution Explorer**, and select **Add** > **New Item...**. Choose **C++ File (.cpp)** for a source file, or **Header File (.h)** for a header file, and type the name, like `scene.hpp`. Now check the **Location** at the bottom of the window. Visual Studio puts new files in the folder of the project file, which is the `tmpl8` folder, where the template's own files are. Change the location to the `Part 18` folder (one folder up), so that your files are next to `game.cpp`. Then click **Add**.

:::tip
Visual Studio remembers the location, so you only have to change it for your first file.
:::

The project already looks for header files in the `Part 18` folder, so `#include "scene.hpp"` finds the new file in every `.cpp` file of the game.

## Scenes

A game is more than its gameplay. Alien Jump has a title screen, a main menu, a settings screen, the levels, and a screen that says GAME OVER. Each of them draws something different, and reacts to the keys in a different way. If all of that were in `Game::Tick`, it would be one huge function full of `if` statements that check which screen is showing.

Instead, each screen is a *scene*, an object of its own class, with its own `Tick` and `Draw`. The game shows one scene at a time, and a scene can tell the game to switch to another scene.

### The Scene Class

Add a new header file `scene.hpp`:

```cpp source="tmpl8-part-18/1-scenes/scene.hpp#L1-L15"
```

`Scene` is a *base class*. It says which functions every scene has, but it doesn't do anything itself. The `virtual` in front of `Tick` and `Draw` means that a class that *derives* from `Scene` can give the function its own code, and that the game calls that code even when it only knows that the object is a `Scene`. The `= 0` makes them *pure virtual* functions. `Scene` has no code for them, so every scene must have its own. A class with a pure virtual function is *abstract*, which means that you can't make a `Scene` object, only objects of the classes that derive from it.

The *destructor* `~Scene` is virtual too. When the game deletes a scene, through a pointer to `Scene`, the virtual destructor makes sure that the destructor of the real class runs (and with it, the destructors of its members).

`class Surface;` is a *forward declaration*. It tells the compiler that `Surface` is a class, without including `surface.hpp`. That's enough for a function that only takes a reference to a `Surface`.

### Text

The scenes show text, like PRESS ENTER TO PLAY. The template's `Surface::Print` uses a tiny font, which would be hard to read in a game with large pixels. The assets contain a pixel font that matches the art. `font.png` is a sprite with a frame for every character from the space to the `~`, in the order of their ASCII codes (16 characters on a row, and 6 rows).

Add a header file `text.hpp` for a class that prints with it:

```cpp source="tmpl8-part-18/1-scenes/text.hpp#L1-L23"
```

`std::string_view` is a view of characters that are stored somewhere else, like a `std::string` or text between quotes. It can't change the text, but it doesn't make a copy of it either, so it's the type to use for a function that only reads text. Every character is equally wide (the font is *monospace*), so `Width` multiplies the number of characters by the distance from one character to the next.

`Advance` and `Height` are `static constexpr` members. `static` means that they belong to the class instead of to each object (`Text::Height` doesn't need a `Text`), and `constexpr` that their value is known while the program is compiled.

Add a source file `text.cpp`, with `Print`:

```cpp source="tmpl8-part-18/1-scenes/text.cpp#L1-L16"
```

`Print` draws the characters one by one, from left to right. The frame of a character is its code minus the code of the space, because the first frame is the space. A space has nothing to draw, so it's skipped, but it still moves `x` on to the next character.

Below `Print`, add `Centre`:

```cpp source="tmpl8-part-18/1-scenes/text.cpp#L18-L22"
```

`Centre` prints the text in the middle of the screen, which is handy for titles and menus. The left edge of the text is half of the space that's left over next to it.

### The Game Switches Scenes

The game owns the scene on the screen, and a scene can ask it to switch to another scene. Open `game.hpp`, and include the new headers at the top:

```cpp source="tmpl8-part-18/1-scenes/game.hpp#L1-L10" {5-6,10}
```

`scene.hpp` and `text.hpp` are the headers that you just wrote. `<memory>` is the header of `std::unique_ptr`, which you'll use in a moment.

At the bottom of the class, remove the example of the template, the `rotatingGun` sprite and `frame`. Then add these members:

```cpp source="tmpl8-part-18/1-scenes/game.hpp#L45-L54" {1-3,8-10}
```

A scene is an object of a class that derives from `Scene`, so the game can't store it as a `Scene` (that's an abstract class). It stores a *pointer* to it instead. A `std::unique_ptr` is a pointer that *owns* the object it points to. When the `unique_ptr` is destroyed, or gets another object, it deletes its object. You never have to call `delete` yourself, and the object can never be forgotten.

There are two pointers, one for the scene on the screen and one for the next scene. A scene calls `ChangeScene` while its own `Tick` runs. If `ChangeScene` replaced the scene right away, the `unique_ptr` would delete the scene whose `Tick` is still running, and the program would crash. So `ChangeScene` only remembers the next scene, and the game switches after the `Tick`. `std::move` moves the pointer into `nextScene`, because a `unique_ptr` can't be copied. Only one pointer can own an object.

The game also owns the `Text`, so that all the scenes can use the same font. `GetText` gives a scene a reference to it.

Now open `game.cpp`. Remove all the code in `Tick`, and the `#include <iostream>` at the top. Include the header of the title screen (which you will write in a moment), and `<algorithm>` for `std::min`. `Init` starts the game with the title screen:

```cpp source="tmpl8-part-18/1-scenes/game.cpp#L1-L15" {3,5,14}
```

`std::make_unique<MenuScene>(*this)` makes a `MenuScene` object, and gives it to a `unique_ptr`. The scene needs the game (for the keys, the text and to switch scenes), so it gets a reference to the game. `*this` is the `Game` object whose `Init` runs.

`Tick` passes the frame on to the scene:

```cpp source="tmpl8-part-18/1-scenes/game.cpp#L27-L37" {3-10}
```

The template gives `Tick` the time since the previous frame in milliseconds. The scenes work in seconds, which makes speeds like "120 pixels per second" easy to read. But when a frame takes very long (for example, when you were in the debugger, or the window was dragged around), everything would jump a long way in one frame, and the player could fall right through the floor. `std::min` makes sure that a frame counts as 1/30 of a second at most. The game slows down a bit when frames are that slow, but nothing goes wrong.

After the `Tick`, the next scene takes over, if there is one. `scene = std::move(nextScene)` deletes the old scene, and leaves `nextScene` empty (a `nullptr`) again.

### The Title Screen and the Play Scene

The first scene is the title screen. Add a header file `menu_scene.hpp` (it becomes the main menu later):

```cpp source="tmpl8-part-18/1-scenes/menu_scene.hpp#L1-L22"
```

`MenuScene` derives from `Scene` (that's the `: public Scene`), and gives `Tick` and `Draw` their code. `override` tells the compiler that these functions replace a virtual function of the base class. If you make a typo in the name or the parameters, the compiler says so, instead of quietly adding a new function that nobody calls.

The scene keeps a reference to the game. `class Game;` is a forward declaration again. A reference doesn't need more than that, so `menu_scene.hpp` doesn't have to include `game.hpp`. Only the source file includes it, because it calls the functions of the game.

Add the source file `menu_scene.cpp`, with `Tick`:

```cpp source="tmpl8-part-18/1-scenes/menu_scene.cpp#L1-L12"
```

When the player presses <kbd>Enter</kbd>, the title screen asks the game to switch to the play scene. `GetKeyPressed` is only true in the frame in which the key goes down, so holding <kbd>Enter</kbd> doesn't switch scenes in every frame.

Below `Tick`, add `Draw`:

```cpp source="tmpl8-part-18/1-scenes/menu_scene.cpp#L14-L20"
```

`0xdff6f5` is the light blue of the sky, and `title.png` is the name of the game on a banner, drawn in the middle of the screen. Below it, the font of the game says what to do.

The play scene doesn't do much yet. Add `play_scene.hpp`:

```cpp source="tmpl8-part-18/1-scenes/play_scene.hpp#L1-L20"
```

It looks just like `MenuScene`, without the title. Add `play_scene.cpp`, with `Tick`:

```cpp source="tmpl8-part-18/1-scenes/play_scene.cpp#L1-L13"
```

For now, <kbd>Enter</kbd> goes back to the title screen, so that you can see that the switch works both ways. Add `Draw`:

```cpp source="tmpl8-part-18/1-scenes/play_scene.cpp#L15-L21"
```

It clears the screen to the same sky, and says where the game will go. Press <kbd>F5</kbd>, and press <kbd>Enter</kbd> a few times:

<iframe class="embed embed-tmpl8 embed-480x270" src="/demos/cpp-fast-track-18-scenes/index.html" title="The title screen of Alien Jump; Enter switches to the play scene and back" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The title screen and the play scene. <kbd>Enter</kbd> switches between them. (Click the demo first, so that it gets the keys.)

</div>

## The Level

A level is a grid of tiles, like the maps in [part 11](/cpp-fast-track-11-tiles/). This time, the levels are text files, with one character for every tile. Open `assets/levels/level1.txt` in Visual Studio (or in Notepad). Here are the bottom rows of its first 48 columns (the rows above them are empty):

```txt
                  C C                     W
                  BBBB     C C E          C    #
                         ########
  P   g         t      E ########            m
###########   ######################^^###   ####
###########   ###########################   ####
###########   ###########################   ####
```

You can almost see the level in it. Every character is a tile of 18×18 pixels:

| Character | Tile |
| --- | --- |
| `#`, `~`, `*` | ground with grass, sand or snow on top |
| `B` | a crate |
| `C` | a coin |
| `^` | spikes |
| `F` and `\|` | the flag at the end of the level, and its pole |
| `g`, `t`, `c`, `m`, `s` | grass, a tree, a cactus, a mushroom and a snowman, which are only decoration |
| `P` | where the player starts |
| `E` and `W` | where an enemy starts, one that walks and one that flies |
| a space | nothing |

A text file is easy to make and to change. You can design a new level in any text editor, without a special tool.

### The Level Class

Add a header file `level.hpp`:

```cpp source="tmpl8-part-18/2-level/level.hpp#L1-L31"
```

The level stores its rows of characters in a `std::vector` of `std::string`s, so `rows[y][x]` is the character of the tile in column `x` of row `y`. `tiles.png` is a sprite sheet of 20×9 tiles, and each one is a frame of the `tiles` sprite.

Add a source file `level.cpp`, with `Load` first:

```cpp source="tmpl8-part-18/2-level/level.cpp#L1-L24"
```

A `std::ifstream` reads a file. `std::getline` reads one line of it into `line`, and returns something that counts as `false` when there are no more lines, which ends the `while` loop. If the file can't be opened, `NotifyUser` (from the template) shows a message.

The rows in the file don't all have the same length, because a text editor doesn't save the spaces at the end of a line. So `Load` finds the length of the longest row, and makes every row that long. `resize` adds spaces at the end of a shorter row.

Below `Load`, add `TileAt`:

```cpp source="tmpl8-part-18/2-level/level.cpp#L26-L31"
```

`TileAt` gives the character of a tile, or a space for a tile outside the level. The code that uses the level often looks at tiles next to another tile, and that tile can be outside the level. Thanks to this check, it doesn't have to check that itself.

### Drawing the Tiles

`Draw` draws every tile of the level at its place on the screen:

```cpp source="tmpl8-part-18/2-level/level.cpp#L33-L46"
```

The two loops go through the rows, and through the columns of each row. A tile in column `x` and row `y` is drawn at `x * TileSize` and `y * TileSize`. `Frame` tells which frame of `tiles.png` belongs to the tile. An empty tile has no frame (`-1`), and `continue` skips it.

Below `Draw`, add `Frame`:

```cpp source="tmpl8-part-18/2-level/level.cpp#L48-L66"
```

A `switch` compares a value with a list of `case`s, and jumps to the one that matches. Several `case`s can share the same code, like the three kinds of ground on the first line. Most characters always get the same frame, like the crate, which is frame 26 (the 7th tile of the 2nd row). Any other character, like a space, ends up at `default`, and gets no frame. Ground is different, because its frame depends on the tiles around it.

### Ground That Fits Together

Look at the ground in the screenshot further down. The top of a piece of ground has grass on it, and the dirt below it doesn't. A piece of ground also has an edge on the left and the right side, but not between two tiles of ground next to each other. So the frame of a ground tile depends on its *neighbours*. This is called *autotiling*. You only write `#` in the level file, and the game picks the right frame.

These are the ground tiles in `tiles.png`, in the first four columns of the first eight rows:

<img src="/images/cpp-fast-track-18/ground-tiles.png" alt="The ground tiles of tiles.png, enlarged: four columns and eight rows. Rows 0 to 5 have grass, sand or snow on top, rows 6 and 7 are dirt. Column 0 has an edge on both sides, column 1 on the left, column 2 on neither side, and column 3 on the right." width="692" height="674" />

<div class="caption">

The ground tiles of `tiles.png`. The column says which sides have an edge, and the row says what is on top.

</div>

Below `Frame`, add `GroundFrame` and `IsGround`:

```cpp source="tmpl8-part-18/2-level/level.cpp#L68-L90"
```

The *column* of the frame depends on the left and the right neighbour. A tile with ground on neither side gets both edges (column 0), and a tile with ground on both sides gets no edges (column 2). The `?:` is the *conditional operator*. `a ? b : c` is `b` when `a` is true, and `c` when it's false. It's a short way to write an `if` and an `else` that each give a value.

The *row* depends on the tile above and below it. A tile without ground above it is the top of the ground, so it gets grass, sand or snow, and row 0, 2 or 4 for a tile that is alone, or row 1, 3 or 5 for a tile with more ground below it. A tile with ground above it is dirt. It gets row 6, or row 7 (with an edge at the bottom) when there's no ground below it.

There are 20 frames on a row of `tiles.png`, so the frame is `row * 20 + column`.

`IsGround` treats the tiles outside the level as ground at the sides and below it. That way, the ground at the bottom and at the ends of the level doesn't get an edge there, but looks like it goes on.

### Showing the Level

The play scene loads the first level, and draws it. In `play_scene.hpp`, include `level.hpp`, add a `Level` to the scene, and give the scene a constructor of its own (instead of the one in the header):

```cpp source="tmpl8-part-18/2-level/play_scene.hpp#L1-L22" {3,14,20}
```

The scene owns its level, so the level is a member of the scene, and it's gone when the scene is.

In `play_scene.cpp`, the constructor loads the level, and `Draw` draws it on the sky. Remove the code in `Tick` and the `#include "menu_scene.hpp"`, because the play scene doesn't go back to the title screen anymore:

```cpp source="tmpl8-part-18/2-level/play_scene.cpp#L1-L22" {7-10,14,20}
```

Press <kbd>F5</kbd>, and <kbd>Enter</kbd> on the title screen:

<img src="/images/cpp-fast-track-18/level.png" alt="The first part of level 1 on a light blue sky: grass-topped ground with a hole in it, a row of four crates with two coins on top, a small tree and a plant, and a higher step of ground at the right edge." width="960" height="540" class="screenshot" />

<div class="caption">

The first part of level 1. The rest of the level is to the right of the screen.

</div>

Only the left part of the level fits on the screen. The rest of it is drawn too, but outside the screen, where the sprite doesn't draw anything. To see the rest, we need a camera.

## Camera and Background

A *camera* decides which part of the level is on the screen. It has a position in the level, the top-left corner of the screen. Everything in the level is drawn at its position in the level minus the position of the camera. When the camera moves to the right, everything on the screen moves to the left.

### The Camera Class

Add a header file `camera.hpp`. The camera is small, so all of its code is in the header:

```cpp source="tmpl8-part-18/3-camera/camera.hpp#L1-L34"
```

The camera follows a *target*, like the player. `Goal` is where the camera should be to have the target in the middle of the screen. But it shouldn't show what's past the edges of the level, so `std::clamp` keeps the goal between 0 and the size of the level minus the size of the screen. In a level that's as high as the screen, that means that the camera can't move up or down at all.

`Follow` doesn't jump to the goal, but moves a part of the way there in every frame. With `Speed` 8, it moves 8 × `deltaTime` of the distance, which is 8/60 (about 13%) of the distance in a frame at 60 frames per second. When the target is far away, the camera moves fast, and it slows down when it gets close. That makes the camera move smoothly, instead of jerking along with every step of the player. `MoveTo` does jump to the goal right away, which is what you want at the start of a level.

`X` and `Y` give the position as whole pixels, because the screen has no half pixels.

### Drawing Through the Camera

In `level.hpp`, add a forward declaration of `Camera`, and give `Draw` the camera. The scene also needs to know how large the level is, in pixels:

```cpp source="tmpl8-part-18/3-camera/level.hpp#L9-L27" {3,10,13,17-19}
```

`ToTile` turns a position in pixels into the number of the tile at that position. `std::floor` rounds down, also for negative positions (`-0.5` becomes `-1`, the tile to the left of the level, while a cast to `int` would make it `0`).

In `level.cpp`, include `camera.hpp` at the top:

```cpp source="tmpl8-part-18/3-camera/level.cpp#L1-L6" {3}
```

The forward declaration in `level.hpp` is enough for a parameter, but `Draw` calls the functions of the camera, so the source file needs the whole class. Now change `Draw`:

```cpp source="tmpl8-part-18/3-camera/level.cpp#L35-L51" {1-9,14-17}
```

The tiles are drawn at their position minus the position of the camera. A level can be much larger than the screen (the levels of this game are about 120 tiles wide), so `Draw` only draws the tiles that are on the screen. `left` is the tile at the left edge of the screen, and `right` the one at the right edge, which may only be partly visible. The same goes for `top` and `bottom`.

### A Background That Scrolls Slower

A sky in one colour looks empty. The background of each level is a strip of clouds with hills or trees below them, in the colours of its ground. It's drawn behind the level, and repeats to the left and to the right.

The strip moves slower than the level when the camera moves. Things that are far away, like clouds, seem to move slower than things nearby when you move. That's called *parallax*, and it makes the flat background look far away.

Add a header file `background.hpp`:

```cpp source="tmpl8-part-18/3-camera/background.hpp#L1-L23"
```

The strip is a sprite with one frame. The top of each strip (in `assets/backgrounds`) is a piece of sky, so the colour of its top-left pixel is the colour of the sky of that background. The constructor takes the file of the strip, so every level can have its own background.

Add `background.cpp`, with the constructor:

```cpp source="tmpl8-part-18/3-camera/background.cpp#L1-L9"
```

In the *initializer list* of the constructor (after the `:`), `strip` loads the file, and `sky` gets the colour of the first pixel of the strip. The members are made in the order in which they're declared in the class, so `strip` is ready when `sky` reads it. `& 0xffffff` keeps the red, green and blue of the pixel, and removes its alpha. That's all the constructor has to do, so its body is empty.

Below the constructor, add `Draw`:

```cpp source="tmpl8-part-18/3-camera/background.cpp#L11-L23"
```

`Draw` fills the screen with the sky, and then draws the strip as many times as it takes to fill the width of the screen. The strip moves at 30% of the speed of the camera. The `%` (the remainder of a division) makes the strip start again after one strip width, so the first copy always sticks out over the left edge of the screen by 0 to `width - 1` pixels, however far the camera has moved.

The strip is drawn at the bottom of the screen. When the camera goes up in a tall level, the strip goes down, by 30% of the height of the camera above the bottom of the level. That's the `height` of `Draw`.

### Flying Through the Level

There's no player yet, so for now, the arrow keys move the target of the camera. This lets you fly through a level, and look at it. In `play_scene.hpp`, include the headers of the camera and the background, and add the camera, the background and the target:

```cpp source="tmpl8-part-18/3-camera/play_scene.hpp#L1-L27" {3-4,23-25}
```

`target` is a position in the level, like the position of the player will be later on.

Level 3 is the tallest level, so the scene loads level 3 for now, with the desert background that goes with its sand. In `play_scene.cpp`, include `<algorithm>`, and change the constructor:

```cpp source="tmpl8-part-18/3-camera/play_scene.cpp#L1-L14" {5,11-13}
```

The constructor puts the target at the bottom-left corner of the level, and the camera jumps there.

In `Tick`, the arrow keys move the target:

```cpp source="tmpl8-part-18/3-camera/play_scene.cpp#L16-L31" {3-15}
```

The target moves 300 pixels per second. `GetKey` is true as long as a key is held down, so the target keeps moving while you hold an arrow key. `std::clamp` keeps the target in the level, and the camera follows it.

`Draw` draws the background first, and the level on top of it:

```cpp source="tmpl8-part-18/3-camera/play_scene.cpp#L33-L38" {3-4}
```

The level is drawn through the camera now, and the background needs to know how high the camera is. The level is `level.Height()` pixels high, and the camera can go as low as `level.Height() - ScreenHeight`, so the camera is `level.Height() - ScreenHeight - camera.Y()` pixels above the bottom of the level.

Press <kbd>F5</kbd>, and fly through the level with the arrow keys:

<iframe class="embed embed-tmpl8 embed-480x270" src="/demos/cpp-fast-track-18-camera/index.html" title="Level 3 of Alien Jump, with the camera flown by the arrow keys; the desert background scrolls slower than the level" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Fly through level 3 with the arrow keys, and watch the background scroll slower than the level. Press <kbd>Enter</kbd> on the title screen first. (Click the demo first, so that it gets the keys.)

</div>

## The Player

Time for the star of the game, a little alien in a space helmet. It runs, falls and jumps, and it stands on the ground instead of falling through it. That's *physics*, like in [part 15](/cpp-fast-track-15-physics/), but simpler. A platformer doesn't need bouncing balls or rotation. It needs boxes that stop at walls and floors.

### A Body

The player and the enemies all move through the level in the same way, so they share a `Body`. Add a header file `body.hpp`:

```cpp source="tmpl8-part-18/4-player/body.hpp#L1-L23"
```

A body is a box, with its top-left corner at `position`. It moves with its `velocity`, in pixels per second, and it knows whether it stands on the ground. This box is called a *collider*. It's used for the collisions, and it's often a bit smaller than the sprite, which has empty space around the character.

`Overlaps` checks whether two boxes overlap. Two boxes overlap when each one starts before the other one ends, both on the x axis and on the y axis. If one box is completely to the left of the other one, or completely above it, they don't.

`Body` is a `struct` instead of a `class`. They're the same, except that the members of a `struct` are `public` unless you say otherwise. A `struct` is a good fit for a group of values that other code reads and changes, like this one.

### Solid Tiles

The level decides where a body can go. In `level.hpp`, include `body.hpp` instead of `template.hpp` (which `body.hpp` includes itself), and add the functions for the collisions and the start of the player:

```cpp source="tmpl8-part-18/4-player/level.hpp#L1-L44" {3,24-26,31,35,41}
```

`IsSolid` tells whether the player can stand on a tile, and `HitBox` gives the box of a tile. `Move` moves a body through the level, and `HitsSolid` helps it. `PlayerStart` is where the player starts.

The player's start is the `P` in the level file. At the end of `Load`, the level looks for it:

```cpp source="tmpl8-part-18/4-player/level.cpp#L20-L39" {8-19}
```

The `P` isn't a tile that should be drawn, so it becomes a space.

Below `TileAt`, add `IsSolid` and `HitBox`:

```cpp source="tmpl8-part-18/4-player/level.cpp#L48-L64"
```

Ground and crates are solid. To the left and the right of the level, there's a wall, so the player can't walk out of the level. But above and below the level there's nothing, so the player can jump higher than the top of the level, and fall into a hole. `HitBox` gives the box of a tile, which is the whole tile for now.

### Moving Through the Level

At the bottom of `level.cpp`, after `IsGround`, add `HitsSolid`:

```cpp source="tmpl8-part-18/4-player/level.cpp#L127-L142"
```

`HitsSolid` checks the tiles under the box of the body, from the tile of its left edge to the tile of its right edge, and from its top to its bottom. The right edge of a box at x 0 with a width of 18 is at 18, which is the first pixel of the next tile. That pixel isn't part of the box, so `HitsSolid` looks at the tile of 18 − 0.01 instead.

Below it, add `Move`:

```cpp source="tmpl8-part-18/4-player/level.cpp#L144-L171"
```

`Move` moves the body by its velocity, but it does that in two steps, first sideways, and then up or down. After each step, it checks whether the body is in a solid tile. If it is, the body moves back to the edge of that tile, and its velocity in that direction becomes 0. Moving right into a wall puts the right edge of the body against the left edge of the wall tile. Falling onto the ground puts the bottom of the body on the top of the ground tile, and then the body is `onGround`.

Why two steps? When a body moves diagonally into a corner, one check can't tell whether it hit the wall from the side or the floor from above. With two steps, the sideways step only knows about walls, and the vertical step only about floors and ceilings. A body that runs along the ground and falls a bit every frame (because of gravity) lands on the ground in the vertical step, and isn't stopped by it in the sideways step.

:::note
`Move` works as long as a body doesn't move more than a tile in one frame. A tile is 18 pixels, and the game runs at least at 30 frames per second (that's the limit in `Game::Tick`), so a body can move up to 540 pixels per second. The player never moves that fast.
:::

### The Player Class

Add a header file `player.hpp`:

```cpp source="tmpl8-part-18/4-player/player.hpp#L1-L25"
```

The player has a body, and remembers which way it faces. `Spawn` puts it at its start.

The sprites of the characters are in `characters.png`. It has 9 frames on a row, and the first three rows are the characters of Kenney's pack, facing left. The template's `Sprite` can't draw a frame mirrored, so the next three rows have the same characters facing right. That's why the frame of a character facing right is 27 (3 rows of 9) frames further on. The last six rows have all of them upside down, which you will need for the enemies.

Add `player.cpp`, with the numbers that make the player move:

```cpp source="tmpl8-part-18/4-player/player.cpp#L1-L18"
```

These constants are in an *unnamed namespace*. Like a `static` function, the names in an unnamed namespace can only be used in this `.cpp` file, so they never clash with names in the rest of the game. The enemies can have a `Gravity` of their own.

Below the constants, in the same namespace, add a small helper function:

```cpp source="tmpl8-part-18/4-player/player.cpp#L20-L27"
```

`MoveTowards` changes a value in the direction of a target, but no further than the target. It's how the player speeds up and slows down. The `}` at the end closes the unnamed namespace.

`Spawn` puts the player on its start:

```cpp source="tmpl8-part-18/4-player/player.cpp#L29-L36"
```

The body is smaller than the 24×24 sprite. The start is the top-left corner of the `P` tile, so the body is placed in the middle of that tile, with its feet on the bottom of the tile.

`Tick` moves the player:

```cpp source="tmpl8-part-18/4-player/player.cpp#L38-L59"
```

The arrow keys, or <kbd>A</kbd> and <kbd>D</kbd>, give the direction. `KeyFor(SDLK_A)` is the key with an A on it. On a French keyboard, that's where the Q is on an English one, so the player always presses the key that says A. The player doesn't start or stop at full speed, but speeds up and slows down with an acceleration. In the air, the acceleration is lower, so the player has less control there, but not none. That's how the players of most platformers expect it to be.

A jump only starts when the player stands on the ground, and gives the player an upward velocity (a negative y, because y goes down on the screen). Gravity pulls the velocity down a bit in every frame, until the player falls at `MaxFallSpeed`. Then `Move` moves the player through the level.

`Draw` draws the sprite:

```cpp source="tmpl8-part-18/4-player/player.cpp#L61-L69"
```

The sprite is centred on the body sideways, and its bottom is at the bottom of the body, so that the feet of the alien are on the ground.

### Seeing the Colliders

The collisions happen with boxes that you can't see, which makes them hard to check. Does the player stand on the ground, or does it float a pixel above it? It helps to show the boxes. Press <kbd>C</kbd> in the game, and the play scene draws them over everything else.

In `play_scene.hpp`, include `player.hpp` and add the player, the sprite of the characters, and the collider view. The level is now level 1, with its green background:

```cpp source="tmpl8-part-18/4-player/play_scene.hpp#L1-L32" {6,22,26,28-30}
```

`characters` has 9 frames on a row, and 12 rows. `showColliders` is `false` at the start, so the boxes only show after you press <kbd>C</kbd>.

In `play_scene.cpp`, remove the `#include <algorithm>`. The constructor now puts the player at its start, and the camera on the player:

```cpp source="tmpl8-part-18/4-player/play_scene.cpp#L1-L12" {9-11}
```

`level.Load` reads the `P` of the level, so the player's start is known after it.

In `Tick`, remove the code for the arrow keys, because the player moves now, and the camera follows the player instead of `target`:

```cpp source="tmpl8-part-18/4-player/play_scene.cpp#L14-L24" {3-4,6-10}
```

<kbd>C</kbd> turns the collider view on and off. A player that falls out of the bottom of the level starts at the beginning again.

In `Draw`, draw the player after the level, so that it's in front of it, and the colliders after that:

```cpp source="tmpl8-part-18/4-player/play_scene.cpp#L26-L33" {5-7}
```

`characters` is the sprite with all the characters, and the player picks its own frame from it.

Add `DrawColliders` at the bottom of the file:

```cpp source="tmpl8-part-18/4-player/play_scene.cpp#L35-L53"
```

`draw` is a *lambda*, a function without a name, that you can make inside another function. `[&]` lets it use the variables of the function around it, like `screen`, `cameraX` and `cameraY`, by reference. It draws the outline of a box, at its place on the screen. `DrawColliders` draws the solid tiles on the screen in green, and the body of the player in magenta.

Press <kbd>F5</kbd>. Run, jump onto the crates, and press <kbd>C</kbd> to see the colliders:

<iframe class="embed embed-tmpl8 embed-480x270" src="/demos/cpp-fast-track-18-player/index.html" title="The alien runs and jumps through level 1; C shows the solid tiles in green and the player's collider in magenta" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Run with the arrows or <kbd>A</kbd> and <kbd>D</kbd>, and jump with <kbd>Space</kbd>. <kbd>C</kbd> shows the colliders. (Click the demo first, and press <kbd>Enter</kbd> on the title screen.)

</div>

## A Better Jump

The player can jump now, but try to jump over the holes and onto the crates, and you'll notice that it doesn't feel great. Sometimes you press <kbd>Space</kbd> and nothing happens. Every jump has the same height, and the alien doesn't move its legs. Players don't know *why* a game feels good or bad, but they feel it right away. Three small tricks that most platformers use make a big difference.

**Coyote time.** When you run off a ledge and press jump a moment too late, the player is already in the air, so nothing happens. That feels unfair, because to you, the alien was still on the ledge. With coyote time, the player can still jump for a short time (a tenth of a second) after running off a ledge. It's named after the coyote in the cartoons, who runs off a cliff and only falls when he looks down.

**A jump buffer.** When you press jump a moment *before* the player lands, nothing happens either, because the player isn't on the ground yet. A jump buffer remembers the press for a tenth of a second, and jumps as soon as the player lands.

**A variable jump height.** When you let go of the jump key while the player still goes up, the upward speed is cut in half. A tap makes a small hop, and holding the key makes a high jump, so you can control how high you jump.

In `player.hpp`, add two timers and the time for the animation:

```cpp source="tmpl8-part-18/5-jump/player.hpp#L21-L27" {4-6}
```

`coyoteTime` and `jumpBuffer` are the time that's left, in seconds. They count down to 0, and while they're above 0, they're active.

In `player.cpp`, add the times of coyote time and the jump buffer to the constants:

```cpp source="tmpl8-part-18/5-jump/player.cpp#L11-L20" {9-10}
```

Both are a tenth of a second. That's long enough to make a late or early press count, and short enough that you don't notice it.

`Spawn` resets the new members:

```cpp source="tmpl8-part-18/5-jump/player.cpp#L31-L39" {8}
```

`coyoteTime = jumpBuffer = animationTime = 0` gives all three the value 0. An assignment is also an expression with the assigned value, so the value goes from right to left through the `=`s.

In `Tick`, replace the jump:

```cpp source="tmpl8-part-18/5-jump/player.cpp#L54-L73" {1,4-9,11-15,19}
```

The two timers count down. Pressing jump starts the jump buffer at 0.1 seconds, and standing on the ground keeps coyote time at 0.1 seconds. The player jumps when both are above 0, which means that jump was pressed less than 0.1 seconds ago, and the player was on the ground less than 0.1 seconds ago. In the normal case, you press jump while you stand on the ground, and both are true right away. After a jump, both timers become 0, so the same press can't make a second jump.

When the jump key goes up while the player still moves up (a negative y velocity), the velocity is cut in half, and gravity stops the jump sooner. The height of a jump grows with the square of its speed, so a jump that is released right away is only a quarter as high as a full one.

At the end of `Tick`, the animation time goes up with the time of the frame.

Now let the alien move its legs. Frame 0 of `characters.png` stands still, and frame 1 has one foot forward. Change `Draw`:

```cpp source="tmpl8-part-18/5-jump/player.cpp#L75-L90" {3-11}
```

In the air, the alien shows frame 1. On the ground, it switches between frame 0 and 1 eight times per second while it runs. `animationTime * 8` counts up by 8 every second, and `% 2` makes that 0, 1, 0, 1, and so on.

Press <kbd>F5</kbd>, and try to jump off the very edge of a ledge, and onto the crates:

<iframe class="embed embed-tmpl8 embed-480x270" src="/demos/cpp-fast-track-18-jump/index.html" title="The alien runs with a walking animation, and jumps higher when the jump key is held longer" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Tap <kbd>Space</kbd> for a small hop, or hold it for a high jump. (Click the demo first, and press <kbd>Enter</kbd> on the title screen.)

</div>

:::tip
The numbers at the top of `player.cpp` decide how the game feels. Try a lower `Gravity` for floaty jumps, a higher `Acceleration` for snappy running, or a `CoyoteTime` of 0 to feel what it's like without coyote time. Tuning these numbers until the game feels right is a big part of making a platformer.
:::

## Coins, Spikes and the Flag

Now it's time to make it a game, with something to collect, something to avoid, and a goal. The player collects the coins, loses a life on the spikes or in a hole, and goes to the next level at the flag. When all lives are gone, it's game over.

### Hit Boxes

The coins, the spikes and the flag aren't solid, so `Move` doesn't stop the player at them. Instead, the play scene checks which of these tiles the player touches. But a tile of spikes is mostly empty, because the spikes are only at its bottom. Losing a life by touching the empty part above them would feel unfair. And a coin is smaller than its tile too. So each tile gets a *hit box*, the part of the tile that the player touches.

In `level.hpp`, add `Tick` and `SetTile`, and the time for the animated tiles:

```cpp source="tmpl8-part-18/6-pickups/level.hpp#L18-L45" {4,8,27}
```

The coins spin, and the flag waves, so the level keeps track of the time. At the end of `Load`, the time starts at 0 again:

```cpp source="tmpl8-part-18/6-pickups/level.cpp#L31-L40" {9}
```

That way, the animations start at the beginning in every level.

Below `TileAt`, add `SetTile`, which the scene uses to remove a coin that the player collected:

```cpp source="tmpl8-part-18/6-pickups/level.cpp#L49-L53"
```

Like `TileAt`, it checks that the tile is in the level. A tile outside the level isn't changed.

Change `HitBox`. The spikes and the coins get a smaller box:

```cpp source="tmpl8-part-18/6-pickups/level.cpp#L64-L83" {1-2,8-18}
```

The spikes of `tiles.png` are 16 pixels wide and 7 pixels high, at the bottom of their tile. `box.position += vec2(1, 11)` moves the box 1 pixel to the right and 11 pixels down. A `case` without a `break` would go on with the code of the next `case`, so every `case` ends with a `break`.

In `Frame`, the coin and the flag switch between two frames:

```cpp source="tmpl8-part-18/6-pickups/level.cpp#L103-L122" {4,9,11}
```

`flip` is 0 or 1, and changes four times per second, so a coin switches between its two frames 151 and 152, and the flag between 111 and 112.

### Lives, Coins and Levels

The play scene keeps the score, and goes through the levels. Change `play_scene.hpp`:

```cpp source="tmpl8-part-18/6-pickups/play_scene.hpp#L1-L49" {13,17,24,26-30,39,42-47}
```

`LevelCount` is the number of levels in the `levels` folder. `State` says what the scene is doing. While the player plays, the state is `Playing`. After dying, the state is `Dying` for a moment, and the player blinks before the level starts again. After the flag, the state is `Finished` for a moment, before the next level starts. `waitTime` is how much longer that moment lasts.

`enum class` is a *scoped enumeration*. Its values are written as `State::Playing` instead of just `Playing`, so they can't clash with other names, and a `State` can't be mixed up with an `int` by accident.

`icons` is a second sprite of `tiles.png`, for the heart and the coin at the top of the screen. It's a separate sprite, because the level changes the frame of its own `tiles` sprite all the time.

In `play_scene.cpp`, include `menu_scene.hpp` (the game goes back to the title screen when it's over) and `<format>`. The level now starts in a function of its own, so that the scene can start a level again, or start the next one. The constructor only calls it:

```cpp source="tmpl8-part-18/6-pickups/play_scene.cpp#L1-L13" {4,6,12}
```

Below the constructor, add `StartLevel`:

```cpp source="tmpl8-part-18/6-pickups/play_scene.cpp#L15-L23" {1-4,7-9}
```

`std::format` makes a string from a pattern, and puts the values into the `{}`s. For level 2, `std::format("assets/levels/level{}.txt", levelNumber)` makes `"assets/levels/level2.txt"`. Every level has its own background too, so `StartLevel` makes a new `Background` with the strip of the level, and assigns it to `background`. A `Sprite` shares its image, so that copy is cheap.

`coinsAtStart` remembers how many coins the player had when the level started. When the player dies, the coins of the level come back, and so does the count.

Change `Tick`:

```cpp source="tmpl8-part-18/6-pickups/play_scene.cpp#L25-L45" {6-13,16-17,19}
```

The level ticks all the time, so the coins keep spinning. While the scene waits, after dying or after the flag, the player doesn't move, and `AfterWait` is called when the wait is over. Falling out of the level now costs a life.

Below `Tick`, add `TouchTiles`:

```cpp source="tmpl8-part-18/6-pickups/play_scene.cpp#L47-L72"
```

The player's body can touch at most a few tiles, the ones under its box. For each of them, `TouchTiles` checks whether the body overlaps the hit box of the tile. A coin is collected and removed from the level. Spikes kill the player. The flag (or its pole) finishes the level, but only once, because the player can touch the flag in several frames.

Add `Die`:

```cpp source="tmpl8-part-18/6-pickups/play_scene.cpp#L74-L81"
```

The player can only die while playing. Without that check, a player who falls onto two tiles of spikes at once would lose two lives.

Add `AfterWait`:

```cpp source="tmpl8-part-18/6-pickups/play_scene.cpp#L83-L100"
```

After dying, the game is over when there are no lives left. Otherwise, the coins of the level come back, and the level starts again. After the flag, `++levelNumber` goes to the next level. The `++` in front adds 1 *before* the comparison, so after the last level, the game is over too. For now, the end of the game goes back to the title screen.

### Blinking and the HUD

In `Draw`, the player blinks after dying, and the *HUD* (the *heads-up display*, the information on top of the game) shows the lives, the coins and the number of the level:

```cpp source="tmpl8-part-18/6-pickups/play_scene.cpp#L102-L113" {5-6,11}
```

`waitTime * 10` counts down by 10 every second, so `% 2` switches between 0 and 1 ten times per second. The player is only drawn when it's 0, which makes it blink.

`DrawColliders` shows the hit boxes too, in the colour of what they do:

```cpp source="tmpl8-part-18/6-pickups/play_scene.cpp#L115-L141" {1-2,15,18-23}
```

The spikes are red, because they hurt. The coins are yellow, and the flag and its pole are blue.

At the bottom of the file, add `DrawHud`:

```cpp source="tmpl8-part-18/6-pickups/play_scene.cpp#L143-L157"
```

The HUD has a heart for each life, and below the hearts a coin with the number of coins. The number of the level is in the top-right corner. `Text::Width` gives the width of the text, so the text ends 6 pixels before the right edge of the screen.

Press <kbd>F5</kbd>, and play all three levels. Press <kbd>C</kbd> to see the hit boxes of the spikes and the coins:

<iframe class="embed embed-tmpl8 embed-480x270" src="/demos/cpp-fast-track-18-pickups/index.html" title="Alien Jump with coins to collect, spikes, lives, a flag at the end of each level and a HUD" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Collect the coins, avoid the spikes and the holes, and reach the flag. (Click the demo first, and press <kbd>Enter</kbd> on the title screen.)

</div>

## Enemies

A platformer needs enemies. Alien Jump has two kinds. A *walker* is a little robot that walks back and forth on a platform, and a *flyer* is a bat-like creature that flies back and forth while it bobs up and down. Touching an enemy costs a life, unless you jump on top of it. Then it's squashed, and it falls off the screen upside down, like in the old platformers.

### Walls

A walker turns around when it walks into a wall, so it has to know when that happens. In `body.hpp`, add a flag for it:

```cpp source="tmpl8-part-18/7-enemies/body.hpp#L8-L14" {7}
```

`Level::Move` will set it, a bit further down.

### The Enemy Class

Add a header file `enemy.hpp`:

```cpp source="tmpl8-part-18/7-enemies/enemy.hpp#L1-L36"
```

Both kinds of enemy are an `Enemy`, with a `Kind` that says whether it walks or flies. They have a lot in common (a body, a sprite, dying when squashed), and they only differ in how they move and which frames they show. A `Kind` and two functions, `Walk` and `Fly`, are enough for that.

:::note
Another way to do this is a base class `Enemy` with a virtual function `Move`, and a class for each kind of enemy that derives from it, like the scenes. That's a good choice when the kinds of enemies are very different, or when there are many of them. For two kinds that differ in a few lines, an `enum` keeps the code simpler, and the enemies can be stored in a plain `std::vector<Enemy>`.
:::

`Squash` is called when the player jumps on the enemy. A squashed enemy is *dying*. It falls off the screen, and it's gone (`IsGone`) when it's below the bottom of the screen.

Add `enemy.cpp`, with the numbers that make the enemies move:

```cpp source="tmpl8-part-18/7-enemies/enemy.cpp#L1-L21"
```

Like the numbers of the player, they're in an unnamed namespace, so the `Gravity` of the enemies doesn't clash with the `Gravity` in `player.cpp`. The enemies walk and fly a lot slower than the player runs, so that the player can get past them.

The constructor sets up the body:

```cpp source="tmpl8-part-18/7-enemies/enemy.cpp#L23-L38"
```

A walker stands on the bottom of its tile, like the player. A flyer starts in the middle of its tile, and remembers that position as the middle of its flight. `vec2(Level::TileSize)` is a `vec2` with the tile size as both its x and its y.

`Squash` makes the enemy hop up a bit:

```cpp source="tmpl8-part-18/7-enemies/enemy.cpp#L40-L45"
```

From now on, the enemy is dying. A velocity of −150 sends it up a bit first, before gravity pulls it down.

`Tick` moves the enemy:

```cpp source="tmpl8-part-18/7-enemies/enemy.cpp#L47-L59"
```

A dying enemy falls with gravity, but it isn't moved with `Level::Move`, so it falls right through the ground and off the screen. The others walk or fly.

### Walking

Add `Walk`:

```cpp source="tmpl8-part-18/7-enemies/enemy.cpp#L61-L76"
```

A walker walks in the direction it faces, falls with gravity, and turns around when it walks into a wall. It also turns around at the edge of a platform, so that it doesn't walk off it. It checks the tile just in front of its feet, one pixel past its front edge and one pixel below its bottom. If that tile isn't solid, there's no ground there, and the walker turns around.

### Flying

Add `Fly`:

```cpp source="tmpl8-part-18/7-enemies/enemy.cpp#L78-L92"
```

A flyer goes back and forth like a walker, but without gravity, and it turns around at two tiles to either side of its start, or at a wall.

It also goes up and down along a *sine wave*. `std::sin` goes smoothly from 0 up to 1, down to −1 and back to 0, every 2π (about 6.28). `animationTime * FlyBobSpeed` goes up by 2.5 every second, so a flyer goes up and down once every 2.5 seconds. Multiplied by `FlyHeight`, the flyer goes 1.5 tiles above and below its start. The y position is set directly, not with `Level::Move`, so a flyer must have room to fly. That's up to whoever designs the level.

### Drawing the Enemies

Add `Draw`:

```cpp source="tmpl8-part-18/7-enemies/enemy.cpp#L94-L110"
```

A walker switches between frames 18 and 19 six times per second, and a flyer flaps its wings with frames 24 and 25 eight times per second. Like the player, the frames facing right are 27 frames further on. The upside-down frames are another 54 frames further, in the bottom half of `characters.png`. The walker stands on its feet, and the flyer is centred on its body.

### Enemies in the Level

The level file says where the enemies start, with an `E` for a walker and a `W` for a flyer. In `level.hpp`, include `enemy.hpp`, and add a `struct` for the start of an enemy, above the `Level` class:

```cpp source="tmpl8-part-18/7-enemies/level.hpp#L1-L19" {4,14-19}
```

An `EnemyStart` is the kind of an enemy and the position where it starts. The level only reads the file, so it remembers where the enemies start, and the play scene makes the enemies.

Further down, add a list of the starts of the enemies, and a function that gives it:

```cpp source="tmpl8-part-18/7-enemies/level.hpp#L38-L56" {5,16}
```

`EnemyStarts` returns a `const` reference to the list, so the scene can read it without making a copy, but it can't change it.

In `Load`, look for the enemies as well as the player:

```cpp source="tmpl8-part-18/7-enemies/level.cpp#L27-L46" {1-2,7,9-15}
```

`enemyStarts.push_back({ Enemy::Kind::Walker, position })` makes an `EnemyStart` from the values between the braces, in the order of its members, and adds it to the list. `continue` skips the rest of the loop for the other characters, so only the `P`, `E` and `W` tiles become a space.

At the bottom of the file, `Move` sets `hitWall`:

```cpp source="tmpl8-part-18/7-enemies/level.cpp#L170-L184" {4,14}
```

At the start of every move, `hitWall` becomes `false`. It only becomes `true` when the sideways step ran into a solid tile, and the body was moved back to its edge. A body that stands on the ground doesn't hit a wall, because the vertical step is the one that stops it there.

### Stomping

Jumping on an enemy makes the player bounce off it. In `player.hpp`, add `Bounce`:

```cpp source="tmpl8-part-18/7-enemies/player.hpp#L12-L20" {8}
```

It's `public`, because the play scene calls it when the player lands on an enemy.

In `player.cpp`, add it below `Tick`:

```cpp source="tmpl8-part-18/7-enemies/player.cpp#L75-L78"
```

The bounce is a jump at 70% of the speed of a normal jump, so the player flies up off the enemy.

In `play_scene.hpp`, include `enemy.hpp` and `<vector>`, and add the enemies and `TouchEnemies`:

```cpp source="tmpl8-part-18/7-enemies/play_scene.hpp#L1-L10" {5,10}
```

`<vector>` is the header of `std::vector`, which holds the enemies. Further down, in the `private` part of the class, declare `TouchEnemies` below `TouchTiles`, and add the vector of enemies below the player:

```cpp source="tmpl8-part-18/7-enemies/play_scene.hpp#L26-L41" {6,15}
```

The number of enemies is different in every level, and they disappear when they're squashed, so they're in a `std::vector`, which can grow and shrink.

In `play_scene.cpp`, `StartLevel` makes the enemies of the level:

```cpp source="tmpl8-part-18/7-enemies/play_scene.cpp#L15-L26" {6-8}
```

`emplace_back` makes a new `Enemy` at the end of the vector, with the values that you give it as the arguments of its constructor. `enemies.clear()` removes the enemies of the previous try first.

In `Tick`, move the enemies, and check whether the player touches one:

```cpp source="tmpl8-part-18/7-enemies/play_scene.cpp#L42-L50" {2-3,5}
```

The enemies move before the player is checked against them, so the player always touches the enemies where they are in this frame.

Below `TouchTiles`, add `TouchEnemies`:

```cpp source="tmpl8-part-18/7-enemies/play_scene.cpp#L80-L103"
```

When the player touches an enemy, what happens depends on how. If the player falls (a positive y velocity), and the bottom of the player is in the top half of the enemy, the player landed on it. The enemy is squashed, and the player bounces. Any other touch costs a life. Dying enemies are skipped, so a squashed enemy can't hurt you on its way down.

`std::erase_if` removes the enemies from the vector for which the lambda returns `true`, which are the ones that fell off the bottom of the screen. `[bottom]` *captures* `bottom`, so that the lambda can use it.

In `Draw`, draw the enemies after the level, and before the player:

```cpp source="tmpl8-part-18/7-enemies/play_scene.cpp#L133-L146" {5-6}
```

The enemies are drawn with the same `characters` sprite as the player. Because the player is drawn last, it's in front of an enemy that it jumps on.

And `DrawColliders` shows the enemies too:

```cpp source="tmpl8-part-18/7-enemies/play_scene.cpp#L173-L179" {1-5}
```

They're red, because they hurt. A dying enemy can't hurt you anymore, so it gets no box.

Press <kbd>F5</kbd>, and stomp on some enemies:

<iframe class="embed embed-tmpl8 embed-480x270" src="/demos/cpp-fast-track-18-enemies/index.html" title="Alien Jump with walking robots and flying bats; jumping on an enemy squashes it, and it falls off the screen upside down" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Jump on the enemies to squash them. Touching them any other way costs a life. (Click the demo first, and press <kbd>Enter</kbd> on the title screen.)

</div>

## Sound

The game plays, but it's silent. A jump, a coin and a squashed enemy should all make a sound, and some music makes a big difference. The game plays its sounds with the [Audio](https://github.com/3dgep/Audio) library, which [part 17](/cpp-fast-track-17-audio/) explains in detail. If you did part 17, you know these steps. Otherwise, here's the short version.

### Adding the Audio Library

Go to the [releases of the Audio library](https://github.com/3dgep/Audio/releases/tag/v2.1.1), and download `Audio-2.1.1-windows-x64-static.zip`. Extract it, make a new folder `Audio` in the `lib` folder of your project (next to the `SDL3` folder), and copy the `include` and `lib` folders of the zip file into it:

```txt
Part 18
├── lib
│   ├── Audio
│   │   ├── include
│   │   └── lib
│   │       ├── Debug
│   │       └── Release
│   ├── SDL3
│   └── stb
└── ...
```

In the **Solution Explorer**, right-click the `tmpl8` project, and select **Properties**. Set **Configuration** to **All Configurations** and **Platform** to **x64** at the top. Then:

1. In **C/C++** > **General**, add this line to **Additional Include Directories**:

   ```txt
   $(SolutionDir)lib\Audio\include
   ```

   <img src="/images/cpp-fast-track-17/02-include-directories.png" alt="The tmpl8 Property Pages at C/C++ > General, for All Configurations, with the Additional Include Directories window open. Below $(ProjectDir), $(SolutionDir) and the folders of SDL and stb, the new line $(SolutionDir)lib\Audio\include is selected." width="1198" height="817" class="screenshot" />

   <div class="caption">

   The Audio library's include folder, added to the Additional Include Directories. (This screenshot is from part 17, so it says `Part 17` where your folder is `Part 18`.)

   </div>

2. In **Linker** > **General**, add this line to **Additional Library Directories**:

   ```txt
   $(SolutionDir)lib\Audio\lib\$(Configuration)
   ```

   <img src="/images/cpp-fast-track-17/03-library-directories.png" alt="The tmpl8 Property Pages at Linker > General, for All Configurations, with the Additional Library Directories window open. Below $(SolutionDir)lib\SDL3\lib\x64, the new line $(SolutionDir)lib\Audio\lib\$(Configuration) is selected." width="1203" height="858" class="screenshot" />

   <div class="caption">

   The Audio library's library folder, added to the Additional Library Directories.

   </div>

3. In **Linker** > **Input**, add a line with `Audio.lib` to **Additional Dependencies** (below `SDL3.lib`).

   <img src="/images/cpp-fast-track-17/04-additional-dependencies.png" alt="The tmpl8 Property Pages at Linker > Input, for All Configurations, with the Additional Dependencies window open. Below SDL3.lib, the new line Audio.lib is selected." width="1220" height="817" class="screenshot" />

   <div class="caption">

   `Audio.lib`, added to the Additional Dependencies.

   </div>

Click **OK**. If the game doesn't build after the next steps, [part 17](/cpp-fast-track-17-audio/#a-first-sound) has a table of the errors that a wrong setting gives, and how to fix them.

### The Sounds Class

All the sounds of the game are in one class. Add a header file `sounds.hpp`, and start with a list of the sound effects:

```cpp source="tmpl8-part-18/8-sound/sounds.hpp#L1-L10"
```

`Effect` lists the sound effects. The game plays an effect by its name, like `Effect::Jump`, so it never has to know the file of the sound. `Count` isn't a sound. It's the number of effects, because the values of an `enum` count up from 0, and it comes after the last one. `Select` and `Confirm` are for the menus, later on.

Below `Effect`, add the `Sounds` class:

```cpp source="tmpl8-part-18/8-sound/sounds.hpp#L12-L35"
```

The sounds play through two *groups*. An `Audio::Group` is like a channel on a mixing desk. Its volume applies to every sound in it, so the game can make all the effects softer, or all the music, with one call. That's what the settings screen will need.

The effects are in a `std::array`, an array with a fixed size that knows its size. `static_cast<int>(Effect::Count)` turns the `enum class` value into a number, which an `enum class` doesn't do by itself.

Add `sounds.cpp`, with the constructor:

```cpp source="tmpl8-part-18/8-sound/sounds.cpp#L1-L18"
```

The constructor loads every effect from `assets/sounds`, in the order of the `Effect` values, into the effects group. The music goes into the music group, as `Type::Music`. Music is long, so it isn't loaded into memory all at once, but *streamed* (read from the file a bit at a time while it plays). Both songs loop.

Add `Play`:

```cpp source="tmpl8-part-18/8-sound/sounds.cpp#L20-L24"
```

`Play` plays a *one-shot* of an effect, a copy of the sound that plays until it ends. When you collect three coins in a row, each coin plays its own copy, and they don't cut each other off.

Add `PlayMusic` and `StopMusic`:

```cpp source="tmpl8-part-18/8-sound/sounds.cpp#L26-L40"
```

Only one song plays at a time. `currentMusic` points to the song that's playing, so `PlayMusic` can stop it before it starts another one, and doesn't start a song again that's already playing. `replay` starts a song at the beginning.

Finally, add the two functions that set the volume:

```cpp source="tmpl8-part-18/8-sound/sounds.cpp#L42-L51"
```

The volumes are in percent, from 0 to 100, and the library wants a number from 0 to 1, so they're divided by 100. That's `100.0f` and not `100`, because `70 / 100` is a division of two `int`s, which gives 0.

### Playing the Sounds

The game owns the sounds, like the text. In `game.hpp`, include `sounds.hpp`:

```cpp source="tmpl8-part-18/8-sound/game.hpp#L1-L11" {6}
```

The game has a `Sounds` member, so it needs the whole class, not only a forward declaration. Then add the sounds below the text, and `GetSounds` below `GetText`:

```cpp source="tmpl8-part-18/8-sound/game.hpp#L46-L57" {4,10}
```

The game loads all the sounds once, when it starts, and every scene plays them through `GetSounds`. That way, the music can keep playing when the game switches to another scene.

The title screen plays the menu music. That needs a constructor in `menu_scene.cpp`, so in `menu_scene.hpp`, change the constructor into a declaration:

```cpp source="tmpl8-part-18/8-sound/menu_scene.hpp#L11-L21" {4}
```

The constructor only has a declaration now, with a `;` instead of a body. Add the constructor to `menu_scene.cpp`, above `Tick`:

```cpp source="tmpl8-part-18/8-sound/menu_scene.cpp#L6-L11" {3-6}
```

It stores the game, like before, and starts the menu music.

In `play_scene.cpp`, the level music starts with every level:

```cpp source="tmpl8-part-18/8-sound/play_scene.cpp#L15-L27" {12}
```

Dying and the flag stop the music (you'll add that next), so the music starts at the beginning with every try and every level.

In `TouchTiles`, a coin plays a sound, and the flag stops the music and plays a fanfare:

```cpp source="tmpl8-part-18/8-sound/play_scene.cpp#L64-L79" {6,13-15}
```

The fanfare takes about 5 seconds, so the scene waits that long before it starts the next level.

In `TouchEnemies`, a squashed enemy makes a sound:

```cpp source="tmpl8-part-18/8-sound/play_scene.cpp#L93-L98" {5}
```

The sound plays together with the squash and the bounce, so you hear it at the moment you land on the enemy.

`Die` stops the music, and plays the sound of losing a life:

```cpp source="tmpl8-part-18/8-sound/play_scene.cpp#L110-L119" {8-9}
```

The music stops first, so the sound of losing a life isn't drowned out by it.

When the last life is gone, `AfterWait` plays a short jingle:

```cpp source="tmpl8-part-18/8-sound/play_scene.cpp#L121-L130" {7}
```

The jingle keeps playing while the game switches to the title screen. A one-shot plays until it ends, also when the scene that played it is gone.

Finally, the jump. In `player.cpp`, play the jump sound when the player jumps:

```cpp source="tmpl8-part-18/8-sound/player.cpp#L61-L66" {5}
```

The player already gets the game in `Tick`, for the keys, so it can reach the sounds too.

Press <kbd>F5</kbd>, and listen:

<iframe class="embed embed-tmpl8 embed-480x270" src="/demos/cpp-fast-track-18-sound/index.html" title="Alien Jump with sound effects for jumping, coins, stomping, dying and the flag, and music on the title screen and in the levels" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The game with sound effects and music. (Click the demo first, because a browser only plays sound after you click the page. Only the demo that you clicked last plays sound, so the demos on this page don't all play at once.)

</div>

:::note
The template can also build your game for the web with `build-web.bat`, but that script doesn't know about the Audio library. The web version needs a version of the library that is built for the web, and the [README of the library](https://github.com/3dgep/Audio#cmake-for-the-web-emscripten) explains how to make it. The demos in this article are built that way.
:::

## Menus

The title screen only says PRESS ENTER TO PLAY, and the game has no way to pause, or to go back to the title screen. In this section, the title screen becomes a main menu with buttons, and the game gets a pause menu, a level picker, and a screen for GAME OVER and YOU WIN!.

All these screens have the same kind of menu, a column of buttons. You choose a button with the arrow keys and <kbd>Enter</kbd>, or with the mouse. So that's a class of its own, which every scene can use.

### The Mouse

The template tells the game when the mouse moves, or when a button of the mouse goes down. In `game.hpp`, include the SDL header for the mouse:

```cpp source="tmpl8-part-18/9-menus/game.hpp#L1-L12" {10}
```

It has `SDL_BUTTON_LEFT`, the number of the left mouse button. Then fill in `MouseDown` and `MouseMove`:

```cpp source="tmpl8-part-18/9-menus/game.hpp#L23-L25" {2-3}
```

`MouseMove` remembers where the mouse is (in pixels on the screen, the template takes care of the scaling), and that it moved. `MouseDown` remembers a click of the left button. These are *events*. They can happen at any time between two frames, so the game keeps them until the next frame starts, in the same way as the keys. In `ProcessInput`, which runs before every frame, the events become the state of this frame:

```cpp source="tmpl8-part-18/9-menus/game.hpp#L37-L48" {9-11}
```

`clicked` and `moved` are only true in the frame after the event, just like a key in `pressed`. Then the events are cleared, ready for the next frame.

Add the functions that the scenes use, and the members for the mouse:

```cpp source="tmpl8-part-18/9-menus/game.hpp#L50-L70" {3-7,19-21}
```

The scenes read the mouse with `MouseX`, `MouseY`, `MouseClicked` and `MouseMoved`. `Quit` is for the QUIT button of the main menu.

In `game.cpp`, include `SDL_events.h`:

```cpp source="tmpl8-part-18/9-menus/game.cpp#L1-L6" {5}
```

That's the header of SDL's events, which `Quit` needs. Add `Quit` at the bottom of the file:

```cpp source="tmpl8-part-18/9-menus/game.cpp#L40-L45"
```

`SDL_PushEvent` puts an event in the queue of SDL, as if it came from the operating system. `SDL_EVENT_QUIT` is the event that SDL sends when you close the window, so the template ends the game in the same way.

### The Menu Class

Add a header file `menu.hpp`:

```cpp source="tmpl8-part-18/9-menus/menu.hpp#L1-L35"
```

A menu has a label for every button, and remembers which button is selected. `top` is the y of the first button, and the buttons are below each other, 28 pixels apart. `button.png` has two frames, a normal button and a selected one. `Tick` returns the number of the button that was chosen in this frame, or -1 when none was.

Add `menu.cpp`, with `Tick`:

```cpp source="tmpl8-part-18/9-menus/menu.cpp#L1-L32"
```

The down arrow selects the next button, and the up arrow the previous one. The `%` makes the selection wrap around, from the last button to the first one, and back. `selected + count - 1` instead of `selected - 1` keeps the number positive, because `%` of a negative number is negative in C++.

The mouse selects the button that it moves onto. It only does that when it moves, so the mouse doesn't take the selection away from the arrow keys when it just sits on a button. A change of selection plays a short sound.

<kbd>Enter</kbd> or <kbd>Space</kbd> chooses the selected button, and so does a click on it.

Add `Draw`:

```cpp source="tmpl8-part-18/9-menus/menu.cpp#L34-L43"
```

Every button is drawn in the middle of the screen, with its label on it. The selected button gets the other frame of the sprite.

And add `ItemAt`, which finds the button under the mouse:

```cpp source="tmpl8-part-18/9-menus/menu.cpp#L45-L53"
```

The mouse isn't on a button when it's to the left or the right of the buttons, or above the first one. Otherwise, `(y - top) / Spacing` is the number of the button, but the mouse can also be in the space between two buttons, or below the last one.

### The Main Menu

The title screen becomes the main menu. Change `menu_scene.hpp`:

```cpp source="tmpl8-part-18/9-menus/menu_scene.hpp#L1-L28" {3-4,11,20,23,25-26}
```

`Item` gives the buttons a name, in their order in the menu. This is a plain `enum`, not an `enum class`, because its values convert to an `int` by themselves. That's handy here, because `Menu::Tick` returns the number of the button as an `int`.

The main menu also shows a background, which slowly scrolls by. `scroll` is how far it has scrolled, and the `120` of the menu is the y of its first button, below the title.

In `menu_scene.cpp`, include `level_select_scene.hpp` (you'll write it next), and add the buttons in the constructor:

```cpp source="tmpl8-part-18/9-menus/menu_scene.cpp#L1-L17" {4,11-15}
```

A web page can't close itself, so the web version of the game has no QUIT button. `__EMSCRIPTEN__` is defined when the game is built for the web, and `#ifndef` (*if not defined*) leaves the line out then.

`Tick` scrolls the background, and reacts to the buttons:

```cpp source="tmpl8-part-18/9-menus/menu_scene.cpp#L19-L28" {3-9}
```

`scroll` goes up by 20 pixels per second, so the background slowly scrolls by, as if the camera moves through a level. The `switch` compares the number of the chosen button with the `Item` values. When no button was chosen, `menu.Tick` returns -1, which matches no `case`, and nothing happens.

`Draw` draws the background, the title and the menu, with a reminder of the keys at the bottom:

```cpp source="tmpl8-part-18/9-menus/menu_scene.cpp#L30-L37" {3,5-6}
```

The background gets `scroll` as the x of the camera, and a height of 0, so its strip stays at the bottom of the screen.

### The Level Picker

The LEVELS button opens a scene with a button for every level. Add `level_select_scene.hpp`:

```cpp source="tmpl8-part-18/9-menus/level_select_scene.hpp#L1-L25"
```

It has the same background as the main menu, but it doesn't scroll here. `panel.png` is a frame with a dark border, behind the menu.

Add `level_select_scene.cpp`, with the constructor:

```cpp source="tmpl8-part-18/9-menus/level_select_scene.cpp#L1-L17"
```

The buttons are made from `PlayScene::LevelCount`, so a fourth level gets a button by itself. The last button goes back to the main menu.

Add `Tick`:

```cpp source="tmpl8-part-18/9-menus/level_select_scene.cpp#L19-L28"
```

Button 0 is level 1, so the chosen level is `chosen + 1`. The only button after the levels is BACK.

Add `Draw`:

```cpp source="tmpl8-part-18/9-menus/level_select_scene.cpp#L30-L37"
```

The panel is drawn in the middle of the screen, with LEVELS at the top, and the menu on top of it.

### Game Over and You Win

At the end of the game, a scene shows a message, with buttons to play again or to go back to the main menu. Add `message_scene.hpp`:

```cpp source="tmpl8-part-18/9-menus/message_scene.hpp#L1-L31"
```

The scene gets the title and the message from whoever makes it, like GAME OVER and the number of coins. It keeps its own copies of them, in two `std::string`s.

Add `message_scene.cpp`, with the constructor:

```cpp source="tmpl8-part-18/9-menus/message_scene.cpp#L1-L15"
```

The constructor stores the title and the message in the members with the same names. In an initializer list, `title(title)` gives the member `title` the value of the parameter `title`, so they can share their name. This scene has no music, only the jingle at the end of the game, so it makes sure that no music is playing. PLAY AGAIN is the first button, so it's selected, and <kbd>Enter</kbd> plays again right away.

Add `Tick`:

```cpp source="tmpl8-part-18/9-menus/message_scene.cpp#L17-L24"
```

PLAY AGAIN starts a new game at level 1, and MAIN MENU goes back to the main menu.

Add `Draw`:

```cpp source="tmpl8-part-18/9-menus/message_scene.cpp#L26-L35"
```

The title and the message are centred on the panel, above the buttons. `Text& text` is a reference to the text of the game, so the three lines below it don't have to call `GetText` every time.

### Pause, and Starting at a Level

The play scene needs a few changes. It can start at any level (for the level picker), it can be paused, and the end of the game shows the message scene. In `play_scene.hpp`, include `menu.hpp`, and give the constructor the first level. The default value `= 1` means that `PlayScene(game)` still starts at level 1:

```cpp source="tmpl8-part-18/9-menus/play_scene.hpp#L1-L29" {7,23,29}
```

`PauseItem` names the two buttons of the pause menu, like `Item` in the main menu.

Further down, add the pause menu, with a panel behind it:

```cpp source="tmpl8-part-18/9-menus/play_scene.hpp#L44-L50" {4-6}
```

`paused` says whether the game is paused, and the pause menu has its first button at a y of 80.

In `play_scene.cpp`, include `message_scene.hpp`. The constructor starts at `firstLevel`, and fills the pause menu:

```cpp source="tmpl8-part-18/9-menus/play_scene.cpp#L1-L16" {5,11-14}
```

`levelNumber(firstLevel)` sets the first level in the initializer list, so it's there before `StartLevel` loads the level.

In `Tick`, <kbd>P</kbd> pauses the game (<kbd>Escape</kbd> would be more usual, but the template closes the game with <kbd>Escape</kbd>). While the game is paused, only the pause menu works, and the rest of `Tick` is skipped, so nothing moves:

```cpp source="tmpl8-part-18/9-menus/play_scene.cpp#L32-L47" {5-16}
```

The game can only be paused while the player plays, not while it waits after dying or after the flag. <kbd>P</kbd> also continues the game, just like RESUME. The `return` skips the rest of `Tick`, so the level, the player and the enemies stand still.

In `AfterWait`, the end of the game shows the message scene, with the coins as the message:

```cpp source="tmpl8-part-18/9-menus/play_scene.cpp#L136-L154" {8,15}
```

At GAME OVER, the message shows the coins that the player had when the last level started, because the coins of the level are lost with the last life.

At the end of `Draw`, the pause menu is drawn on top of everything:

```cpp source="tmpl8-part-18/9-menus/play_scene.cpp#L156-L175" {14-19}
```

The game is still drawn behind the pause menu, so you can see where you were. The panel and the menu go on top of it, after the HUD.

Press <kbd>F5</kbd>. Try the buttons with the keys and with the mouse, pick a level, and pause the game with <kbd>P</kbd>:

<iframe class="embed embed-tmpl8 embed-480x270" src="/demos/cpp-fast-track-18-menus/index.html" title="Alien Jump with a main menu, a level picker, a pause menu and GAME OVER and YOU WIN screens" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The main menu, the level picker, and the pause menu (<kbd>P</kbd>). (Click the demo first, so that it gets the keys and can play sound.)

</div>

## Settings

The last screen is the settings screen. Not every player wants the music as loud as you do, so the player can set the volume of the music and of the sound effects, and choose between a window and fullscreen. The game saves the settings in a file, so it remembers them the next time it starts.

### Saving the Settings

Add a header file `settings.hpp`:

```cpp source="tmpl8-part-18/10-settings/settings.hpp#L1-L15"
```

`Settings` is a `struct`, like `Body`, because it's a group of values that other code reads and changes. The values after the `=` are the defaults, for the first time the game runs.

Add `settings.cpp`, with `Load`:

```cpp source="tmpl8-part-18/10-settings/settings.cpp#L1-L19"
```

`Load` reads three values from `settings.txt`. `>>` reads the next value from the file into a variable, and `file >> music >> effects >> full` reads three of them in a row. A read that fails (because the file isn't there, or doesn't contain three numbers) makes the stream count as `false`, so the `if` only uses the values when all three were read. Otherwise, the defaults stay.

Below `Load`, add `Save`:

```cpp source="tmpl8-part-18/10-settings/settings.cpp#L21-L26"
```

`Save` writes the three values to `settings.txt`, with a space between them, like `70 100 0` (a `bool` is written as `0` or `1`). That's exactly what `Load` reads back. Opening a `std::ofstream` replaces the old file, so it only ever has the latest settings.

The file is saved in the *working directory* of the game, the folder that the game runs in. When you start the game from Visual Studio, that's the `Part 18` folder.

The game owns the settings. In `game.hpp`, include `settings.hpp`:

```cpp source="tmpl8-part-18/10-settings/game.hpp#L1-L8" {6}
```

Like the sounds, the settings are a member of the game, so `game.hpp` needs the whole `struct`. Then add the settings above the sounds, and `GetSettings` above `GetSounds`:

```cpp source="tmpl8-part-18/10-settings/game.hpp#L51-L67" {9,16}
```

`GetSettings` returns a reference, so the settings screen changes the settings of the game itself, not a copy of them.

In `game.cpp`, `Init` loads the settings, and uses them before the first scene starts:

```cpp source="tmpl8-part-18/10-settings/game.cpp#L13-L21" {3-7}
```

`SetFullscreen` is a function of the template. It does the same as <kbd>F11</kbd>.

### The Settings Screen

Add `settings_scene.hpp`:

```cpp source="tmpl8-part-18/10-settings/settings_scene.hpp#L1-L28"
```

It looks a lot like the level picker, with a background, a panel and a menu. `Item` names the four buttons, and `UpdateLabels` writes the current values on them, like MUSIC 70%.

Add `settings_scene.cpp`, starting with a helper function for the volumes:

```cpp source="tmpl8-part-18/10-settings/settings_scene.cpp#L1-L17"
```

A volume goes from 0% to 100% in steps of 10%, which are 11 values. `NextVolume` adds or subtracts 10, and `% 110` makes it wrap around, so after 100% comes 0%. The `+ 110` keeps the number positive when you go down from 0%, which then becomes 100%.

The constructor adds four buttons, and `UpdateLabels` gives them their text:

```cpp source="tmpl8-part-18/10-settings/settings_scene.cpp#L19-L24"
```

The labels start empty, because `UpdateLabels` fills them in right away, and again after every change.

`Tick` changes the setting of the selected button:

```cpp source="tmpl8-part-18/10-settings/settings_scene.cpp#L26-L63"
```

The left and right arrows make a value lower or higher. <kbd>Enter</kbd> or a click also makes it higher, so the mouse can change every setting too. The up and down arrows still go through the buttons, because `menu.Tick` takes care of them.

A new volume is passed on to the sounds right away, so you hear the difference. A change of the effects volume plays the sound of a coin, so you hear how loud the effects are now. BACK saves the settings, and goes back to the main menu. It only reacts to <kbd>Enter</kbd> or a click, not to the arrows.

Add `UpdateLabels`:

```cpp source="tmpl8-part-18/10-settings/settings_scene.cpp#L65-L72"
```

`{:>3}` in a `std::format` pattern makes the number 3 characters wide, aligned to the right, so the % sign stays in the same place for 0%, 50% and 100%.

Add `Draw`:

```cpp source="tmpl8-part-18/10-settings/settings_scene.cpp#L74-L80"
```

It's the same as the `Draw` of the level picker, with SETTINGS at the top.

### The Settings Button

Finally, the main menu gets a SETTINGS button. In `menu_scene.hpp`, add it to `Item`, between LEVELS and QUIT:

```cpp source="tmpl8-part-18/10-settings/menu_scene.hpp#L11-L20" {1,10}
```

`QuitButton` now has the number 3, so it still matches the fourth button.

In `menu_scene.cpp`, include `settings_scene.hpp`, and add the button in the same place:

```cpp source="tmpl8-part-18/10-settings/menu_scene.cpp#L1-L19" {6,14}
```

The order of the `Add`s has to match the order of `Item`, because `menu.Tick` returns the number of the button.

Open the settings when the button is chosen:

```cpp source="tmpl8-part-18/10-settings/menu_scene.cpp#L21-L31" {8}
```

That's one more `case` in the `switch`.

Press <kbd>F5</kbd>, open the settings, and change the volumes. Close the game, start it again, and the settings are still the same.

<iframe class="embed embed-tmpl8 embed-480x270" src="/demos/cpp-fast-track-18-settings/index.html" title="Alien Jump, complete, with a settings screen for the volume of the music and the sound effects, and fullscreen" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The complete game, with the settings screen. (Click the demo first, so that it gets the keys and can play sound.) The web version saves the settings in the browser's memory, so it forgets them when you reload the page.

</div>

That's Alien Jump, a complete platformer with scenes, tile levels, a camera, physics, enemies, sound, menus and settings. All of it is in small classes, each in its own files, with only a few lines in `game.cpp`.

## Assignment

### Basic

1. A player who loses a life on the last level has to be careful with the ones that are left. Add a heart that the player can collect for an extra life. Use `H` for it in the level files, and frame 44 of `tiles.png` (the heart of the HUD). A heart gives a life back, but the player can have 3 lives at most. When the player already has 3 lives, the heart stays where it is. Put a heart in each level.

   :::tip[Hint]
   A heart is a lot like a coin. Look at everything that the game does for a coin (in `Level::Frame`, `Level::HitBox`, `PlayScene::TouchTiles` and `PlayScene::DrawColliders`), and do the same for the heart. Think about what should happen to a heart that the player collects, and then loses the level.
   :::

<!-- TODO: the demo of the basic assignment -->

### Intermediate

1. Add a third kind of enemy, a robot with a spike on its head (frames 15 and 16 of `characters.png`). It walks like the walker, but it can't be stomped. Jumping on it costs a life, just like touching it from the side, so the player has to jump over it. Use `S` for it in the level files, and add a few of them to the levels.

   :::tip[Hint]
   Add a value to `Enemy::Kind`. The spiked robot moves in the same way as a walker, so it can use `Walk`. `PlayScene::TouchEnemies` has to know whether an enemy can be stomped, so give `Enemy` a function that tells it.
   :::

<!-- TODO: the demo of the intermediate assignment -->

### Hard

1. Add moving platforms. A platform is a few tiles wide, and moves back and forth between two points, sideways or up and down. The player can stand on it, and moves along with it.

   :::tip[Hint]
   A platform can't be a tile, because tiles don't move. Make it a class with a `Body`, like an enemy. The player only collides with the top of a platform, so that the player can jump through it from below, which is easier to make and feels better. The player lands on a platform when it falls, and its bottom was above the top of the platform in the previous frame. While the player stands on a platform, move the player by the same distance as the platform in every frame.
   :::

<!-- TODO: the demo of the hard assignment -->

**Congratulations on getting here!** You built a complete game from a fresh template, with everything that a game needs, from the title screen to the last level. Use what you learned to make the game your own. Design new levels, add new enemies, or make a completely different game. Keep building games, because that's the best way to keep learning.
