#include "game.hpp"

#include <cmath>
#include <iostream>

namespace Tmpl8
{
    float x = 400, y = 256;

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
        screen.Line(mousex, 0, mousex, 511, 0xff0000);
        screen.Line(0, mousey, 799, mousey, 0xff0000);
        float dx = x - mousex, dy = y - mousey;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > 0 && dist < 50)
            x += dx / dist, y += dy / dist;
        screen.Plot(static_cast<int>(x), static_cast<int>(y), 0xffffff);
    }
}
