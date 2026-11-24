#pragma once

#include "body.hpp"
#include "surface.hpp"

namespace Tmpl8
{
    class Camera;
    class Game;
    class Level;

    // The alien that the player controls.
    class Player
    {
    public:
        void Spawn(vec2 position);
        void Tick(Game& game, const Level& level, float deltaTime);
        void Draw(Surface& screen, Sprite& characters, const Camera& camera) const;
        const Body& GetBody() const { return body; }

    private:
        Body body;
        bool facingRight = true;
    };
}
