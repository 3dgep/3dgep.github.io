#include "play_scene.hpp"

#include "game.hpp"

namespace Tmpl8
{
    PlayScene::PlayScene(Game& game) : game(game)
    {
        level.Load("assets/levels/level1.txt");
        player.Spawn(level.PlayerStart());
        camera.MoveTo(player.GetBody().position, level.Size());
    }

    void PlayScene::Tick(float deltaTime)
    {
        if (game.GetKeyPressed(game.KeyFor(SDLK_C)))
            showColliders = !showColliders;

        player.Tick(game, level, deltaTime);
        // Falling out of the bottom of the level: start again.
        if (player.GetBody().position.y > level.Height())
            player.Spawn(level.PlayerStart());
        camera.Follow(player.GetBody().position, level.Size(), deltaTime);
    }

    void PlayScene::Draw(Surface& screen)
    {
        background.Draw(screen, camera.X(), static_cast<int>(level.Height()) - ScreenHeight - camera.Y());
        level.Draw(screen, camera);
        player.Draw(screen, characters, camera);
        if (showColliders)
            DrawColliders(screen);
    }

    // The boxes that the game uses for collisions: green is solid, and magenta the player.
    void PlayScene::DrawColliders(Surface& screen)
    {
        const int cameraX = camera.X(), cameraY = camera.Y();
        auto draw = [&](const Body& box, Pixel color)
        {
            const int x = static_cast<int>(box.position.x) - cameraX, y = static_cast<int>(box.position.y) - cameraY;
            screen.Box(x, y, x + static_cast<int>(box.size.x) - 1, y + static_cast<int>(box.size.y) - 1, color);
        };
        for (int y = Level::ToTile(static_cast<float>(cameraY)); y <= Level::ToTile(static_cast<float>(cameraY + ScreenHeight)); y++)
        {
            for (int x = Level::ToTile(static_cast<float>(cameraX)); x <= Level::ToTile(static_cast<float>(cameraX + ScreenWidth)); x++)
            {
                if (level.IsSolid(x, y))
                    draw(level.HitBox(x, y), 0x00c000);
            }
        }
        draw(player.GetBody(), 0xff00ff);
    }
}
