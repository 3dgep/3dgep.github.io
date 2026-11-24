#include "play_scene.hpp"

#include "game.hpp"
#include "menu_scene.hpp"

namespace Tmpl8
{
    void PlayScene::Tick(float deltaTime)
    {
        // For now, Enter goes back to the title screen.
        if (game.GetKeyPressed(SDL_SCANCODE_RETURN))
            game.ChangeScene(std::make_unique<MenuScene>(game));
    }

    void PlayScene::Draw(Surface& screen)
    {
        screen.Clear(0xdff6f5); // the colour of the sky
        game.GetText().Centre(screen, "THE GAME GOES HERE", 120);
        game.GetText().Centre(screen, "PRESS ENTER FOR THE TITLE SCREEN", 140);
    }
}
