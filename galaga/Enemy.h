#ifndef ENEMY_H
#define ENEMY_H

#include "CollisionObject.h"
#include "GameContext.h"

class Enemy : public CMPUT350::CollisionObject
{
public:
    Enemy(CMPUT350::Point2D loc);

    // GameObject Functions
    void Initialize(CMPUT350::GameContext* context) override;
    void Update(CMPUT350::GameContext* context) override;
    void LateUpdate(CMPUT350::GameContext* context) override;
    bool IsAlive() const override;
    void Kill() override;

    // Graphics Object Functions
    void RenderBackground(CMPUT350::GameContext* context) override;
    void RenderForeground(CMPUT350::GameContext* context) override;
    // TODO: implement these!! new function overrides
    CMPUT350::Point2D GetLocation() const override;
    float GetRotation() const override;

    // Collision Object Functions
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    const CMPUT350::Rect& GetBounds() override;
    // TODO: implement
    const std::vector<CMPUT350::Shape> &GetShapes() override;
private:
    CMPUT350::Point2D mLocation{};
    bool mIsAlive{};
    CMPUT350::Rect mEnemyRender{};  // enemy is rendered as a square
    CMPUT350::Rect mBoundingBox{};
    std::vector<CMPUT350::Shape> mShapes{};
};


#endif

