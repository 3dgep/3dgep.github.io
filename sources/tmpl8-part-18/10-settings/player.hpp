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
        void Bounce(); // jump up from the head of an enemy
        const Body& GetBody() const { return body; }

    private:
        Body body;
        bool facingRight = true;
        float coyoteTime = 0;    // the time left to jump after running off a ledge, in seconds
        float jumpBuffer = 0;    // the time left in which a jump that was pressed too early still counts
        float animationTime = 0; // for the walking animation
    };
}
