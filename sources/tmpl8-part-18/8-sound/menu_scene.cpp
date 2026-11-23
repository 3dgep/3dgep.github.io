#include "menu_scene.hpp"

#include "game.hpp"
#include "play_scene.hpp"

namespace Tmpl8
{
    MenuScene::MenuScene(Game& game) : game(game)
    {
        game.GetSounds().PlayMenuMusic();
    }

    void MenuScene::Tick(float deltaTime)
    {
        if (game.GetKeyPressed(SDL_SCANCODE_RETURN))
            game.ChangeScene(std::make_unique<PlayScene>(game));
    }

    void MenuScene::Draw(Surface& screen)
    {
        screen.Clear(0xdff6f5); // the colour of the sky
        title.Draw(screen, (ScreenWidth - title.GetWidth()) / 2, 40);
        game.GetText().Centre(screen, "PRESS ENTER TO PLAY", 140);
    }
}
