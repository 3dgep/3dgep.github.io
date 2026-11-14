#include "game.hpp"

#include <iostream>

namespace Tmpl8
{
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
    // Draws the letter I, x pixels to the right and y pixels down
    // -----------------------------------------------------------
    void Game::DrawI(int x, int y)
    {
        screen.Line(100 + x, 50 + y, 200 + x, 50 + y, 0xffffff);
        screen.Line(150 + x, 50 + y, 150 + x, 300 + y, 0xffffff);
        screen.Line(100 + x, 300 + y, 200 + x, 300 + y, 0xffffff);
    }

    // -----------------------------------------------------------
    // Main application tick function
    // -----------------------------------------------------------
    void Game::Tick(float deltaTime)
    {
        screen.Clear(80);
        DrawI(0, 0);
        DrawI(1, 0);
        DrawI(0, 1);
        DrawI(1, 1);
    }
}
