#pragma once

#include "GameObject.h"
#include "MathUtil.h"

// Waiting, Startup, Waves, Gameplay, GameOver, then back to Waiting.
class Galaga : public CMPUT350::GameObject {
public:
    Galaga(int width, int height);

    void Initialize(CMPUT350::GameContext* context) override;
    // Spawns the ship at the start of a level and releases the five waves
    void Update(CMPUT350::GameContext* context) override;
    // Runs after collision, so deaths from this frame are already counted
    void LateUpdate(CMPUT350::GameContext* context) override;
    // Draws the score, lives, and level. Draws the "insert coin" message while waiting for a coin
    void RenderUI(CMPUT350::GameContext* context) override;
    void ReceiveNotification(const std::string& key) override;

private:
    enum class State { Waiting, Startup, Waves, Gameplay, GameOver };

    void SpawnPlayer(CMPUT350::GameContext* context);
    void SpawnOneEnemy(CMPUT350::GameContext* context);
    // Removes ships, bullets, and the player without adding score for that cleanup
    void DestroyPlayfield(CMPUT350::GameContext* context);
    // True while any enemy is entering, diving, or returning
    bool AnyTravelling(CMPUT350::GameContext* context) const;
    bool PlayerIsAlive(CMPUT350::GameContext* context) const;
    CMPUT350::Point2D PlayerLocation(CMPUT350::GameContext* context) const;

    State mState{State::Waiting};
    int mWidth;
    int mHeight;
    int mCredits{0};
    int mScore{0};
    int mLives{3};
    int mLevel{1};
    int mStateTimer{0};     // frames spent in the current state
    int mSpawnTimer{0};     // frames since the last ship in this wave
    int mWave{0};           // five waves
    int mShipsThisWave{0};  // 0..8
    int mTotalSpawned{0};   // 0..39, also picks the formation slot and enemy type
    int mEnemiesAlive{0};
    int mAttackTimer{0};
    int mNextAttackSlot{0};  // which formation ship is asked to dive next
    int mRespawnTimer{0};
    bool mWavesDone{false};
    bool mStarsMade{false};
    bool mPlayerDown{false};
    bool mIgnoreScore{false};  // set while cleaning up, so Kill() does not add points
    bool mClearedField{false};
};