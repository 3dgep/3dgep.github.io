#pragma once

#include "scene.hpp"
#include "surface.hpp"

namespace Tmpl8
{
    class Game;

    // The title screen: the name of the game, until the player presses Enter.
    class MenuScene : public Scene
    {
    public:
        explicit MenuScene(Game& game);
        void Tick(float deltaTime) override;
        void Draw(Surface& screen) override;

    private:
        Game& game;
        Sprite title{ "assets/ui/title.png", 1 };
    };
}
