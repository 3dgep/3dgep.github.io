#include "play_scene.hpp"

#include "game.hpp"

namespace Tmpl8
{
    PlayScene::PlayScene(Game& game) : game(game)
    {
        level.Load("assets/levels/level1.txt");
    }

    void PlayScene::Tick(float deltaTime)
    {
        // Nothing moves yet.
    }

    void PlayScene::Draw(Surface& screen)
    {
        screen.Clear(0xdff6f5); // the colour of the sky
        level.Draw(screen);
    }
}
