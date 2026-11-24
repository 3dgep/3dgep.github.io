#pragma once

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
    };
}
