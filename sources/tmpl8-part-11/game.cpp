#include "game.hpp"

#include <iostream>

namespace Tmpl8
{
    constexpr int TileSize = 64;    // the tiles are 64 x 64 pixels
    constexpr int SheetColumns = 10; // terrain.png has 10 tiles on each row
    constexpr int MapWidth = 12, MapHeight = 8;

    Surface terrain("assets/terrain.png");
    Sprite objectSprites("assets/objects.png", 8, 2);
    Sprite tank("assets/tank.png", 16), barrel("assets/barrel.png", 16);

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
        { -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  1 },
        {  0, -1, 14, -1, -1,  3, -1, -1, -1, -1,  2, -1 },
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

    float tankX = 3 * TileSize + 32, tankY = 2 * TileSize + 32; // the middle of the tank
    int tankFrame = 0;

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

        const float speed = 0.15f * deltaTime; // pixels per millisecond
        float newX = tankX, newY = tankY;
        if (GetKey(SDL_SCANCODE_UP) || GetKey(SDL_SCANCODE_W)) newY -= speed, tankFrame = 0;
        if (GetKey(SDL_SCANCODE_RIGHT) || GetKey(SDL_SCANCODE_D)) newX += speed, tankFrame = 4;
        if (GetKey(SDL_SCANCODE_DOWN) || GetKey(SDL_SCANCODE_S)) newY += speed, tankFrame = 8;
        if (GetKey(SDL_SCANCODE_LEFT) || GetKey(SDL_SCANCODE_A)) newX -= speed, tankFrame = 12;
        if (TankFits(newX, tankY)) tankX = newX;
        if (TankFits(tankX, newY)) tankY = newY;

        tank.SetFrame(tankFrame);
        barrel.SetFrame(tankFrame);
        tank.Draw(screen, static_cast<int>(tankX) - 32, static_cast<int>(tankY) - 32);
        barrel.Draw(screen, static_cast<int>(tankX) - 32, static_cast<int>(tankY) - 32);
    }
}
