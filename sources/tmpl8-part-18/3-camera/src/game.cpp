#include "game.hpp"

#include "menu_scene.hpp"

#include <algorithm>

namespace Tmpl8
{
    // -----------------------------------------------------------
    // Initialize the application
    // -----------------------------------------------------------
    void Game::Init()
    {
        scene = std::make_unique<MenuScene>(*this);
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
        // The scenes work in seconds. A slow frame counts as 1/30 of a second at most, so that nothing moves too far
        // in one frame (like after a break in the debugger).
        const float seconds = std::min(deltaTime / 1000, 1.0f / 30);
        scene->Tick(seconds);
        scene->Draw(screen);
        // A scene can't replace itself during its Tick (that would delete it), so the switch happens after Draw.
        if (nextScene)
            scene = std::move(nextScene);
    }
}
