#include "Player.h"

#include <algorithm>
#include <memory>

#include "Bullet.h"
#include "Enemy.h"
#include "GameContext.h"

// Constructor, only sets player location to loc.
Player::Player(CMPUT350::Point2D loc) : mLocation{loc} {}

// CurrOject is set to this player before Initialize is called, so we can save a weak pointer to it
// for firing bullets. The notification manager is also saved for firing and for sending
// "PlayerDown" when killed
void Player::Initialize(CMPUT350::GameContext* context) {
    mIsAlive = true;
    mRotation = 0.0f;
    mSelf = context->CurrObject;
    mNotes = context->mNotificationManager;
    mEngineView = context->mEngineView;
    mNotes->Register(mSelf, "MoveLeft");
    mNotes->Register(mSelf, "StopLeft");
    mNotes->Register(mSelf, "MoveRight");
    mNotes->Register(mSelf, "StopRight");
    mNotes->Register(mSelf, "FirePressed");
    mNotes->Register(mSelf, "FireReleased");

    // The ship is a triangle with a small circle in the middle. The bounding box is a square around
    // the triangle.
    mShapes.clear();
    mShapes.push_back(
        CMPUT350::Shape(CMPUT350::Circle(CMPUT350::Point2D(0.f, 0.f), mHalfSize * 0.8f)));
    mBounds = CMPUT350::Rect(-mHalfSize, -mHalfSize, mHalfSize * 2.f, mHalfSize * 2.f);
}

// Handles key events for player movement and firing
void Player::ReceiveNotification(const std::string& key) {
    if (key == "MoveLeft") mLeft = true;
    if (key == "StopLeft") mLeft = false;
    if (key == "MoveRight") mRight = true;
    if (key == "StopRight") mRight = false;
    if (key == "FirePressed") mFireHeld = true;
    if (key == "FireReleased") mFireHeld = false;
}

// 15 pixels per frame is the old per-press step, now applied every frame while the key is held.
// The ship is kept on screen by clamping the x coordinate to the half-width of the ship and the
// window width minus the half-width
void Player::Update(CMPUT350::GameContext* context) {
    if (!mIsAlive) return;
    float width = static_cast<float>(context->ScreenContext->GetWindowWidth());
    if (mLeft && !mRight) mLocation.x -= 15.f;
    if (mRight && !mLeft) mLocation.x += 15.f;
    if (mLocation.x < mHalfSize) mLocation.x = mHalfSize;
    if (mLocation.x > width - mHalfSize) mLocation.x = width - mHalfSize;

    if (mFireCooldown > 0) mFireCooldown--;
    if (!mFireHeld || mFireCooldown > 0 || mEngineView == nullptr) return;

    // handle the bullets vector, removing any dead bullets and returning if there are already 2
    // bullets on screen
    mBullets.erase(std::remove_if(mBullets.begin(), mBullets.end(),
                                  [](const std::weak_ptr<Bullet>& bullet) {
                                      auto live = bullet.lock();
                                      return !live || !live->IsAlive();
                                  }),
                   mBullets.end());
    if (mBullets.size() >= 2) return;

    // create a new bullet and add it to the engine view. The bullet is created at the nose of the
    // ship, with a heading straight up. The bullet is marked as a player bullet, and a weak pointer
    // to this player is passed to the bullet so it can ignore its own bullets.
    auto bullet = std::make_shared<Bullet>(mLocation + CMPUT350::Point2D(0.f, -mHalfSize),
                                           CMPUT350::Point2D(0.f, -15.f), true, mSelf);
    mBullets.push_back(bullet);
    mEngineView->AddGameObject(bullet);
    mFireCooldown = 8;
}

void Player::LateUpdate(CMPUT350::GameContext* context) {}

void Player::RenderBackground(CMPUT350::GameContext* context) {}

// Draws the ship as a triangle with a small circle in the middle. The triangle is drawn with cyan
// lines, and the circle is drawn in white.
void Player::RenderForeground(CMPUT350::GameContext* context) {
    CMPUT350::DrawContext* draw = context->ScreenContext;
    const float h = mHalfSize;
    const CMPUT350::Point2D nose(0.f, -h);
    const CMPUT350::Point2D left(-h, h * 0.6f);
    const CMPUT350::Point2D right(h, h * 0.6f);
    const CMPUT350::Point2D tail(0.f, h * 0.2f);
    draw->DrawLine(nose, left, 2.f, CMPUT350::Colors::cyan);
    draw->DrawLine(left, tail, 2.f, CMPUT350::Colors::cyan);
    draw->DrawLine(tail, right, 2.f, CMPUT350::Colors::cyan);
    draw->DrawLine(right, nose, 2.f, CMPUT350::Colors::cyan);
    draw->DrawCircle(CMPUT350::Point2D(0.f, 0.f), 4.f, CMPUT350::Colors::white);
}

// The player is killed if it collides with an enemy or an enemy bullet. The player ignores its own
// bullets
void Player::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) {
    auto bullet = std::dynamic_pointer_cast<Bullet>(obj);
    if (bullet && bullet->IsPlayerBullet()) return;
    if (bullet && !bullet->IsPlayerBullet()) Kill();
    if (std::dynamic_pointer_cast<Enemy>(obj)) Kill();
}

// The alive flag stops a second call from sending "PlayerDown" again
void Player::Kill() {
    if (!mIsAlive) return;
    mIsAlive = false;
    if (mNotes) mNotes->Notify("PlayerDown");
}

bool Player::IsAlive() const { return mIsAlive; }

const CMPUT350::Rect& Player::GetBounds() { return mBounds; }

CMPUT350::Point2D Player::GetLocation() const { return mLocation; }

float Player::GetRotation() const { return mRotation; }

const std::vector<CMPUT350::Shape>& Player::GetShapes() { return mShapes; }