#pragma once

#include "background.hpp"
#include "camera.hpp"
#include "level.hpp"
#include "player.hpp"
#include "scene.hpp"

namespace Tmpl8
{
    class Game;

    // Playing the game.
    class PlayScene : public Scene
    {
    public:
        explicit PlayScene(Game& game);
        void Tick(float deltaTime) override;
        void Draw(Surface& screen) override;

    private:
        void DrawColliders(Surface& screen);

        Game& game;
        Level level;
        Player player;
        Camera camera;
        Background background{ "assets/backgrounds/level1.png" };
        Sprite characters{ "assets/characters.png", 9, 12 };
        bool showColliders = false; // C shows the boxes that the game uses for collisions
    };
}
