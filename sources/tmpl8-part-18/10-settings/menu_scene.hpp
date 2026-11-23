#pragma once

#include "background.hpp"
#include "menu.hpp"
#include "scene.hpp"

namespace Tmpl8
{
    class Game;

    // The main menu: the title of the game, and the buttons to play, to change the settings, or to quit.
    class MenuScene : public Scene
    {
    public:
        explicit MenuScene(Game& game);
        void Tick(float deltaTime) override;
        void Draw(Surface& screen) override;

    private:
        enum Item { PlayButton, LevelsButton, SettingsButton, QuitButton };

        Game& game;
        Background background{ "assets/backgrounds/menu.png" };
        Sprite title{ "assets/ui/title.png", 1 };
        Menu menu{ 120 };
        float scroll = 0; // the background slowly scrolls by
    };
}
