#pragma once

#include "background.hpp"
#include "menu.hpp"
#include "scene.hpp"

namespace Tmpl8
{
    class Game;

    // Choose the level to start at.
    class LevelSelectScene : public Scene
    {
    public:
        explicit LevelSelectScene(Game& game);
        void Tick(float deltaTime) override;
        void Draw(Surface& screen) override;

    private:
        Game& game;
        Background background{ "assets/backgrounds/menu.png" };
        Sprite panel{ "assets/ui/panel.png", 1 };
        Menu menu{ 80 };
    };
}
