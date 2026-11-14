#include "game.hpp"

#include <format>
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
    float spriteY = 0, speed = 1;
    void Game::Tick(float deltaTime)
    {
        std::cout << deltaTime << "\n";
        if (GetKeyPressed(SDL_SCANCODE_V)) ToggleVSync();
        screen.Clear(0);
        screen.Print(std::format("{:.0f} FPS", 1000.0f / deltaTime), 2, 2, 0xffffff);
        speed += 1.0f;
        spriteY += speed;
        if (spriteY > (512 - 50))
        {
            spriteY = 512 - 50;
            speed = -speed * 0.8f;
        }
        ball.Draw(screen, 20, static_cast<int>(spriteY));
    }
}
