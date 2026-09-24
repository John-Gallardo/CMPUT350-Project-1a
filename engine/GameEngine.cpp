#include "GameEngine.h"
#include <memory>
#include <vector>

/// @brief
namespace CMPUT350 {
#include "FontData.h"

GameEngine::GameEngine(unsigned int width, unsigned int height, const std::string& name)
: mWindow{std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), name)} {
    // Sample font loading code
    //	if (!mFont->openFromMemory(&_font, _font_len))
    //	{
    //		fprintf(stderr, "WARNING: Font did not load.\n");
    //	}
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
        auto isDead = [](const auto &gameObject) {return gameObject->IsAlive();};
        std::erase_if(mGameObjects, isDead);

        // 1. Activate and initialize any objects added during the last frame

        // 2. Process events

        // 3. Update game objects

        // 4. Process collision events

        // 5. Late updates

        // Clear window

        // 6. Render background

        // 7. Render foreground

        // Actually render to window
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
