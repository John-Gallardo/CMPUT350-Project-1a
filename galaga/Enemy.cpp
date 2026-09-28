#include "Enemy.h"

#include "Bullet.h"
#include "Player.h"

/**
 * @brief Constructs an enemy with its location
 */
Enemy::Enemy(CMPUT350::Point2D loc) 
: mLocation{loc} {
}

/**
 * @brief Initializes other member variables not explicitly initialized in the constructor
 */
void Enemy::Initialize(CMPUT350::GameContext* context) {
    mIsAlive = true;
    // using center, radius constructor. radius is arbitrarily chosen
    mEnemyRender = CMPUT350::Rect(mLocation, 50.0f);
    mBoundingBox = CMPUT350::Rect(mLocation, 50.0f);
}

/**
 * @brief Updates the enemy's position
 */
void Enemy::Update(CMPUT350::GameContext* context) {
    // NOTE: this does nothing in 1a.
}

/**
 * @brief Late update for this enemy (does nothing)
 */
void Enemy::LateUpdate(CMPUT350::GameContext* context) {
    return;
}

/**
 * @brief Handles key events for the enemy (processes nothing)
 */
bool Enemy::HandleKeyEvent(CMPUT350::GameContext* context, char key) { return false; }

/**
 * @brief Renders this enemy's background (does nothing)
 */
void Enemy::RenderBackground(CMPUT350::GameContext* context) {
    // NOTE: enemies don't have backgrounds, at least in this project version
    return;
}

/**
 * @brief Renders the foreground for the enemy (a square)
 */
void Enemy::RenderForeground(CMPUT350::GameContext* context) {
    context->ScreenContext->DrawRect(mEnemyRender, CMPUT350::Colors::green);
}

/**
 * @brief Processes enemy collisions with 1. a player, or 2. a bullet.
 */
void Enemy::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) {
    const CMPUT350::Rect &objBounds{obj->GetBounds()};
    // check if bounding boxes are NOT intersecting
    if (mBoundingBox.topLeft.x + mBoundingBox.width < objBounds.topLeft.x ||  // 1. Check if enemy is to the left
        mBoundingBox.topLeft.y > objBounds.topLeft.y + objBounds.height ||    // 2. Check if enemy is below
        mBoundingBox.topLeft.x > objBounds.topLeft.x + objBounds.width ||     // 3. Check if enemy is to the right
        mBoundingBox.topLeft.y + mBoundingBox.height < objBounds.topLeft.y)   // 4. Check if enemy is above 
    {
        return;
    }

    // Case 1: enemy & player bullet intersection -> kill bullet
    auto bullet{std::dynamic_pointer_cast<Bullet>(obj)};
    if (bullet && bullet->IsPlayerBullet()) {
        bullet->Kill();
        return;
    }

    // Case 2: enemy & player collision -> kill player
    auto player{std::dynamic_pointer_cast<Player>(obj)};
    if (player) {
        player->Kill();
        return;
    }
}

/**
 * @brief Kills this enemy.
 */
void Enemy::Kill() {
    mIsAlive = false;
}

/**
 * @brief Returns the status of the enemy (dead or alive)
 */
bool Enemy::IsAlive() const {
    return mIsAlive;
}

/**
 * @brief Returns the bounding box for this enemy
 */
const CMPUT350::Rect& Enemy::GetBounds() {
    return mBoundingBox;
}
