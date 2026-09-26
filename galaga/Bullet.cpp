#include "Bullet.h"

Bullet::Bullet(CMPUT350::Point2D location, CMPUT350::Point2D heading, bool player)
: mLocation{location},
mHeading{heading},
mPlayer{player},
mIsAlive{true}
{
}

/**
 * @brief Returns if the bullet is from the player
 */
bool Bullet::IsPlayerBullet()
{
    return mPlayer;
}

void Bullet::Initialize(CMPUT350::GameContext* context)
{
    // TODO: not sure what this is supposed to do
}

/**
 * @brief Updates bullet position
 */
void Bullet::Update(CMPUT350::GameContext* context)
{
    mLocation += mHeading;
}

void Bullet::LateUpdate(CMPUT350::GameContext* context)
{
    // TODO:
}

bool Bullet::HandleKeyEvent(CMPUT350::GameContext* context, char key)
{
    // TODO: is this even supposed to do anything?
}

/**
 * @brief Renders the background for the bullet
 */
void Bullet::RenderBackground(CMPUT350::GameContext* context)
{
    // TODO:
    CMPUT350::DrawContext *drawContext{context->ScreenContext};
}

/**
 * @brief Renders the foreground for the bullet 
 */
void Bullet::RenderForeground(CMPUT350::GameContext* context)
{
    // TODO:
    CMPUT350::DrawContext *drawContext{context->ScreenContext};
}

/**
 * @brief Checks for a collision with another collision object
 */
void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    // TODO:
}

/**
 * @brief Kills the bullet
 */
void Bullet::Kill()
{
    mIsAlive = false;
}

/**
 * @brief Returns true if the bullet is alive, false if dead
 */
bool Bullet::IsAlive() const
{
    return mIsAlive;
}

/**
 * @brief Returns the current bounding box for the bullet
 */
const CMPUT350::Rect& Bullet::GetBounds()
{
    // TODO:
    static CMPUT350::Rect sBounds(0, 0, 0, 0);
    return sBounds;
}
