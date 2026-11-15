#include "game.hpp"

#include <iostream>

namespace Tmpl8
{
    constexpr int CellSize = 32;                    // the board is made of cells of 32 x 32 pixels
    constexpr int BoardWidth = 25, BoardHeight = 16; // 25 x 16 cells: the whole screen
    constexpr float StepTime = 100;                 // the snake moves one cell every 100 milliseconds

    Sprite tiles("assets/snake.png", 8, 2);

    struct Cell
    {
        int x, y;
    };

    Cell snake[8] = { { 5, 8 }, { 6, 8 }, { 7, 8 }, { 8, 8 }, { 9, 8 }, { 10, 8 }, { 11, 8 }, { 12, 8 } };
    int heading = 1;  // 0 is up, 1 is right, 2 is down, 3 is left
    int lastStep = 1; // the heading of the last step
    Cell directions[4] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
    float timer = 0;

    // Moves the snake one cell forward: every cell moves to the next one, and the head moves on (at the edge of the
    // board, it comes back on the other side).
    void Step()
    {
        lastStep = heading;
        for (int i = 0; i < 7; i++)
            snake[i] = snake[i + 1];
        snake[7].x = (snake[7].x + directions[heading].x + BoardWidth) % BoardWidth;
        snake[7].y = (snake[7].y + directions[heading].y + BoardHeight) % BoardHeight;
    }

    void DrawCell(Surface& target, int frame, Cell cell)
    {
        tiles.SetFrame(frame);
        tiles.Draw(target, cell.x * CellSize, cell.y * CellSize);
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
        // the keys choose a direction; the snake can only turn to the side (not turn around)
        int wanted = heading;
        if (GetKeyPressed(SDL_SCANCODE_UP) || GetKeyPressed(SDL_SCANCODE_W)) wanted = 0;
        if (GetKeyPressed(SDL_SCANCODE_RIGHT) || GetKeyPressed(SDL_SCANCODE_D)) wanted = 1;
        if (GetKeyPressed(SDL_SCANCODE_DOWN) || GetKeyPressed(SDL_SCANCODE_S)) wanted = 2;
        if (GetKeyPressed(SDL_SCANCODE_LEFT) || GetKeyPressed(SDL_SCANCODE_A)) wanted = 3;
        if (wanted % 2 != lastStep % 2) heading = wanted;
        timer += deltaTime;
        if (timer >= StepTime)
        {
            timer -= StepTime;
            Step();
        }

        // the board: walls around the edge, grass (with a few flowers) inside
        for (int y = 0; y < BoardHeight; y++)
            for (int x = 0; x < BoardWidth; x++)
            {
                const bool wall = x == 0 || y == 0 || x == BoardWidth - 1 || y == BoardHeight - 1;
                DrawCell(screen, wall ? 2 : (x * 7 + y * 13) % 9 == 0 ? 1 : 0, { x, y });
            }
        for (int i = 0; i < 7; i++)
            DrawCell(screen, 4 + i % 2, snake[i]);
        DrawCell(screen, 8 + heading, snake[7]);
    }
}
