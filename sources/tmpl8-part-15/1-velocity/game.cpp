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

    // Which objects block the tank: trees, crates, sandbags, barrels and barricades do; oil and twigs don't.
    bool solid[16] = {
        true, true, true, true, true, true, true, true,
        true, true, true, true, false, false, false, false
    };

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

    // Can the tank go here? (x, y) is a point in the map, in pixels.
    bool IsFree(float x, float y)
    {
        if (x < 0 || y < 0 || x >= MapWidth * TileSize || y >= MapHeight * TileSize)
            return false; // outside the map
        const int object = objects[static_cast<int>(y) / TileSize][static_cast<int>(x) / TileSize];
        return object == -1 || !solid[object];
    }

    // Can the tank stand here? Checks the four corners of a 36 x 36 box around the middle of the tank.
    bool TankFits(float x, float y)
    {
        constexpr float r = 18;
        return IsFree(x - r, y - r) && IsFree(x + r, y - r) && IsFree(x - r, y + r) && IsFree(x + r, y + r);
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
        if (TankFits(position.x + step.x, position.y))
            position.x += step.x;
        else
            velocity.x = 0;
        if (TankFits(position.x, position.y + step.y))
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

    // -----------------------------------------------------------
    // Initialize the application
    // -----------------------------------------------------------
    void Game::Init()
    {
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
    }
}
