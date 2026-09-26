#include "Bullet.h"
#include <memory>
// for collision checking
#include "Enemy.h"
#include "Player.h"

/**
 * @brief Constructor, sets location, heading, and player
 */
Bullet::Bullet(CMPUT350::Point2D location, CMPUT350::Point2D heading, bool player)
: mLocation{location}, mHeading{heading}, mPlayer{player}
{
}

/**
 * @brief Returns if the bullet is from the player
 */
bool Bullet::IsPlayerBullet()
{
    return mPlayer;
}

/**
 * @brief Initializes the other member variables of Bullet
 */
void Bullet::Initialize(CMPUT350::GameContext* context)
{
    mIsAlive = true;
    // using center, radius constructor. radius was arbitrarily chosen
    mBoundingBox = CMPUT350::Rect(mLocation, 10);
    // picture it as location as the middle, and the start and end points below and above
    mBulletRender = CMPUT350::Line({mLocation.x, mLocation.y + 5}, {mLocation.x, mLocation.y - 5});
}

/**
 * @brief Updates bullet position & its bounding box
 */
void Bullet::Update(CMPUT350::GameContext* context)
{
    mLocation += mHeading;
    mBoundingBox += mHeading;
}

/**
 * @brief Late update for a bullet (does nothing)
 */
void Bullet::LateUpdate(CMPUT350::GameContext* context)
{
    // NOTE: not sure if this is supposed to do anything, project description mentions nothing
    return;
}

/**
 * @brief Handles key events for a bullet (does nothing)
 */
bool Bullet::HandleKeyEvent(CMPUT350::GameContext* context, char key)
{
    // NOTE: not sure if this is supposed to do anything, project description mentions nothing
    return true;
}

/**
 * @brief Renders the background for the bullet (does nothing, no background)
 */
void Bullet::RenderBackground(CMPUT350::GameContext* context)
{
    // NOTE: I am pretty sure a Galaga bullet has no background so nothing is done
}

/**
 * @brief Renders the foreground for the bullet 
 */
void Bullet::RenderForeground(CMPUT350::GameContext* context)
{
    CMPUT350::DrawContext *drawContext{context->ScreenContext};
    // width & color arbitrarily chosen
    drawContext->DrawLine(mBulletRender.p1, mBulletRender.p2, 5, CMPUT350::Colors::red);
}

/**
 * @brief Checks for a collision with another collision object
 */
void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    const CMPUT350::Rect &objBounds{obj->GetBounds()};
    // check if bounding boxes are NOT intersecting
    if (mBoundingBox.topLeft.x + mBoundingBox.width < objBounds.topLeft.x ||  // 1. Check if bullet is to the left
        mBoundingBox.topLeft.y > objBounds.topLeft.y + objBounds.height ||    // 2. Check if bullet is below
        mBoundingBox.topLeft.x > objBounds.topLeft.x + objBounds.width ||     // 3. Check if bullet is to the right
        mBoundingBox.topLeft.y + mBoundingBox.height < objBounds.topLeft.y)   // 4. Check if bullet is above 
    {
        return;
    }

    // they are intersecting -> check if it's an enemy or player
    auto enemy{std::dynamic_pointer_cast<Enemy>(obj)};
    // Case 1: enemy & player bullet intersection
    if (enemy && IsPlayerBullet()) {
        enemy->Kill();
        return;
    }

    auto player{std::dynamic_pointer_cast<Player>(obj)};
    // Case 2: player and enemy bullet intersection
    if (player && !IsPlayerBullet()) {
        player->Kill();
        return;
    }
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
    return mBoundingBox;
}
