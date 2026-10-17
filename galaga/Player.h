#ifndef PLAYER_H
#define PLAYER_H

#include "CollisionObject.h"
#include "EngineView.h"
#include <memory>
#include <vector>
class Bullet;  // forward declaration for our member variable

class Player : public CMPUT350::CollisionObject {
public:
    Player(CMPUT350::Point2D loc);

    // GameObject Functions
    void Initialize(CMPUT350::GameContext* context) override;
    void Update(CMPUT350::GameContext* context) override;
    void LateUpdate(CMPUT350::GameContext* context) override;
    bool IsAlive() const override;
    void Kill() override;
    void ReceiveNotification(const std::string &key);

    // Graphics Object Functions
    void RenderBackground(CMPUT350::GameContext* context) override;
    void RenderForeground(CMPUT350::GameContext* context) override;
    // TODO: finish these
    CMPUT350::Point2D GetLocation() const override;
    float GetRotation() const override;

    // Collision Object Functions
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    const CMPUT350::Rect& GetBounds() override;
    // TODO: finish
    const std::vector<CMPUT350::Shape> &GetShapes() override;

private:
    CMPUT350::Point2D mLocation{};
    bool mIsAlive{};
    float mHalfSize{20.f};
    std::vector<std::weak_ptr<Bullet>> mBullets{};
    CMPUT350::EngineView *mEngineView{nullptr};  // save engine view for firing bullets
    std::weak_ptr<CMPUT350::GameObject> mSelf{};
};

#endif
