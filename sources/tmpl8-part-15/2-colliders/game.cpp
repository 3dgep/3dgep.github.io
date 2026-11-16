#include "game.hpp"

#include <algorithm>
#include <iostream>

namespace Tmpl8
{
    constexpr int TileSize = 64;    // the tiles are 64 x 64 pixels
    constexpr int SheetColumns = 10; // terrain.png has 10 tiles on each row
    constexpr int MapWidth = 12, MapHeight = 8;

    Surface terrain("assets/terrain.png");
    Sprite objectSprites("assets/objects.png", 8, 2);
    Sprite tank("assets/tank.png", 16, 2), barrel("assets/barrel.png", 16, 2);

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

    // The colliders of the objects in the map (filled in by Init).
    constexpr int MaxColliders = MapWidth * MapHeight;
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

    constexpr float TankRadius = 20; // the collider of a tank is a circle

    // Can the tank stand here? Its circle must be inside the map, and it may not overlap any collider.
    bool TankFits(vec2 position)
    {
        const Circle tankCircle = { position, TankRadius };
        if (position.x < TankRadius || position.y < TankRadius ||
            position.x > MapWidth * TileSize - TankRadius || position.y > MapHeight * TileSize - TankRadius)
            return false; // (partly) outside the map
        for (int i = 0; i < circleCount; i++)
            if (Overlaps(tankCircle, circles[i]))
                return false;
        for (int i = 0; i < boxCount; i++)
            if (Overlaps(tankCircle, boxes[i]))
                return false;
        return true;
    }

    constexpr float Acceleration = 0.003f; // how fast a tank speeds up, in pixels per millisecond per millisecond
    constexpr float Drag = 0.02f;          // how fast a tank slows down: this part of its speed is lost every millisecond

    class Tank
    {
    public:
        Tank(vec2 startPosition, int startFrame, int startColor)
        {
            position = startPosition;
            frame = startFrame;
            color = startColor;
        }
        void Move(vec2 direction, float deltaTime);
        void Update(float deltaTime);
        void Draw(Surface& target) const;
        vec2 position;  // the middle of the tank, in pixels
        vec2 velocity;  // in pixels per millisecond
        int frame;      // the direction: 0 is up, 4 is right, 8 is down, 12 is left
        int color;      // the row in tank.png: 0 is green, 1 is red
    };

    // Drives the tank in a direction (a vector with length 1, or 0 to brake), if it fits at its new position.
    void Tank::Move(vec2 direction, float deltaTime)
    {
        const vec2 acceleration = direction * Acceleration - velocity * Drag;
        velocity += acceleration * deltaTime; // first the velocity...
        const vec2 step = velocity * deltaTime; // ...then the position, with the new velocity
        if (TankFits({ position.x + step.x, position.y }))
            position.x += step.x;
        else
            velocity.x = 0;
        if (TankFits({ position.x, position.y + step.y }))
            position.y += step.y;
        else
            velocity.y = 0;
        // face the direction in which the tank drives: one of the 16 frames
        if (velocity.length() > 0.01f)
        {
            const float angle = std::atan2(velocity.x, -velocity.y); // 0 is up, PI / 2 is right
            frame = (static_cast<int>(std::round(angle / (2 * PI) * 16)) + 16) % 16;
        }
    }

    // Drives to the right, and starts again on the left.
    void Tank::Update(float deltaTime)
    {
        position.x += 0.1f * deltaTime;
        if (position.x > MapWidth * TileSize + 32)
            position.x = -32;
    }

    void Tank::Draw(Surface& target) const
    {
        tank.SetFrame(frame, color);
        barrel.SetFrame(frame, color);
        tank.Draw(target, static_cast<int>(position.x) - 32, static_cast<int>(position.y) - 32);
        barrel.Draw(target, static_cast<int>(position.x) - 32, static_cast<int>(position.y) - 32);
    }

    Tank player({ 3 * TileSize + 32, 2 * TileSize + 32 }, 0, 0);
    Tank enemy({ -32, 4 * TileSize + 32 }, 4, 1);
    bool showColliders = false; // Tab shows the colliders

    // -----------------------------------------------------------
    // Initialize the application
    // -----------------------------------------------------------
    void Game::Init()
    {
        // a collider for every solid object in the map
        for (int y = 0; y < MapHeight; y++)
            for (int x = 0; x < MapWidth; x++)
            {
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
        player.Move(direction, deltaTime);
        enemy.Update(deltaTime);

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
            DrawCircle(screen, { player.position, TankRadius }, 0xffff00);
            DrawCircle(screen, { enemy.position, TankRadius }, 0xffff00);
        }
    }
}
