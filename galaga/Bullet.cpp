#include "Bullet.h"

#include <memory>

// for collision checking
#include "Enemy.h"
#include "Player.h"

/**
 * @brief Constructor, sets location, heading, player, and the ship that fired this shot.
 * parent is stored so the bullet can die when that ship dies.
 */
Bullet::Bullet(CMPUT350::Point2D location, CMPUT350::Point2D heading, bool player,
               std::weak_ptr<CMPUT350::GameObject> parent)
    : mLocation{location},
      mPrevLocation{location},
      mHeading{heading},
      mParent{parent},
      mPlayer{player} {
    RebuildSegment();
}

/**
 * @brief Returns if the bullet is from the player
 */
bool Bullet::IsPlayerBullet() { return mPlayer; }

/// Rebuilds the local travel line and its box after the bullet moves
void Bullet::RebuildSegment() {
    CMPUT350::Point2D tail = mPrevLocation - mLocation;
    mShapes.clear();
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Line(tail, CMPUT350::Point2D(0.f, 0.f))));
    mBounds = CMPUT350::Rect(tail, CMPUT350::Point2D(0.f, 0.f));
    if (mBounds.width < 4.f) {
        mBounds.topLeft.x -= 2.f;
        mBounds.width += 4.f;
    }
    if (mBounds.height < 4.f) {
        mBounds.topLeft.y -= 2.f;
        mBounds.height += 4.f;
    }
}

/**
 * @brief Initializes the other member variables of Bullet
 */
void Bullet::Initialize(CMPUT350::GameContext* context) {
    mIsAlive = true;
    mRotation = 0.0f;
    // The previous location is set to the current location so that the bullet's travel line starts
    // at its initial position
    mPrevLocation = mLocation;
    RebuildSegment();
}

/**
 * @brief Updates bullet position and rebuilds its local bounding box
 */
void Bullet::Update(CMPUT350::GameContext* context) {
    auto parent = mParent.lock();
    if (!parent || !parent->IsAlive()) {
        Kill();
        return;
    }
    mPrevLocation = mLocation;
    mLocation += mHeading;
    // Check if the bullet is out of bounds, with a 20-pixel buffer
    float width = static_cast<float>(context->ScreenContext->GetWindowWidth());
    float height = static_cast<float>(context->ScreenContext->GetWindowHeight());
    if (mLocation.x < -20.f || mLocation.x > width + 20.f || mLocation.y < -20.f ||
        mLocation.y > height + 20.f) {
        Kill();
        return;
    }
    RebuildSegment();
}

/**
 * @brief Late update for a bullet. The spec does not ask for any work here.
 */
void Bullet::LateUpdate(CMPUT350::GameContext* context) {}

/**
 * @brief Renders the background for the bullet. A bullet has no background.
 */
void Bullet::RenderBackground(CMPUT350::GameContext* context) {}

// Renders the foreground for the bullet. The mark is centered on the bullet, with the ends just
// above and below that center
void Bullet::RenderForeground(CMPUT350::GameContext* context) {
    CMPUT350::DrawContext* drawContext{context->ScreenContext};
    CMPUT350::RGBColor color = mPlayer ? CMPUT350::Colors::blue : CMPUT350::Colors::red;
    if (mHasHit) {
        drawContext->DrawCircle(mHitPoint, 5.f, color);
        return;
    }
    CMPUT350::Line bulletRender({0.f, 5.f}, {0.f, -5.f});
    drawContext->DrawLine(bulletRender.p1, bulletRender.p2, 5, color);
}

/**
 * @brief Checks for a collision with another collision object.
 * Case 1: a player bullet meets an enemy. The enemy lowers its own health, so this does not call
 *         enemy->Kill(). That call would destroy the 2-health boss on the first hit.
 * Case 2: an enemy bullet meets the player. The player is killed, and so is this bullet.
 */
void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) {
    auto enemy{std::dynamic_pointer_cast<Enemy>(obj)};
    if (enemy && IsPlayerBullet()) {
        Kill();
        return;
    }

    auto player{std::dynamic_pointer_cast<Player>(obj)};
    if (player && !IsPlayerBullet()) {
        player->Kill();
        Kill();
    }
}

/**
 * @brief Records the contact point, then runs the collision check above.
 * Several contacts can arrive in one frame. The spec says to keep the one with the largest y.
 */
void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj,
                            const CMPUT350::Point2D& collisionPoint) {
    if (!mHasHit || collisionPoint.y > mHitPoint.y) mHitPoint = collisionPoint;
    mHasHit = true;
    CollisionEnter(obj);
}

/**
 * @brief Kills the bullet
 */
void Bullet::Kill() { mIsAlive = false; }

/**
 * @brief Returns true if the bullet is alive, false if dead
 */
bool Bullet::IsAlive() const { return mIsAlive; }

/**
 * @brief Returns the current bounding box for the bullet.
 * The box is the segment from the previous position to the current one, in local space.
 */
const CMPUT350::Rect& Bullet::GetBounds() { return mBounds; }

/**
 * @brief Returns this bullet's shapes. The one shape is the travel line rebuilt in Update.
 */
const std::vector<CMPUT350::Shape>& Bullet::GetShapes() { return mShapes; }

/**
 * Returns this bullet's location
 */
CMPUT350::Point2D Bullet::GetLocation() const { return mLocation; }

/**
 * Returns this bullet's rotation
 */
float Bullet::GetRotation() const { return mRotation; }