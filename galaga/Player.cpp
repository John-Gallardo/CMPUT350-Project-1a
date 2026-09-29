#include "Player.h"

#include <algorithm>
#include <cassert>

#include "Bullet.h"

/**
 * @brief Constructor, only sets player location to loc
 */
Player::Player(CMPUT350::Point2D loc) : mLocation{loc} {}

/**
 * @brief Initializes other member variables of the player
 */
void Player::Initialize(CMPUT350::GameContext* context) { mIsAlive = true; }

void Player::Update(CMPUT350::GameContext* context) {}

void Player::LateUpdate(CMPUT350::GameContext* context) {}

bool Player::HandleKeyEvent(CMPUT350::GameContext* context, char key) {
    const float width = static_cast<float>(context->ScreenContext->GetWindowWidth());
    const float height = static_cast<float>(context->ScreenContext->GetWindowHeight());

    if (key == 'a' || key == 'A') {
        mLocation.x -= 15.f;
        if (mLocation.x < mHalfSize) mLocation.x = mHalfSize;
        return true;
    }
    if (key == 'd' || key == 'D') {
        mLocation.x += 15.f;
        if (mLocation.x > width - mHalfSize) {
            mLocation.x = width - mHalfSize;
        }
        return true;
    }
    if (key == ' ') {
        // remove expired bullets from the vector
        mBullets.erase(
            std::remove_if(mBullets.begin(), mBullets.end(),
                           [](const std::weak_ptr<Bullet>& bullet) { return bullet.expired(); }),
            mBullets.end());
        // check if we can shoot a bullet
        if (mBullets.size() >= 2) {
            return true;
        }
        auto bullet =
            std::make_shared<Bullet>(CMPUT350::Point2D(mLocation.x, mLocation.y - mHalfSize),
                                     CMPUT350::Point2D(0.f, -15.f), true);
        mBullets.push_back(bullet);
        context->mEngineView->AddGameObject(bullet);
        return true;
    }
    return false;
}

void Player::RenderBackground(CMPUT350::GameContext* context) {}

void Player::RenderForeground(CMPUT350::GameContext* context) {
    CMPUT350::DrawContext* draw = context->ScreenContext;
    const float x = mLocation.x;
    const float y = mLocation.y;
    const float h = mHalfSize;
    const CMPUT350::Point2D nose(x, y - h);
    const CMPUT350::Point2D left(x - h, y + h * 0.6f);
    const CMPUT350::Point2D right(x + h, y + h * 0.6f);
    const CMPUT350::Point2D tail(x, y + h * 0.2f);
    draw->DrawLine(nose, left, 2.f, CMPUT350::Colors::cyan);
    draw->DrawLine(left, tail, 2.f, CMPUT350::Colors::cyan);
    draw->DrawLine(tail, right, 2.f, CMPUT350::Colors::cyan);
    draw->DrawLine(right, nose, 2.f, CMPUT350::Colors::cyan);
    draw->DrawCircle(CMPUT350::Point2D(x, y), 4.f, CMPUT350::Colors::white);
}

void Player::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) {
    auto bullet = std::dynamic_pointer_cast<Bullet>(obj);
    if (bullet && bullet->IsPlayerBullet()) {
        return;
    }
}

/**
 * @brief Kills the player
 */
void Player::Kill() { mIsAlive = false; }

/**
 * @brief Returns true if the player is alive, false if not
 */
bool Player::IsAlive() const { return mIsAlive; }

const CMPUT350::Rect& Player::GetBounds() {
    // TODO: Update code
    static CMPUT350::Rect sBounds(mLocation, mHalfSize);
    sBounds = CMPUT350::Rect(mLocation, mHalfSize);
    return sBounds;
}
