#pragma once

#include "background.hpp"
#include "menu.hpp"
#include "scene.hpp"

namespace Tmpl8
{
    class Game;

    // The settings: the volume of the music and the sound effects, and fullscreen.
    class SettingsScene : public Scene
    {
    public:
        explicit SettingsScene(Game& game);
        void Tick(float deltaTime) override;
        void Draw(Surface& screen) override;

    private:
        enum Item { Music, Effects, Fullscreen, Back };
        void UpdateLabels();

        Game& game;
        Background background{ "assets/backgrounds/menu.png" };
        Sprite panel{ "assets/ui/panel.png", 1 };
        Menu menu{ 80 };
    };
}
