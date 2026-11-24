#pragma once

#include "background.hpp"
#include "menu.hpp"
#include "scene.hpp"

#include <string>

namespace Tmpl8
{
    class Game;

    // A message on a panel, like "GAME OVER", with buttons to play again or to go back to the main menu.
    class MessageScene : public Scene
    {
    public:
        MessageScene(Game& game, const std::string& title, const std::string& message);
        void Tick(float deltaTime) override;
        void Draw(Surface& screen) override;

    private:
        enum Item { PlayAgainButton, MainMenuButton };

        Game& game;
        std::string title;
        std::string message;
        Background background{ "assets/backgrounds/menu.png" };
        Sprite panel{ "assets/ui/panel.png", 1 };
        Menu menu{ 150 };
    };
}
