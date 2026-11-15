#include "game.hpp"

#include <format>
#include <iostream>

namespace Tmpl8
{
    constexpr int Particles = 4096;
    int x[Particles], y[Particles];

    // -----------------------------------------------------------
    // Initialize the application
    // -----------------------------------------------------------
    void Game::Init()
    {
        for (int i = 0; i < Particles; i++)
            x[i] = IRand(800), y[i] = IRand(512);
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
        for (int i = 0; i < Particles; i++)
        {
            x[i] = (x[i] + 800 + (((i & 1) * 2) - 1)) % 800;
            y[i] = (y[i] + 512 + ((((i >> 2) & 1) * 2) - 1)) & 511;
            screen.GetBuffer()[x[i] + y[i] * 800] = 0xffffff;
        }
        screen.Print(std::format("{:.0f} FPS", 1000.0f / deltaTime), 2, 2, 0xffffff);
        if (GetKeyPressed(SDL_SCANCODE_V)) ToggleVSync();
    }
}
