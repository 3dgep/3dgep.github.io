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
    int spriteX = 0;
    int spriteY = 100;
    int speed = 1;
    void Game::Tick(float deltaTime)
    {
        speed++;
        spriteY += speed;
        bool hitBottom = spriteY > (512 - 50);
        std::cout << "hitBottom: " << hitBottom << "\n";
        if (hitBottom)
        {
            spriteY = 512 - 50;
            speed = -speed;
            screen.Clear(0xff0000);
            ball.Draw(screen, spriteX, spriteY);
        }
        else
        {
            screen.Clear(0);
            ball.Draw(screen, spriteX, spriteY);
        }
    }
}
