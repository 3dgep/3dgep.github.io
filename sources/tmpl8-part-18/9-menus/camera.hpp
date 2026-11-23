#pragma once

#include "template.hpp"

#include <algorithm>

namespace Tmpl8
{
    // The part of the level that is on the screen. It scrolls sideways, and up and down in a level that is taller
    // than the screen.
    class Camera
    {
    public:
        // Move towards the target, so that it ends up in the middle of the screen.
        void Follow(vec2 target, vec2 levelSize, float deltaTime)
        {
            position += (Goal(target, levelSize) - position) * std::min(1.0f, Speed * deltaTime);
        }
        // Jump to the target right away.
        void MoveTo(vec2 target, vec2 levelSize) { position = Goal(target, levelSize); }
        int X() const { return static_cast<int>(position.x); } // the left edge of the screen, in the level
        int Y() const { return static_cast<int>(position.y); } // the top edge of the screen, in the level

    private:
        // Where the camera should be: the target in the middle, but never past the edges of the level.
        static vec2 Goal(vec2 target, vec2 levelSize)
        {
            return { std::clamp(target.x - ScreenWidth / 2.0f, 0.0f, levelSize.x - ScreenWidth),
                     std::clamp(target.y - ScreenHeight / 2.0f, 0.0f, levelSize.y - ScreenHeight) };
        }
        static constexpr float Speed = 8; // how quickly the camera catches up
        vec2 position;
    };
}
