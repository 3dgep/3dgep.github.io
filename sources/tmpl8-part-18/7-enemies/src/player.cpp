#include "player.hpp"

#include "camera.hpp"
#include "game.hpp"
#include "level.hpp"

#include <algorithm>

namespace Tmpl8
{
    namespace
    {
        constexpr float RunSpeed = 120;        // in pixels per second
        constexpr float Acceleration = 900;    // how fast the player speeds up and slows down, on the ground
        constexpr float AirAcceleration = 500; // the same, in the air
        constexpr float Gravity = 1000;        // in pixels per second per second
        constexpr float JumpSpeed = 360;       // the upward speed at the start of a jump
        constexpr float MaxFallSpeed = 400;
        constexpr float CoyoteTime = 0.1f;     // in seconds
        constexpr float JumpBufferTime = 0.1f;

        // Changes value towards target, but by at most step.
        float MoveTowards(float value, float target, float step)
        {
            if (value < target)
                return std::min(value + step, target);
            return std::max(value - step, target);
        }
    }

    void Player::Spawn(vec2 position)
    {
        body = {};
        body.size = { 14, 20 };
        // Stand on the bottom of the tile where the player starts.
        body.position = { position.x + (Level::TileSize - body.size.x) / 2, position.y + Level::TileSize - body.size.y };
        facingRight = true;
        coyoteTime = jumpBuffer = animationTime = 0;
    }

    void Player::Tick(Game& game, const Level& level, float deltaTime)
    {
        // Running: speed up towards the run speed in the direction of the keys, or slow down to a stop.
        float direction = 0;
        if (game.GetKey(SDL_SCANCODE_LEFT) || game.GetKey(game.KeyFor(SDLK_A)))
            direction -= 1;
        if (game.GetKey(SDL_SCANCODE_RIGHT) || game.GetKey(game.KeyFor(SDLK_D)))
            direction += 1;
        const float acceleration = body.onGround ? Acceleration : AirAcceleration;
        body.velocity.x = MoveTowards(body.velocity.x, direction * RunSpeed, acceleration * deltaTime);
        if (direction != 0)
            facingRight = direction > 0;

        // Jumping: a jump that is pressed a moment before landing, or a moment after running off a ledge, still counts.
        const bool jumpPressed = game.GetKeyPressed(SDL_SCANCODE_SPACE) || game.GetKeyPressed(SDL_SCANCODE_UP) ||
                                 game.GetKeyPressed(game.KeyFor(SDLK_W));
        const bool jumpReleased = game.GetKeyReleased(SDL_SCANCODE_SPACE) || game.GetKeyReleased(SDL_SCANCODE_UP) ||
                                  game.GetKeyReleased(game.KeyFor(SDLK_W));
        jumpBuffer = jumpPressed ? JumpBufferTime : jumpBuffer - deltaTime;
        coyoteTime = body.onGround ? CoyoteTime : coyoteTime - deltaTime;
        if (jumpBuffer > 0 && coyoteTime > 0)
        {
            body.velocity.y = -JumpSpeed;
            jumpBuffer = coyoteTime = 0;
        }
        // Letting go of the jump key early makes a lower jump.
        if (jumpReleased && body.velocity.y < 0)
            body.velocity.y *= 0.5f;

        body.velocity.y = std::min(body.velocity.y + Gravity * deltaTime, MaxFallSpeed);
        level.Move(body, deltaTime);
        animationTime += deltaTime;
    }

    void Player::Bounce()
    {
        body.velocity.y = -JumpSpeed * 0.7f;
    }

    void Player::Draw(Surface& screen, Sprite& characters, const Camera& camera) const
    {
        // Frame 0 stands, frame 1 steps (and jumps). The frames facing right are 27 frames further on.
        int frame = 0;
        if (!body.onGround)
            frame = 1;
        else if (body.velocity.x != 0)
            frame = static_cast<int>(animationTime * 8) % 2;
        if (facingRight)
            frame += 27;
        characters.SetFrame(frame);
        // The sprite is 24x24 pixels: centre it on the body, with its feet at the bottom.
        const int x = static_cast<int>(body.position.x + body.size.x / 2) - 12 - camera.X();
        const int y = static_cast<int>(body.Bottom()) - 24 - camera.Y();
        characters.Draw(screen, x, y);
    }
}
