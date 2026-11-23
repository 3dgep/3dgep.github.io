#include "level_select_scene.hpp"

#include "game.hpp"
#include "menu_scene.hpp"
#include "play_scene.hpp"

#include <format>

namespace Tmpl8
{
    LevelSelectScene::LevelSelectScene(Game& game) : game(game)
    {
        // One button for every level, and the last one goes back.
        for (int level = 1; level <= PlayScene::LevelCount; level++)
            menu.Add(std::format("LEVEL {}", level));
        menu.Add("BACK");
    }

    void LevelSelectScene::Tick(float deltaTime)
    {
        const int chosen = menu.Tick(game);
        if (chosen == -1)
            return;
        if (chosen < PlayScene::LevelCount)
            game.ChangeScene(std::make_unique<PlayScene>(game, chosen + 1));
        else
            game.ChangeScene(std::make_unique<MenuScene>(game));
    }

    void LevelSelectScene::Draw(Surface& screen)
    {
        background.Draw(screen, 0);
        panel.Draw(screen, (ScreenWidth - panel.GetWidth()) / 2, 30);
        game.GetText().Centre(screen, "LEVELS", 50);
        menu.Draw(screen, game.GetText());
    }
}
