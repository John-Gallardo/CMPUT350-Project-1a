#include "Enemy.h"

#include <cmath>
#include <memory>
#include <string>

#include "Bezier.h"
#include "Bullet.h"
#include "Player.h"

// The ship starts at the first control point so the first frame is already on the curve.
Enemy::Enemy(CMPUT350::Point2D home, const std::vector<CMPUT350::Point2D>& path, int health,
             int formationScore, int flyingScore, float speed, float radius, int firePeriod)
    : mHome(home),
      mHealth(health),
      mRadius(radius),
      mSpeed(speed),
      mFormationScore(formationScore),
      mFlyingScore(flyingScore),
      mFirePeriod(firePeriod) {
    mLocation = path.empty() ? home : path.front();
    StartPath(path, Mode::Entering);
}

// CurrObject is only set during Initialize, so the weak pointer to this ship is saved here.
// The circle and the box are both around local (0, 0). The engine adds GetLocation() later.
void Enemy::Initialize(CMPUT350::GameContext* context) {
    mIsAlive = true;
    mSelf = context->CurrObject;
    mNotes = context->mNotificationManager;
    mShapes.clear();
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Circle(CMPUT350::Point2D(0.f, 0.f), mRadius)));
    mBounds = CMPUT350::Rect(-mRadius, -mRadius, mRadius * 2.f, mRadius * 2.f);
}

// The path is built by StartPath. Formation sways with the shared frame clock. Attacks and returns
// follow the Bezier curve. Firing is limited to the attack.
bool Enemy::StartPath(const std::vector<CMPUT350::Point2D>& path, Mode mode) {
    if (path.size() < 4 || ((path.size() - 1) % 3) != 0) return false;
    mPath = path;
    mMode = mode;
    mDistance = 0.f;
    BuildTable();
    return true;
}

// The Bezier curve is sampled at a fixed number of steps. Each sample stores the distance from the
// start of the curve, so movement can advance by a constant number of pixels instead of a constant
// t
void Enemy::BuildTable() {
    mTable.clear();
    int segments = static_cast<int>((mPath.size() - 1) / 3);
    const int steps = segments * 24;
    CMPUT350::Point2D previous = CMPUT350::Bezier::GetPoint(mPath, 0.f);
    float covered = 0.f;
    mTable.push_back({0.f, 0.f, previous});
    for (int step = 1; step <= steps; step++) {
        float t = segments * (static_cast<float>(step) / static_cast<float>(steps));
        CMPUT350::Point2D point = CMPUT350::Bezier::GetPoint(mPath, t);
        covered += static_cast<float>(previous.Distance(point));
        mTable.push_back({t, covered, point});
        previous = point;
    }
    mPathLength = mTable.empty() ? 0.f : mTable.back().distance;
}

// The path is walked by distance, not t. The Bezier slope is turned into a rotation whose local +y
// points along that slope. The ship is dead if it has no health, so it doesn't move or fire. The
// ship is in formation if it is not entering, attacking, or returning.
CMPUT350::Point2D Enemy::PositionAt(float distance, float& tOut) const {
    if (mTable.empty()) {
        tOut = 0.f;
        return mHome;
    }
    if (distance <= 0.f) {
        tOut = mTable.front().t;
        return mTable.front().position;
    }
    if (distance >= mTable.back().distance) {
        tOut = mTable.back().t;
        return mTable.back().position;
    }
    for (size_t i = 1; i < mTable.size(); i++) {
        if (mTable[i].distance < distance) continue;
        float span = mTable[i].distance - mTable[i - 1].distance;
        float u = span <= 0.f ? 0.f : (distance - mTable[i - 1].distance) / span;
        tOut = mTable[i - 1].t + (mTable[i].t - mTable[i - 1].t) * u;
        return mTable[i - 1].position + (mTable[i].position - mTable[i - 1].position) * u;
    }
    tOut = mTable.back().t;
    return mTable.back().position;
}

// The slope is a vector tangent to the Bezier curve. The rotation is in radians, and 0 means facing
// down the screen. The slope is normalized so the rotation is not affected by the speed of the ship
float Enemy::FacingRotation(CMPUT350::Point2D slope) const {
    float len = std::sqrt(slope.x * slope.x + slope.y * slope.y);
    if (len < 1e-4f) return mRotation;
    return std::atan2(-slope.x / len, slope.y / len);
}

// mSpeed is pixels per frame, read from the distance table rather than added to Bezier t.
// Ending an attack starts the return curve. Ending an entrance or a return snaps to the home slot.
void Enemy::AdvanceAlongPath() {
    mDistance += mSpeed;
    if (mDistance >= mPathLength) {
        if (mMode == Mode::Attacking) {
            CMPUT350::Point2D side = mLocation.x < mHome.x ? CMPUT350::Point2D(-90.f, 50.f)
                                                           : CMPUT350::Point2D(90.f, 50.f);
            StartPath({mLocation, mLocation + CMPUT350::Point2D(0.f, 70.f), mHome + side, mHome},
                      Mode::Returning);
            return;
        }
        mMode = Mode::Formation;
        mLocation = mHome;
        mRotation = 0.f;
        return;
    }
    float t = 0.f;
    mLocation = PositionAt(mDistance, t);
    mRotation = FacingRotation(CMPUT350::Bezier::GetSlope(mPath, t));
}

// Shots only happen on a timer, and only while this function is called from an attack.
// The aim is toward the live player. With no player, the shot goes straight down.
void Enemy::MaybeFire(CMPUT350::GameContext* context) {
    if (mFireCooldown > 0) {
        mFireCooldown--;
        return;
    }
    mFireCooldown = mFirePeriod;
    CMPUT350::Point2D aim(0.f, 6.f);
    for (auto it = context->mEngineView->begin(); it != context->mEngineView->end(); ++it) {
        auto player = std::dynamic_pointer_cast<Player>(*it);
        if (!player || !player->IsAlive()) continue;
        CMPUT350::Point2D delta = player->GetLocation() - mLocation;
        float dist = static_cast<float>(delta.Distance({0.f, 0.f}));
        if (dist > 1.f) aim = delta * (6.f / dist);
        break;
    }
    // false means this is an enemy bullet. mSelf makes the bullet die if this ship dies.
    context->mEngineView->AddGameObject(std::make_shared<Bullet>(mLocation, aim, false, mSelf));
}

// The ship is dead if it has no health, so it doesn't move or fire
void Enemy::Update(CMPUT350::GameContext* context) {
    if (!mIsAlive) return;
    if (mMode == Mode::Formation) {
        float sway = std::sin(context->currentFrame * 0.08f) * 16.f;
        mLocation = mHome + CMPUT350::Point2D(sway, 0.f);
        mRotation = 0.f;
        return;
    }
    AdvanceAlongPath();
    if (mMode == Mode::Attacking) MaybeFire(context);
}

void Enemy::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) {
    auto bullet = std::dynamic_pointer_cast<Bullet>(obj);
    if (bullet && bullet->IsPlayerBullet()) {
        mHealth--;
        if (mHealth <= 0) Kill();
        return;
    }
    if (std::dynamic_pointer_cast<Player>(obj)) Kill();
}

void Enemy::Kill() {
    if (!mIsAlive) return;
    mIsAlive = false;
    if (!mNotes) return;
    int points = mMode == Mode::Formation ? mFormationScore : mFlyingScore;
    mNotes->Notify("Score" + std::to_string(points));
    mNotes->Notify("EnemyDown");
}

bool Enemy::IsAlive() const { return mIsAlive; }

// Galaga uses this to choose who may be sent on a dive
bool Enemy::IsInFormation() const { return mIsAlive && mMode == Mode::Formation; }

// Galaga waits until this is false before respawning the player
bool Enemy::IsTravelling() const {
    return mIsAlive &&
           (mMode == Mode::Entering || mMode == Mode::Attacking || mMode == Mode::Returning);
}

CMPUT350::Point2D Enemy::GetLocation() const { return mLocation; }
float Enemy::GetRotation() const { return mRotation; }
const CMPUT350::Rect& Enemy::GetBounds() { return mBounds; }
const std::vector<CMPUT350::Shape>& Enemy::GetShapes() { return mShapes; }

// 1 health, 50 points in formation, 100 while flying, 6 pixels per frame, radius 16, fire every 22
// frames.
Enemy1::Enemy1(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D>& path)
    : Enemy(loc, path, 1, 50, 100, 6.f, 16.f, 22) {}

// pattern is straight down, then back to formation. The path is built by StartPath.
bool Enemy1::Attack(CMPUT350::GameContext*, const CMPUT350::Point2D& target) {
    if (!IsInFormation()) return false;
    return StartPath({mLocation, mLocation + CMPUT350::Point2D(0.f, 160.f),
                      target + CMPUT350::Point2D(-40.f, -60.f), target},
                     Mode::Attacking);
}

// Drawn as a yellow triangle. The rectangle is local, around (0, 0).
void Enemy1::RenderForeground(CMPUT350::GameContext* context) {
    auto* draw = context->ScreenContext;
    draw->DrawLine({0.f, 16.f}, {-16.f, -8.f}, 2.f, CMPUT350::Colors::yellow);
    draw->DrawLine({-16.f, -8.f}, {16.f, -8.f}, 2.f, CMPUT350::Colors::yellow);
    draw->DrawLine({16.f, -8.f}, {0.f, 16.f}, 2.f, CMPUT350::Colors::yellow);
}

// 1 health, 80 / 160 points, a little slower than Enemy1.
Enemy2::Enemy2(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D>& path)
    : Enemy(loc, path, 1, 80, 160, 5.5f, 16.f, 20) {}

// pattern is a sweep to one side, then back to formation
bool Enemy2::Attack(CMPUT350::GameContext*, const CMPUT350::Point2D& target) {
    if (!IsInFormation()) return false;
    float side = mLocation.x < target.x ? -160.f : 160.f;
    return StartPath(
        {mLocation, mLocation + CMPUT350::Point2D(side, 40.f),
         target + CMPUT350::Point2D(side, -120.f), target + CMPUT350::Point2D(0.f, 10.f)},
        Mode::Attacking);
}

// Two circles so this ship is easy to tell apart from the triangle.
void Enemy2::RenderForeground(CMPUT350::GameContext* context) {
    auto* draw = context->ScreenContext;
    draw->DrawCircle({-10.f, 0.f}, 10.f, CMPUT350::Colors::cyan);
    draw->DrawCircle({10.f, 0.f}, 10.f, CMPUT350::Colors::blue);
    draw->DrawCircle({0.f, 8.f}, 4.f, CMPUT350::Colors::white);
}

// More health than the other enemies. Turns yellow after one hit. Slower, and moves sideways before
// the dive
Enemy3::Enemy3(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D>& path)
    : Enemy(loc, path, 2, 150, 400, 3.5f, 24.f, 12) {}

// Moves to the top of the screen before diving, so the path is different from the other two
bool Enemy3::Attack(CMPUT350::GameContext*, const CMPUT350::Point2D& target) {
    if (!IsInFormation()) return false;
    return StartPath({mLocation, CMPUT350::Point2D(mLocation.x, 40.f),
                      CMPUT350::Point2D(target.x, 80.f), target + CMPUT350::Point2D(0.f, -20.f)},
                     Mode::Attacking);
}

// Red at full health, yellow after the first hit. The rectangle is local, around (0, 0).
void Enemy3::RenderForeground(CMPUT350::GameContext* context) {
    CMPUT350::RGBColor color = mHealth > 1 ? CMPUT350::Colors::red : CMPUT350::Colors::yellow;
    context->ScreenContext->DrawRect(CMPUT350::Rect(-22.f, -18.f, 44.f, 36.f), color);
    context->ScreenContext->DrawCircle({0.f, 6.f}, 5.f, CMPUT350::Colors::white);
}