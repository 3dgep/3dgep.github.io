#pragma once

#include "background.hpp"
#include "camera.hpp"
#include "enemy.hpp"
#include "level.hpp"
#include "player.hpp"
#include "scene.hpp"

#include <vector>

namespace Tmpl8
{
    class Game;

    // Playing the game: the levels, one after the other.
    class PlayScene : public Scene
    {
    public:
        static constexpr int LevelCount = 3;

        explicit PlayScene(Game& game);
        void Tick(float deltaTime) override;
        void Draw(Surface& screen) override;

    private:
        enum class State { Playing, Dying, Finished };

        void StartLevel();
        void TouchTiles();
        void TouchEnemies();
        void Die();
        void AfterWait(); // when the wait after dying or finishing a level is over
        void DrawHud(Surface& screen);
        void DrawColliders(Surface& screen);

        Game& game;
        Level level;
        Player player;
        std::vector<Enemy> enemies;
        Camera camera;
        Background background{ "assets/backgrounds/level1.png" };
        Sprite characters{ "assets/characters.png", 9, 12 };
        Sprite icons{ "assets/tiles.png", 20, 9 };
        bool showColliders = false; // C shows the boxes that the game uses for collisions

        State state = State::Playing;
        float waitTime = 0;  // how long to wait after dying or finishing a level, in seconds
        int levelNumber = 1;
        int lives = 3;
        int coins = 0;
        int coinsAtStart = 0; // the coins when the level started: dying takes the coins of the level away again
    };
}
