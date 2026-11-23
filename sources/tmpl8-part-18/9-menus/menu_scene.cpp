#include "menu_scene.hpp"

#include "game.hpp"
#include "level_select_scene.hpp"
#include "play_scene.hpp"

namespace Tmpl8
{
    MenuScene::MenuScene(Game& game) : game(game)
    {
        menu.Add("PLAY");
        menu.Add("LEVELS");
#ifndef __EMSCRIPTEN__ // a web page can't be closed by the game
        menu.Add("QUIT");
#endif
        game.GetSounds().PlayMenuMusic();
    }

    void MenuScene::Tick(float deltaTime)
    {
        scroll += 20 * deltaTime;
        switch (menu.Tick(game))
        {
        case PlayButton: game.ChangeScene(std::make_unique<PlayScene>(game)); break;
        case LevelsButton: game.ChangeScene(std::make_unique<LevelSelectScene>(game)); break;
        case QuitButton: game.Quit(); break;
        }
    }

    void MenuScene::Draw(Surface& screen)
    {
        background.Draw(screen, static_cast<int>(scroll));
        title.Draw(screen, (ScreenWidth - title.GetWidth()) / 2, 40);
        menu.Draw(screen, game.GetText());
        game.GetText().Centre(screen, "ARROWS OR A/D TO RUN, SPACE TO JUMP, P TO PAUSE", ScreenHeight - 16);
    }
}
