#pragma once

#include "body.hpp"
#include "surface.hpp"

namespace Tmpl8
{
    class Camera;
    class Level;

    // An enemy: a robot that walks back and forth, or a bat that flies back and forth while it goes up and down.
    class Enemy
    {
    public:
        enum class Kind { Walker, Flyer };

        Enemy(Kind kind, vec2 position);
        void Tick(const Level& level, float deltaTime);
        void Draw(Surface& screen, Sprite& characters, const Camera& camera) const;
        const Body& GetBody() const { return body; }
        void Squash(); // the player jumped on it: it falls off the screen, upside down
        bool IsDying() const { return dying; }
        bool IsGone(float bottom) const { return dying && body.position.y > bottom; } // it fell below the bottom

    private:
        void Walk(const Level& level, float deltaTime);
        void Fly(const Level& level, float deltaTime);

        Kind kind;
        Body body;
        vec2 start; // where a flyer starts: the middle of its flight
        bool facingRight = false;
        bool dying = false;
        float animationTime = 0;
    };
}
