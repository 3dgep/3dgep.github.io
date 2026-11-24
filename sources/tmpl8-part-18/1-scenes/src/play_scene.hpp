#pragma once

#include "scene.hpp"

namespace Tmpl8
{
    class Game;

    // Playing the game.
    class PlayScene : public Scene
    {
    public:
        explicit PlayScene(Game& game) : game(game) {}
        void Tick(float deltaTime) override;
        void Draw(Surface& screen) override;

    private:
        Game& game;
    };
}
