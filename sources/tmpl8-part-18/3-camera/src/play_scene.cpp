#include "play_scene.hpp"

#include "game.hpp"

#include <algorithm>

namespace Tmpl8
{
    PlayScene::PlayScene(Game& game) : game(game)
    {
        level.Load("assets/levels/level3.txt");
        target = { 0, level.Height() }; // the bottom-left corner
        camera.MoveTo(target, level.Size());
    }

    void PlayScene::Tick(float deltaTime)
    {
        // The arrow keys move the target, 300 pixels per second, and the camera follows it.
        const float step = 300 * deltaTime;
        if (game.GetKey(SDL_SCANCODE_LEFT))
            target.x -= step;
        if (game.GetKey(SDL_SCANCODE_RIGHT))
            target.x += step;
        if (game.GetKey(SDL_SCANCODE_UP))
            target.y -= step;
        if (game.GetKey(SDL_SCANCODE_DOWN))
            target.y += step;
        target.x = std::clamp(target.x, 0.0f, level.Width());
        target.y = std::clamp(target.y, 0.0f, level.Height());
        camera.Follow(target, level.Size(), deltaTime);
    }

    void PlayScene::Draw(Surface& screen)
    {
        background.Draw(screen, camera.X(), static_cast<int>(level.Height()) - ScreenHeight - camera.Y());
        level.Draw(screen, camera);
    }
}
