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
    // Main application tick function
    // -----------------------------------------------------------
    void Game::Tick(float deltaTime)
    {
        // clear the graphics window
        screen.Clear(0);
        // print something in the graphics window
        screen.Print("hello world", 2, 2, 0xffffff);
        // print something to the text window
        if (GetKeyPressed(SDL_SCANCODE_SPACE))
        {
            std::cout << "You pressed the space bar.\n";
        }
        // draw a sprite
        rotatingGun.SetFrame(frame);
        rotatingGun.Draw(screen, 100, 100);
        if (++frame == 36) frame = 0;
    }
}
