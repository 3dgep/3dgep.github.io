#include "enemy.hpp"

#include "camera.hpp"
#include "level.hpp"

#include <algorithm>
#include <cmath>

namespace Tmpl8
{
    namespace
    {
        constexpr float WalkSpeed = 40; // in pixels per second
        constexpr float Gravity = 1000;
        constexpr float MaxFallSpeed = 400;

        constexpr float FlySpeed = 30;                      // in pixels per second
        constexpr float FlyRange = 2 * Level::TileSize;     // how far a flyer goes to the left and the right of its start
        constexpr float FlyHeight = 1.5f * Level::TileSize; // how far it goes up and down
        constexpr float FlyBobSpeed = 2.5f;                 // how quickly it goes up and down (radians per second)
    }

    Enemy::Enemy(Kind kind, vec2 position) : kind(kind)
    {
        if (kind == Kind::Walker)
        {
            // It stands on the bottom of its tile.
            body.size = { 16, 12 };
            body.position = { position.x + (Level::TileSize - body.size.x) / 2, position.y + Level::TileSize - body.size.y };
        }
        else
        {
            // It flies in the middle of its tile.
            body.size = { 14, 12 };
            body.position = position + (vec2(Level::TileSize) - body.size) / 2;
        }
        start = body.position;
    }

    void Enemy::Squash()
    {
        // Like in the old platformers: a little hop, and then it falls through the level, upside down.
        dying = true;
        body.velocity = { 0, -150 };
    }

    void Enemy::Tick(const Level& level, float deltaTime)
    {
        animationTime += deltaTime;
        if (dying)
        {
            body.velocity.y = std::min(body.velocity.y + Gravity * deltaTime, MaxFallSpeed);
            body.position += body.velocity * deltaTime; // not with Level::Move: it falls through the ground
        }
        else if (kind == Kind::Walker)
            Walk(level, deltaTime);
        else
            Fly(level, deltaTime);
    }

    void Enemy::Walk(const Level& level, float deltaTime)
    {
        body.velocity.x = facingRight ? WalkSpeed : -WalkSpeed;
        body.velocity.y = std::min(body.velocity.y + Gravity * deltaTime, MaxFallSpeed);
        level.Move(body, deltaTime);
        if (body.hitWall)
            facingRight = !facingRight;

        // Is there ground just in front of its feet? If not, turn around before it walks off the edge.
        if (body.onGround)
        {
            const float aheadX = facingRight ? body.Right() + 1 : body.position.x - 1;
            if (!level.IsSolid(Level::ToTile(aheadX), Level::ToTile(body.Bottom() + 1)))
                facingRight = !facingRight;
        }
    }

    void Enemy::Fly(const Level& level, float deltaTime)
    {
        // Back and forth, between two points on either side of its start, and around walls.
        body.velocity = { facingRight ? FlySpeed : -FlySpeed, 0 };
        level.Move(body, deltaTime);
        if (body.hitWall)
            facingRight = !facingRight;
        if (body.position.x > start.x + FlyRange)
            facingRight = false;
        if (body.position.x < start.x - FlyRange)
            facingRight = true;

        // Up and down along a sine wave. A flyer goes up and down in the open air, so it doesn't need Level::Move.
        body.position.y = start.y + FlyHeight * std::sin(animationTime * FlyBobSpeed);
    }

    void Enemy::Draw(Surface& screen, Sprite& characters, const Camera& camera) const
    {
        // A walker walks with frames 18 and 19, and a flyer flaps its wings with frames 24 and 25. The frames facing
        // right are 27 frames further on, and the upside-down ones 54.
        int frame = kind == Kind::Walker ? 18 + static_cast<int>(animationTime * 6) % 2
                                         : 24 + static_cast<int>(animationTime * 8) % 2;
        if (facingRight)
            frame += 27;
        if (dying)
            frame += 54;
        characters.SetFrame(frame);
        // The 24 x 24 sprite is centred on the body. A walker has its feet at the bottom of the body.
        const int x = static_cast<int>(body.position.x + body.size.x / 2) - 12 - camera.X();
        const int y = kind == Kind::Walker ? static_cast<int>(body.Bottom()) - 24 - camera.Y()
                                           : static_cast<int>(body.position.y + body.size.y / 2) - 12 - camera.Y();
        characters.Draw(screen, x, y);
    }
}
