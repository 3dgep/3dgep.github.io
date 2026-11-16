#include "game.hpp"

#include <algorithm>
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

    constexpr float Acceleration = 0.003f; // how fast a tank speeds up, in pixels per millisecond per millisecond
    constexpr float TankDrag = 0.02f;      // how fast a tank slows down: this part of its speed is lost every millisecond
    constexpr float TankMass = 10;         // a tank is ten times as heavy as a drum

    class Tank
    {
    public:
        Tank(vec2 startPosition, int startFrame, int startColor)
        {
            body.position = startPosition;
            body.drag = TankDrag;
            body.radius = 20;
            body.inverseMass = 1 / TankMass;
            frame = startFrame;
            color = startColor;
        }
        void Drive(vec2 direction);
        void Update();
        void Draw(Surface& target) const;
        Body body;
        int frame;      // the direction: 0 is up, 4 is right, 8 is down, 12 is left
        int color;      // the row in tank.png: 0 is green, 1 is red
        float patrol = 1; // for the enemy: 1 drives to the right, -1 to the left
    };

    // Drives the tank in a direction (a vector with length 1, or 0 to brake).
    void Tank::Drive(vec2 direction)
    {
        body.drive = direction * Acceleration;
        // face the direction in which the tank drives: one of the 16 frames
        if (body.velocity.length() > 0.01f)
        {
            const float angle = std::atan2(body.velocity.x, -body.velocity.y); // 0 is up, PI / 2 is right
            frame = (static_cast<int>(std::round(angle / (2 * PI) * 16)) + 16) % 16;
        }
    }

    // The enemy drives to the right and back; it turns around when it is blocked.
    void Tank::Update()
    {
        if (body.velocity.x * patrol < 0.001f)
            patrol = -patrol; // (almost) not moving in the patrol direction
        Drive({ patrol, 0 });
    }

    void Tank::Draw(Surface& target) const
    {
        tank.SetFrame(frame, color);
        barrel.SetFrame(frame, color);
        tank.Draw(target, static_cast<int>(body.position.x) - 32, static_cast<int>(body.position.y) - 32);
        barrel.Draw(target, static_cast<int>(body.position.x) - 32, static_cast<int>(body.position.y) - 32);
    }

    Tank player({ 3 * TileSize + 32, 2 * TileSize + 32 }, 0, 0);
    Tank enemy({ 32, 4 * TileSize + 32 }, 4, 1);

    // The drums: the barrels of objects.png (tile 7) as loose bodies. Each tile of barrels becomes 4 drums.
    constexpr int MaxDrums = 16;
    Body drums[MaxDrums];
    int drumColors[MaxDrums]; // the frame in drums.png: green, red, rust or black
    int drumCount = 0;

    // All bodies that bump into each other: the tanks and the drums (filled in by Init).
    Body* bodies[2 + MaxDrums];
    int bodyCount = 0;

    // One step of the physics: every body moves, the overlaps are solved by moving the bodies, and then the
    // velocity of each body follows from how far it really moved.
    void Simulate(float deltaTime)
    {
        for (int i = 0; i < bodyCount; i++)
        {
            Body& body = *bodies[i];
            body.previous = body.position;
            body.velocity += (body.drive - body.velocity * body.drag) * deltaTime;
            body.position += body.velocity * deltaTime;
        }
        for (int iteration = 0; iteration < 4; iteration++) // solving one overlap can cause another: repeat
        {
            for (int i = 0; i < bodyCount; i++)
                for (int j = i + 1; j < bodyCount; j++)
                    Separate(*bodies[i], *bodies[j]);
            for (int i = 0; i < bodyCount; i++)
                Collide(*bodies[i]);
        }
        for (int i = 0; i < bodyCount; i++)
            bodies[i]->velocity = (bodies[i]->position - bodies[i]->previous) / deltaTime;
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
        bodies[bodyCount++] = &enemy.body;
        for (int i = 0; i < drumCount; i++)
            bodies[bodyCount++] = &drums[i];
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
        player.Drive(direction);
        enemy.Update();
        Simulate(deltaTime);

        for (int i = 0; i < drumCount; i++)
        {
            drumSprite.SetFrame(drumColors[i]);
            drumSprite.Draw(screen, static_cast<int>(drums[i].position.x) - 12, static_cast<int>(drums[i].position.y) - 12);
        }
        player.Draw(screen);
        enemy.Draw(screen);

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
        }
    }
}
