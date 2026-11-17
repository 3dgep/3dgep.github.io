---
title: "C++ Fast Track for Games Programming Part 16: AI"
description: "AI: give the enemy tanks a brain: let them see, aim, fire and decide what to do with a state machine, and build an arena battle with waves of enemies."
slug: cpp-fast-track-16-ai
lastUpdated: 2026-11-16
---

<img src="/images/cpp-logo.png" alt="C++ Fast Track" width="150" class="thumb" />

In [part 15](/cpp-fast-track-15-physics/), the tanks got physics: they drive, slide along obstacles, and push drums around. But the red tank is still rather dumb: it drives back and forth along a road, whatever you do. In this part, we give it a brain. *Artificial intelligence* (*AI*) in games is about making computer-controlled characters behave in a way that looks smart, and is fun to play against. By the end of this part, you will have an arena battle against waves of enemy tanks that:

1. see you (but only when you are in front of them, close enough, and not behind an obstacle),
2. aim and fire at you (but not perfectly),
3. and decide what to do: patrol, chase you, attack you, or search for you when they lose you.

One important idea before we start: *intelligent* doesn't mean *perfect*. A computer can aim perfectly and react instantly, but an enemy that never misses isn't fun. Good game AI makes mistakes on purpose, so that the player can win, and feels clever doing so.

## Getting Started

For this tutorial, you will start where [part 15](/cpp-fast-track-15-physics/) ended: the tank game with physics. Download it here:

[<img src="/images/zip1.png" alt="Download the starting point for part 16 as a zip file" width="120" class="thumb" />part-16.zip](/downloads/part-16.zip)

Extract the zip file to `C:\Projects\C++ Fast Track`: it contains a `Part 16` folder. Open `tmpl8.slnx` in the `C:\Projects\C++ Fast Track\Part 16` folder.

The code from the end of part 15, with one difference: the tow cable is gone (and so is the code that only the cable used: `KeepDistance`, `DrawCable`, and the keys <kbd>1</kbd>, <kbd>2</kbd> and <kbd>3</kbd>). It has no job in a battle. The drums are still there: they make good cover. The `assets` folder has two new images: `shells.png`, a shell in 16 directions (a green one and a red one), and `explosion.png`, the 5 frames of an explosion. Like the other images, both are made from [Top-down Tanks Remastered](https://kenney.nl/assets/top-down-tanks-remastered) by Kenney, who shares them for free (under the [CC0](https://creativecommons.org/publicdomain/zero/1.0/) license).

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-16-start/index.html" title="The tank game of part 15 without the tow cable: the green tank, the red tank driving back and forth, and the drums" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The starting point: the tank game of part 15. (Click the demo first, so that it gets the keys.)

</div>

## Weapons

A battle needs weapons. The player drives with the keys, aims the turret with the mouse, and fires with the left mouse button.

### The Mouse

First, the game needs to know where the mouse is, and whether its left button is down. Open `game.hpp`, and change the mouse methods:

```cpp source="tmpl8-part-16/game.hpp#L18-L21"
```

And add these variables, in the `private` part of the class (below `frame`):

```cpp source="tmpl8-part-16/game.hpp#L48-L50"
```

The template calls `MouseMove` when the mouse moves, and `MouseDown` and `MouseUp` when a button goes down or up (button 1 is the left mouse button). `mouseDown` is `true` as long as the button is held down. But a quick click can go down *and* up between two frames: then `Tick` would never see `mouseDown` as `true`. That's what `mousePressed` is for: it remembers that the button went down, until `Tick` has seen it.

### Aiming

The barrel of a tank can point in another direction than its body. Both have 16 frames, so we need the frame that faces a direction twice. Move that code out of `Drive`, into a function of its own:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L206-L211"
```

Add a few constants for the weapons below the other constants:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L213-L217" {4-5}
```

The tank gets two new methods, `Aim` and `Fire`, and three new variables:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L219-L243" {14-15,21-23}
```

- `aim` is the direction of the barrel, as a vector with length 1. It starts pointing up.
- `reload` counts down: a tank can only fire when it is 0 (or less).
- `health` starts at `MaxHealth`. Every hit takes some of it.

Update `Drive` to use `FrameOf` now:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L245-L252" {7}
```

`Aim` turns the barrel towards a target: a position on the screen, for example the mouse:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L254-L261"
```

The direction from the tank to the target is the difference of the two positions. Dividing it by its length gives it a length of 1. `normalized` would do that too, but we need the length anyway, for the test, and `normalized` would calculate it again: a second square root. So we calculate the length once, and use it twice. When the target is exactly the middle of the tank, there is no direction, and the barrel stays as it is.

:::note
Why would a tank aim at itself? It doesn't do that on purpose, but the target is the mouse, and the player can put the mouse anywhere, also right on top of the tank. The tank starts in the middle of a tile, on a whole pixel, so that's not hard to do. Then the difference is $(0, 0)$, with a length of 0, and normalizing it divides by that length. Dividing 0 by 0 doesn't give a number: the result is *NaN* ("not a number"). Then `aim` is NaN, and every calculation with it gives NaN too. `FrameOf` can't turn NaN into a proper frame (converting NaN to an `int` is *undefined behaviour* in C++), and a shell fired in direction NaN flies to position NaN: it never hits anything, and it is never drawn. Later in this part, the enemies turn their turrets *from* their current `aim`, so there a NaN would never go away: the turret would be broken for good. So before you normalize a vector, ask yourself: can the length be zero? The `normalized` function assumes you know what you're doing and doesn't check this for you.
:::

`Draw` uses the direction of the barrel for the barrel's frame, and draws a health bar above the tank:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L272-L286" {4-14}
```

The health bar is a dark bar of 32 pixels, with a green part (red when the health is low) that is as wide as the part of the health that is left: `32 * health / MaxHealth`.

:::note
Remember the bug in `Surface::Bar` from [part 7](/cpp-fast-track-7-debugging/)? The tmpl8 code still has this bug in it: `Bar` crashes when the bar doesn't fit on the screen. Near the top of the map, a bar of 34 pixels above the tank would stick out of the screen. In that case, it goes below the tank. If you fixed `Bar` in part 7, you can copy your fix into this part's `surface.cpp`, and always draw the bar above the tank.
:::

### Shells

A shell flies in a straight line, until it hits something, or its time is up. Shells are not bodies: they are much faster than the tanks, nothing pushes them around, and they disappear when they hit something anyway. (A shell does push a drum when it hits it, but we'll handle that ourselves later, instead of leaving it to the physics engine.) Shells get a struct of their own, with an array of them. Add this below `bodyCount`:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L301-L313"
```

Like the drums, there is a fixed number of shells. A shell with a `time` of 0 (or less) is free for reuse: when a tank fires, `Fire` looks for a free shell and sends it on its way.

:::note[Is 32 Enough?]
A tank fires at most once every 400 milliseconds (`ReloadTime`), and a shell flies for 1500 milliseconds (`ShellTime`), so one tank never has more than 4 shells in the air. Even with the player and 5 enemies, that's 24 at most. (And if all shells are in use, `Fire` simply doesn't fire.)
:::

`UpdateShells` uses `Explode`, which comes in the next section, with the rest of the explosion code. C++ reads the code from top to bottom, so it has to know about `Explode` before `UpdateShells` can use it. A *forward declaration* tells the compiler that a function exists before it is defined, just like the declarations of the methods in a class (see [part 12](/cpp-fast-track-12-classes/)). Add it below the shells:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L315-L317"
```

Every frame, the shells move, and we check what they hit. That's the job of `UpdateShells`. It's a long function, so we'll break it down into a few parts. First, update the shell's position:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L319-L330"
```

The loop goes over all shells. `shell` is a reference, so changing `shell` changes the shell in the array. A shell with no time left isn't flying: `continue` skips the rest of the loop for that shell, and goes on with the next one. A flying shell moves in a straight line, like the bodies of part 15 (but without drag), and has a bit less time left.

Then we check whether the shell hits a tank:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L332-L343"
```

This uses the overlap tests of part 15: the shell is a small circle, and so is a tank. `tanks` is an array of *pointers* to tanks (see [part 8](/cpp-fast-track-8-memory-addresses/)), so that the function can change the health of the tanks themselves, not of copies. `*tanks[t]` is the tank that `tanks[t]` points to, and the reference `tank` gives it a short name, so that we don't have to keep writing `tanks[t]->` every time.

A shell that hits a tank takes `ShellDamage` of its health: 5 hits destroy a tank. A destroyed tank (with no health left) can't be hit anymore. `hit` is a boolean variable which we'll set to `true` if the bullet should explode.

The drums are next:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L344-L352"
```

A drum is a circle too, and `drum` is a reference to the drum in the array, like `shell`. A shell that hits a drum gives it a push: a part of the shell's velocity. The drum is light, so it flies away.

:::note[Force vs. Impulse]
In part 15, the engine of a tank and the drag change its velocity a little bit every frame: `(drive - velocity * drag) * h`. That's a *force*: it works over time, like gravity, so its effect depends on how long it works (that's why it's multiplied by the time step). The push of a shell is different: it changes the drum's velocity all at once, in a single frame, so there's no time step in it. A sudden change of velocity like this is called an *impulse*. Hits, collisions and explosions are impulses: in the real world, they are huge forces that work for a very short time, and a game simply skips that short time and changes the velocity right away. (A real impulse changes the velocity less for a heavier body: divide it by the mass, or multiply it by the inverse mass. All drums are equally heavy, so the `0.5f` takes care of that here.)
:::

Then the obstacles of the map:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L353-L359"
```

A shell that hits an obstacle (any of the colliders, including the walls around the map) just stops. Finally:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L361-L368"
```

Regardless of what the shell hit, it explodes, and its time becomes 0 so it's free again for the next shot.

`DrawShells` draws the flying shells:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L370-L382"
```

A shell's frame is the frame of its direction (`FrameOf` again), and its row in `shells.png` is its colour. `shell` is a reference again, but a `const` one: drawing a shell doesn't change it.

### Explosions

A shell that hits something explodes. An explosion is an animation that plays for a moment, so the game has to remember it for a few frames:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L384-L393"
```

Like the shells, there is a fixed number of explosions. To start one:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L395-L404"
```

`Explode` starts an explosion in a free spot of the array (if there is one: with more than 16 explosions at the same time, one is simply not shown).

:::note[Is 16 Enough?]
An explosion lasts 400 milliseconds (`ExplosionTime`): exactly as long as a tank needs to reload. A shell only explodes when it hits something, and each tank fires at most one shell per 400 milliseconds, so each tank causes about one explosion at a time. Even with the player and 5 enemies, that's about 6 explosions, plus one for each tank that is destroyed: 12 at most. That isn't a hard limit like the one for the shells (two shells of the same tank can hit at almost the same moment, when the first one flies further than the second), but there's room to spare. And when it does happen, one explosion more or less in the chaos isn't something anyone will notice.
:::

Every frame, the explosions get older, until they are over:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L406-L411"
```

`DrawExplosions` draws the explosions:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L413-L426"
```

An explosion shows the first frame of `explosion.png` when it starts, and the last one when its time is almost up.

### Firing

Now the tanks can fire. `Fire` sends a free shell on its way:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L428-L440"
```

The shell starts 30 pixels in front of the middle of the tank (at the end of the barrel, and outside the tank's own circle), and flies in the direction of the barrel. Its colour is the colour of the tank: green or red. Then the tank has to reload.

<svg class="memory-diagram fire-diagram" viewBox="0 70 700 290" role="img" aria-label="A green tank facing up, with its barrel turned 45° to the right. The middle of the tank is body.position. A dashed circle with a radius of 30 pixels shows the distance from the middle at which shells start; a smaller circle with a radius of 20 pixels is the tank's collision circle. An arrow of 30 pixels goes from the middle of the tank in the direction of aim, along the barrel, to the end of the barrel, where the new shell starts: just outside the circle.">
<defs><marker id="fire-arrow" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse"><path d="M0 0 10 5 0 10z" class="arrow-head"/></marker></defs>
<image href="/images/cpp-fast-track-16/fire-tank.png" x="60" y="70" width="256" height="256"/>
<circle class="radius" cx="188" cy="198" r="120"/>
<circle class="collider" cx="188" cy="198" r="80"/>
<path class="arrow aim" d="M188 198 L 267 119" marker-end="url(#fire-arrow)"/>
<circle class="dot" cx="273" cy="113" r="5"/>
<circle class="dot" cx="188" cy="198" r="5"/>
<path class="leader" d="M194 204 L 376 316"/><text x="384" y="321"><tspan class="mono">body.position</tspan>: the middle of the tank</text>
<path class="leader" d="M236 150 L 376 166"/><text x="384" y="171"><tspan class="mono">aim * 30</tspan>: 30 pixels along the barrel</text>
<path class="leader" d="M280 113 L 376 116"/><text x="384" y="121">the new shell starts here</text>
<path class="leader" d="M307.5 209.4 L 376 216"/><text x="384" y="221">30 pixels from the middle, all around</text>
<path class="leader" d="M263.2 225.2 L 376 266"/><text x="384" y="271">the tank's collision circle: a radius of 20</text>
<image class="made-by" href="/images/claude-logo.svg" x="679" y="339" width="16.8" height="16.8" opacity="0.35"><title>Made with Claude</title></image>
</svg>

<div class="caption">

Where a shell starts: 30 pixels from the middle of the tank, in the direction of the barrel (`aim`), not of the tank itself. That's outside the tank's own circle (a radius of 20), so the tank doesn't shoot itself.

</div>

### Putting It All Together

Finally, `Tick`. The player aims at the mouse, reloads, and fires:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L558-L563" {2-6}
```

Then the shells move, the explosions get older, and when the enemy is destroyed, it explodes, and starts over at its starting point:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L565-L576" {2-12}
```

And the shells and the explosions are drawn on top of the tanks:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L584-L586" {2-3}
```

When you press <kbd>Tab</kbd>, the flying shells show their circles too, like the other colliders:

```cpp source="tmpl8-part-16/1-weapons/game.cpp#L597-L604" {3-8}
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-16-weapons/index.html" title="The green tank's turret follows the mouse; clicking fires shells that explode on obstacles, push drums away, and damage the red tank" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Aim with the mouse, and click (or hold the button) to fire. Shells explode on obstacles, push drums away, and 5 hits destroy the red tank. (Click the demo first, so that it gets the keys.)

</div>

## Seeing

An enemy should only attack what it can *see*. That's three questions: is the player close enough, is he in front of the enemy, and is there an obstacle in between?

### A Line Through a Circle

For the obstacles, we need a new kind of test: does a *line* (from the enemy to the player) cross a collider? Add these two functions below the overlap tests. First a circle:

```cpp source="tmpl8-part-16/2-sight/game.cpp#L149-L159"
```

This is like the circle - box test of part 15, which looks for the point of the box that is closest to the middle of the circle. Here, we look for the point of the *line* that is closest to the middle of the circle. Every point of the line from $a$ to $b$ is $a + t\,(b - a)$, for some $t$ between 0 (at $a$) and 1 (at $b$). The closest point has

$$
t = \frac{(c - a) \cdot (b - a)}{|b - a|^2}
$$

where $c$ is the middle of the circle. The dot product measures how far $c$ lies along the line (part 15 used the dot product for that too, in `Push`). Clamping $t$ between 0 and 1 keeps the point on the line. Then the line crosses the circle when that point is closer to the middle than the radius.

<iframe class="embed embed-demo embed-line-tests" src="/demos/line-tests.html?test=lc" title="Drag the ends of a line, or the circle: the closest point of the line to the middle of the circle is shown, also on the x and y axes, where the line runs from t = 0 to t = 1; the line turns green when that point is closer than the radius" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Drag $a$ or $b$ to move one end of the line, drag the line itself to move both ends, or drag the circle to move it. The purple arrow is $c - a$. The dot product with $b - a$ gives the yellow part: how far $c$ lies along the line, where a line from $c$ meets it at a right angle. Divided by the length of the line, that is $t$: 0 at $a$, 1 at $b$. The axes at the bottom and the left show the same thing for $x$ and $y$: the line from $t = 0$ to $t = 1$, the yellow part, and the closest point. When the closest point would be beyond $a$ or $b$ (the grey dot), $t$ is clamped to 0 or 1.

</div>

### A Line Through a Box

A box takes a different trick: look at one axis at a time:

```cpp source="tmpl8-part-16/2-sight/game.cpp#L161-L187"
```

- On the x axis, the line enters the space between the left and the right side of the box at some $t$, and leaves it at another $t$. Same for the y axis: between the top and the bottom.
- The line is inside the box where it is between the left and right sides *and* between the top and bottom sides: where both ranges of $t$ overlap. But the line starts at $a$ and ends at $b$, so only values of $t$ between 0 and 1 count. That's why `first` and `last` start as 0 and 1, and each axis can only make that range smaller.
- When the range is empty (`first > last`), the line misses the box.
- When the line is exactly horizontal (or vertical), it never crosses the left and right sides (or the top and bottom): it's between them all the way, or never. Then there is nothing to divide by, so that is handled separately.

This is called the *slab test*: the space between two opposite sides is a *slab*, and a box is where the slabs of both axes overlap.

<iframe class="embed embed-demo embed-line-tests" src="/demos/line-tests.html?test=lb" title="Drag the ends of a line, or the box: the slabs of the box are shaded; the line from t = 0 to t = 1 is shown on the x axis next to the x slab, and on the y axis next to the y slab; the line turns green where it is inside both slabs" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Drag $a$ or $b$ to move one end of the line, drag the line itself to move both ends, or drag the box to move it. The dots show where the line enters and leaves each slab. Along the bottom, the line runs from $t = 0$ (at $a$) to $t = 1$ (at $b$) next to the purple x slab; along the left, the same line next to the yellow y slab. The green part, on the line and on both axes, is where the line is inside both slabs: from `first` to `last`.

</div>

### What a Tank Can See

Now the three questions together. Add these below the `Tank::Draw` method:

```cpp source="tmpl8-part-16/2-sight/game.cpp#L327-L347"
```

- Too far away: then it can't see the point.
- The *field of view*: the dot product of two vectors with length 1 is the cosine of the angle between them. 1 means the same direction, 0 means 90° apart, and -1 opposite directions. So when the dot product of the tank's `aim` and the direction to the point is less than 0.5 (the cosine of 60°), then the point is more than 60° to the side and is outside the field of view of the enemy.
- An obstacle in between: If the line from the tank to the point crosses a collider, then the enemy can't see the player.

All of these points determine if the enemy can see the player. This is demonstrated in the interactive demo below:

<iframe class="embed embed-demo embed-sight-test" src="/demos/sight-test.html" title="A tank with its view cone, a player and some obstacles; the line from the tank to the player is green when the tank can see the player, and red when the player is too far away, outside the field of view, or behind an obstacle. Sliders change the sight range and the field of view." loading="lazy" allowfullscreen></iframe>

<div class="caption">

Drag the tank, the end of its barrel (to aim), the player, or the obstacles. The three questions are at the top: the line turns red as soon as one of them says no, and the obstacles in the way turn red too. The sliders change `SightRange` and the field of view (`FieldOfView` is the cosine of the angle to each side).

</div>

<div id="only-a-point"></div>

:::note[Only a Point]
`CanSee` checks a single point: the middle of the player. But the player is a circle. When the player peeks out from behind a tree, with his middle still behind it, the enemy can't see him, although a real tank driver would. To see *any* part of the player, check more than one point: the middle isn't the only point of the circle that can stick out. Which points of the circle stick out the furthest to the sides, as seen from the enemy? You can try this in the [intermediate assignment](#intermediate).
:::

Now let's put `CanSee` to work: the enemy's turret should follow the player while the enemy can see him. Until now, `Tank::Update` pointed the turret in the direction the enemy drives (`aim = { patrol, 0 }`). Remove that line, so that `Update` only drives the enemy:

```cpp source="tmpl8-part-16/2-sight/game.cpp#L303-L309"
```

The turret is aimed in `Game::Tick` instead, right after `enemy.Update()`: the enemy itself doesn't know where the player is, but the game does.

```cpp source="tmpl8-part-16/2-sight/game.cpp#L625-L631" {2-7}
```

- When the enemy can see the player, `Aim` turns its turret towards the player: the same `Aim` that turns the player's turret towards the mouse.
- When it can't, the turret looks where the enemy drives again. `Aim` needs a position to aim at, not a direction, so we give it a point right in front of the enemy: its position plus `{ enemy.patrol, 0 }`, one step to the right or to the left.
- The answer of `CanSee` goes in a variable, `seen`, because we'll use it once more below, for the debug drawing.

To see what the enemy sees, <kbd>Tab</kbd> draws its *view cone*: the two edges of its field of view, as long as it can see. And a line to the player: green when the enemy can see him, red when it can't. Add this to the drawing of the colliders:

```cpp source="tmpl8-part-16/2-sight/game.cpp#L672-L679"
```

The edges are the direction of the turret, turned 60° to both sides. Turning a vector by an angle $\theta$ uses its cosine and sine: here, they are 0.5 and $\sqrt{1 - 0.5^2}$.

:::note[A Sine from a Cosine]
Draw a line of length 1 from the origin, at an angle $\theta$. Its end is the point $(\cos\theta, \sin\theta)$: the cosine is how far it goes along $x$, the sine how far along $y$. The line is the long side of a right triangle with those two sides, so Pythagoras says $\cos^2\theta + \sin^2\theta = 1$. That means $\sin\theta = \sqrt{1 - \cos^2\theta}$.

`FieldOfView` already holds the cosine, so this gives us the sine without knowing the angle itself, and without calling `std::acos` and `std::sin`. A square root is also much cheaper for the CPU than `acos` and `sin`: it is a single instruction, while the trigonometric functions calculate their answer in many steps. (The square root is never negative, so this only works for angles from 0° to 180°, where the sine is positive. Half a field of view always is such an angle.)
:::

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-16-sight/index.html" title="With Tab, the red tank's view cone is shown, and a line to the green tank: green when the red tank can see it, red when it can't" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Press <kbd>Tab</kbd>, and drive around the red tank: hide behind the trees and crates, get out of its range, or behind its back, and watch the line. (Click the demo first, so that it gets the keys.)

</div>

## Fighting Back

So far, the enemy can only patrol and aim at the player. In this section, we teach it to fire back. But not perfectly: an enemy that reacts instantly, turns its turret instantly and never misses, would destroy you before you even see it. So we add three imperfections:

1. A *reaction time*: the enemy must see the player for a moment, before it reacts.
2. A turret that turns at a limited speed: drive fast, and its aim lags behind.
3. A *spread*: its shells fly a little bit off.

### Turning

To turn the turret step by step, we need two helpers. Add them below `FrameOf`. The first one turns a vector by an angle:

```cpp source="tmpl8-part-16/3-fight/game.cpp#L253-L258"
```

This is the formula of a rotation: $x' = x \cos\theta - y \sin\theta$ and $y' = x \sin\theta + y \cos\theta$. (The view cone of the last section used it already, with the cosine and the sine of 60°.)

:::note[Why Not the Square Root Trick?]
In the last section, we got the sine from the cosine with $\sin\theta = \sqrt{1 - \cos^2\theta}$. That doesn't work here, for two reasons:

- The square root is never negative, but the sine of a negative angle is. We need negative angles to turn the other way.
- The angles are often tiny: when the turret is almost on target, it only needs to turn a little bit. Then $\cos\theta$ is very close to 1, and $1 - \cos^2\theta$ subtracts two numbers that are almost equal. A `float` only holds about 7 digits, so many of the digits of the answer are lost. For an angle of 0.0001 radians, the result is even exactly 0.

In the view cone, neither was a problem: half a field of view is never negative, and never that small. Also, there we already *had* the cosine, but here we start from an angle, so we need a call to `std::cos` anyway.
:::

The `TurnTowards` function turns a direction vector towards another direction, clamped to a maximum turn angle:

```cpp source="tmpl8-part-16/3-fight/game.cpp#L260-L266"
```

`TurnTowards` finds the angle from one direction to the other: the *cross product* $x_1 y_2 - y_1 x_2$ says on which side the other direction is, and the dot product how far it is. `std::atan2` turns those two into an angle, between $-\pi$ and $\pi$. Then the angle is clamped to at most `maxAngle`, so the direction turns, but no further than that.

<iframe class="embed embed-demo embed-turn-towards" src="/demos/turn-towards.html" title="Two directions, from and to, and the result of TurnTowards: from, turned towards to, but by at most maxAngle. Sliders set how fast the turret turns and the frames per second; Step plays one frame, and Play plays them in real time." loading="lazy" allowfullscreen></iframe>

<div class="caption">

Drag the dots at the ends of `from` and `to`. The dashed arc is the angle from `from` to `to`, and the shaded wedge shows how far `from` may turn: `maxAngle` to both sides. The green arc is the turn that is made, and ends at the `result`. The sliders set how fast the turret turns, in degrees per second, and the number of frames per second (FPS). One frame takes `deltaTime` = 1 / FPS, and `maxAngle` is the turret speed times `deltaTime`.

Press **Step** to play one frame, or **Play** to play it at frames per second. With a few frames per second, `from` turns in big jumps that stop at the edge of the wedge. With 60 frames per second, the wedge is tiny, and `from` turns smoothly. Either way, it takes about the same time to get there.

</div>

:::note[Why Not the Inverse Cosine?]
Both directions have length 1, so their dot product is the cosine of the angle between them. Why not just use `std::acos(from.dot(to))`?

- `std::acos` only gives angles from 0 to $\pi$: it doesn't say on which side `to` is. Turned 30° to the left or 30° to the right, the dot product is the same. The cross product tells them apart: it is positive on one side and negative on the other.
- Rounding can make the dot product a tiny bit larger than 1, like `1.0000001`. Then `std::acos` returns NaN ("not a number"), and the turret's direction becomes NaN too.

`std::atan2` uses both: the cross product (the sine) gives the side, the dot product (the cosine) gives how far. Together they give the angle with its sign, and they never cause a NaN.
:::

Add the constants of the imperfections:

```cpp source="tmpl8-part-16/3-fight/game.cpp#L273-L275"
```

### Firing with a Spread

`Fire` gets a spread: the shell flies in the direction of the barrel, turned by a random angle between `-spread` and `spread`. (`Rand` comes with the template: `Rand(x)` is a random number from 0 up to `x`.) The player fires with a spread of 0: he aims himself.

```cpp source="tmpl8-part-16/3-fight/game.cpp#L521-L535" {1-3,10-11}
```

### The Enemy's Brain

The enemy's `Update` becomes `Think`: it gets to know the target, and the time. Change the declarations in the `Tank` class:

```cpp source="tmpl8-part-16/3-fight/game.cpp#L291-L292"
```

The `Fire` method takes the `spread` parameter that that determines how accurate the bullet is. The `= 0` is a *default argument*: when you leave the argument out, as the player's `Fire()` does, `spread` is 0. (The default argument is only specified in the declaration of the method, not in it's definition.) `Think` replaces `Update`, and gets the tank it should look for, and the change in time since the last frame.

The enemy also has to remember something from one frame to the next: how long it has been since it noticed the player. We'll use this parameter to compute the reaction time. Add the `noticed` member variable below `patrol`:

```cpp source="tmpl8-part-16/3-fight/game.cpp#L301-L301"
```

That's the reaction time at work: `Think` adds to `noticed` every frame the enemy sees the player, and the enemy only reacts once `noticed` reaches `ReactionTime`.

Remove `Update`, and add `Think` below `CanSee` (it uses `CanSee`, so it must come after it):

```cpp source="tmpl8-part-16/3-fight/game.cpp#L360-L379"
```

- The driving is the same as before.
- `noticed` adds up the time that the enemy has seen the target, and starts over at 0 when it loses sight of it. Only after `ReactionTime`, the enemy reacts.
- `wanted` is where the turret should point: where the enemy drives, or at the target once it is noticed. `TurnTowards` turns the turret towards it, but at most `TurretSpeed` radians per millisecond.
- The enemy fires when the turret points close enough at the target: the dot product of `aim` and `wanted` is more than 0.98, which is within 11°.

In `Tick`, the enemy thinks:

```cpp source="tmpl8-part-16/3-fight/game.cpp#L659-L659"
```

And the player can be destroyed now. For now, he starts over too:

```cpp source="tmpl8-part-16/3-fight/game.cpp#L664-L671"
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-16-fight/index.html" title="The red tank turns its turret towards the green tank when it sees it, and fires" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Now the red tank fires back. Drive past it quickly: its turret can't keep up. Stay in its sight, and it hits you. (Click the demo first, so that it gets the keys.)

</div>

Try the constants: a `ReactionTime` of 0, a `TurretSpeed` of 1 and a `Spread` of 0 make an enemy that hits you every time. That's not fun at all. Tuning these numbers is how game designers set the *difficulty*.

## Decisions

The enemy still drives back and forth, whatever happens. A real opponent decides what to do: drive around when it doesn't know where you are, come after you when it sees you, stop to fire when you are close, and look for you when it loses you. The most common way to organize such decisions is a *finite state machine*: the enemy is always in one of a few *states*, each state has its own behaviour, and *transitions* say when it switches to another state.

<svg class="memory-diagram state-diagram" viewBox="0 0 700 310" role="img" aria-label="A state machine with four states: Patrol, Chase, Attack and Search. Patrol goes to Chase when it sees the player. Chase goes to Attack when the player is close, and to Search when it loses sight. Attack goes to Chase when the player is far away, and to Search when it loses sight. Search goes to Chase when it sees the player, and back to Patrol when the search time is up.">
<defs><marker id="state-arrow" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse"><path d="M0 0 10 5 0 10z" class="arrow-head"/></marker></defs>
<rect class="var" x="40" y="130" width="130" height="56" rx="10"/><text class="value" x="105" y="164" text-anchor="middle">Patrol</text>
<rect class="var" x="285" y="30" width="130" height="56" rx="10"/><text class="value" x="350" y="64" text-anchor="middle">Chase</text>
<rect class="var" x="530" y="130" width="130" height="56" rx="10"/><text class="value" x="595" y="164" text-anchor="middle">Attack</text>
<rect class="var" x="285" y="234" width="130" height="56" rx="10"/><text class="value" x="350" y="268" text-anchor="middle">Search</text>
<path class="arrow" d="M150 130 C 190 80, 230 64, 283 60" marker-end="url(#state-arrow)"/><text class="dim" x="168" y="100" text-anchor="end">sees the player</text>
<path class="arrow" d="M415 52 C 480 60, 540 90, 575 128" marker-end="url(#state-arrow)"/><text class="dim" x="478" y="62">close</text>
<path class="arrow" d="M560 130 C 520 104, 470 90, 417 82" marker-end="url(#state-arrow)"/><text class="dim" x="452" y="126">far away</text>
<path class="arrow" d="M380 88 L 380 232" marker-end="url(#state-arrow)"/><text class="dim" x="392" y="222">loses sight</text>
<path class="arrow" d="M580 188 C 550 230, 480 246, 417 248" marker-end="url(#state-arrow)"/>
<path class="arrow" d="M285 270 C 210 268, 140 240, 110 188" marker-end="url(#state-arrow)"/><text class="dim" x="80" y="284">search time up</text>
<path class="arrow" d="M320 234 L 320 88" marker-end="url(#state-arrow)"/><text class="dim" x="314" y="214" text-anchor="end">sees the player</text>
<image class="made-by" href="/images/claude-logo.svg" x="679" y="289" width="16.8" height="16.8" opacity="0.35"><title>Made with Claude</title></image>
</svg>

<div class="caption">

The four states of an enemy, and when it switches.

</div>

### States

A state is one of a few fixed values: that's what an `enum class` is for. Add the `State` enum and the `AttackRange` and `SearchTime` constants below the other constants:

```cpp source="tmpl8-part-16/4-states/game.cpp#L276-L286"
```

`State::Patrol`, `State::Chase` and so on are the four possible values of a `State`. They are called *enumerators*: named constants of the type `State`. The compiler checks that you only use these values: a `State` can't be 7, or the string `"Chase"`, by accident.

The enemy remembers its state, its goal, and a timer, instead of `patrol`:

```cpp source="tmpl8-part-16/4-states/game.cpp#L311-L315"
```

For the patrols, the enemy needs random places to drive to. Add this below `CanSee`:

```cpp source="tmpl8-part-16/4-states/game.cpp#L374-L383"
```

`IRand(n)` comes with the template: a random whole number from 0 up to `n - 1`. The loop tries random tiles until it finds one without an object.

### The State Machine

Now `Think` becomes the state machine. It's a long function, so we'll go through it piece by piece: replace `Think` with the code of the blocks below, from top to bottom.

First, the enemy finds out what it knows:

```cpp source="tmpl8-part-16/4-states/game.cpp#L385-L402" {1-2,5,10-18}
```

- `seen` is true when the enemy has seen the target long enough to react, like before.
- `toTarget` and `distance` say where the target is, and `toGoal` and `goalDistance` where the enemy's goal is. They are calculated once, here at the top, because several states need them.
- `direction` and `look` are what the states decide: where to drive, and where to point the turret. `direction` starts at zero, so a state that doesn't set it makes the tank brake. `look` starts at `aim`, so a state that doesn't set it leaves the turret where it is.
- The `switch` picks the code of the current state. Each `case` ends with `break`, so only the code of that one state runs.

In the **Patrol** state, the enemy drives around, to random spots:

```cpp source="tmpl8-part-16/4-states/game.cpp#L403-L416"
```

The enemy drives to its goal, and looks where it drives. When it gets there, it picks another spot. It also picks another spot when it's stuck behind an obstacle: `timer` adds up the time that the tank hardly moves, and starts over at 0 as soon as it moves again. After a second of being stuck, it gives up on that spot. When it sees the target, it will transition to the `Chase` state. (Even when the target is close: then `Chase` switches to `Attack` in the next frame.)

In the **Chase** state, the enemy comes after the target:

```cpp source="tmpl8-part-16/4-states/game.cpp#L417-L427"
```

It drives straight at the target, and looks at it. When it loses sight: Search, with the timer at 0, to measure how long it searches. When it gets close enough: Attack.

In the **Attack** state, the enemy stands still and fires at the target:

```cpp source="tmpl8-part-16/4-states/game.cpp#L428-L439"
```

`direction` stays zero, so the tank brakes, and the turret aims at the target. Once the turret points close enough at the target, it fires: the dot product of `aim` and `look` is more than 0.98, about 11°, like before. (`Fire` itself checks whether the tank has reloaded.) When it loses sight: Search. When the target gets away: Chase. Note the `AttackRange * 1.2f`: if Attack switched to Chase at exactly the same distance as Chase switches to Attack, an enemy at that distance would switch every frame. The difference between the two distances is called *hysteresis*.

:::note[Where the Word *Hysteresis* Comes From]
*Hysteresis* comes from the Greek *hysterēsis*: "lagging behind", or "coming too late". The physicist James Alfred Ewing used it in the 1880s for magnetism: when you magnetize iron and then take the magnet away, the iron stays a bit magnetic. Its state lags behind the cause.

You know it from a thermostat, too: it turns the heating on at 19°, but only turns it off again at 21°. Otherwise it would switch on and off all the time around 20°. The enemy does the same: it switches to Attack at 250 pixels, but only back to Chase at 300.
:::

In the **Search** state, the enemy looks for the target where it lost it:

```cpp source="tmpl8-part-16/4-states/game.cpp#L440-L453"
```

It drives to `goal`: the place where it saw the target last. Meanwhile, it looks around by turning the turret a bit every frame. When it sees the target again: Chase. When the search takes longer than `SearchTime`: back to Patrol, with a new random goal.

After the `switch` comes the code that all states share:

```cpp source="tmpl8-part-16/4-states/game.cpp#L454-L460" {1-5}
```

- While the enemy sees the target, it remembers where: that's the goal for Search, when it loses the target.
- It drives in the `direction` and turns its turret towards `look`, as its state decided.
- It reloads. That happens in every state, not only in Attack: otherwise, an enemy that just came into Attack would still have to reload from the last shot it fired, which may be a long time ago.

The enemy needs a goal to patrol to from the start. Give it a first one at the end of `Init`:

```cpp source="tmpl8-part-16/4-states/game.cpp#L692-L697" {5}
```

When the enemy is destroyed in `Tick`, it starts over on the left. It should also start over in the Patrol state, with a new goal, and not go on chasing or searching where it left off:

```cpp source="tmpl8-part-16/4-states/game.cpp#L754-L763" {8-9}
```

And with <kbd>Tab</kbd>, show the state of the enemy above it, and its goal as a small blue circle:

```cpp source="tmpl8-part-16/4-states/game.cpp#L800-L802"
```

The names are in the same order as the values of `State`: `static_cast<int>` turns a state into its number (0 for Patrol, 1 for Chase, and so on), which is the index in the array.

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-16-states/index.html" title="With Tab, the red tank's state is shown above it: it patrols to random spots, chases and attacks the green tank, and searches where it lost it" loading="lazy" allowfullscreen></iframe>

<div class="caption">

Press <kbd>Tab</kbd> to see the enemy's state and goal. Let it find you, then hide behind a tree, and watch it search for you. (Click the demo first, so that it gets the keys.)

</div>

:::note[Getting stuck]
Chase drives *straight* at the player. When there is an obstacle in between, the enemy slides along it, and usually gets around it; but sometimes it gets stuck in a corner. A real game would use *pathfinding*: finding a way around the obstacles. You will build that in the assignment.
:::

## The Arena

Time for a battle: waves of enemies. Wave 1 has 1 enemy, wave 2 has 2, and so on, up to 5 at the same time. Destroy all enemies of a wave, and the next wave comes. Every enemy destroyed is a point; and when you are destroyed, the game is over.

### Many Enemies

Instead of one enemy, there is an array of them. An array of objects needs a constructor without arguments, to make the objects before the game knows their positions. The simplest way: give the constructor default arguments, like the `spread` of `Fire`:

```cpp source="tmpl8-part-16/game.cpp#L292-L292"
```

Now `Tank enemies[MaxEnemies];` makes 5 tanks with the default values: red ones (colour 1) at position (0, 0). Replace `enemy` by the enemies, and the variables of the battle:

```cpp source="tmpl8-part-16/game.cpp#L465-L471"
```

The array of bodies needs room for all enemies:

```cpp source="tmpl8-part-16/game.cpp#L480-L480"
```

### Waves

An enemy that isn't in the battle (destroyed, or not needed yet in this wave) can't just disappear: it's in the array of bodies, and the physics moves all bodies. So it hides, far outside the map, where it's never in the way. Add these three functions above `showColliders` (they use the shells, so they must come after them):

```cpp source="tmpl8-part-16/game.cpp#L658-L665"
```

Each hidden enemy gets its own spot (500 pixels apart), so that the hidden enemies don't overlap, and push each other around. A health of 0 means: not in the battle.

```cpp source="tmpl8-part-16/game.cpp#L667-L690"
```

`StartWave` puts the enemies of the next wave on random spots. The `do`-`while` loop tries spots until it finds one that is at least 300 pixels away from the player: an enemy that appears right next to you isn't fair. The enemies that this wave doesn't need are hidden.

```cpp source="tmpl8-part-16/game.cpp#L692-L703"
```

`Restart` starts a whole new battle: the player as good as new, the score at 0, no shells in the air, and wave 1.

In `Init`, all enemies go into the array of bodies, and the first battle starts:

```cpp source="tmpl8-part-16/game.cpp#L747-L752" {2-3,6}
```

### The Battle

In `Tick`, the player only drives and fires while the game isn't over, and every enemy that is in the battle thinks:

```cpp source="tmpl8-part-16/game.cpp#L791-L801" {1-2,6,9-11}
```

The shells check all tanks: the player, and all enemies.

```cpp source="tmpl8-part-16/game.cpp#L803-L807" {1-4}
```

Then the destroyed tanks, and the waves:

```cpp source="tmpl8-part-16/game.cpp#L809-L840" {1-2,5-32}
```

- When the player is destroyed, he explodes, the game is over, and he hides too: otherwise the enemies would keep firing at his wreck.
- An enemy with a health of 0 that is still on the map was destroyed in this frame: it explodes, hides, and the score goes up. (Hidden enemies are at negative positions, so `position.x > 0` means: still on the map.)
- `remaining` counts the enemies that are still in the battle. When it is 0, `waveTimer` counts 2 seconds, and then the next wave comes.
- When the game is over, <kbd>R</kbd> starts a new battle.

The drawing of the tanks skips the destroyed ones:

```cpp source="tmpl8-part-16/game.cpp#L847-L853" {1,3-5}
```

With <kbd>Tab</kbd>, every enemy shows what it sees. `Rotate` makes the view cone simpler now:

```cpp source="tmpl8-part-16/game.cpp#L872-L888" {1-8,10-12}
```

And at the end of `Tick`, a status line at the top, and a message in the middle of the screen when the game is over, or the next wave is coming (add `#include <format>` at the top of `game.cpp` for `std::format`):

```cpp source="tmpl8-part-16/game.cpp#L891-L897"
```

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-16-arena/index.html" title="The arena battle: waves of red tanks patrol, chase, attack and search; the green tank fights them with the mouse" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The arena battle. Drive with the arrow keys (or <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd> and <kbd>D</kbd>), aim with the mouse, and click to fire. How many waves can you survive? <kbd>Tab</kbd> shows what the enemies see and do, and <kbd>R</kbd> starts a new battle when the game is over. (Click the demo first, so that it gets the keys.)

</div>

## Assignment

### Basic

1. Make the battle harder with every wave: the enemies of wave 1 react slowly, turn their turrets slowly and miss a lot; every next wave, they react a bit faster, turn a bit faster and aim a bit better, up to a limit.

:::tip[Hint]
`ReactionTime`, `TurretSpeed` and `Spread` are `constexpr` now, so they can't change. Make them normal variables, and give them new values in `StartWave`, based on `wave`. `std::min` and `std::max` keep them within their limits.
:::

2. Every 10 seconds, put a repair kit on a random free spot. When the player drives over it, the player gets half of the maximum health back (but not more than the maximum), and the kit is gone, until the next one appears.

:::tip[Hint]
`RandomSpot` already finds a free spot. A repair kit doesn't need a sprite: a white square with a red cross, drawn with `Bar`, will do.
:::

If you completed the basic assignment, the solution should look like this:

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-16-basic/index.html" title="The arena with a repair kit (a white square with a red cross); the enemies get better every wave" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: enemies that get better every wave, and a repair kit every 10 seconds. (Click the demo first, so that it gets the keys.)

</div>

### Intermediate

1. The enemies aim at where the player *is*, so a player that keeps driving is hard to hit. Let the enemies *lead the target*: aim at the spot where the player will be when the shell gets there.

:::tip[Hint]
The shell needs about $t = d / s$ milliseconds to get to the player, where $d$ is the distance to the player and $s$ is the speed of a shell. In that time, the player moves $\vec{v} t$, where $\vec{v}$ is the velocity of the player's body. So aim at $\vec{p} + \vec{v} t$ instead of $\vec{p}$. (This is an estimate: the distance to that spot is a bit different from $d$. It's close enough.)
:::

2. Add a fifth state: `Flee`. When an enemy's health drops to a quarter or less while it is fighting (in `Chase`, `Attack` or `Search`), it drives away from the player, with its turret still pointing at the player. When it is out of sight range, it goes back to `Patrol`.

:::tip[Hint]
Don't forget to add `"FLEE"` to the state names that <kbd>Tab</kbd> shows. Which state switches to `Flee` doesn't matter, so you can check the health once, after the `switch`.
:::

3. `CanSee` only checks the middle of the player (see the note [Only a Point](#only-a-point)). Make the enemies see the player as soon as *any* part of him sticks out from behind an obstacle.

:::tip[Hint]
Write a second `CanSee` that takes a `Circle` instead of a point (C++ allows two functions with the same name, as long as their arguments are different). The points of the circle that stick out the most to the sides, as seen from the enemy, are a step of the radius away from the middle, at a right angle to the line from the enemy to the middle. Turning a vector $(x, y)$ by 90° gives $(-y, x)$. Check the middle and those two points with the first `CanSee`: if the enemy can see one of them, it sees the player.
:::

If you completed the intermediate assignment, the solution should look like this:

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-16-intermediate/index.html" title="The arena; the enemies aim ahead of the moving player, damaged enemies flee, and the enemies see the player as soon as part of him sticks out from behind an obstacle" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: enemies that lead their target, flee when they are badly damaged, and see you as soon as part of your tank sticks out from behind an obstacle (press <kbd>Tab</kbd> to see their states). (Click the demo first, so that it gets the keys.)

</div>

### Hard

1. An enemy in `Chase` drives straight at the player, so it gets stuck behind crates and trees. Give the enemies *pathfinding*: make a *flow field* over the tiles, with a *breadth-first search* from the player's tile. A chasing enemy drives to the neighbouring tile that is closest to the player.

:::tip[Hint]
A flow field is an array with a number for every tile of the map: the number of steps (up, down, left or right) to the player's tile, going around the objects. The player's tile gets 0; tiles with an object, or that can't be reached, get -1. Fill it like this: put the player's tile in a queue (like the `CellQueue` of [part 13](/cpp-fast-track-13-data-structures/)). Then, as long as the queue isn't empty, take the tile at the front, and look at its four neighbours: each free neighbour that doesn't have a number yet gets the number of the tile plus one, and goes in at the back of the queue. That's all! Because the queue handles the tiles in the order in which they were found, every tile gets the *shortest* number of steps. You only need to make a new flow field when the player moves to another tile. To see whether it works, print the numbers on the tiles when <kbd>Tab</kbd> is pressed.
:::

2. The enemies hit each other when one of them is between another one and the player. Make sure that an enemy only fires when no other enemy is in the way.

:::tip[Hint]
`Crosses` already tells you whether a line crosses a circle. But `Think` is a method of `Tank`, so it doesn't know about the other enemies: give it access to them, for example with an array of pointers to the enemies, which `Init` fills.
:::

If you completed the hard assignment, the solution should look like this:

<iframe class="embed embed-tmpl8 embed-768x512" src="/demos/cpp-fast-track-16-hard/index.html" title="The arena; chasing enemies drive around the obstacles, and pressing Tab shows the flow field numbers on the tiles" loading="lazy" allowfullscreen></iframe>

<div class="caption">

The result: chasing enemies that drive around the obstacles, and don't fire when a friend is in the way (press <kbd>Tab</kbd> to see the flow field). (Click the demo first, so that it gets the keys.)

</div>

**Congratulations on getting here!** You started with a black window and a "Hello world", and now you have built a tank battle with physics and AI, from tiles and sprites to state machines. That's the end of the C++ Fast Track. Keep building games: that's the best way to keep learning.
