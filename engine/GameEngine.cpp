#include "GameEngine.h"

#include <iostream>
#include <memory>
#include <vector>

#include "CollisionObject.h"
#include "DrawContext.h"
#include <SFML/Graphics.hpp>

/// @brief
namespace CMPUT350 {
#include "FontData.h"

GameEngine::GameEngine(unsigned int width, unsigned int height, const std::string& name)
    : mWindow{std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), name)} {
    mWindow->setFramerateLimit(30);
    mFont = std::make_shared<sf::Font>();
    if (!mFont->openFromMemory(_font, _font_len)) {
        std::cerr << "WARNING: Font did not load.\n";
    }

    // NOTE: we need the font for these two so they're initialized here
    mScreenContext = DrawContext(mWindow, mFont);
    mGUIContext = DrawContext(mWindow, mFont);
    mGameContext.mEngineView = this;
    mGameContext.ScreenContext = &mScreenContext;
    mGameContext.GUIContext = &mGUIContext;
    mGameContext.mNotificationManager = &mNotificationManager;
    mGameContext.currentFrame = 0;
}

GameEngine::~GameEngine() {
    // Cleanup resources
    mWindow->close();
}

/**
 * @brief Adds a game object to a temporary list, mCreatedObjects, to be added to the next frame
 * @param gameObject The game object we want to add in the next frame.
 */
void GameEngine::AddGameObject(std::shared_ptr<GameObject> gameObject) {
    mCreatedObjects.push_back(gameObject);
}

/**
 * @method Run
 * @arguments None
 * @description Gives control to the game engine. Will not return until the game window is closed or
 * all objects have been destroyed.
 */
void GameEngine::Run() {
    while (true)  // window is open
    {
        // 0. Remove any objects that are now dead
        // Reference: https://en.cppreference.com/cpp/container/vector/erase2
        auto isDead = [](const auto& gameObject) { return !gameObject->IsAlive(); };
        std::erase_if(mGameObjects, isDead);

        // 1. Activate and initialize any objects added during the last frame
        for (const auto& gameObject : mCreatedObjects) {
            gameObject->Initialize(&mGameContext);
            mGameObjects.push_back(gameObject);
        }
        mCreatedObjects.clear();

        // 2. Process events
        // Reference: CMPUT350 Lab 2 Exercise's processInputs()
        for (auto event{mWindow->pollEvent()}; event.has_value(); event = mWindow->pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                mWindow->close();
                return;
            }

            // NOTE: Switched to key press instead of text entered because of GameEngine.h/.cpp spec
            // 1. process pressed event
            const auto *pressed{event->getIf<sf::Event::KeyPressed>()};
            if (pressed) {
                const int scancode{static_cast<int>(pressed->scancode)};
                // send notifications ONLY if key is one of the keys we want to send notifications for in keyPressed
                if (mKeyDownNotifications.contains(scancode)) {
                    const auto &notifications{mKeyDownNotifications[scancode]};
                    // loop through all associated notifications with this key press
                    for (const auto &notification : notifications) {
                        mNotificationManager.Notify(notification);
                    }
                }
            }

            // 2. process released event
            const auto *released{event->getIf<sf::Event::KeyReleased>()};
            if (released) {
                const int scancode{static_cast<int>(released->scancode)};
                // again, send notifications ONLY for same idea as above
                if (mKeyUpNotifications.contains(scancode)) {
                    const auto &notifications{mKeyUpNotifications[scancode]};
                    // loop through all associated notifications with this key release event
                    for (const auto &notification : notifications) {
                        mNotificationManager.Notify(notification);
                    }
                }
            }

        }

        // 3. Update game objects
        for (const auto& gameObject : mGameObjects) {
            gameObject->Update(&mGameContext);
        }

        // 4. Process collision events
        // TODO: Compute collisions of all types!!!
        // grab all collision objects
        std::vector<std::shared_ptr<CollisionObject>> collisionObjects{};
        for (const auto& gameObject : mGameObjects) {
            std::shared_ptr<CollisionObject> collisionObject{
                std::dynamic_pointer_cast<CollisionObject>(gameObject)};
            // Check if is a collision object
            if (collisionObject == nullptr) continue;
            collisionObjects.push_back(collisionObject);
        }

        // process collisions only for objects that collide
        for (size_t i{0}; i < collisionObjects.size(); i++) {
            // grab bounding box for current object
            auto collisionObject{collisionObjects[i]};
            const auto &collisionBoundingBox{collisionObject->GetBounds()};

            for (size_t j{i + 1}; j < collisionObjects.size(); j++) {
                // grab bounding box for other object
                auto otherObject{collisionObjects[j]};
                const auto &otherBoundingBox{otherObject->GetBounds()};

                // check if bounding boxes are NOT intersecting
                if (collisionBoundingBox.topLeft.x + collisionBoundingBox.width < otherBoundingBox.topLeft.x ||  // 1. Check if is to the left
                    collisionBoundingBox.topLeft.y > otherBoundingBox.topLeft.y + otherBoundingBox.height ||     // 2. Check if is below
                    collisionBoundingBox.topLeft.x > otherBoundingBox.topLeft.x + otherBoundingBox.width ||      // 3. Check if is to the right
                    collisionBoundingBox.topLeft.y + collisionBoundingBox.height < otherBoundingBox.topLeft.y)   // 4. Check if is above 
                {
                    continue;
                }

                // check both ways for collision events
                collisionObject->CollisionEnter(otherObject);
                otherObject->CollisionEnter(collisionObject); 
            }
        }

        // 5. Late updates
        for (const auto& gameObject : mGameObjects) {
            gameObject->LateUpdate(&mGameContext);
        }

        // Clear window
        mWindow->clear();

        // grab all graphics objects for rendering
        std::vector<std::shared_ptr<GraphicsObject>> graphicsObjects{};
        for (const auto& gameObject : mGameObjects) {
            // Check if is a graphics object
            std::shared_ptr<GraphicsObject> graphicsObject{
                std::dynamic_pointer_cast<GraphicsObject>(gameObject)};
            if (graphicsObject == nullptr) continue;
            graphicsObjects.push_back(graphicsObject);
        }

        // TODO: "Get offsets and rotations from objects before they are asked to render to the screen"

        // 6. Render background
        for (const auto& graphicsObject : graphicsObjects) {
            graphicsObject->RenderBackground(&mGameContext);
        }

        // 7. Render foreground
        for (const auto& graphicsObject : graphicsObjects) {
            graphicsObject->RenderForeground(&mGameContext);
        }

        // Actually render to window
        mWindow->display();

        // Move onto next frame by incrementing currentFrame
        mGameContext.currentFrame++;
    }
}

/**
 * @brief Add string notification associated to this particular key press for key down
 */
void GameEngine::InstallKeyDownNotification(int key, const std::string &notification) {
    mKeyDownNotifications[key].push_back(notification);
}

/**
 * @brief Add string notification associated to this particular key press for key up
 */
void GameEngine::InstallKeyUpNotification(int key, const std::string &notification) {
    mKeyUpNotifications[key].push_back(notification);
}

/**
 * @brief Returns mGameObject's cbegin iterator
 */
EngineView::const_iterator GameEngine::cbegin() const {
    return mGameObjects.cbegin();
}

/**
 * @brief Returns mGameObject's cend iterator
 */
EngineView::const_iterator GameEngine::cend() const {
    return mGameObjects.cend();
}

/**
 * @brief Returns mGameObject's begin iterator
 */
EngineView::const_iterator GameEngine::begin() const {
    return mGameObjects.begin();
}

/**
 * @brief Returns mGameObject's end iterator
 */
EngineView::const_iterator GameEngine::end() const {
    return mGameObjects.end();
}


// Sample code for processing events

// bool GameEngine::ProcessEvents(GameContext *context)
//{
//	while (const std::optional event = mWindow->pollEvent())
//	{
//		if (event->is<sf::Event::Closed>())
//		{
//		}
//		else if (event->is<sf::Event::Resized>())
//		{
//		}
//		else if (const auto* keyPressed = event->getIf<sf::Event::TextEntered>())
//		{
//			// use keyPressed->unicode to get character
//		}
//	}
// }

}  // namespace CMPUT350
