#ifndef PLAYER_H
#define PLAYER_H

#include <memory>
#include <vector>

#include "CollisionObject.h"
#include "EngineView.h"

class Bullet;

namespace CMPUT350 {
class NotificationManager;
}

/// @brief Player class for Galaga game
class Player : public CMPUT350::CollisionObject {
public:
    Player(CMPUT350::Point2D loc);

    void Initialize(CMPUT350::GameContext* context) override;
    // Applies the held movement keys and fires while space is held
    void Update(CMPUT350::GameContext* context) override;
    void LateUpdate(CMPUT350::GameContext* context) override;
    bool IsAlive() const override;
    // Kills the player and removes it from the engine view. The player is not removed from the
    // engine view until the next update.
    void Kill() override;
    // Handles key events for player movement and firing. Returns true if the key was handled, false
    // otherwise.
    void ReceiveNotification(const std::string& key) override;

    void RenderBackground(CMPUT350::GameContext* context) override;
    void RenderForeground(CMPUT350::GameContext* context) override;
    CMPUT350::Point2D GetLocation() const override;
    float GetRotation() const override;

    // Ignores our own bullets. An enemy bullet or an enemy ship destroys this ship.
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    const CMPUT350::Rect& GetBounds() override;
    const std::vector<CMPUT350::Shape>& GetShapes() override;

private:
    CMPUT350::Point2D mLocation{};
    bool mIsAlive{};
    float mHalfSize{20.f};
    float mRotation{};
    bool mLeft{false};
    bool mRight{false};
    bool mFireHeld{false};
    int mFireCooldown{0};
    CMPUT350::Rect mBounds{-20.f, -20.f, 40.f, 40.f};
    std::vector<CMPUT350::Shape> mShapes;
    std::vector<std::weak_ptr<Bullet>> mBullets{};
    CMPUT350::EngineView* mEngineView{nullptr};  // saved so firing does not need the context later
    std::weak_ptr<CMPUT350::GameObject> mSelf{};
    CMPUT350::NotificationManager* mNotes{nullptr};
};

#endif