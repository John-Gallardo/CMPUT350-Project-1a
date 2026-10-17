#include "Galaga.h"

#include <memory>
#include <string>
#include <vector>

#include "Bullet.h"
#include "Enemy.h"
#include "Keyboard.h"
#include "Player.h"
#include "Stars.h"

// The main game object. Owns the whole game. main adds this one object, and this object creates
// everything else.
Galaga::Galaga(int width, int height) : mWidth(width), mHeight(height) {}

void Galaga::Initialize(CMPUT350::GameContext* context) {
    auto* view = context->mEngineView;
    // A and Left are the same message. The player listens for the message, not the key.
    view->InstallKeyDownNotification(CMPUT350::Key::A, "MoveLeft");
    view->InstallKeyDownNotification(CMPUT350::Key::Left, "MoveLeft");
    view->InstallKeyUpNotification(CMPUT350::Key::A, "StopLeft");
    view->InstallKeyUpNotification(CMPUT350::Key::Left, "StopLeft");
    view->InstallKeyDownNotification(CMPUT350::Key::D, "MoveRight");
    view->InstallKeyDownNotification(CMPUT350::Key::Right, "MoveRight");
    view->InstallKeyUpNotification(CMPUT350::Key::D, "StopRight");
    view->InstallKeyUpNotification(CMPUT350::Key::Right, "StopRight");
    view->InstallKeyDownNotification(CMPUT350::Key::Space, "FirePressed");
    view->InstallKeyUpNotification(CMPUT350::Key::Space, "FireReleased");
    view->InstallKeyDownNotification(CMPUT350::Key::Num5, "InsertCoin");
    view->InstallKeyDownNotification(CMPUT350::Key::Num1, "StartGame");

    auto* notes = context->mNotificationManager;
    notes->Register(context->CurrObject, "InsertCoin");
    notes->Register(context->CurrObject, "StartGame");
    notes->Register(context->CurrObject, "EnemyDown");
    notes->Register(context->CurrObject, "PlayerDown");
    // The score messages are sent by the enemies and bullets. The player listens for them to add to
    // the score.
    notes->Register(context->CurrObject, "Score50");
    notes->Register(context->CurrObject, "Score80");
    notes->Register(context->CurrObject, "Score100");
    notes->Register(context->CurrObject, "Score150");
    notes->Register(context->CurrObject, "Score160");
    notes->Register(context->CurrObject, "Score400");
}

void Galaga::ReceiveNotification(const std::string& key) {
    if (mIgnoreScore) return;
    if (key == "InsertCoin" && mState == State::Waiting) mCredits++;
    if (key == "StartGame" && mState == State::Waiting && mCredits > 0) {
        mCredits--;
        mScore = 0;
        mLives = 3;
        mLevel = 1;
        mWave = 0;
        mShipsThisWave = 0;
        mTotalSpawned = 0;
        mEnemiesAlive = 0;
        mWavesDone = false;
        mPlayerDown = false;
        mState = State::Startup;
        mStateTimer = 0;
    }
    if (key == "PlayerDown") {
        mPlayerDown = true;
        mLives--;
        mRespawnTimer = 0;
    }
    if (key == "EnemyDown" && mEnemiesAlive > 0) mEnemiesAlive--;
    if (key.rfind("Score", 0) == 0 && key.size() > 5) mScore += std::stoi(key.substr(5));
}

// Skips the spawn when a ship is already alive, so the next level does not add a second player
void Galaga::SpawnPlayer(CMPUT350::GameContext* context) {
    if (PlayerIsAlive(context)) return;
    context->mEngineView->AddGameObject(
        std::make_shared<Player>(CMPUT350::Point2D(mWidth / 2.f, mHeight - 80.f)));
}

// Spawns one enemy in the formation. The formation is 8 columns and 5 rows, for a total of 40
// ships. The ships enter from alternating sides, and each ship has a different path to its
// formation slot.
void Galaga::SpawnOneEnemy(CMPUT350::GameContext* context) {
    int index = mTotalSpawned++;
    int col = index % 8;
    int row = index / 8;
    float x = 70.f + col * 80.f;
    float y = 90.f + row * 64.f;
    CMPUT350::Point2D home(x, y);
    bool fromLeft = (row % 2) == 0;
    CMPUT350::Point2D start =
        fromLeft ? CMPUT350::Point2D(-40.f, y + 80.f) : CMPUT350::Point2D(mWidth + 40.f, y + 80.f);
    CMPUT350::Point2D control =
        fromLeft ? CMPUT350::Point2D(mWidth * 0.3f, 36.f) : CMPUT350::Point2D(mWidth * 0.7f, 36.f);
    // Four points is one cubic segment. The last point is the formation slot.
    std::vector<CMPUT350::Point2D> path{start, control, CMPUT350::Point2D(x, y - 40.f), home};

    std::shared_ptr<Enemy> enemy;
    if (index % 3 == 0)
        enemy = std::make_shared<Enemy1>(home, path);
    else if (index % 3 == 1)
        enemy = std::make_shared<Enemy2>(home, path);
    else
        enemy = std::make_shared<Enemy3>(home, path);
    context->mEngineView->AddGameObject(enemy);
    mEnemiesAlive++;
}

bool Galaga::AnyTravelling(CMPUT350::GameContext* context) const {
    for (auto it = context->mEngineView->begin(); it != context->mEngineView->end(); ++it) {
        auto enemy = std::dynamic_pointer_cast<Enemy>(*it);
        if (enemy && enemy->IsTravelling()) return true;
    }
    return false;
}

bool Galaga::PlayerIsAlive(CMPUT350::GameContext* context) const {
    for (auto it = context->mEngineView->begin(); it != context->mEngineView->end(); ++it) {
        auto player = std::dynamic_pointer_cast<Player>(*it);
        if (player && player->IsAlive()) return true;
    }
    return false;
}

CMPUT350::Point2D Galaga::PlayerLocation(CMPUT350::GameContext* context) const {
    for (auto it = context->mEngineView->begin(); it != context->mEngineView->end(); ++it) {
        auto player = std::dynamic_pointer_cast<Player>(*it);
        if (player && player->IsAlive()) return player->GetLocation();
    }
    return {mWidth / 2.f, mHeight - 80.f};
}

void Galaga::DestroyPlayfield(CMPUT350::GameContext* context) {
    mIgnoreScore = true;
    for (auto it = context->mEngineView->begin(); it != context->mEngineView->end(); ++it) {
        auto enemy = std::dynamic_pointer_cast<Enemy>(*it);
        auto bullet = std::dynamic_pointer_cast<Bullet>(*it);
        auto player = std::dynamic_pointer_cast<Player>(*it);
        if (enemy || bullet || player) (*it)->Kill();
    }
    mIgnoreScore = false;
    mEnemiesAlive = 0;
}

void Galaga::Update(CMPUT350::GameContext* context) {
    // mStateTimer is still 0 only on the first update after entering Startup.
    if (mState == State::Startup && mStateTimer == 0) {
        if (!mStarsMade) {
            context->mEngineView->AddGameObject(std::make_shared<Stars>(
                250,
                CMPUT350::Rect(0.f, 0.f, static_cast<float>(mWidth), static_cast<float>(mHeight))));
            mStarsMade = true;
        }
        SpawnPlayer(context);
    }

    // Eight ships, a short gap, then the next wave. Five waves is 40 ships.
    if (mState == State::Waves && !mWavesDone) {
        mSpawnTimer++;
        if (mShipsThisWave < 8 && mSpawnTimer >= 6) {
            SpawnOneEnemy(context);
            mShipsThisWave++;
            mSpawnTimer = 0;
        } else if (mShipsThisWave >= 8 && mSpawnTimer >= 40) {
            mWave++;
            mShipsThisWave = 0;
            mSpawnTimer = 0;
            if (mWave >= 5) mWavesDone = true;
        }
    }
}

void Galaga::LateUpdate(CMPUT350::GameContext* context) {
    mStateTimer++;

    if (mState == State::Startup && mStateTimer > 60) {
        mState = State::Waves;
        mStateTimer = 0;
        mWave = 0;
        mShipsThisWave = 0;
        mTotalSpawned = 0;
        mSpawnTimer = 0;
        mWavesDone = false;
        mAttackTimer = 0;
    }

    // Gameplay starts once every ship has reached its slot.
    if (mState == State::Waves && mWavesDone && !AnyTravelling(context)) {
        mState = State::Gameplay;
        mStateTimer = 0;
    }

    // Every 50 frames, ask two ships in formation to dive. The slot moves forward each time.
    if (mState == State::Gameplay && !mPlayerDown) {
        mAttackTimer++;
        if (mAttackTimer >= 50) {
            mAttackTimer = 0;
            int seen = 0;
            for (auto it = context->mEngineView->begin(); it != context->mEngineView->end(); ++it) {
                auto enemy = std::dynamic_pointer_cast<Enemy>(*it);
                if (!enemy || !enemy->IsInFormation()) continue;
                if (seen == mNextAttackSlot || seen == mNextAttackSlot + 1) {
                    enemy->Attack(context, PlayerLocation(context));
                }
                seen++;
            }
            if (seen > 0) mNextAttackSlot = (mNextAttackSlot + 2) % seen;
        }
    }

    if ((mState == State::Waves || mState == State::Gameplay) && mWavesDone && mEnemiesAlive == 0 &&
        !mPlayerDown) {
        mLevel++;
        mState = State::Startup;
        mStateTimer = 0;
        mTotalSpawned = 0;
    }

    if (mPlayerDown && mState != State::Waiting && mState != State::GameOver) {
        // Ships already in the air finish their path before the player comes back.
        if (!AnyTravelling(context)) {
            mRespawnTimer++;
            if (mLives <= 0) {
                mState = State::GameOver;
                mStateTimer = 0;
                mPlayerDown = false;
                mClearedField = false;
            } else if (mRespawnTimer > 60) {
                SpawnPlayer(context);
                mPlayerDown = false;
            }
        }
    }

    if (mState == State::GameOver) {
        if (!mClearedField) {
            DestroyPlayfield(context);
            mClearedField = true;
        }
        if (mStateTimer > 90) {
            mState = State::Waiting;
            mStateTimer = 0;
            mClearedField = false;
        }
    }
}

void Galaga::RenderUI(CMPUT350::GameContext* context) {
    auto* gui = context->GUIContext;
    float cx = mWidth / 2.f;
    if (mState == State::Waiting) {
        gui->DrawCenteredText("GALAGA", 52, {cx, 280.f}, CMPUT350::Colors::white);
        gui->DrawCenteredText("Press 5 to insert a coin", 24, {cx, 420.f},
                              CMPUT350::Colors::yellow);
        gui->DrawCenteredText("Press 1 to start", 24, {cx, 470.f}, CMPUT350::Colors::yellow);
        gui->DrawCenteredText("Credits: " + std::to_string(mCredits), 24, {cx, 540.f},
                              CMPUT350::Colors::white);
        return;
    }
    gui->DrawText("SCORE " + std::to_string(mScore), 22, {24.f, 16.f}, CMPUT350::Colors::white);
    gui->DrawText("LIVES " + std::to_string(mLives), 22, {24.f, 44.f}, CMPUT350::Colors::white);
    if (mState == State::Startup) {
        gui->DrawCenteredText("LEVEL " + std::to_string(mLevel), 40, {cx, 460.f},
                              CMPUT350::Colors::cyan);
    }
    if (mPlayerDown && mLives > 0) {
        gui->DrawCenteredText("Lives left: " + std::to_string(mLives), 28, {cx, 520.f},
                              CMPUT350::Colors::yellow);
    }
    if (mState == State::GameOver) {
        gui->DrawCenteredText("GAME OVER", 48, {cx, 460.f}, CMPUT350::Colors::red);
    }
}