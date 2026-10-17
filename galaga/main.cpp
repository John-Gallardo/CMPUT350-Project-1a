#include <random>
#include "GameEngine.h"
#include "Player.h"
#include "Enemy.h"
#include "Stars.h"

int main()
{
    CMPUT350::GameEngine engine(768, 1024, "Galaga");
    // notifications for moving left & right
    engine.InstallKeyDownNotification(static_cast<int>(sf::Keyboard::Scan::A), "MoveLeft");
    engine.InstallKeyDownNotification(static_cast<int>(sf::Keyboard::Scan::D), "MoveRight");

    auto player = std::make_shared<Player>(CMPUT350::Point2D(768 / 2, 900));
    engine.AddGameObject(player);
    engine.AddGameObject(std::make_shared<Stars>(250, CMPUT350::Rect(0, 0, 768, 1024)));
    // 10 enemies per row, we have 4 rows
    for (int i{0}; i < 4; i++) {
        for (int j{0}; j < 10; j++) {
            // NOTE: x & y offsets were arbitrarily chosen
            auto enemy{std::make_shared<Enemy>(CMPUT350::Point2D(20 + j * 80, 100 + i * 100))};
            engine.AddGameObject(enemy);
        }
    }
    engine.Run();
    return 0;
}
