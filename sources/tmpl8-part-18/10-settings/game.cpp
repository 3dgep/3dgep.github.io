#include "game.hpp"

#include "src/menu_scene.hpp"

#include <SDL3/SDL_events.h>
#include <algorithm>

namespace Tmpl8
{
    // -----------------------------------------------------------
    // Initialize the application
    // -----------------------------------------------------------
    void Game::Init()
    {
        settings.Load();
        sounds.SetMusicVolume(settings.musicVolume);
        sounds.SetEffectsVolume(settings.effectsVolume);
        if (settings.fullscreen)
            SetFullscreen(true);
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
        // A scene can't replace itself while its Tick runs: that would delete it. So the switch happens here.
        if (nextScene)
            scene = std::move(nextScene);
        scene->Draw(screen);
    }

    void Game::Quit()
    {
        SDL_Event event{};
        event.type = SDL_EVENT_QUIT; // the same as clicking the close button of the window
        SDL_PushEvent(&event);
    }
}
