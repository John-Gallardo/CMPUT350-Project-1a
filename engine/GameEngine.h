
#ifndef GAMEENGINE_H
#define GAMEENGINE_H

namespace CMPUT350 {
class GameEngine;
}

#include "EngineView.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "GameContext.h"
#include <SFML/Graphics.hpp>
#include <memory>  // for std::shared_ptr
#include <vector>

namespace CMPUT350 {

class DrawContext;

class GameEngine : public EngineView {
public:
    GameEngine(unsigned int width, unsigned int height, const std::string& name);
    ~GameEngine();

    GameEngine(const GameEngine&) = delete;             // Prevent copy-construction
    GameEngine(GameEngine&&) = delete;                  // Prevent move-construction
    GameEngine& operator=(const GameEngine&) = delete;  // Prevent assignment
    GameEngine& operator=(GameEngine&&) = delete;       // Prevent move-assignment

    void AddGameObject(std::shared_ptr<GameObject> gameObject) override;

    void Run();

private:
    std::shared_ptr<sf::RenderWindow> mWindow;
    //	std::shared_ptr<sf::Font> mFont;
    std::vector<std::shared_ptr<GameObject>> mCreatedObjects{};  // waiting to be added
    std::vector<std::shared_ptr<GameObject>> mGameObjects{};    // already in
    // TODO: this should be initialized properly
    CMPUT350::GameContext mGameContext{};  
};

}  // namespace CMPUT350

#endif  // GAMEENGINE_H
