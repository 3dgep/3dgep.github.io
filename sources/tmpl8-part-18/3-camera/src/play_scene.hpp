#pragma once

#include "background.hpp"
#include "camera.hpp"
#include "level.hpp"
#include "scene.hpp"

namespace Tmpl8
{
    class Game;

    // Playing the game.
    class PlayScene : public Scene
    {
    public:
        explicit PlayScene(Game& game);
        void Tick(float deltaTime) override;
        void Draw(Surface& screen) override;

    private:
        Game& game;
        Level level;
        Camera camera;
        Background background{ "assets/backgrounds/level3.png" };
        vec2 target; // the point that the camera looks at (for now, the arrow keys move it)
    };
}
