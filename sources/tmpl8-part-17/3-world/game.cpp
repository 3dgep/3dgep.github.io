#include "game.hpp"

#include <Audio/Audio.hpp>

#include <algorithm>
#include <format>
#include <iostream>

namespace Tmpl8
{
    constexpr int TileSize = 64;     // the tiles are 64 x 64 pixels
    constexpr int SheetColumns = 10; // terrain.png has 10 tiles on each row
    constexpr int MapWidth = 12, MapHeight = 8;

    Surface terrain("assets/terrain.png");
    Sprite objectSprites("assets/objects.png", 8, 2);
    Sprite tank("assets/tank.png", 16, 2), barrel("assets/barrel.png", 16, 2);
    Sprite drumSprite("assets/drums.png", 4);
    Sprite shellSprite("assets/shells.png", 16, 2), explosionSprite("assets/explosion.png", 5);

    int ground[MapHeight][MapWidth] = {
        {  0,  0, 10,  1,  0,  0, 10,  0,  0,  0, 10,  0 },
        {  2,  2,  2, 12,  2,  2,  2, 14,  0,  0,  0,  0 },
        {  0, 10,  0,  1,  0,  0,  0,  1,  0, 10,  0,  0 },
        {  0,  0,  0,  1,  0, 10,  0,  1,  0,  0,  0, 10 },
        { 18, 18, 18, 38, 18, 18, 18, 38, 18, 18, 18, 18 },
        { 20, 30, 20, 21, 20, 20, 20, 21, 20, 30, 20, 20 },
        { 22, 22, 22, 25, 22, 22, 22, 25, 22, 22, 22, 22 },
        { 20, 20, 30, 20, 20, 30, 20, 20, 20, 20, 30, 20 }
    };

    int objects[MapHeight][MapWidth] = {
        { -1,  0, -1, -1, -1,  1, -1, -1,  2, -1,  0, -1 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1 },
        {  0, -1, 14, -1, -1,  3, -1, -1, -1, -1, -1, -1 },
        { -1,  1, -1, -1, -1, -1, -1, -1,  5,  5, -1,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        {  7, -1, -1, -1, -1,  4, -1, -1, -1, 11, -1,  3 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, -1, -1 },
        { -1, -1, 10, 10, -1, -1,  8, -1, -1, -1,  7, -1 }
    };

    struct Circle
    {
        vec2 center;
        float radius;
    };

    // An axis-aligned bounding box (AABB): a rectangle with horizontal and vertical sides.
    struct Box
    {
        vec2 min, max; // the top-left and the bottom-right corner
    };

    // The shapes of the objects, in pixels within their tile of 64 x 64.
    const Circle BigTree = { { 32, 32 }, 30 }, SmallTrees = { { 32, 31 }, 27 };
    const Box Square = { { 3, 3 }, { 61, 61 } };          // crates and barricades: almost the whole tile
    const Box Barrels = { { 6, 6 }, { 58, 58 } };
    const Box SandbagsAcross = { { 0, 21 }, { 64, 43 } }; // a horizontal row of sandbags
    const Box SandbagsDown = { { 21, 0 }, { 43, 64 } };   // a vertical row

    // The collider of each object, by its number in objects.png. Some objects have the same shape in another colour
    // (like trees 0 and 1), so they get the same collider. Trees get a circle, the other solid objects a box. Oil and
    // twigs (12 to 15) don't block anything: they get neither (a radius of 0, and an empty box).
    Circle objectCircles[16] = { BigTree, BigTree, SmallTrees };
    Box objectBoxes[16] = {
        {}, {}, {},                                           // trees: no box
        Square, Square, SandbagsAcross, SandbagsDown,         // crates and sandbags
        Barrels, Square, Square, SandbagsAcross, SandbagsDown // barrels, barricades and sandbags
    };

    // The colliders of the objects in the map, and four walls around it (filled in by Init).
    constexpr int MaxColliders = MapWidth * MapHeight + 4;
    Circle circles[MaxColliders];
    Box boxes[MaxColliders];
    int circleCount = 0, boxCount = 0;

    bool Overlaps(const Circle& a, const Circle& b)
    {
        const float distance = (a.center - b.center).length();
        return distance < a.radius + b.radius;
    }

    bool Overlaps(const Box& a, const Box& b)
    {
        return a.min.x < b.max.x && b.min.x < a.max.x && a.min.y < b.max.y && b.min.y < a.max.y;
    }

    // The point in the box that is closest to p (p itself, when it is inside the box).
    vec2 ClosestPoint(const Box& box, vec2 p)
    {
        return { std::clamp(p.x, box.min.x, box.max.x), std::clamp(p.y, box.min.y, box.max.y) };
    }

    bool Overlaps(const Circle& circle, const Box& box)
    {
        const float distance = (circle.center - ClosestPoint(box, circle.center)).length();
        return distance < circle.radius;
    }

    // How far circle a must move to stop overlapping circle b: a vector that points away from b, as long as the
    // overlap. A zero vector when they don't overlap.
    vec2 PushOut(const Circle& a, const Circle& b)
    {
        const vec2 difference = a.center - b.center;
        const float distance = difference.length();
        const float overlap = a.radius + b.radius - distance;
        if (overlap <= 0)
            return {};
        if (distance == 0)
            return { overlap, 0 }; // exactly on top of each other: any direction will do
        return difference / distance * overlap;
    }

    // How far box a must move to stop overlapping box b: the shortest of the four ways out (left, right, up or down).
    // A zero vector when they don't overlap.
    vec2 PushOut(const Box& a, const Box& b)
    {
        if (!Overlaps(a, b))
            return {};
        const float left = b.min.x - a.max.x, right = b.max.x - a.min.x; // move a to the left (negative) or right
        const float up = b.min.y - a.max.y, down = b.max.y - a.min.y;    // move a up (negative) or down
        const float x = -left < right ? left : right;                    // the shorter way out on the x axis
        const float y = -up < down ? up : down;                          // and on the y axis
        if (std::abs(x) < std::abs(y))
            return { x, 0 };
        return { 0, y };
    }

    // How far the circle must move to stop overlapping the box (a zero vector when they don't overlap).
    vec2 PushOut(const Circle& circle, const Box& box)
    {
        const vec2 difference = circle.center - ClosestPoint(box, circle.center);
        const float distance = difference.length();
        if (distance >= circle.radius)
            return {};
        if (distance > 0)
            return difference / distance * (circle.radius - distance);
        // the middle of the circle is inside the box: push it out through the nearest side
        const float left = circle.center.x - box.min.x, right = box.max.x - circle.center.x;
        const float top = circle.center.y - box.min.y, bottom = box.max.y - circle.center.y;
        const float nearest = std::min({ left, right, top, bottom });
        if (nearest == left)
            return { -(left + circle.radius), 0 };
        if (nearest == right)
            return { right + circle.radius, 0 };
        if (nearest == top)
            return { 0, -(top + circle.radius) };
        return { 0, bottom + circle.radius };
    }

    // Does the line from a to b cross the circle? Finds the point on the line that is closest to the middle of the
    // circle, like ClosestPoint does for a box.
    bool Crosses(vec2 a, vec2 b, const Circle& circle)
    {
        const vec2 line = b - a;
        const float length2 = line.sqrLength();
        // how far along the line the closest point is: 0 at a, 1 at b
        const float t = length2 > 0 ? std::clamp((circle.center - a).dot(line) / length2, 0.0f, 1.0f) : 0;
        const vec2 closest = a + t * line;
        return (circle.center - closest).length() < circle.radius;
    }

    // Does the line from a to b cross the box? The part of the line between the left and the right side of the box is
    // a range of t (0 at a, 1 at b); so is the part between the top and the bottom. The line crosses the box when the
    // two ranges overlap, somewhere between 0 and 1.
    bool Crosses(vec2 a, vec2 b, const Box& box)
    {
        float first = 0, last = 1; // the range of t that is inside the box on both axes (so far)
        const float start[2] = { a.x, a.y }, step[2] = { b.x - a.x, b.y - a.y };
        const float low[2] = { box.min.x, box.min.y }, high[2] = { box.max.x, box.max.y };
        for (int axis = 0; axis < 2; axis++)
        {
            if (step[axis] == 0)
            {
                // the line runs parallel to these sides: it is between them all the time, or never
                if (start[axis] < low[axis] || start[axis] >= high[axis])
                    return false;
                continue;
            }
            float t1 = (low[axis] - start[axis]) / step[axis], t2 = (high[axis] - start[axis]) / step[axis];
            if (t1 > t2)
                std::swap(t1, t2); // the line runs from high to low on this axis
            first = std::max(first, t1);
            last = std::min(last, t2);
            if (first > last)
                return false; // the ranges don't overlap: the line misses the box
        }
        return true;
    }

    // Draws the outline of a circle, as 32 short lines.
    void DrawCircle(Surface& target, const Circle& circle, Pixel color)
    {
        for (int i = 0; i < 32; i++)
        {
            const float a1 = i * 2 * PI / 32, a2 = (i + 1) * 2 * PI / 32;
            target.Line(circle.center.x + circle.radius * std::cos(a1), circle.center.y + circle.radius * std::sin(a1),
                circle.center.x + circle.radius * std::cos(a2), circle.center.y + circle.radius * std::sin(a2), color);
        }
    }

    void DrawTile(Surface& target, int tile, int x, int y)
    {
        const int column = tile % SheetColumns, row = tile / SheetColumns;
        const Pixel* src = terrain.GetBuffer() + column * TileSize + row * TileSize * terrain.GetPitch();
        Pixel* dst = target.GetBuffer() + x + y * target.GetPitch();
        for (int i = 0; i < TileSize; i++)
        {
            for (int j = 0; j < TileSize; j++)
                dst[j] = src[j];
            src += terrain.GetPitch();
            dst += target.GetPitch();
        }
    }

    // A body: a circle that moves, and bumps into colliders and other bodies.
    struct Body
    {
        vec2 position, velocity; // in pixels, and pixels per millisecond
        vec2 previous;           // the position before the last step
        vec2 drive;              // the acceleration that the body gives itself (the engine of a tank)
        float drag = 0;          // the part of its velocity that the body loses every millisecond
        float radius = 0;
        float inverseMass = 0;   // 1 / mass: a heavy body has a small inverse mass
    };

    // Moves a body out of all colliders: the colliders don't move.
    void Collide(Body& body)
    {
        for (int i = 0; i < circleCount; i++)
            body.position += PushOut(Circle{ body.position, body.radius }, circles[i]);
        for (int i = 0; i < boxCount; i++)
            body.position += PushOut(Circle{ body.position, body.radius }, boxes[i]);
    }

    // Moves two overlapping bodies apart. The lighter body moves the most: each moves a part of the overlap that
    // matches its inverse mass.
    void Separate(Body& a, Body& b)
    {
        const vec2 push = PushOut(Circle{ a.position, a.radius }, Circle{ b.position, b.radius });
        const float total = a.inverseMass + b.inverseMass;
        if (total == 0)
            return; // two bodies that can't move
        a.position += push * (a.inverseMass / total);
        b.position -= push * (b.inverseMass / total);
    }

    // The audio world is 3D, and its y axis points up. The map lies flat in it: the x of the screen is its x, and the
    // y of the screen is its z. The listener (the ears of the game) hovers ListenerHeight pixels above the map.
    constexpr float ListenerHeight = 200;

    // A position on the map (and a height above it) as a position in the audio world.
    Audio::Vector ToAudio(vec2 position, float height = 0)
    {
        return { position.x, height, position.y };
    }

    // The frame (0 to 15) that faces a direction: 0 is up, 4 is right, 8 is down, 12 is left.
    int FrameOf(vec2 direction)
    {
        const float angle = std::atan2(direction.x, -direction.y); // 0 is up, PI / 2 is right
        return (static_cast<int>(std::round(angle / (2 * PI) * 16)) + 16) % 16;
    }

    // The vector v, turned by an angle (in radians, clockwise on the screen, because y points down).
    vec2 Rotate(vec2 v, float angle)
    {
        const float c = std::cos(angle), s = std::sin(angle);
        return { v.x * c - v.y * s, v.x * s + v.y * c };
    }

    // Turns the direction from towards the direction to, but at most by maxAngle (both directions have length 1).
    vec2 TurnTowards(vec2 from, vec2 to, float maxAngle)
    {
        const float cross = from.x * to.y - from.y * to.x; // positive: to is clockwise from from
        const float angle = std::atan2(cross, from.dot(to)); // the angle from from to to, between -PI and PI
        return Rotate(from, std::clamp(angle, -maxAngle, maxAngle));
    }

    constexpr float Acceleration = 0.003f; // how fast a tank speeds up, in pixels per millisecond per millisecond
    constexpr float TankDrag = 0.02f;      // how fast a tank slows down: this part of its speed is lost every millisecond
    constexpr float TankMass = 10;         // a tank is ten times as heavy as a drum
    constexpr int MaxHealth = 100;
    constexpr float ReloadTime = 400;      // milliseconds between two shots
    constexpr float ReactionTime = 400;    // how long an enemy must see the player before it reacts, in milliseconds
    constexpr float TurretSpeed = 0.003f;  // how fast an enemy turns its turret, in radians per millisecond
    constexpr float Spread = 0.06f;        // how far an enemy's shell can be off, in radians (to both sides)
    constexpr float AttackRange = 250;     // closer than this, an enemy stops to fire; further away, it chases
    constexpr float SearchTime = 3000;     // how long an enemy looks around where it lost the player, in milliseconds

    // What an enemy is doing.
    enum class State
    {
        Patrol, // driving to random spots, until it sees the player
        Chase,  // driving towards the player, who is too far away to attack
        Attack, // standing still, and firing at the player
        Search  // driving to where it lost the player, and looking around there
    };

    class Tank
    {
    public:
        Tank(vec2 startPosition = {}, int startFrame = 0, int startColor = 1)
        {
            body.position = startPosition;
            body.drag = TankDrag;
            body.radius = 20;
            body.inverseMass = 1 / TankMass;
            frame = startFrame;
            color = startColor;
        }
        void Drive(vec2 direction);
        void Aim(vec2 target);
        void Fire(float spread = 0);
        void Think(const Tank& target, float deltaTime);
        void Draw(Surface& target) const;
        Body body;
        int frame;               // the direction: 0 is up, 4 is right, 8 is down, 12 is left
        int color;               // the row in tank.png: 0 is green, 1 is red
        vec2 aim = { 0, -1 };    // the direction of the barrel, with length 1
        float reload = 0;        // milliseconds until the tank can fire again
        int health = MaxHealth;
        // for the enemy only:
        State state = State::Patrol;
        vec2 goal;               // where it drives to: a random spot (Patrol), or where it saw the player (Search)
        float noticed = 0;       // how long it has seen the player, in milliseconds
        float timer = 0;         // milliseconds: how long it has been stuck (Patrol), or has searched (Search)
    };

    // Drives the tank in a direction (a vector with length 1, or 0 to brake).
    void Tank::Drive(vec2 direction)
    {
        body.drive = direction * Acceleration;
        // face the direction in which the tank drives: one of the 16 frames
        if (body.velocity.length() > 0.01f)
            frame = FrameOf(body.velocity);
    }

    // Turns the barrel towards a target: a position on the screen.
    void Tank::Aim(vec2 target)
    {
        const vec2 difference = target - body.position;
        const float length = difference.length(); // (one square root, for both the test and the normalization)
        if (length > 0)
            aim = difference / length;
    }

    void Tank::Draw(Surface& target) const
    {
        tank.SetFrame(frame, color);
        barrel.SetFrame(FrameOf(aim), color);
        const int x = static_cast<int>(body.position.x), y = static_cast<int>(body.position.y);
        tank.Draw(target, x - 32, y - 32);
        barrel.Draw(target, x - 32, y - 32);
        // the health bar, above the tank: green for the health that is left, dark for the rest. Bar doesn't check the
        // edges of the screen, so near the top of the map, the bar goes below the tank instead.
        const int width = 32 * health / MaxHealth;
        const int top = y >= 34 ? y - 34 : y + 28;
        target.Bar(x - 16, top, x + 15, top + 3, 0x202020);
        if (width > 0)
            target.Bar(x - 16, top, x - 17 + width, top + 3, health > MaxHealth / 3 ? 0x40e040 : 0xe04040);
    }

    constexpr float SightRange = 350;    // how far a tank can see, in pixels
    constexpr float FieldOfView = 0.5f;  // the cosine of half the field of view: 0.5 is 60 degrees to each side

    // Can the tank see the point? It must be close enough, in front of the turret (inside the field of view), and no
    // obstacle may block the line from the tank to the point.
    bool CanSee(const Tank& tank, vec2 point)
    {
        const vec2 difference = point - tank.body.position;
        const float distance = difference.length();
        if (distance > SightRange)
            return false; // too far away
        if (distance > 0 && tank.aim.dot(difference / distance) < FieldOfView)
            return false; // outside the field of view
        for (int i = 0; i < circleCount; i++)
            if (Crosses(tank.body.position, point, circles[i]))
                return false;
        for (int i = 0; i < boxCount; i++)
            if (Crosses(tank.body.position, point, boxes[i]))
                return false;
        return true;
    }

    // A random spot in the middle of a tile without an object: a goal for an enemy on patrol.
    vec2 RandomSpot()
    {
        while (true)
        {
            const int x = IRand(MapWidth), y = IRand(MapHeight);
            if (objects[y][x] == -1)
                return { x * TileSize + 32.0f, y * TileSize + 32.0f };
        }
    }

    // The enemy's brain: a state machine. In each state, the enemy drives and aims in its own way, and it checks whether
    // it should switch to another state.
    void Tank::Think(const Tank& target, float deltaTime)
    {
        // what the enemy knows: whether it sees the target (long enough to react), and how far away the target and its goal are
        if (CanSee(*this, target.body.position))
            noticed += deltaTime;
        else
            noticed = 0;
        const bool seen = noticed > ReactionTime;
        const vec2 toTarget = target.body.position - body.position;
        const float distance = toTarget.length();
        const vec2 toGoal = goal - body.position;
        const float goalDistance = toGoal.length();
        vec2 direction;           // where to drive (zero: brake)
        vec2 look = aim;          // where to point the turret
        switch (state)
        {
        case State::Patrol:
            if (goalDistance > 40)
                direction = toGoal / goalDistance;
            look = direction;
            // a new goal when it got there, or when it is stuck (behind an obstacle) for a second
            timer = body.velocity.length() < 0.02f ? timer + deltaTime : 0;
            if (goalDistance <= 40 || timer > 1000)
            {
                goal = RandomSpot();
                timer = 0;
            }
            if (seen)
                state = State::Chase; // (Chase switches to Attack when the target is close)
            break;
        case State::Chase:
            direction = toTarget / distance;
            look = direction;
            if (!seen)
            {
                state = State::Search;
                timer = 0;
            }
            else if (distance < AttackRange)
                state = State::Attack;
            break;
        case State::Attack:
            look = toTarget / distance; // stand still, and aim at the target
            if (!seen)
            {
                state = State::Search;
                timer = 0;
            }
            else if (distance > AttackRange * 1.2f)
                state = State::Chase; // (a bit further than AttackRange, so that it doesn't switch back and forth)
            else if (aim.dot(look) > 0.98f)
                Fire(Spread); // aimed well enough at the target: fire!
            break;
        case State::Search:
            if (goalDistance > 40)
                direction = toGoal / goalDistance;               // drive to where it saw the target...
            look = Rotate(aim, 0.002f * deltaTime);              // ...and look around, by turning the turret
            timer += deltaTime;
            if (seen)
                state = State::Chase;
            else if (timer > SearchTime)
            {
                state = State::Patrol;
                goal = RandomSpot();
                timer = 0;
            }
            break;
        }
        if (seen)
            goal = target.body.position; // remember where it saw the target last, for Search
        Drive(direction);
        aim = TurnTowards(aim, look, TurretSpeed * deltaTime);
        reload -= deltaTime;
    }

    Tank player({ 3 * TileSize + 32, 2 * TileSize + 32 }, 0, 0);

    // The enemies of the arena. Wave 1 has 1 enemy, wave 2 has 2, and so on, up to MaxEnemies. An enemy that isn't in
    // the battle (destroyed, or not needed in this wave) waits far outside the map, where it's never in the way.
    constexpr int MaxEnemies = 5;
    Tank enemies[MaxEnemies];
    int wave = 0, score = 0;
    float waveTimer = 0;   // milliseconds until the next wave starts (when all enemies are destroyed)
    bool gameOver = false;

    // The drums: the barrels of objects.png (tile 7) as loose bodies. Each tile of barrels becomes 4 drums.
    constexpr int MaxDrums = 16;
    Body drums[MaxDrums];
    int drumColors[MaxDrums]; // the frame in drums.png: green, red, rust or black
    int drumCount = 0;

    // All bodies that bump into each other: the tanks and the drums (filled in by Init).
    Body* bodies[1 + MaxEnemies + MaxDrums];
    int bodyCount = 0;

    // A sound effect that can be heard several times at once, like when two tanks fire at the same moment. A copy of an
    // Audio::Sound is the same sound, so an effect has a few sounds of its own (its "voices"), which take turns.
    struct Effect
    {
        static constexpr int Voices = 6;
        Audio::Sound voices[Voices];
        int next = 0; // the voice that plays next

        void Load(const std::string& name, int variations = 1, float volume = 1);
        void Play(vec2 position);
    };

    // Loads the voices from assets/sounds/name.ogg, or, with variations, from name1.ogg, name2.ogg, ... in turn.
    void Effect::Load(const std::string& name, int variations, float volume)
    {
        for (int i = 0; i < Voices; i++)
        {
            const std::string file = variations > 1
                ? std::format("assets/sounds/{}{}.ogg", name, i % variations + 1)
                : std::format("assets/sounds/{}.ogg", name);
            voices[i].load(file);
            voices[i].setVolume(volume);
            voices[i].setMinDistance(ListenerHeight); // closer than this, a voice plays at its full volume
        }
    }

    // Plays the next voice from the start, at a position on the map, a little higher or lower each time.
    void Effect::Play(vec2 position)
    {
        Audio::Sound& voice = voices[next];
        next = (next + 1) % Voices;
        voice.setPosition(ToAudio(position));
        voice.setPitch(0.9f + Rand(0.2f)); // between 0.9 and 1.1
        voice.replay();
    }

    Effect fireSound, explosionSound, hitSound, destroyedSound;

    // Shells fly in a straight line, until they hit something or their time is up. They are not bodies: they are too
    // fast and too small for the physics, and they disappear when they hit something anyway.
    struct Shell
    {
        vec2 position, velocity;
        float time = 0;   // milliseconds left; 0 when the shell isn't flying
        int color;        // the row in shells.png: 0 is green, 1 is red
    };

    constexpr int MaxShells = 32;
    constexpr float ShellSpeed = 0.5f, ShellRadius = 4, ShellTime = 1500;
    constexpr int ShellDamage = 20;
    Shell shells[MaxShells];

    // Explode is further down, with the rest of the explosion code. C++ reads the code from top to bottom, so we need to
    // declare Explode here: that tells the compiler that the function exists, before UpdateShells uses it.
    void Explode(vec2 position);

    // Moves the shells, and checks what they hit: a tank loses health, a drum gets a push, and an obstacle stops them.
    void UpdateShells(float deltaTime, Tank* tanks[], int tankCount)
    {
        for (int i = 0; i < MaxShells; i++)
        {
            Shell& shell = shells[i]; // a reference: changing shell changes shells[i]
            if (shell.time <= 0)
                continue;             // this shell isn't flying: on to the next one

            // the shell flies on in a straight line, and has a bit less time left
            shell.time -= deltaTime;
            shell.position += shell.velocity * deltaTime;

            // What does the shell hit?
            const Circle circle = { shell.position, ShellRadius };
            bool hit = false;
            for (int t = 0; t < tankCount; t++)
            {
                Tank& tank = *tanks[t]; // tanks[t] is a pointer so we need to use * to dereference it
                if (tank.health > 0 && Overlaps(circle, Circle{ tank.body.position, tank.body.radius }))
                {
                    tank.health -= ShellDamage; // a tank that is hit loses health (a destroyed tank can't be hit)
                    hit = true;
                    hitSound.Play(shell.position);
                }
            }
            for (int d = 0; d < drumCount; d++)
            {
                Body& drum = drums[d];
                if (Overlaps(circle, Circle{ drum.position, drum.radius }))
                {
                    drum.velocity += shell.velocity * 0.5f; // a light drum gets a good push
                    hit = true;
                }
            }
            // the obstacles of the map (trees, crates, walls, ...) simply stop the shell
            for (int c = 0; c < circleCount; c++)
                if (Overlaps(circle, circles[c]))
                    hit = true;
            for (int b = 0; b < boxCount; b++)
                if (Overlaps(circle, boxes[b]))
                    hit = true;

            // a shell that hit something explodes, and is free again for the next shot
            if (hit)
            {
                Explode(shell.position);
                shell.time = 0;
                explosionSound.Play(shell.position);
            }
        }
    }

    // Draws the flying shells: each one facing the direction in which it flies, in the colour of its tank.
    void DrawShells(Surface& target)
    {
        for (int i = 0; i < MaxShells; i++)
        {
            const Shell& shell = shells[i];
            if (shell.time > 0)
            {
                shellSprite.SetFrame(FrameOf(shell.velocity), shell.color);
                shellSprite.Draw(target, static_cast<int>(shell.position.x) - 12, static_cast<int>(shell.position.y) - 12);
            }
        }
    }

    // An explosion is just an animation: 5 frames of explosion.png.
    struct Explosion
    {
        vec2 position;
        float time = 0;   // milliseconds left; 0 when it is over
    };

    constexpr int MaxExplosions = 16;
    constexpr float ExplosionTime = 400;
    Explosion explosions[MaxExplosions];

    // Starts an explosion at a position, in a free spot of the array (if there is one).
    void Explode(vec2 position)
    {
        for (int i = 0; i < MaxExplosions; i++)
            if (explosions[i].time <= 0)
            {
                explosions[i] = { position, ExplosionTime };
                return;
            }
    }

    // The explosions get older: an explosion with no time left is over, and its spot in the array is free again.
    void UpdateExplosions(float deltaTime)
    {
        for (int i = 0; i < MaxExplosions; i++)
            explosions[i].time -= deltaTime;
    }

    // Draws the explosions that aren't over yet.
    void DrawExplosions(Surface& target)
    {
        for (int i = 0; i < MaxExplosions; i++)
        {
            const Explosion& explosion = explosions[i];
            if (explosion.time > 0)
            {
                // the first frame when the explosion starts, the last one when it ends
                explosionSprite.SetFrame(static_cast<int>((1 - explosion.time / ExplosionTime) * 5));
                explosionSprite.Draw(target, static_cast<int>(explosion.position.x) - 32, static_cast<int>(explosion.position.y) - 32);
            }
        }
    }

    // Fires a shell from the end of the barrel, if the tank has reloaded. With a spread, the shell flies in a random
    // direction that is at most spread radians off.
    void Tank::Fire(float spread)
    {
        if (reload > 0)
            return;
        for (int i = 0; i < MaxShells; i++)
            if (shells[i].time <= 0)
            {
                const vec2 direction = Rotate(aim, Rand(2 * spread) - spread);
                shells[i] = { body.position + aim * 30, direction * ShellSpeed, ShellTime, color };
                reload = ReloadTime;
                fireSound.Play(body.position);
                return;
            }
    }

    // Moves a body during a (sub)step: it remembers where it was, its drive and drag change its velocity, and its
    // velocity changes its position.
    void Integrate(Body& body, float h)
    {
        body.previous = body.position;
        body.velocity += (body.drive - body.velocity * body.drag) * h;
        body.position += body.velocity * h;
    }

    constexpr int Substeps = 8;

    // The physics of one frame, in a number of small steps. In each step, every body moves; the overlaps are solved by
    // moving the bodies; and then the velocity of each body follows from how far it really moved.
    void Simulate(float deltaTime)
    {
        const float h = deltaTime / Substeps;
        for (int step = 0; step < Substeps; step++)
        {
            for (int i = 0; i < bodyCount; i++)
                Integrate(*bodies[i], h);

            for (int i = 0; i < bodyCount; i++)
                for (int j = i + 1; j < bodyCount; j++)
                    Separate(*bodies[i], *bodies[j]);
            for (int i = 0; i < bodyCount; i++)
                Collide(*bodies[i]);

            for (int i = 0; i < bodyCount; i++)
                bodies[i]->velocity = (bodies[i]->position - bodies[i]->previous) / h;
        }
    }

    // Hides an enemy far outside the map: each one at its own spot, so that they don't push each other around.
    void Hide(Tank& enemy, int i)
    {
        enemy.body.position = enemy.body.previous = { -(i + 1) * 500.0f, -(i + 1) * 500.0f };
        enemy.body.velocity = {};
        enemy.body.drive = {};
        enemy.health = 0;
    }

    // Starts the next wave: puts its enemies on random spots, not too close to the player.
    void StartWave()
    {
        wave++;
        for (int i = 0; i < MaxEnemies; i++)
        {
            if (i >= wave)
            {
                Hide(enemies[i], i);
                continue;
            }
            vec2 spot;
            do
                spot = RandomSpot();
            while ((spot - player.body.position).length() < 300);
            enemies[i].body.position = enemies[i].body.previous = spot;
            enemies[i].body.velocity = {};
            enemies[i].health = MaxHealth;
            enemies[i].state = State::Patrol;
            enemies[i].goal = RandomSpot();
            enemies[i].noticed = 0;
            enemies[i].reload = ReloadTime;
        }
    }

    // A new battle: the player starts over, as good as new, and the first wave starts.
    void Restart()
    {
        player.body.position = player.body.previous = { 3 * TileSize + 32, 2 * TileSize + 32 };
        player.body.velocity = {};
        player.health = MaxHealth;
        wave = score = 0;
        gameOver = false;
        for (int i = 0; i < MaxShells; i++)
            shells[i].time = 0;
        StartWave();
    }

    bool showColliders = false; // Tab shows the colliders

    // -----------------------------------------------------------
    // Initialize the application
    // -----------------------------------------------------------
    void Game::Init()
    {
        // a collider for every solid object in the map; the barrels become drums
        for (int y = 0; y < MapHeight; y++)
            for (int x = 0; x < MapWidth; x++)
            {
                if (objects[y][x] == 7)
                {
                    const vec2 offsets[4] = { { 18, 18 }, { 46, 18 }, { 18, 46 }, { 46, 46 } };
                    for (int i = 0; i < 4; i++)
                    {
                        Body& drum = drums[drumCount];
                        drum.position = vec2(static_cast<float>(x * TileSize), static_cast<float>(y * TileSize)) + offsets[i];
                        drum.drag = 0.004f; // drums slide further than tanks
                        drum.radius = 12;
                        drum.inverseMass = 1;
                        drumColors[drumCount++] = i;
                    }
                    objects[y][x] = -1; // the tile itself is empty now
                }
                if (objects[y][x] == -1)
                    continue;
                const vec2 tile = { static_cast<float>(x * TileSize), static_cast<float>(y * TileSize) };
                const Circle circle = objectCircles[objects[y][x]];
                const Box box = objectBoxes[objects[y][x]];
                if (circle.radius > 0)
                    circles[circleCount++] = { tile + circle.center, circle.radius };
                else if (box.max.x > box.min.x)
                    boxes[boxCount++] = { tile + box.min, tile + box.max };
            }
        // four walls around the map, so that the tanks stay on it
        constexpr float W = MapWidth * TileSize, H = MapHeight * TileSize;
        boxes[boxCount++] = { { -100, -100 }, { W + 100, 0 } };    // above the map
        boxes[boxCount++] = { { -100, H }, { W + 100, H + 100 } }; // below
        boxes[boxCount++] = { { -100, 0 }, { 0, H } };             // to the left
        boxes[boxCount++] = { { W, 0 }, { W + 100, H } };          // to the right

        bodies[bodyCount++] = &player.body;
        for (int i = 0; i < MaxEnemies; i++)
            bodies[bodyCount++] = &enemies[i].body;
        for (int i = 0; i < drumCount; i++)
            bodies[bodyCount++] = &drums[i];

        // the sound effects: the shots, the explosions and the hits have 3 variations each
        fireSound.Load("fire", 3, 0.6f);
        explosionSound.Load("explosion", 3, 0.5f);
        hitSound.Load("hit", 3);
        destroyedSound.Load("destroyed");
        Restart();
    }

    // -----------------------------------------------------------
    // Close down application
    // -----------------------------------------------------------
    void Game::Shutdown()
    {
    }

    // -----------------------------------------------------------
    // Main application tick function
    // -----------------------------------------------------------
    void Game::Tick(float deltaTime)
    {
        deltaTime = std::min(deltaTime, 33.0f); // a very long frame (when you drag the window) is too big a step
        screen.Clear(0);
        for (int y = 0; y < MapHeight; y++)
            for (int x = 0; x < MapWidth; x++)
            {
                DrawTile(screen, ground[y][x], x * TileSize, y * TileSize);
                if (objects[y][x] != -1)
                {
                    objectSprites.SetFrame(objects[y][x]);
                    objectSprites.Draw(screen, x * TileSize, y * TileSize);
                }
            }

        vec2 direction;
        if (GetKey(SDL_SCANCODE_UP) || GetKey(SDL_SCANCODE_W))
            direction.y -= 1;
        if (GetKey(SDL_SCANCODE_RIGHT) || GetKey(SDL_SCANCODE_D))
            direction.x += 1;
        if (GetKey(SDL_SCANCODE_DOWN) || GetKey(SDL_SCANCODE_S))
            direction.y += 1;
        if (GetKey(SDL_SCANCODE_LEFT) || GetKey(SDL_SCANCODE_A))
            direction.x -= 1;
        if (direction.length() > 0)
            direction.normalize(); // diagonally, the tank isn't faster
        if (gameOver)
            direction = {}; // a destroyed tank doesn't drive
        player.Drive(direction);
        player.Aim({ static_cast<float>(mousex), static_cast<float>(mousey) });
        player.reload -= deltaTime;
        if ((mouseDown || mousePressed) && !gameOver)
            player.Fire(); // click, or hold the left mouse button to keep firing
        mousePressed = false;
        for (int i = 0; i < MaxEnemies; i++)
            if (enemies[i].health > 0)
                enemies[i].Think(player, deltaTime);
        Simulate(deltaTime);
        // the listener hovers above the player (but stays where it is when the player is gone)
        if (!gameOver)
            Audio::getListener().setPosition(ToAudio(player.body.position, ListenerHeight));
        Tank* tanks[1 + MaxEnemies] = { &player };
        for (int i = 0; i < MaxEnemies; i++)
            tanks[1 + i] = &enemies[i];
        UpdateShells(deltaTime, tanks, 1 + MaxEnemies);
        UpdateExplosions(deltaTime);

        // destroyed tanks: the player's game is over; an enemy explodes and leaves the battle
        if (player.health <= 0 && !gameOver)
        {
            Explode(player.body.position);
            destroyedSound.Play(player.body.position);
            Hide(player, MaxEnemies); // out of sight, so that the enemies stop firing at the wreck
            gameOver = true;
        }
        int remaining = 0; // the number of enemies that are still in the battle
        for (int i = 0; i < MaxEnemies; i++)
        {
            if (enemies[i].health > 0)
                remaining++;
            else if (enemies[i].body.position.x > 0) // destroyed, but not hidden yet
            {
                Explode(enemies[i].body.position);
                destroyedSound.Play(enemies[i].body.position);
                Hide(enemies[i], i);
                score++;
            }
        }
        // when all enemies of a wave are destroyed, the next wave comes after 2 seconds
        if (remaining == 0 && !gameOver)
        {
            waveTimer += deltaTime;
            if (waveTimer > 2000)
            {
                waveTimer = 0;
                StartWave();
                remaining = std::min(wave, MaxEnemies); // the new enemies are in the battle now
            }
        }
        if (gameOver && GetKeyPressed(KeyFor(SDLK_R)))
            Restart();

        for (int i = 0; i < drumCount; i++)
        {
            drumSprite.SetFrame(drumColors[i]);
            drumSprite.Draw(screen, static_cast<int>(drums[i].position.x) - 12, static_cast<int>(drums[i].position.y) - 12);
        }
        if (!gameOver)
            player.Draw(screen);
        for (int i = 0; i < MaxEnemies; i++)
            if (enemies[i].health > 0)
                enemies[i].Draw(screen);
        DrawShells(screen);
        DrawExplosions(screen);

        if (GetKeyPressed(SDL_SCANCODE_TAB))
            showColliders = !showColliders;
        if (showColliders)
        {
            for (int i = 0; i < circleCount; i++)
                DrawCircle(screen, circles[i], 0xffffff);
            for (int i = 0; i < boxCount; i++)
                screen.Box(static_cast<int>(boxes[i].min.x), static_cast<int>(boxes[i].min.y),
                    static_cast<int>(boxes[i].max.x) - 1, static_cast<int>(boxes[i].max.y) - 1, 0xffffff);
            for (int i = 0; i < bodyCount; i++)
                DrawCircle(screen, { bodies[i]->position, bodies[i]->radius }, 0xffff00);
            for (int i = 0; i < MaxShells; i++)
            {
                const Shell& shell = shells[i];
                if (shell.time > 0)
                    DrawCircle(screen, { shell.position, ShellRadius }, 0xffff00); // the flying shells too
            }
            // what each enemy sees: its view cone (the two edges of its field of view), the line to the player, its
            // state, and its goal
            const float half = std::acos(FieldOfView); // half the field of view, in radians
            for (int i = 0; i < MaxEnemies; i++)
            {
                const Tank& enemy = enemies[i];
                if (enemy.health <= 0)
                    continue;
                const vec2 e = enemy.body.position, p = player.body.position;
                const vec2 left = Rotate(enemy.aim, -half) * SightRange, right = Rotate(enemy.aim, half) * SightRange;
                screen.Line(e.x, e.y, e.x + left.x, e.y + left.y, 0xffffff);
                screen.Line(e.x, e.y, e.x + right.x, e.y + right.y, 0xffffff);
                screen.Line(e.x, e.y, p.x, p.y, CanSee(enemy, p) ? 0x40e040 : 0xe04040);
                const char* names[4] = { "PATROL", "CHASE", "ATTACK", "SEARCH" };
                screen.Print(names[static_cast<int>(enemy.state)], static_cast<int>(e.x) - 16, static_cast<int>(e.y) - 44, 0xffffff);
                DrawCircle(screen, { enemy.goal, 8 }, 0xff40ff); // its goal (a random spot, or where it saw the player)
            }
        }

        // the status line at the top, and the end of the game
        screen.Bar(0, 0, ScreenWidth - 1, 15, 0x202020);
        screen.Print(std::format("WAVE {}   SCORE {}   HEALTH {}", wave, score, std::max(player.health, 0)), 4, 5, 0xffffff);
        if (gameOver)
            screen.Centre(std::format("GAME OVER: {} ENEMIES DESTROYED. PRESS R TO PLAY AGAIN", score), ScreenHeight / 2, 0xffffff);
        else if (remaining == 0)
            screen.Centre(std::format("WAVE {} IS COMING", wave + 1), ScreenHeight / 2, 0xffffff);
    }
}
