#include "GameEngine.h"
#include "CollisionObject.h"
#include "DrawContext.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <iostream>

/// @brief
namespace CMPUT350 {
#include "FontData.h"

GameEngine::GameEngine(unsigned int width, unsigned int height, const std::string& name)
: mWindow{std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), name)},
mFont{std::make_shared<sf::Font>()} {
    mWindow->setFramerateLimit(30);
    if (!mFont->openFromMemory(_font, _font_len)) {
        std::cerr << "WARNING: Font did not load.\n";
    }

    // NOTE: we need the font for these two so they're initialized here
    mScreenContext = new DrawContext(mWindow, mFont);
    mGameContext.mEngineView = this;
    mGameContext.ScreenContext = mScreenContext;
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
    // NOTE: not sure if this should have a std::move() or not
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
        auto isDead = [](const auto &gameObject) {return !gameObject->IsAlive();};
        std::erase_if(mGameObjects, isDead);

        // 1. Activate and initialize any objects added during the last frame
        for (const auto &gameObject : mCreatedObjects) {
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
        }

        // 3. Update game objects
        for (const auto &gameObject : mGameObjects) {
            gameObject->Update(&mGameContext);
        }

        // 4. Process collision events
        // grab all collision objects
        std::vector<std::shared_ptr<CollisionObject>> collisionObjects{};
        for (const auto &gameObject : mGameObjects) {
            std::shared_ptr<CollisionObject> collisionObject{std::dynamic_pointer_cast<CollisionObject>(gameObject)};
            // Check if is a collision object
            if (collisionObject == nullptr) 
                continue;
            collisionObjects.push_back(collisionObject);
        }

        // check for collisions with each other with a double loop
        for (size_t i{0}; i < collisionObjects.size(); i++) {
            auto collisionObject{collisionObjects[i]};
            for (size_t j{i+1}; j < collisionObjects.size(); j++) {
                auto otherObject{collisionObjects[j]};
                collisionObject->CollisionEnter(otherObject);
            }
        }

        // 5. Late updates
        for (const auto &gameObject : mGameObjects) {
            gameObject->LateUpdate(&mGameContext);
        }

        // Clear window
        mWindow->clear();

        // grab all graphics objects for rendering
        std::vector<std::shared_ptr<GraphicsObject>> graphicsObjects{};
        for (const auto &gameObject : mGameObjects) {
            std::shared_ptr<GraphicsObject> graphicsObject{std::dynamic_pointer_cast<GraphicsObject>(gameObject)};
            if (graphicsObject == nullptr)
                continue;
            graphicsObjects.push_back(graphicsObject);
        }

        // 6. Render background
        for (const auto &graphicsObject : graphicsObjects) {
            graphicsObject->RenderBackground(&mGameContext);
        }
    

        // 7. Render foreground
        for (const auto &graphicsObject : graphicsObjects) {
            graphicsObject->RenderForeground(&mGameContext);
        }

        // Actually render to window
        mWindow->display();
    }
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
