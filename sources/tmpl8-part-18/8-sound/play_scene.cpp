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
        enemies.clear();
        for (const EnemyStart& start : level.EnemyStarts())
            enemies.emplace_back(start.kind, start.position);
        camera.MoveTo(player.GetBody().position, level.Size());
        coinsAtStart = coins;
        state = State::Playing;
        game.GetSounds().PlayLevelMusic();
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
        for (Enemy& enemy : enemies)
            enemy.Tick(level, deltaTime);
        TouchTiles();
        TouchEnemies();
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
                    game.GetSounds().Play(Effect::Coin);
                }
                else if (tile == '^')
                    Die();
                else if ((tile == 'F' || tile == '|') && state == State::Playing)
                {
                    state = State::Finished;
                    waitTime = 5; // the length of the fanfare
                    game.GetSounds().StopMusic();
                    game.GetSounds().Play(Effect::Win);
                }
            }
        }
    }

    void PlayScene::TouchEnemies()
    {
        const Body& body = player.GetBody();
        for (Enemy& enemy : enemies)
        {
            const Body& other = enemy.GetBody();
            if (enemy.IsDying() || !body.Overlaps(other))
                continue;
            // Falling onto the top half of an enemy squashes it. Any other touch is deadly.
            if (body.velocity.y > 0 && body.Bottom() < other.position.y + other.size.y / 2)
            {
                enemy.Squash();
                player.Bounce();
                game.GetSounds().Play(Effect::Stomp);
            }
            else
            {
                Die();
                return;
            }
        }
        // An enemy that falls off the bottom of the screen is gone.
        const float bottom = static_cast<float>(camera.Y() + ScreenHeight);
        std::erase_if(enemies, [bottom](const Enemy& enemy) { return enemy.IsGone(bottom); });
    }

    void PlayScene::Die()
    {
        if (state != State::Playing)
            return;
        state = State::Dying;
        waitTime = 1.5f;
        lives--;
        game.GetSounds().StopMusic();
        game.GetSounds().Play(Effect::Hurt);
    }

    void PlayScene::AfterWait()
    {
        if (state == State::Dying)
        {
            if (lives == 0)
            {
                game.GetSounds().Play(Effect::Lose);
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
        for (const Enemy& enemy : enemies)
            enemy.Draw(screen, characters, camera);
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
        for (const Enemy& enemy : enemies)
        {
            if (!enemy.IsDying())
                draw(enemy.GetBody(), 0xff0000);
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
