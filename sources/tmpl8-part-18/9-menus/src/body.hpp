#pragma once

#include "template.hpp"

namespace Tmpl8
{
    // A box that moves through the level, like the player or an enemy.
    struct Body
    {
        vec2 position; // the top-left corner, in pixels
        vec2 size;     // the width and the height, in pixels
        vec2 velocity; // in pixels per second
        bool onGround = false; // standing on something solid (Level::Move sets it)
        bool hitWall = false;  // ran into a wall in the last move (Level::Move sets it)

        float Right() const { return position.x + size.x; }
        float Bottom() const { return position.y + size.y; }
        bool Overlaps(const Body& other) const
        {
            return position.x < other.Right() && other.position.x < Right() &&
                   position.y < other.Bottom() && other.position.y < Bottom();
        }
    };
}
