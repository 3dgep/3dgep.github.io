---
title: "C++ Fast Track for Games Programming Part 17: Audio"
description: "Audio: add a sound library to your project, learn what sound is, and give the tank battle sound effects, 3D sound, engines that hum, music, and jingles."
slug: cpp-fast-track-17-audio
lastUpdated: 2026-11-21
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

In [part 16](/cpp-fast-track-16-ai/), you built a tank battle against waves of enemies. But it's a silent battle, in which the shells fly and the tanks explode without a sound. In this part, we add the sound. You will:

1. add a library to your project, so that the game can play sounds,
2. learn what sound is, and how a computer stores it,
3. give every shot, hit and explosion a sound, which comes from the place where it happens,
4. let the engines of the tanks hum, higher when they drive faster,
5. and play music, and a few jingles that you make from nothing but numbers.

Sound is easy to forget, but it makes a big difference. Turn the sound off in your favourite game, and see how much less fun it is. Sound also tells the player things that they can't see, like an enemy that fires from behind a tree.

## Getting Started

For this tutorial, you will start where [part 16](/cpp-fast-track-16-ai/) ended: the arena battle. Download it here:

[<img src="/images/zip1.png" alt="Download the starting point for part 17 as a zip file" width="120" class="thumb" />part-17.zip](/downloads/part-17.zip)

Extract the zip file to `C:\Projects\C++ Fast Track`: it contains a `Part 17` folder. Open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 17` folder.

The code is the code from the end of part 16. The `assets` folder has a new `sounds` folder with the sounds of this part, as OGG files. The cannon and the explosion of a tank are made from [Cannon Fire](https://opengameart.org/content/cannon-fire) and [Battle at Sea](https://opengameart.org/content/battle-at-sea) by Thimras, and the explosion of a shell from the [Tiny Naval Battle Sounds Set](https://opengameart.org/content/tiny-naval-battle-sounds-set) by Iwan Gabovitch. The hits and the engine come from two sound packs by Kenney, [Impact Sounds](https://kenney.nl/assets/impact-sounds) and [Sci-fi Sounds](https://kenney.nl/assets/sci-fi-sounds). The music is [Fast fight / battle music](https://opengameart.org/content/fast-fight-battle-music-looped) by Ville Nousiainen. They all share their work for free, under the [CC0](https://creativecommons.org/publicdomain/zero/1.0/) license. `CREDITS.md` in the `sounds` folder lists which sound came from where.

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-16-arena/index.html" title="The arena battle of part 16, without sound" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The starting point: the arena battle of part 16, without a sound. (Click the demo first, so that it gets the keys.)

</div>

## Adding a Library

C++ itself can't play sounds. The template uses [SDL](https://www.libsdl.org/) for the window, the keyboard and the mouse, and SDL can play sound too. But SDL leaves a lot of work to you. It plays a stream of numbers, and loading a file, mixing several sounds together, and placing a sound to the left or the right of the player are up to you. Instead, we use a *library* that does that work for us. It's called [Audio](https://github.com/3dgep/Audio), a small audio library for games, made for this course.

A library is code that someone else wrote, ready for you to use. It comes in two parts:

- The *header files* (`.hpp` or `.h`) tell the compiler which classes and functions the library has. You `#include` them, like `game.hpp`.
- The *library file* (`.lib`) contains the compiled code. The *linker* adds it to your program, after the compiler is done.

So your project needs to know where both are. That's what this section is about. You only need to do this once for every project that uses the library.

### Downloading the Library

Go to the [releases of the Audio library](https://github.com/3dgep/Audio/releases/tag/v2.0.0), and download `Audio-2.0.0-windows-x64-static.zip`. *Static* means that the code of the library becomes part of your `.exe`, so your game doesn't need an extra file (a DLL) to run.

Extract the zip file. It contains an `include` folder (the header files) and a `lib` folder (the library files). Make a new folder `Audio` in the `lib` folder of your project (next to the `SDL3` folder, which holds the SDL library), and copy the `include` and `lib` folders into it. (You don't need the `cmake` folder in `lib`, which is for another way to build.) Your project now looks like this:

```txt
Part 17
├── assets
├── lib
│   ├── Audio
│   │   ├── include
│   │   │   └── Audio
│   │   │       ├── Audio.hpp
│   │   │       └── ...
│   │   └── lib
│   │       ├── Debug
│   │       │   └── Audio.lib
│   │       └── Release
│   │           └── Audio.lib
│   ├── SDL3
│   └── stb
├── tmpl8
├── game.cpp
├── game.hpp
└── tmpl8.slnx
```

There are two versions of `Audio.lib`, one for the **Debug** build of your game, and one for the **Release** build.

### Telling Visual Studio Where It Is

In the **Solution Explorer**, right-click the `tmpl8` project, and select **Properties**. At the top of the window, set **Configuration** to **All Configurations**, so that your changes are made for both the Debug and the Release build. **Platform** is **x64**.

1. Go to **C/C++** > **General**. Click **Additional Include Directories**, and then the arrow at the end of the line, and select **Edit...**. The list already has the folders of SDL (`$(TemplateRoot)lib\SDL3\include`) and stb. Add a new line:

   ```txt
   $(TemplateRoot)lib\Audio\include
   ```

   `$(TemplateRoot)` is the folder of your project (the `Part 17` folder). The template's project file defines it, so the paths keep working when you move your project to another folder.

2. Go to **Linker** > **General**, and add this to **Additional Library Directories** in the same way:

   ```txt
   $(TemplateRoot)lib\Audio\lib\$(Configuration)
   ```

   `$(Configuration)` is `Debug` or `Release`, depending on the build you are making. So each build finds its own version of `Audio.lib`.

3. Go to **Linker** > **Input**, and add a line with `Audio.lib` to **Additional Dependencies** (below `SDL3.lib`).

Click **OK** to save the settings.

### A First Sound

Time to test it. Open `game.cpp`, and include the header of the library at the top:

```cpp source="tmpl8-part-17/1-library/game.cpp#L1-L5" {3}
```

The angle brackets (`<` and `>`) tell the compiler to look for the file in the include directories, which are the folders in the project settings. That's where it finds `Audio/Audio.hpp` now.

Now make some noise. In `Tank::Fire`, play a sound when the tank fires:

```cpp source="tmpl8-part-17/1-library/game.cpp#L614-L627" {11}
```

`Audio::playSound` loads a sound file and plays it. The `Audio::` in front means that the function is part of the *namespace* `Audio`. Just like the template puts its code in the namespace `Tmpl8`, the library puts all of its code in `Audio`, so that its names never clash with yours. A game can have its own `playSound` function, which is a different function than `Audio::playSound`.

Press <kbd>F5</kbd>, and fire. Do you hear the cannon? Then the library works. The enemies fire with the same `Fire`, so you hear them too.

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-17-library/index.html" title="The arena battle, with a sound for every shot" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Every shot makes a sound. (Click the demo first, because a browser only plays sound after you click the page. Only the demo that you clicked last plays sound, so the demos on this page don't all play at once.)

</div>

If it doesn't build, check the settings again:

| Error | Fix |
| --- | --- |
| `cannot open source file "Audio/Audio.hpp"` or `C1083: Cannot open include file: 'Audio/Audio.hpp'` | The include directory isn't right (step 1). Is the `include` folder in `lib\Audio`? Did you change the settings for **All Configurations**? |
| `LNK1104: cannot open file 'Audio.lib'` | The library directory isn't right (step 2). Are the `Debug` and `Release` folders in `lib\Audio\lib`? |
| `LNK2019: unresolved external symbol audio_...` | `Audio.lib` is missing in **Additional Dependencies** (step 3). |

If the game runs, but stays silent, look at the console window. The library prints a message there when it can't find a sound file.

:::note
The template can also build your game for the web with `build-web.bat`, but that script doesn't know about the Audio library. The web version needs a version of the library that is built for the web, and the [README of the library](https://github.com/3dgep/Audio#cmake-for-the-web-emscripten) explains how to make it. The demos in this article are built that way.
:::

## What Is Sound?

Before we add more sounds, let's look at what a sound actually is.

When a shell explodes, it pushes the air around it away, very fast. That air pushes the air next to it, and so on. A wave of air pressure spreads in all directions, at about 343 meters per second. When the wave reaches your ear, it pushes your eardrum in and out, and you hear a bang. A speaker does the same thing in reverse. An electric signal moves a cone back and forth, and the cone pushes the air.

So a sound is a *wave*, in which the air pressure goes up and down over time. Two things describe a simple wave:

- The *frequency* is how many times per second the wave goes up and down, in *hertz* (Hz). The time of one up and down is the *period*. A higher frequency sounds higher. A low drum is around 60 Hz, the A that orchestras tune to is 440 Hz, and a whistle can be over 2000 Hz. People hear frequencies from about 20 Hz to 20,000 Hz (less when they get older).
- The *amplitude* is how far the wave goes up and down. A bigger amplitude sounds louder.

Real sounds, like an explosion or a voice, are mixes of many frequencies at once, which change all the time. But a single, simple wave is a sound too, which is called a *tone*. The demo below shows four simple waves, the *sine*, the *square*, the *triangle* and the *sawtooth*. Old game consoles used them, because they are easy to make with electronics. They have the same frequency, but they don't sound the same. Press **Play** to hear them.

<iframe class="embed embed-demo embed-sound-wave" src="/demos/sound-wave.html" title="A sound wave over 10 milliseconds, with buttons for its shape (sine, square, triangle or sawtooth) and sliders for its frequency, its amplitude and the sample rate. The dots on the wave are the samples. Play makes the wave audible." loading="lazy" allowfullscreen></iframe>

### Samples

A computer can't store a smooth wave. Instead, it measures the wave many times per second, and stores each measurement as a number, which is called a *sample*. That's the dots in the demo. The number of samples per second is the *sample rate*. Most sounds use 44,100 or 48,000 samples per second. Why so many? To follow a wave, you need at least two samples in every period, one when it is up and one when it is down. So to store frequencies up to 20,000 Hz (the highest that people can hear), you need more than 40,000 samples per second. Set the sample rate in the demo to 2000 Hz, and turn up the frequency. When there are fewer than two samples in a period, the dots can't follow the wave anymore.

A sample is a number between -1 and 1 (often stored as a 16-bit integer, between -32,768 and 32,767). A sound file is a long list of these numbers. A *stereo* sound has two lists, one for the left speaker and one for the right speaker. A *mono* sound has only one. All of this adds up. One second of stereo sound at 44,100 samples per second, with 2 bytes per sample, is 176,400 bytes. The music of this part takes 25 seconds, so it would need 4.4 MB.

That's why the sounds of this part are *OGG* files. Like a JPG for images, OGG (and MP3) leave out the details that you hardly notice, and pack the rest very tightly, so the music is only 449 KB as an OGG file. The other big sound format, *WAV*, stores all the samples, like a BMP for images. The Audio library can play all of these.

## Sound Effects

`Audio::playSound` is easy to use, but it doesn't give you any control. You can't change the volume of the sound, or stop it. For that, the library has the `Audio::Sound` class. You load a sound file into an `Audio::Sound` once, and then you can play it as often as you like:

```cpp
Audio::Sound coin("assets/coin.wav");

coin.play();   // play the sound (when it is already playing, it just keeps playing)
coin.replay(); // play the sound from the start
coin.stop();   // stop the sound (play continues where it stopped)
coin.setVolume(0.5f); // 0 is silent, 1 is the volume of the file
coin.setPitch(1.5f);  // 1 is the pitch of the file, higher is higher (and faster)
```

There is one catch. A sound can only play once at the same time. Two tanks that fire at the same moment need two sounds. And copying an `Audio::Sound` doesn't help, because a copy is the same sound, not a new one (just like a copy of a pointer points to the same thing). To play a sound several times at once, you load it several times. The library loads the file only once, and lets the sounds share it, so that's cheap.

### Voices

We put this in a struct of its own, an *effect*, with a few copies of a sound that take turns. These copies are called *voices*. Add this to `game.cpp`, below the sprites:

```cpp source="tmpl8-part-17/2-effects/game.cpp#L22-L31"
```

`static constexpr` makes `Voices` a constant that belongs to the struct, so every effect has 6 voices. `next` is the voice that plays the next time. With 6 voices, an effect can play 6 times at once. That's plenty. A seventh shot would restart the voice of the first one, which is almost over anyway.

`Load` loads the sound into every voice:

```cpp source="tmpl8-part-17/2-effects/game.cpp#L34-L45"
```

Hearing exactly the same explosion over and over again soon gets boring. So an effect can have *variations*, which are a few different files of the same kind of sound, like `explosion1.ogg`, `explosion2.ogg` and `explosion3.ogg`. With 3 variations, the voices get the files 1, 2, 3, 1, 2, 3, because `i % variations` goes 0, 1, 2, 0, 1, 2, and the `+ 1` makes that 1, 2, 3. Without variations, the file name has no number. `std::format` builds the file name, just like it builds the status line at the top of the screen. It puts its arguments into the `{}` of the text, one after the other: `name` and the number, or only `name`. (Add `#include <string>` at the top of `game.cpp`, for `std::string`.) The volume is the same for every voice.

`Play` plays the next voice, and moves `next` on to the voice after it:

```cpp source="tmpl8-part-17/2-effects/game.cpp#L47-L54"
```

The `%` makes `next` go back to 0 after the last voice. `replay` starts the voice from the beginning, even when it is still playing. The *pitch* changes a little every time. `Rand(0.2f)` is a random number between 0 and 0.2, so the pitch is between 0.9 and 1.1. A higher pitch plays the sound faster, so it sounds higher and shorter, like a record that spins too fast. A small random change makes every shot sound a little different, without that you notice why.

Below `Effect`, add the effects of the game:

```cpp source="tmpl8-part-17/2-effects/game.cpp#L56-L56"
```

They are still empty, and an `Audio::Sound` that isn't loaded is silent. Load them at the end of `Init`, just before `Restart()`:

```cpp source="tmpl8-part-17/2-effects/game.cpp#L795-L800" {1-5}
```

The shots, the explosions and the hits have 3 variations each. The cannon is a bit softer, and so are the explosions, of which there are many. The volumes are a matter of taste, so try what you like.

### Playing the Effects

Now play them. In `Tank::Fire`, replace `Audio::playSound` with the fire effect:

```cpp source="tmpl8-part-17/2-effects/game.cpp#L653-L666" {11}
```

In `UpdateShells`, a shell that hits a tank makes a clang of metal, and every hit explodes:

```cpp source="tmpl8-part-17/2-effects/game.cpp#L556-L565" {8}
```

```cpp source="tmpl8-part-17/2-effects/game.cpp#L583-L589" {5}
```

And a tank that is destroyed explodes with a deep boom. That happens in `Tick`:

```cpp source="tmpl8-part-17/2-effects/game.cpp#L857-L864" {5}
```

```cpp source="tmpl8-part-17/2-effects/game.cpp#L865-L877" {9}
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-17-effects/index.html" title="The arena battle with sound effects: shots, explosions, hits and destroyed tanks" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Shots, explosions, hits and destroyed tanks. (Click the demo first, so that it gets the keys and can play sound.)

</div>

## Sound in the World

Every sound now plays at the same volume, in the middle. But an enemy that fires on the right side of the screen should sound like it's on the right, and an explosion far away should be softer than one next to you. That's called *spatial audio* (or *3D audio*). The library places every sound somewhere in the world, and calculates what a *listener* (the ears of the game) hears from there.

The world of the library is 3D, with an x, a y and a z axis, and `Audio::Vector` is a 3D vector. Its y axis points up. Our map is flat, so it lies on the ground of that 3D world. The x of the screen is the x of the audio world, and the y of the screen is its z. By default, the listener looks along the negative z axis, so "forward" is up on the screen, and "right" is the right of the screen.

Where is the listener? In a game where you see your character from behind, the listener is the camera. But we look down on the map, from above. So we put the listener above the player's tank, as if the player sits in a helicopter that follows the tank. That height turns out to be important, as you will see in the demo below. Add this above `Effect`:

```cpp source="tmpl8-part-17/3-world/game.cpp#L22-L30"
```

`ToAudio` turns a position on the map into a position in the audio world. `position.y` becomes the z, and the height becomes the y.

### Distance

A sound gets softer when it is further away. How much softer depends on the *attenuation model* of the sound. The library uses the *inverse* model by default, in which the volume of a sound is:

$$
\text{volume} = \frac{\text{minDistance}}{\text{minDistance} + \text{rolloff} \cdot (\text{distance} - \text{minDistance})}
$$

Closer than `minDistance`, the sound plays at its full volume. At twice that distance, it plays at half the volume, at three times the distance at a third, and so on (with a `rolloff` of 1, the default). That's roughly how sound behaves in the real world. The default `minDistance` is 1, which works when your world is in meters. But our world is in pixels, and at 1 pixel, the volume would already drop at the edge of the tank. So every voice gets a `minDistance` of `ListenerHeight`. A sound right below the listener plays at full volume, and the volume drops from there. Add one line to `Load`:

```cpp source="tmpl8-part-17/3-world/game.cpp#L44-L56" {11}
```

### Left and Right

The listener has two ears (or two speakers), one on the left and one on the right. Each speaker plays a part of the sound, and a bigger part when the sound is on its side. A sound straight ahead is played by both speakers equally. A sound on the far right is mostly played by the right speaker, but a bit of it still comes out of the left speaker, just like your left ear still hears a sound on your right.

Drag the listener and the sound around in this demo, to see how loud each speaker plays the sound. The meters on the right show the volume of the sound (from the distance), and the parts that the left and the right speaker play of it. Then change `ListenerHeight`. When the listener is low (close to the map), the sound jumps from one speaker to the other when it passes the listener. That's what the height is for. From high above, every sound on the map is more or less in front of the listener, and only drifts to the left or the right.

<iframe class="embed embed-demo embed-sound-position" src="/demos/sound-position.html" title="The map seen from above, with the listener and a sound that can be dragged around. Meters show the volume of the sound and the parts of it that the left and the right speaker play. Sliders set the height of the listener and the minimum distance; Play repeats a bang with these volumes." loading="lazy" allowfullscreen></iframe>

The dashed circle around the listener shows where the distance (in 3D) is less than `minDistance`. Inside it, the sound plays at its full volume. When `ListenerHeight` is as big as `minDistance`, the circle is gone, because every sound on the map is at least `ListenerHeight` away.

### Placing the Sounds

`Play` gets a position now. It puts the voice there before it plays:

```cpp source="tmpl8-part-17/3-world/game.cpp#L58-L66" {1-2,6}
```

Also change the declaration of `Play` in `Effect`:

```cpp source="tmpl8-part-17/3-world/game.cpp#L34-L42" {8}
```

Then give every effect the position where it happens. A tank fires from its own position:

```cpp source="tmpl8-part-17/3-world/game.cpp#L665-L678" {11}
```

A shell hits and explodes where the shell is:

```cpp source="tmpl8-part-17/3-world/game.cpp#L568-L577" {8}
```

```cpp source="tmpl8-part-17/3-world/game.cpp#L595-L601" {5}
```

And a destroyed tank explodes where the tank is:

```cpp source="tmpl8-part-17/3-world/game.cpp#L872-L879" {5}
```

```cpp source="tmpl8-part-17/3-world/game.cpp#L880-L892" {9}
```

Finally, the listener must follow the player. `Audio::getListener()` gives you the listener, and `setPosition` moves it. Do that in `Tick`, right after `Simulate`, when the player has moved:

```cpp source="tmpl8-part-17/3-world/game.cpp#L862-L865" {2-4}
```

When the game is over, the player is hidden far outside the map. The listener stays where it was, because far away from the map, you wouldn't hear the battle anymore.

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-17-world/index.html" title="The arena battle with spatial audio: sounds come from where they happen" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Every sound comes from where it happens (use headphones to hear it best). (Click the demo first, so that it gets the keys and can play sound.)

</div>

## Engines

A tank that drives makes noise. Each tank gets an engine sound that *loops*, which means that it starts at the beginning again when it gets to the end, for as long as the tank is in the battle. A sound must be made for that. When the end of the sound doesn't match its start, you hear a click (or a jump) every time it starts over. `engine.ogg` is a steady hum, and its end fades into its start, so you can't hear where it begins.

The engine should sound faster when the tank drives faster. Below `TankDrag`, add the top speed of a tank:

```cpp source="tmpl8-part-17/4-engines/game.cpp#L321-L322" {2}
```

Why is that the top speed? Every millisecond, the drive adds `Acceleration` to the velocity, and the drag takes `TankDrag` times the velocity away. The faster the tank goes, the more the drag takes away, until it takes away as much as the drive adds. Then the speed doesn't change anymore. That happens when `velocity * TankDrag` equals `Acceleration`, so at a speed of `Acceleration / TankDrag`, which is 0.15 pixels per millisecond.

Every tank gets its own engine sound, and two methods to use it:

```cpp source="tmpl8-part-17/4-engines/game.cpp#L341-L372" {18-19,26}
```

`LoadEngine` loads the sound, and makes it loop:

```cpp source="tmpl8-part-17/4-engines/game.cpp#L408-L414"
```

`UpdateEngine` sets the sound to the speed and the position of the tank:

```cpp source="tmpl8-part-17/4-engines/game.cpp#L416-L429"
```

`speed` goes from 0 when the tank stands still to 1 at its top speed. (The `std::min` keeps it at 1, when a push makes the tank go faster than its top speed.) The pitch goes from 0.7 to 1.2, and the volume from 0.15 to 0.4, so a tank that stands still hums low and soft, and roars at full speed. `play` starts the sound when it isn't playing, and does nothing when it is, so it can be called every frame. A tank that isn't in the battle (destroyed, or hidden until a next wave) stops its engine.

Load the engines in `Init`, below the effects:

```cpp source="tmpl8-part-17/4-engines/game.cpp#L838-L845" {2-7}
```

The engines of the enemies get a *rolloff* of 3, instead of the default of 1. In the formula of [Distance](#distance), a bigger rolloff makes a sound softer faster when it is further away. With a whole wave of enemies on the field, their engines would otherwise hum almost as loud as your own. Now you mostly hear the enemies that are close to you.

And update them every frame in `Tick`, right after `UpdateShells`, with the `tanks` array that `UpdateShells` gets:

```cpp source="tmpl8-part-17/4-engines/game.cpp#L899-L904" {5-6}
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-17-engines/index.html" title="The arena battle with engine sounds that follow the speed of each tank" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The engines hum, higher and louder when the tanks drive faster. (Click the demo first, so that it gets the keys and can play sound.)

</div>

## Music

Music is a sound too, but a long one. A sound effect is *decoded* when it is loaded, which means that the library turns the whole OGG file into samples in memory, so that it can start to play right away. For 25 seconds of music, that would be 4.4 MB of memory (and a few minutes of music would be a lot more). So the library *streams* music instead. It decodes a bit of the file at a time, just before it plays that bit.

Music isn't placed in the world either. It doesn't come from somewhere on the map, so it plays in the middle, at the same volume everywhere. Add the music below the effects:

```cpp source="tmpl8-part-17/5-music/game.cpp#L72-L73"
```

In `Init`, below the engines, load the music as `Audio::Sound::Type::Music`, so that the library streams it, and let it loop:

```cpp source="tmpl8-part-17/5-music/game.cpp#L852-L856" {1-4}
```

The music should fade in when a battle starts, and fade out when it is over. `setFade` changes the volume of a sound gradually, over a time. For the time, the library accepts a *duration* of the C++ standard library, like `2s` for 2 seconds. Add this line at the top of `game.cpp`, above `namespace Tmpl8`:

```cpp source="tmpl8-part-17/5-music/game.cpp#L10-L10"
```

The `s` in `2s` is a *literal suffix*, just like the `f` in `0.5f`. It tells C++ what kind of value the number is. `2s` is 2 seconds, `500ms` is 500 milliseconds, and `1min` is a minute. They live in the namespace `std::chrono_literals`, and the `using namespace` lets you use them without writing that namespace every time.

`Restart` starts a new battle, so that's where the music fades in:

```cpp source="tmpl8-part-17/5-music/game.cpp#L778-L790" {11}
```

`setFade(0.0f, MusicVolume, 2s)` fades the music from silence (0) to `MusicVolume`, in 2 seconds. When the player is destroyed, the music fades out, from its current volume to 0, in 3 seconds:

```cpp source="tmpl8-part-17/5-music/game.cpp#L918-L926" {6}
```

Some players don't like music in a game, or want to play their own. Let <kbd>M</kbd> switch the music off and on, in `Tick`, just above <kbd>Tab</kbd>:

```cpp source="tmpl8-part-17/5-music/game.cpp#L967-L976" {1-8}
```

`stop` pauses the music, and `play` continues where it stopped.

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-17-music/index.html" title="The arena battle with music that fades in when a battle starts and fades out when it is over" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Music, which fades in when a battle starts and fades out when it is over. Press <kbd>M</kbd> to switch it off and on. (Click the demo first, so that it gets the keys and can play sound.)

</div>

## Jingles

Remember the simple waves from the start of this part? The sound chips of old game consoles could only play those, and composers made all the music and the effects of their games with them. You can play them too, with `Audio::Waveform`. It makes a sine, square, triangle or sawtooth wave while it plays, from nothing but its frequency and amplitude. We use it for two short tunes (*jingles*), one for when a new wave of enemies comes, and one for when the game is over.

### Notes

A tune is a series of notes, and every note has its own frequency. Musicians start from the A of 440 Hz. One octave higher (the next A, 12 keys further on a piano) is twice the frequency, 880 Hz. An octave has 12 *semitones* (the steps between two keys of a piano, white or black), and every semitone multiplies the frequency by the same number. 12 of them must double it, so that number is the 12th root of 2, which is about 1.0595. Add this function below `MusicVolume`, with `#include <cmath>` at the top of `game.cpp` for `std::pow`:

```cpp source="tmpl8-part-17/game.cpp#L76-L81"
```

$2^{n/12}$ multiplies by the 12th root of 2, $n$ times. So `Note(0)` is the A of 440 Hz, `Note(12)` is 880 Hz, and `Note(-12)` is 220 Hz. Next come the jingles:

```cpp source="tmpl8-part-17/game.cpp#L83-L87"
```

C is 3 semitones above A, E is 7, G is 10, and the C of the next octave is 15. Those four notes, one after the other, go up cheerfully, to announce a new wave. The jingle of the game over goes down instead, a semitone at a time, and the last note is three times as long. Every note takes 120 milliseconds.

### Playing a Jingle

A waveform plays one frequency at a time, so a jingle changes the frequency at every note. These variables keep track of the jingle that is playing:

```cpp source="tmpl8-part-17/game.cpp#L89-L92"
```

`PlayJingle` starts a jingle:

```cpp source="tmpl8-part-17/game.cpp#L94-L102"
```

An array can't be passed to a function by itself. Instead, the function gets a pointer to its first element (remember [part 8](/cpp-fast-track-8-memory-addresses/)?). That's why `PlayJingle` gets the length of the array too, because from the pointer alone, it can't know how many notes there are. `start` starts the waveform, with the frequency of the first note.

`UpdateJingle` plays the note that matches the time:

```cpp source="tmpl8-part-17/game.cpp#L104-L118"
```

`jingleTime / NoteTime` is the number of the note, as a float. From 0 to 1 is the first note, 1 to 2 the second, and so on. After the last note, the waveform stops, and `jingle` becomes `nullptr` again, because no jingle is playing anymore.

Create the waveform in `Init`, below the music:

```cpp source="tmpl8-part-17/game.cpp#L903-L903"
```

A square wave at full volume is very loud, so its amplitude is only 0.1. Create it before `Restart`, because `Restart` starts the first wave, and that plays a jingle.

The jingle of a new wave plays in `StartWave`:

```cpp source="tmpl8-part-17/game.cpp#L799-L802" {4}
```

The jingle of the game over plays when the player is destroyed:

```cpp source="tmpl8-part-17/game.cpp#L967-L976" {7}
```

And `Tick` updates the jingle every frame, below `UpdateExplosions`:

```cpp source="tmpl8-part-17/game.cpp#L964-L965" {2}
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-17-jingles/index.html" title="The arena battle with jingles for a new wave and for the game over" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The final result: sound effects, spatial audio, engines, music and jingles. (Click the demo first, so that it gets the keys and can play sound.)

</div>

## Assignment

### Basic

1. Not every player wants the battle as loud as you do. Let <kbd>+</kbd> and <kbd>-</kbd> (on the main keyboard and on the numeric keypad) make *all* sounds louder or softer, in steps of 10%, from 0% to 100%. Show the volume in the status line at the top.

:::tip[Hint]
`Audio::setMasterVolume` sets the volume of all sounds at once (1 is 100%), and `Audio::getMasterVolume` tells you what it is now. The key with the `+` on it is `SDLK_EQUALS` (the `+` is above the `=`), and the keys on the numeric keypad are `SDL_SCANCODE_KP_PLUS` and `SDL_SCANCODE_KP_MINUS`.
:::

If you completed the basic assignment, the solution should look like this:

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-17-basic/index.html" title="The arena battle with a volume in the status line, which + and - change" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: <kbd>+</kbd> and <kbd>-</kbd> change the volume of all sounds. (Click the demo first, so that it gets the keys and can play sound.)

</div>

### Intermediate

1. In the heat of the battle, it's easy to miss that your health bar turned red. Add an alarm, which beeps when the player's health is a third or less. The beep repeats faster when the health gets lower, and it stops when the game is over.

:::tip[Hint]
Use a second `Audio::Waveform` for the alarm (a sine sounds less harsh than a square), so that it doesn't get in the way of the jingles. A beep is the waveform playing for a short time, like 80 milliseconds. Keep track of the time since the last beep started, like `UpdateJingle` does. When it is more than the time between two beeps, the next beep starts. A function of the health gives that time, for example 900 milliseconds at a third of the health, down to 300 when it is almost gone.
:::

If you completed the intermediate assignment, the solution should look like this:

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-17-intermediate/index.html" title="The arena battle with an alarm that beeps faster when the player's health is low" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: an alarm that beeps faster when your health gets lower. (Click the demo first, so that it gets the keys and can play sound.)

</div>

### Hard

1. The drums are silent. A shell that pushes a drum makes an explosion, but a tank that rams a drum, or a drum that slides into a crate, makes no sound at all. Let a drum *clang* when something pushes it or when it bumps into something. The harder the push or the bump, the louder the clang. Use the hit sounds for it, but deeper, because a drum is hollow.

:::tip[Hint]
You don't have to find out *what* hits a drum, because a sudden change in its velocity tells you that something did. Remember the velocities of the drums before `Simulate`, and compare them after it. A drum that only slides loses a bit of its speed every millisecond, because of its drag. In one frame, its velocity becomes `velocity * std::pow(1 - drag, deltaTime)`. Any other change is a push or a bump, and the size of that change is how hard it was. Ignore very small changes, or the drums clang all the time when a tank leans against them. A new effect, loaded from the hit sounds, can set the volume and the pitch of the voice that it plays.
:::

If you completed the hard assignment, the solution should look like this:

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-17-hard/index.html" title="The arena battle in which the drums clang when they are pushed or bump into something" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: drums that clang when you ram them or shoot them, louder when it's harder. (Click the demo first, so that it gets the keys and can play sound.)

</div>

**Congratulations on getting here!** You started with a black window and a "Hello world", and now you have built a tank battle with physics, AI and sound, from tiles and sprites to state machines and spatial audio. That's the end of the C++ Fast Track. Keep building games: that's the best way to keep learning.
