#include "play_scene.hpp"

#include "game.hpp"
#include "menu_scene.hpp"

#include <format>

namespace Tmpl8
{
    PlayScene::PlayScene(Game& game) : game(game)
    {
        StartLevel();
    }

    void PlayScene::StartLevel()
    {
        level.Load(std::format("assets/levels/level{}.txt", levelNumber));
        background = Background(std::format("assets/backgrounds/level{}.png", levelNumber)); // with the colours of the level
        player.Spawn(level.PlayerStart());
        camera.MoveTo(player.GetBody().position, level.Size());
        coinsAtStart = coins;
        state = State::Playing;
    }

    void PlayScene::Tick(float deltaTime)
    {
        if (game.GetKeyPressed(game.KeyFor(SDLK_C)))
            showColliders = !showColliders;

        level.Tick(deltaTime);
        if (state != State::Playing)
        {
            waitTime -= deltaTime;
            if (waitTime <= 0)
                AfterWait();
            return;
        }

        player.Tick(game, level, deltaTime);
        TouchTiles();
        // Falling out of the bottom of the level.
        if (player.GetBody().position.y > level.Height())
            Die();
        camera.Follow(player.GetBody().position, level.Size(), deltaTime);
    }

    // What the player touches: coins, spikes or the flag.
    void PlayScene::TouchTiles()
    {
        const Body& body = player.GetBody();
        for (int y = Level::ToTile(body.position.y); y <= Level::ToTile(body.Bottom() - 0.01f); y++)
        {
            for (int x = Level::ToTile(body.position.x); x <= Level::ToTile(body.Right() - 0.01f); x++)
            {
                if (!body.Overlaps(level.HitBox(x, y)))
                    continue;
                const char tile = level.TileAt(x, y);
                if (tile == 'C')
                {
                    level.SetTile(x, y, ' ');
                    coins++;
                }
                else if (tile == '^')
                    Die();
                else if ((tile == 'F' || tile == '|') && state == State::Playing)
                {
                    state = State::Finished;
                    waitTime = 2; // a moment to celebrate
                }
            }
        }
    }

    void PlayScene::Die()
    {
        if (state != State::Playing)
            return;
        state = State::Dying;
        waitTime = 1.5f;
        lives--;
    }

    void PlayScene::AfterWait()
    {
        if (state == State::Dying)
        {
            if (lives == 0)
            {
                game.ChangeScene(std::make_unique<MenuScene>(game));
                return;
            }
            coins = coinsAtStart; // the coins of this level come back
        }
        else if (++levelNumber > LevelCount)
        {
            game.ChangeScene(std::make_unique<MenuScene>(game));
            return;
        }
        StartLevel();
    }

    void PlayScene::Draw(Surface& screen)
    {
        background.Draw(screen, camera.X(), static_cast<int>(level.Height()) - ScreenHeight - camera.Y());
        level.Draw(screen, camera);
        if (state != State::Dying || static_cast<int>(waitTime * 10) % 2 == 0) // blink after dying
        {
            player.Draw(screen, characters, camera);
        }
        if (showColliders)
            DrawColliders(screen);
        DrawHud(screen);
    }

    // The boxes that the game uses for collisions: green is solid, red hurts, yellow is a coin, blue the flag,
    // and magenta the player.
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
                const char tile = level.TileAt(x, y);
                if (level.IsSolid(x, y))
                    draw(level.HitBox(x, y), 0x00c000);
                else if (tile == '^')
                    draw(level.HitBox(x, y), 0xff0000);
                else if (tile == 'C')
                    draw(level.HitBox(x, y), 0xffd000);
                else if (tile == 'F' || tile == '|')
                    draw(level.HitBox(x, y), 0x0060ff);
            }
        }
        draw(player.GetBody(), 0xff00ff);
    }

    // The heads-up display: the lives, the coins and the level.
    void PlayScene::DrawHud(Surface& screen)
    {
        for (int i = 0; i < lives; i++)
        {
            icons.SetFrame(44); // a heart
            icons.Draw(screen, 4 + i * 16, 2);
        }
        icons.SetFrame(151); // a coin
        icons.Draw(screen, 4, 18);
        Text& text = game.GetText();
        text.Print(screen, std::format("{}", coins), 22, 23);
        const std::string name = std::format("LEVEL {}", levelNumber);
        text.Print(screen, name, ScreenWidth - Text::Width(name) - 6, 6);
    }
}
