#ifndef ENEMY_H
#define ENEMY_H

#include <vector>

#include "CollisionObject.h"
#include "GameContext.h"

class Enemy : public CMPUT350::CollisionObject {
public:
    // Constructs an enemy with its home location. The path is used for entering and returning.
    Enemy(CMPUT350::Point2D home, const std::vector<CMPUT350::Point2D>& path, int health,
          int formationScore, int flyingScore, float speed, float radius, int firePeriod);

    void Initialize(CMPUT350::GameContext* context) override;
    // Moves along the current path, sways in formation, and fires during an attack
    // Runs before collision, so the hit test sees the position for this frame
    void Update(CMPUT350::GameContext* context) override;
    bool IsAlive() const override;
    // Notifies Galaga with the score string and "EnemyDown". Safe to call more than once
    void Kill() override;

    CMPUT350::Point2D GetLocation() const override;
    float GetRotation() const override;
    // Player bullets and the player can collide with enemies, so we need a bounding box and shapes
    // for collision detection
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    const CMPUT350::Rect& GetBounds() override;
    const std::vector<CMPUT350::Shape>& GetShapes() override;

    // Galaga calls this. Returns false when the ship is not sitting in formation.
    virtual bool Attack(CMPUT350::GameContext* context, const CMPUT350::Point2D& target) = 0;
    bool IsInFormation() const;
    // True while entering, diving, or returning. Galaga waits for this before a respawn.
    bool IsTravelling() const;

protected:
    // Entering: fly in on the path Galaga gave us
    // Formation: sit on mHome and sway with the shared frame clock
    // Attacking: follow the curve built by Attack
    // Returning: fly back to mHome after the dive
    enum class Mode { Entering, Formation, Attacking, Returning };

    // Stores a new cubic path and starts walking it from distance 0
    bool StartPath(const std::vector<CMPUT350::Point2D>& path, Mode mode);

    CMPUT350::Point2D mHome{};
    CMPUT350::Point2D mLocation{};  // current world position, also the rotation pivot
    float mRotation{0.f};           // facing down the screen
    int mHealth{1};
    float mRadius{16.f};  // collision circle, also used to size the local bounds

private:
    // One sample of the Bezier curve. distance is the pixel length from the start of the path,
    // so movement can advance by a constant number of pixels instead of a constant t
    struct PathSample {
        float t;
        float distance;
        CMPUT350::Point2D position;
    };

    void BuildTable();
    void AdvanceAlongPath();
    void MaybeFire(CMPUT350::GameContext* context);
    CMPUT350::Point2D PositionAt(float distance, float& tOut) const;
    // Turns the Bezier slope into a rotation whose local +y points along that slope.
    float FacingRotation(CMPUT350::Point2D slope) const;

    std::vector<CMPUT350::Point2D> mPath;
    std::vector<PathSample> mTable;
    std::vector<CMPUT350::Shape> mShapes;
    CMPUT350::Rect mBounds{};
    Mode mMode{Mode::Entering};
    float mDistance{0.f};  // pixels traveled along mTable
    float mPathLength{0.f};
    float mSpeed{5.f};  // pixels per frame
    int mFormationScore{50};
    int mFlyingScore{100};
    int mFirePeriod{20};  // frames between shots while attacking
    int mFireCooldown{0};
    bool mIsAlive{false};
    std::weak_ptr<CMPUT350::GameObject> mSelf;  // passed to bullets so they die with this ship
    CMPUT350::NotificationManager* mNotes{nullptr};
};

// Drawn as a yellow triangle with 1 health. Dives straight down, then returns to formation.
class Enemy1 : public Enemy {
public:
    Enemy1(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D>& path);
    bool Attack(CMPUT350::GameContext* context, const CMPUT350::Point2D& target) override;
    void RenderForeground(CMPUT350::GameContext* context) override;
};

//  Drawn as two circles with 1 health. Sweeps out to one side before reaching the player.
class Enemy2 : public Enemy {
public:
    Enemy2(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D>& path);
    bool Attack(CMPUT350::GameContext* context, const CMPUT350::Point2D& target) override;
    void RenderForeground(CMPUT350::GameContext* context) override;
};

// More health than the other enemies. Turns yellow after one hit. Slower, and moves sideways before
// the dive
class Enemy3 : public Enemy {
public:
    Enemy3(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D>& path);
    bool Attack(CMPUT350::GameContext* context, const CMPUT350::Point2D& target) override;
    void RenderForeground(CMPUT350::GameContext* context) override;
};

#endif