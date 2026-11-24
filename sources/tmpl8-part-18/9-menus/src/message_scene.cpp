#include "message_scene.hpp"

#include "game.hpp"
#include "menu_scene.hpp"
#include "play_scene.hpp"

namespace Tmpl8
{
    MessageScene::MessageScene(Game& game, const std::string& title, const std::string& message)
        : game(game), title(title), message(message)
    {
        game.GetSounds().StopMusic();
        menu.Add("PLAY AGAIN"); // selected, so Enter plays again
        menu.Add("MAIN MENU");
    }

    void MessageScene::Tick(float deltaTime)
    {
        switch (menu.Tick(game))
        {
        case PlayAgainButton: game.ChangeScene(std::make_unique<PlayScene>(game)); break;
        case MainMenuButton: game.ChangeScene(std::make_unique<MenuScene>(game)); break;
        }
    }

    void MessageScene::Draw(Surface& screen)
    {
        background.Draw(screen, 0);
        panel.Draw(screen, (ScreenWidth - panel.GetWidth()) / 2, 50);
        Text& text = game.GetText();
        text.Centre(screen, title, 90);
        text.Centre(screen, message, 120);
        menu.Draw(screen, text);
    }
}
