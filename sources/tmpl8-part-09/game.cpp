#include "game.hpp"

#include <iostream>
#include <print>

namespace Tmpl8
{
    // -----------------------------------------------------------
    // Initialize the application
    // -----------------------------------------------------------
    void Game::Init()
    {
        for (int i = 0; i <= 8; i++)
        {
            std::println("1 << {} = {:3} = {:016b}", i, 1 << i, 1 << i);
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
        Pixel colour = 0xD7FFED00; // orange, garbage in alpha
        Pixel mask = 255 << 8;     // the mask for green
        Pixel green = colour & mask;
        screen.Clear(green);
    }
}
