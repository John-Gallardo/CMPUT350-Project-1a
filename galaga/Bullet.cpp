#include "Bullet.h"
#include <memory>
// for collision checking
#include "Enemy.h"
#include "Player.h"

/**
 * @brief Constructor, sets location, heading, and player
 */
Bullet::Bullet(CMPUT350::Point2D location, CMPUT350::Point2D heading, bool player, std::weak_ptr<CMPUT350::GameObject> parent)
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
    mRotation = 0.0f;
    mPrevLocation = mLocation;  // technically not correct but IDK what else to initialize it to
}

/**
 * @brief Updates bullet position & its bounding box
 */
void Bullet::Update(CMPUT350::GameContext* context)
{
    mPrevLocation = mLocation;
    mLocation += mHeading;

    // NOTE: this isn't in the spec but I thought it makes sense:
    // we kill this bullet if it's out of bounds
    if (mLocation.y < 0) {
        mIsAlive = false;
    }
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
    // picture it as location as the middle, and the start and end points below and above
    CMPUT350::Line bulletRender = CMPUT350::Line({mLocation.x, mLocation.y + 5}, {mLocation.x, mLocation.y - 5});

    CMPUT350::DrawContext *drawContext{context->ScreenContext};
    // width arbitrarily chosen. fires blue if player, red otherwise
    if (mPlayer) {
        drawContext->DrawLine(bulletRender.p1, bulletRender.p2, 5, CMPUT350::Colors::blue);
    } else {
        drawContext->DrawLine(bulletRender.p1, bulletRender.p2, 5, CMPUT350::Colors::red);
    }
}

/**
 * @brief Checks for a collision with another collision object
 */
void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    // Case 1: enemy & player bullet intersection
    auto enemy{std::dynamic_pointer_cast<Enemy>(obj)};
    if (enemy && IsPlayerBullet()) {
        enemy->Kill();
        return;
    }

    // Case 2: player and enemy bullet intersection
    auto player{std::dynamic_pointer_cast<Player>(obj)};
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
    // the box formed by the previous and current location
    static CMPUT350::Rect boundingBox{};
    boundingBox = CMPUT350::Rect(mPrevLocation, mLocation);
    return boundingBox;
}

/**
 * @brief Returns this bullet's shapes
 */
const std::vector<CMPUT350::Shape> &Bullet::GetShapes() {
    return mShapes;
}

/**
 * Returns this bullet's location
 */
CMPUT350::Point2D Bullet::GetLocation() const {
    return mLocation;
}

/**
 * Returns this bullet's rotation
 */
float Bullet::GetRotation() const {
    return mRotation;
}

/**
 * TODO: finish
 */
void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj, const CMPUT350::Point2D &collisionPoint) {
    return;
}
